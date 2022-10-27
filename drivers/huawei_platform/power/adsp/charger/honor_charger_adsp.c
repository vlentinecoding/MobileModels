/*
 * honor_charger_adsp.c
 *
 * honor charger adsp driver
 *
 * Copyright (c) 2021-2021 Honor Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/usb/otg.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/spmi.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <log/hw_log.h>

#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_interface.h>
#include <huawei_platform/power/adsp/honor_charger_adsp.h>
#include <chipset_common/hwpower/power_supply_interface.h>
#include <chipset_common/hwpower/power_log.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_cmdline.h>
#include <huawei_platform/power/adsp/adsp_dts_interface.h>
#include <huawei_platform/power/hihonor_charger_glink.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_fg_glink.h>
#include <chipset_common/hwpower/power_dsm.h>

#define HWLOG_TAG honor_charger_adsp
HWLOG_REGIST();

static struct charge_device_info *g_chg_di = NULL;

static int battery_get_bat_temperature(int *temp)
{
	int ret;

	if (!temp)
		return -EINVAL;

	ret = power_supply_get_int_property_value("battery", POWER_SUPPLY_PROP_TEMP, temp);
	if (ret)
		return ret;

	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
static bool is_batt_temp_normal(int temp)
{
	if (((temp > BATT_EXIST_TEMP_LOW) && (temp <= NO_CHG_TEMP_LOW)) ||
		(temp >= NO_CHG_TEMP_HIGH))
		return false;
	return true;
}
#endif

static int dcp_charger_set_rt_current(struct charge_device_info *di, int val)
{
	int iin_rt_curr;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	if ((val == 0) || (val == 1)) {
		iin_rt_curr = MAX_CURRENT;
	}
	else if ((val <= MIN_CURRENT) && (val > 1)) {
#ifndef CONFIG_HLTHERM_RUNTEST
		iin_rt_curr = MIN_CURRENT;
#else
		iin_rt_curr = HLTHERM_CURRENT;
#endif
	} else {
		iin_rt_curr = val;
	}

	hihonor_charger_glink_set_input_current(iin_rt_curr);

	di->sysfs_data.iin_rt_curr = iin_rt_curr;
	return 0;
}

static int sdp_set_iin_limit(unsigned int val)
{
	struct charge_device_info *di = g_chg_di;
	int ret;

	if(!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	/* sdp current limit > 450mA */
	if (power_cmdline_is_factory_mode() && (val >= 450)) {
		ret = hihonor_charger_glink_set_sdp_input_current(val);
		if (ret)
			return ret;

		hwlog_info("set sdp ibus current is: %u\n", val);
	}
	return 0;
}

static int dcp_charger_enable_charge(struct charge_device_info *di, int val)
{
	int ret;
	int online;

	ret = hihonor_charger_glink_enable_charge(val);
	if (ret)
		return -1;

	
	di->sysfs_data.charge_enable = val;

	ret = power_supply_get_int_property_value("usb", POWER_SUPPLY_PROP_ONLINE, &online);
	if (ret || !online)
		return -1;

	if (val == 1)
		power_event_notify(POWER_NT_CHARGING, POWER_NE_START_CHARGING, NULL);
	else
		power_event_notify(POWER_NT_CHARGING, POWER_NE_STOP_CHARGING, NULL);

	return 0;
}

static int dcp_set_enable_charger(unsigned int val)
{
	struct charge_device_info *di = g_chg_di;
	int ret;
	int batt_temp;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	if ((val < 0) || (val > 1))
		return -EINVAL;

	hwlog_info("%s, set charge enable = %d\n", __func__, val);
	ret = battery_get_bat_temperature(&batt_temp);
	if (ret)
		return ret;

#ifndef CONFIG_HLTHERM_RUNTEST
	if ((val == 1) && !is_batt_temp_normal(batt_temp)) {
		hwlog_err("dcp: battery temp is %d, abandon enable_charge\n",
			batt_temp);
		return -1;
	}
#endif

	ret = dcp_charger_enable_charge(di, val);
	return ret;
}

static int dcp_get_enable_charger(unsigned int *val)
{
	struct charge_device_info *di = g_chg_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->sysfs_data.charge_enable;
	return 0;
}

static int dcp_set_iin_limit(unsigned int val)
{
	struct charge_device_info *di = g_chg_di;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	ret = dcp_charger_set_rt_current(di, val);

	hwlog_info("set input current is:%d\n", val);

	return ret;
}

static int dcp_get_iin_limit(unsigned int *val)
{
	struct charge_device_info *di = g_chg_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -EINVAL;
	}

	*val = di->sysfs_data.iin_rt_curr;

	return 0;
}

static int battery_get_voltage_max_design(int *max)
{
	int ret;

	if (!max)
		return -EINVAL;

	ret = power_supply_get_int_property_value("battery", 
		POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN, max);
	if (ret)
		return ret;

	hwlog_info("%s max=%d\n", __func__, *max);
	return 0;
}

static void battery_set_voltage_max(int max)
{
	power_supply_set_int_property_value("battery", POWER_SUPPLY_PROP_VOLTAGE_MAX, max);
}

static int dcp_set_vterm_dec(unsigned int val)
{
	int vterm_max;
	int vterm_basp;
	int ret;
	struct charge_device_info *di = g_chg_di;

	if (!di) {
		hwlog_err("%s di or core_data is null\n", __func__);
		return -EINVAL;
	}

	if (!val)
		return 0;

	ret = battery_get_voltage_max_design(&vterm_max);
	if (ret) {
		hwlog_err("get voltage_max_design fail\n");
		vterm_max = DEFAULT_VTERM;
	}
#if 0
	hihonor_glink_set_vterm_dec(val);
#endif
	val *= 1000;
	vterm_basp = vterm_max - val;
	vterm_basp /= 1000;
	battery_set_voltage_max(vterm_basp);
	hwlog_info("%s set charger terminal voltage is:%d\n",
		__func__, vterm_basp);
	return 0;
}

static int dcp_get_hota_iin_limit(unsigned int *val)
{
	struct charge_device_info *di = g_chg_di;
	struct charger_config *conf = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	conf = &di->chg_config;
	*val = conf->hota_iin_limit;
	return 0;
}

static int dcp_get_startup_iin_limit(unsigned int *val)
{
	struct charge_device_info *di = g_chg_di;
	struct charger_config *conf = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	conf = &di->chg_config;
	*val = di->startup_iin_limit;
	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
static int dcp_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	struct charge_device_info *di = g_chg_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	/*
	 * set default iin when value is 0 or 1
	 * set 100mA for iin when value is between 2 and 100
	 */
	if ((val == 0) || (val == 1))
		di->sysfs_data.iin_thl_array[idx] = DEFAULT_IIN_THL;
	else if ((val > 1) && (val <= MIN_CURRENT))
		di->sysfs_data.iin_thl_array[idx] = MIN_CURRENT;
	else
		di->sysfs_data.iin_thl_array[idx] = val;

	hwlog_info("thermal send input current = %d, type: %u\n",
		di->sysfs_data.iin_thl_array[idx], idx);

	return 0;
}
#else
static int dcp_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	return 0;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int dcp_set_iin_thermal(unsigned int index, unsigned int value)
{
	struct charge_device_info *di = g_chg_di;
	
	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (index >= IIN_THERMAL_CHARGE_TYPE_END) {
		pr_err("error index: %u, out of boundary\n", index);
		return -EINVAL;
	}
	if (dcp_set_iin_limit_array(index, value))
		return -1;

	hihonor_charger_glink_set_input_current(di->sysfs_data.iin_thl_array[IIN_THERMAL_WCURRENT_5V]);
	return 0;
}

static int dcp_set_iin_thermal_all(unsigned int value)
{
	int i;
	struct charge_device_info *di = g_chg_di;

	if (!di)
		return -1;

	for (i = IIN_THERMAL_CHARGE_TYPE_BEGIN; i < IIN_THERMAL_CHARGE_TYPE_END; i++) {
		if (dcp_set_iin_limit_array(i, value))
			return -1;
	}

	hihonor_charger_glink_set_input_current(di->sysfs_data.iin_thl_array[IIN_THERMAL_WCURRENT_5V]);
	return 0;
}

static void charger_glink_sync_work(void *dev_data)
{
	struct charge_device_info *di = (struct charge_device_info *)dev_data;

	if (!di)
		return;

#if 0
	(void)hihonor_oem_glink_oem_set_prop(USB_OEM_OTG_TYPE, &(di->otg_type),
		sizeof(di->otg_type));
#endif
}

static struct hihonor_glink_ops charge_glink_ops = {
	.sync_data = charger_glink_sync_work,
};

static struct power_if_ops sdp_if_ops = {
	.set_iin_limit = sdp_set_iin_limit,
	.type_name = "sdp",
};

static struct power_if_ops dcp_if_ops = {
	.set_enable_charger = dcp_set_enable_charger,
	.get_enable_charger = dcp_get_enable_charger,
	.set_iin_limit = dcp_set_iin_limit,
	.get_iin_limit = dcp_get_iin_limit,
	.set_vterm_dec = dcp_set_vterm_dec,
	.get_hota_iin_limit = dcp_get_hota_iin_limit,
	.get_startup_iin_limit = dcp_get_startup_iin_limit,
	.set_iin_thermal = dcp_set_iin_thermal,
	.set_iin_thermal_all = dcp_set_iin_thermal_all,
	.type_name = "dcp",
};

static int honor_charger_sysfs_set_hz_mode(const char *buf)
{
	int val;
	struct charge_device_info *di = g_chg_di;

	if ((kstrtoint(buf, 10, &val) < 0) || (val < 0) || (val > 1))
		return -EINVAL;

	hwlog_info("set hz mode val = %d\n", val);
	if (hihonor_charger_glink_enable_hiz(val))
		return -1;

	di->sysfs_data.hiz_mode = val;
	return 0 ;
}

static int honor_charger_sysfs_set_rt_current(const char *buf)
{
	int iin_rt_curr;
	int val;
	struct charge_device_info *di = g_chg_di;

	if ((kstrtoint(buf, 10, &val) < 0)||(val < 0)||(val > MAX_CURRENT)) {
		return -EINVAL;
	}

	if (val <= 0)
		return -EINVAL;
	else if (val <= MIN_CURRENT)
#ifndef CONFIG_HLTHERM_RUNTEST
        iin_rt_curr = MIN_CURRENT;
#else
        iin_rt_curr = HLTHERM_CURRENT;
#endif
    else
        iin_rt_curr = val;

    hihonor_charger_glink_set_input_current(iin_rt_curr);
    di->sysfs_data.iin_rt_curr= iin_rt_curr;
    pr_info("[%s]:set iin_rt_curr %d\n", __func__, iin_rt_curr);
    return 0 ;
}

static int honor_charger_sysfs_get_rt_current(char *buf)
{
	int len;
	struct charge_device_info *di = g_chg_di;

	len = snprintf(buf, MAX_SIZE, "%u\n", di->sysfs_data.iin_rt_curr);
    return len;
}

static int honor_charger_sysfs_get_ibus(char *buf)
{
	int len;
	batt_mngr_get_buck_info buck_info = {0};

	if (hihonor_oem_glink_oem_get_prop(CHARGER_OEM_BUCK_INFO, &buck_info, sizeof(buck_info)))
		return -1;

	len = snprintf(buf, MAX_SIZE, "%u\n", buck_info.buck_ibus);
    return len;
}

static int honor_charger_sysfs_get_vbus(char *buf)
{
	int len;
	batt_mngr_get_buck_info buck_info = {0};

	if (hihonor_oem_glink_oem_get_prop(CHARGER_OEM_BUCK_INFO, &buck_info, sizeof(buck_info)))
		return -1;

	len = snprintf(buf, MAX_SIZE, "%u\n", buck_info.buck_vbus);
    return len;
}

static void convert_usb_type(int usb_type, int *charge_type)
{
	if (!charge_type)
		return;

	switch (usb_type) {
	case POWER_SUPPLY_USB_TYPE_UNKNOWN:
		*charge_type = CHARGER_TYPE_NON_STANDARD;
		break;
	case POWER_SUPPLY_USB_TYPE_SDP:
		*charge_type = CHARGER_TYPE_USB;
		break;
	case POWER_SUPPLY_USB_TYPE_DCP:
		*charge_type = CHARGER_TYPE_STANDARD;
		break;
	default:
		*charge_type = CHARGER_TYPE_NON_STANDARD;
		break;
	}
}

static int honor_charger_sysfs_get_charge_type(char *buf)
{
	int len;
	int usb_type;
	int usb_online;
	int chg_type;

	if (power_supply_get_int_property_value("usb", POWER_SUPPLY_PROP_ONLINE, &usb_online))
		return -1;

	if(!usb_online) {
		chg_type = CHARGER_REMOVED;
	} else {
		if (power_supply_get_int_property_value("usb", POWER_SUPPLY_PROP_USB_TYPE, &usb_type))
			return -1;
		convert_usb_type(usb_type, &chg_type);
	}
	len = snprintf(buf, MAX_SIZE, "%d\n", chg_type);
    return len;
}

static int honor_charger_sysfs_get_fcp_support(char *buf)
{
	return snprintf(buf, MAX_SIZE, "%d\n", 1);
}

static int honor_charger_sysfs_get_update_volt_now(char *buf)
{
	return snprintf(buf, MAX_SIZE, "%d\n", 1);
}

static int honor_charger_sysfs_set_update_volt_now(const char *buf)
{
	int val;
	if ((kstrtoint(buf, 10, &val) < 0) || (1 != val))
		return -EINVAL;
	return 0;
}

static struct charger_sysfs_ops charger_sysfs_ops_tbl[CHARGE_SYSFS_END] = {
	[CHARGE_SYSFS_IIN_RT_CURRENT] = {
		.get_property = honor_charger_sysfs_get_rt_current,
		.set_property = honor_charger_sysfs_set_rt_current,
	},
	[CHARGE_SYSFS_HIZ] = {
		.get_property = NULL,
		.set_property = honor_charger_sysfs_set_hz_mode,
	},
	[CHARGE_SYSFS_IBUS] = {
		.get_property = honor_charger_sysfs_get_ibus,
		.set_property = NULL,
	},
	[CHARGE_SYSFS_VBUS] = {
		.get_property = honor_charger_sysfs_get_vbus,
		.set_property = NULL,
	},
	[CHARGE_SYSFS_CHARGE_TYPE] = {
		.get_property = honor_charger_sysfs_get_charge_type,
		.set_property = NULL,
	},
	[CHARGE_SYSFS_FCP_SUPPORT] = {
		.get_property = honor_charger_sysfs_get_fcp_support,
		.set_property = NULL,
	},
	[CHARGE_SYSFS_UPDATE_VOLT_NOW] = {
		.get_property = honor_charger_sysfs_get_update_volt_now,
		.set_property = honor_charger_sysfs_set_update_volt_now,
	},
};

static ssize_t charge_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t charge_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info charge_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(charge, 0664, CHARGE_SYSFS_IIN_RT_CURRENT, iin_rt_current),
	power_sysfs_attr_rw(charge, 0664, CHARGE_SYSFS_HIZ, enable_hiz),
	power_sysfs_attr_rw(charge, 0444, CHARGE_SYSFS_IBUS, ibus),
	power_sysfs_attr_rw(charge, 0444, CHARGE_SYSFS_VBUS, vbus),
	power_sysfs_attr_rw(charge, 0444, CHARGE_SYSFS_CHARGE_TYPE, chargerType),
	power_sysfs_attr_rw(charge, 0444, CHARGE_SYSFS_FCP_SUPPORT, fcp_support),
	power_sysfs_attr_rw(charge, 0660, CHARGE_SYSFS_UPDATE_VOLT_NOW, update_volt_now),
};

#define CHARGE_SYSFS_ATTRS_SIZE  ARRAY_SIZE(charge_sysfs_field_tbl)

static struct attribute *charge_sysfs_attrs[CHARGE_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group charge_sysfs_attr_group = {
	 .attrs = charge_sysfs_attrs,
};

/* show the value for all charge device's node */
static ssize_t charge_sysfs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct charger_sysfs_ops *ops = NULL;
	struct charge_device_info *di = g_chg_di;
	int len;

	if (!di || !buf)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_sysfs_field_tbl, CHARGE_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	ops = &charger_sysfs_ops_tbl[info->name];
	if (!ops->get_property)
		return 0;

	len = ops->get_property(buf);
	return len;
}

/* set the value for charge_data's node which is can be written */
static ssize_t charge_sysfs_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct charger_sysfs_ops *ops = NULL;
	struct charge_device_info *di = g_chg_di;
	int ret;

	if (!di || !buf)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		charge_sysfs_field_tbl, CHARGE_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	ops = &charger_sysfs_ops_tbl[info->name];
	if (!ops->set_property)
		return -EINVAL;

	ret = ops->set_property(buf);
	if (ret)
		return -EINVAL;
	return count;

}

static void charge_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(charge_sysfs_attrs,
		charge_sysfs_field_tbl, CHARGE_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "charge_data", dev, &charge_sysfs_attr_group);
}

static void charge_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "charge_data", dev, &charge_sysfs_attr_group);
}

static void charge_device_info_free(struct charge_device_info *di)
{
	if(di != NULL) {

		if(di->sysfs_data.reg_head != NULL) {
			kfree(di->sysfs_data.reg_head);
			di->sysfs_data.reg_head = NULL;
		}
		if(di->sysfs_data.reg_value != NULL) {
			kfree(di->sysfs_data.reg_value);
			di->sysfs_data.reg_value = NULL;
		}
		kfree(di);
	}
}

static void parameter_init(struct charge_device_info *di)
{
	if (!di)
		return;

	di->sysfs_data.iin_rt_curr = DEFAULT_IIN_CURRENT;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WCURRENT_5V] = DEFAULT_IIN_THL;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WCURRENT_9V] = DEFAULT_IIN_THL;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WLCURRENT_5V] = DEFAULT_IIN_THL;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WLCURRENT_9V] = DEFAULT_IIN_THL;
	di->sysfs_data.hiz_mode = 0;
	di->sysfs_data.charge_enable = 1;
}

static int parse_charger_temp_para(struct device_node *np, struct charger_temp_para *temp_para)
{
	int row, col, len;
	int idata[CHARGE_TEMP_PARA_MAX_NUM * CHARGE_TEMP_PARA_MAX] = { 0 };

	hwlog_info("parse_charger_temp_para\n");

	if (!np || !temp_para)
		return -EINVAL;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"charge_temp_para", idata, CHARGE_TEMP_PARA_MAX_NUM, CHARGE_TEMP_PARA_MAX);
	if (len < 0)
		return -EINVAL;

	for (row = 0; row < len / CHARGE_TEMP_PARA_MAX; row++) {
		col = row * CHARGE_TEMP_PARA_MAX + CHARGE_TEMP_PARA_TEMP_MIN;
		temp_para[row].temp_min = idata[col];
		col = row * CHARGE_TEMP_PARA_MAX + CHARGE_TEMP_PARA_TEMP_MAX;
		temp_para[row].temp_max = idata[col];
		col = row * CHARGE_TEMP_PARA_MAX + CHARGE_TEMP_PARA_MAX_CUR;
		temp_para[row].max_current = idata[col];
		col = row * CHARGE_TEMP_PARA_MAX + CHARGE_TEMP_PARA_VTERM;
		temp_para[row].vterm = idata[col];
		col = row * CHARGE_TEMP_PARA_MAX + CHARGE_TEMP_PARA_ITERM;
		temp_para[row].iterm = idata[col];
	}

	return 0;
}

static int parse_charge_dts(struct device_node *np, struct charger_config *conf)
{
	int ret = 0;

	if (!np || !conf) {
		hwlog_err("%s: np or conf is null\n");
		return -EINVAL;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"startup_iin_limit", &(conf->startup_iin_limit), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hota_iin_limit", &(conf->hota_iin_limit), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"en_eoc_max_delay", &(conf->en_eoc_max_delay), 0);

	ret += parse_charger_temp_para(np, conf->chg_temp_para);

	return ret;
}

static void charger_dts_sync_callback(void *dev_data)
{
	struct charge_device_info *di = (struct charge_device_info *)dev_data;

	if (!di)
		return;

	schedule_delayed_work(&di->sync_work, 0);
}

static void charger_dts_glink_sync_work(struct work_struct *work)
{
	int ret;
	struct charge_device_info *di = container_of(work, struct charge_device_info, sync_work.work);

	if (!di)
		return;

	hwlog_info("sync buck charger dts to adsp\n");
	ret = hihonor_oem_glink_oem_update_config(BUCK_CHARGER_ID, &di->chg_config, sizeof(di->chg_config));
	if (ret) {
		hwlog_err("sync buck charger dts to adsp fail\n");
		goto next;
	}
	return;

next:
	schedule_delayed_work(&di->sync_work, msecs_to_jiffies(CHARGE_DTS_SYNC_INTERVAL));
}

static void charger_notify_callback(void *dev_data, unsigned int notification, void *data)
{
	struct charge_dsm_info *dsm_info = NULL;

	if (notification != OEM_NOTIFY_DSM_INFO || !data)
		return;

	dsm_info = (struct charge_dsm_info *)data;
	power_dsm_dmd_report(dsm_info->dsm_type, dsm_info->dsm_no, dsm_info->dsm_buff);
}

static struct hihonor_glink_ops charger_dts_glink_ops = {
	.sync_data = charger_dts_sync_callback,
	.notify_event = charger_notify_callback,
};

static int charge_probe(struct platform_device *pdev)
{
	struct charge_device_info *di;
    struct device_node *np = NULL;
	int ret;

	if (!pdev) {
		hwlog_err("%s: invalid param, fatal error\n", __func__);
		return -EINVAL;
	}

    di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	g_chg_di = di;
	dev_set_drvdata(&(pdev->dev), di);
	parameter_init(di);
	parse_charge_dts(np, &di->chg_config);

	charger_dts_glink_ops.dev_data = di;
	ret = hihonor_oem_glink_ops_register(&charger_dts_glink_ops);
	if (ret) {
		hwlog_err("%s fail to register glink ops\n", __func__);
		goto fail_free_mem;
	}

	INIT_DELAYED_WORK(&di->sync_work, charger_dts_glink_sync_work);

	power_if_ops_register(&dcp_if_ops);
	power_if_ops_register(&sdp_if_ops);
	charge_glink_ops.dev_data = di;
	hihonor_oem_glink_ops_register(&charge_glink_ops);
	charger_glink_sync_work(di);

	charge_sysfs_create_group(di->dev);

	hwlog_info("honor charger probe ok!\n");
	return 0;

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;

	return ret;
}

static int charge_remove(struct platform_device *pdev)
{
	struct charge_device_info *di = NULL;

    if (!pdev) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return -EINVAL;
    }

    di = dev_get_drvdata(&pdev->dev);
    if (!di) {
        pr_err("%s: Cannot get charge device info, fatal error\n", __func__);
        return -EINVAL;
    }
	charge_sysfs_remove_group(di->dev);
	charge_device_info_free(di);
	g_chg_di = NULL;
	di = NULL;
	return 0;
}

static void charge_shutdown(struct platform_device  *pdev)
{
	if (!pdev)
		pr_err("%s: invalid param\n", __func__);

	hihonor_oem_glink_notify_state(OEM_NOTIFY_AP_SHUTDOWN, NULL, 0);
}

#ifdef CONFIG_PM
static int charge_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int charge_resume(struct platform_device *pdev)
{
	return 0;
}
#endif

static const struct of_device_id charge_match_table[] = {
	{
		.compatible = "honor,charger_adsp",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver charge_driver =
{
	.probe = charge_probe,
	.remove = charge_remove,
#ifdef CONFIG_PM
	.suspend = charge_suspend,
	.resume = charge_resume,
#endif
	.shutdown = charge_shutdown,
	.driver =
	{
		.name = "honor,charger_adsp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(charge_match_table),
	},
};

static int __init charge_init(void)
{
	int ret;

	ret = platform_driver_register(&charge_driver);
	if (ret) {
		pr_info("charge_init register platform_driver_register failed!\n");
		return ret;
	}

	return 0;
}

static void __exit charge_exit(void)
{
	platform_driver_unregister(&charge_driver);
}

late_initcall(charge_init);
module_exit(charge_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("honor charger adsp module driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
