/*
 * dc_sc_adsp.c
 *
 * power supply for direct charge sc
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
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <chipset_common/hwpower/power_debug.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_interface.h>
#include <chipset_common/hwpower/power_cmdline.h>
#include <chipset_common/hwpower/power_test.h>
#include <huawei_platform/power/adsp/direct_charger_adsp.h>
#include <huawei_platform/power/adsp/dc_glink_interface.h>
#include <huawei_platform/power/adsp/adsp_dc_dts_parse.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>

#define HWLOG_TAG dc_sc_adsp
HWLOG_REGIST();

static struct dc_device_adsp *g_sc_adsp_di;

static int sc_adsp_check_iin_limit(struct dc_device_adsp *di, int orig_iin_limit)
{
	struct dc_sc_config *conf = &di->dc_sc_conf;
	int iin_limit;
	int index;
	int cur_low;

	if (orig_iin_limit > di->iin_thermal_default) {
		hwlog_err("val is too large: %u, tuned as default\n", orig_iin_limit);
		iin_limit = di->iin_thermal_default;
	}

	index = conf->stage_group_size - 1;
	cur_low = conf->orig_volt_para[0].volt_info[index].cur_th_low;
	if (orig_iin_limit == 0)
		iin_limit = di->iin_thermal_default;
	else if (orig_iin_limit < cur_low)
		iin_limit = cur_low;
	else
		iin_limit = orig_iin_limit;
	return iin_limit;
}

static int sc_adsp_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	int iin_limit;

	if (!di)
		return -1;

	iin_limit = sc_adsp_check_iin_limit(di, val);
	di->sysfs_state.sysfs_iin_thermal_array[idx] = iin_limit;

	hwlog_info("set input current: %u, limit current: %d, channel type: %u\n",
		val, iin_limit, idx);
	return 0;
}

static int sc_adsp_set_iin_thermal(unsigned int value)
{
	int i;
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di)
		return -1;

	for (i = DC_CHANNEL_TYPE_BEGIN; i < DC_CHANNEL_TYPE_END; i++) {
		if (sc_adsp_set_iin_limit_array(i, value))
			return -1;
	}
	ret = dc_glink_set_iin_thermal(di->sysfs_state.sysfs_iin_thermal_array, DC_CHANNEL_TYPE_END);
	if (ret)
		return -1;

	return 0;

}

static int sc_adsp_sysfs_set_iin_thermal(const char *buf)
{
	int val;

	if (kstrtouint(buf, 10, &val) < 0)
		return -EINVAL;

	return sc_adsp_set_iin_thermal(val);
}

static int sc_adsp_get_iin_thermal(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	*val = di->sysfs_state.sysfs_iin_thermal_array[DC_DUAL_CHANNEL];
	return 0;
}

static int sc_adsp_sysfs_get_iin_thermal(char *buf)
{
	int len;
	unsigned int val;
	int ret;

	ret = sc_adsp_get_iin_thermal(&val);
	if (ret)
		return 0;

	len = snprintf(buf, PAGE_SIZE, "%u\n", val);
	return len;
}

static int sc_adsp_sysfs_set_iin_limit_ichg_control(const char *buf)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	int val;
	int iin_limit;

	if (kstrtoint(buf, 10, &val) < 0)
		return -EINVAL;

	iin_limit = sc_adsp_check_iin_limit(di, val);
	// TODO update adsp
	di->sysfs_iin_thermal_ichg_control = iin_limit;
	hwlog_info("ichg_control set input current: %u, limit current: %d\n",
		val, di->sysfs_iin_thermal_ichg_control);
	return 0;
}

static int sc_adsp_sysfs_get_iin_limit_ichg_control(char *buf)
{
	int len;
	struct dc_device_adsp *di = g_sc_adsp_di;

	len = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_iin_thermal_ichg_control);
	return len;
}

static int sc_adsp_sysfs_set_ichg_control_enable(const char *buf)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	int val;

	if (kstrtoint(buf, 10, &val) < 0)
		return -EINVAL;

	di->ichg_control_enable = val;
	// TODO update adsp
	hwlog_info("ichg_control_enable set %d\n", di->ichg_control_enable);
	return 0;
}

static int sc_adsp_sysfs_get_ichg_control_enable(char *buf)
{
	int len;
	struct dc_device_adsp *di = g_sc_adsp_di;

	len = snprintf(buf, PAGE_SIZE, "%d\n", di->ichg_control_enable);
	return len;
}

static int sc_adsp_sysfs_set_thermal_reason(const char *buf)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	snprintf(di->thermal_reason, strlen(buf), "%s", buf);
	sysfs_notify(&di->dev->kobj, NULL, "thermal_reason");
	hwlog_info("THERMAL set reason = %s, buf = %s\n", di->thermal_reason, buf);
	return 0;
}

static int sc_adsp_sysfs_get_thermal_reason(char *buf)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	int len;

	len = snprintf(buf, PAGE_SIZE, "%s\n", di->thermal_reason);
	return len;
}

static int sc_adsp_sysfs_get_adapter_detect(char *buf)
{
	int len;

	// TODO
	len = snprintf(buf, PAGE_SIZE, "%s\n", "test");
	hwlog_info("%s, val = %s\n", __func__, buf);
	return len;
}

static int sc_adsp_sysfs_get_iadapt(char *buf)
{
	int len;

	// TODO get from adsp
	len = snprintf(buf, PAGE_SIZE, "%s\n", "test");
	hwlog_info("%s, val = %s\n", __func__, buf);
	return len;
}

static int sc_adsp_sysfs_set_resistance_threshold(const char *buf)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct dc_sc_config *conf = &di->dc_sc_conf;
	int val;

	if ((kstrtoint(buf, 10, &val) < 0) ||
		(val < 0) || (val > DC_MAX_RESISTANCE))
		return -EINVAL;
	
	hwlog_info("set resistance_threshold=%ld\n", val);

	// TODO update adsp
	conf->std_cable_full_path_res_max = val;
	conf->nonstd_cable_full_path_res_max = val;
	//conf->ctc_cable_full_path_res_max = val;
	//conf->ignore_full_path_res = true;
	return 0;
}

static int sc_adsp_sysfs_get_full_path_resistance(char *buf)
{
	int len;
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct dc_sc_config *conf = &di->dc_sc_conf;

	len = snprintf(buf, PAGE_SIZE, "%d\n", conf->std_cable_full_path_res_max);
	return len;
}

static int sc_adsp_sysfs_set_chargetype_priority(const char *buf)
{
	int val;

	if ((kstrtoint(buf, 10, &val) < 0) ||
		(val < 0) || (val > DC_MAX_RESISTANCE))
		return -EINVAL;

	hwlog_info("set chargertype_priority=%ld\n", val);
	// TODO set mode
	return 0;
}

static int sc_adsp_sysfs_get_direct_charge_succ(char *buf)
{
	int len;
	int dc_succ_flag;

	// TODO get from adsp
	dc_succ_flag = 1;
	len = snprintf(buf, PAGE_SIZE, "%d\n", dc_succ_flag);
	return len;
}

static int sc_adsp_sysfs_set_antifake(const char *buf)
{
	hwlog_info("%s, val = %s\n", __func__, buf);
	// TODO
	return 0;
}

static int sc_adsp_sysfs_get_antifake(char *buf)
{
	int len;

	// TODO
	len = snprintf(buf, PAGE_SIZE, "%s\n", "test");
	hwlog_info("%s, val = %s\n", __func__, buf);
	return len;
}

static int sc_adsp_sysfs_get_multi_sc_cur(char *buf)
{
	int ret;
	int i = 0;
	char temp_buf[DC_SC_CUR_LEN] = {0};
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct dc_sc_config *conf = &di->dc_sc_conf;
	struct dc_charge_info *info = &di->charge_info;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (conf->multi_ic_mode_para.support_multi_ic == 0) {
		hwlog_err("not support multi sc\n");
		return 0;
	}

	ret = dc_glink_get_multi_cur(info);
	if (ret)
		return 0;

	ret = snprintf(buf, PAGE_SIZE, "%d,%d", info->channel_num, info->ibat_max);
	while ((ret > 0) && (i < info->channel_num)) {
		memset(temp_buf, 0, sizeof(temp_buf));
		ret += snprintf(temp_buf, DC_SC_CUR_LEN,
			"\r\n^MULTICHARGER:%s,%d,%d,%d,%d",
			(i == CHARGE_IC_TYPE_MAIN ? "main" : "aux"), 
			info->ibus[i], info->vout[i], info->vbat[i], info->tbat[i]);
		strncat(buf, temp_buf, strlen(temp_buf));
		i++;
	}

	return ret;
}

static int sc_adsp_sysfs_get_sc_state(char *buf)
{
	int len;
	int ret;
	int curr_mode;

	// TODO get from adsp
	ret = dc_glink_get_cur_mode(&curr_mode);
	if (ret)
		return -1;

	len = snprintf(buf, PAGE_SIZE, "%d\n", curr_mode);
	return len;
}

static ssize_t adsp_dc_sc_send_conf_store(void *dev_data, const char *buf, size_t size)
{
	struct dc_device_adsp *di = (struct dc_device_adsp *)dev_data;

	if (!di || !buf)
		return -1;

	schedule_delayed_work(&di->sync_work, 0);
	return size;
}

static int sc_adsp_set_enable_charger(unsigned int val)
{
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di)
		return -1;

	if (val < 0 || val > 1) {
		hwlog_err("val must be 0 or 1\n");
		return -EINVAL;
	}

	hwlog_info("%s, val = %u\n", __func__, val);
	ret = dc_glink_set_enable_charge(val);
	if (ret)
		return -1;

	di->sysfs_state.sysfs_enable_charger = val;
	return 0;
}

static int sc_adsp_get_enable_charger(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di || !val)
		return -1;

	*val = di->sysfs_state.sysfs_enable_charger;
	return 0;
}

static int sc_adsp_mainsc_set_enable_charger(unsigned int val)
{
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di)
		return -1;

	if (val < 0 || val > 1) {
		hwlog_err("val must be 0 or 1\n");
		return -EINVAL;
	}

	hwlog_info("%s, val = %u\n", __func__, val);
	ret = dc_glink_mainsc_set_enable_charge(val);
	if (ret)
		return -1;

	di->sysfs_state.sysfs_mainsc_enable_charger = val;
	return 0;
}

static int sc_adsp_mainsc_get_enable_charger(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di || !val)
		return -1;

	*val = di->sysfs_state.sysfs_mainsc_enable_charger;
	return 0;
}

static int sc_adsp_auxsc_set_enable_charger(unsigned int val)
{
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di)
		return -1;

	if (val < 0 || val > 1) {
		hwlog_err("val must be 0 or 1\n");
		return -EINVAL;
	}

	hwlog_info("%s, val = %u\n", __func__, val);
	ret = dc_glink_auxsc_set_enable_charge(val);
	if (ret)
		return -1;

	di->sysfs_state.sysfs_auxsc_enable_charger = val;
	return 0;
}

static int sc_adsp_auxsc_get_enable_charger(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di || !val)
		return -1;

	*val = di->sysfs_state.sysfs_auxsc_enable_charger;
	return 0;
}

static int sc_adsp_set_iin_thermal_index(unsigned int index, unsigned int iin_thermal_value)
{
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di)
		return -1;

	if (index >= DC_CHANNEL_TYPE_END) {
		hwlog_err("error index: %u, out of boundary\n", index);
		return -1;
	}
	if (sc_adsp_set_iin_limit_array(index, iin_thermal_value))
		return -1;

	ret = dc_glink_set_iin_thermal(di->sysfs_state.sysfs_iin_thermal_array, DC_CHANNEL_TYPE_END);
	if (ret)
		return -1;
	return 0;
}

static int sc_adsp_get_ibus(int *ibus)
{
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct dc_charge_info *info = NULL;

	if (!ibus || !di)
		return -1;

	info = &di->charge_info;
	ret = dc_glink_get_multi_cur(info);
	if (ret)
		return -1;

	*ibus = info->ibus[CHARGE_IC_TYPE_MAIN] + info->ibus[CHARGE_IC_TYPE_AUX];
	return 0;
}

static int sc_adsp_get_vbus(int *vbus)
{
	int ret;

	if (!vbus)
		return -1;

	ret = dc_glink_get_vbus(vbus);
	if (ret)
		return -1;

	return 0;
}

static int sc_adsp_set_ichg_ratio(unsigned int val)
{
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = dc_glink_set_ichg_ratio(val);
	if (ret)
		return -1;

	di->sysfs_state.ichg_ratio = val;
	hwlog_info("set ichg_ratio=%d\n", val);
	return 0;
}

static int sc_adsp_get_ichg_ratio(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_state.ichg_ratio;
	return 0;
}

static int sc_adsp_set_vterm_dec(unsigned int val)
{
	int ret;
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = dc_glink_set_vterm_dec(val);
	if (ret)
		return -1;

	di->sysfs_state.vterm_dec = val;
	hwlog_info("set vterm_dec=%d\n", val);
	return 0;
}

static int sc_adsp_get_vterm_dec(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_state.vterm_dec;
	return 0;
}

static int sc_adsp_get_rt_test_time_by_mode(unsigned int *val, int mode)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct dc_sc_config *conf = NULL;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	conf = &di->dc_sc_conf;
	*val = conf->rt_test_para[mode].rt_test_time;
	return 0;
}

static int sc_adsp_get_rt_test_time(unsigned int *val)
{
	return sc_adsp_get_rt_test_time_by_mode(val, DC_NORMAL_MODE);
}

static int sc_adsp_mainsc_get_rt_test_time(unsigned int *val)
{
	return sc_adsp_get_rt_test_time_by_mode(val, DC_CHAN1_MODE);
}

static int sc_adsp_auxsc_get_rt_test_time(unsigned int *val)
{
	return sc_adsp_get_rt_test_time_by_mode(val, DC_CHAN2_MODE);
}

static int sc_adsp_get_rt_test_result_by_mode(unsigned int *val, int mode)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	int ret;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	ret = dc_glink_get_rt_test_result(di->rt_test_result);
	if (ret)
		return -1;

	*val = di->rt_test_result[mode];
	return 0;
}

static int sc_adsp_get_rt_test_result(unsigned int *val)
{
	return sc_adsp_get_rt_test_result_by_mode(val, DC_NORMAL_MODE);
}

static int sc_adsp_mainsc_get_rt_test_result(unsigned int *val)
{
	return sc_adsp_get_rt_test_result_by_mode(val, DC_CHAN1_MODE);
}

static int sc_adsp_auxsc_get_rt_test_result(unsigned int *val)
{
	return sc_adsp_get_rt_test_result_by_mode(val, DC_CHAN2_MODE);
}

static int sc_adsp_get_hota_iin_limit(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct dc_sc_config *conf = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	conf = &di->dc_sc_conf;
	*val = conf->hota_iin_limit;
	return 0;
}

static int sc_adsp_get_startup_iin_limit(unsigned int *val)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct dc_sc_config *conf = NULL;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	conf = &di->dc_sc_conf;
	*val = conf->startup_iin_limit;
	return 0;
}

static struct dc_sysfs_ops dc_sysfs_ops_tbl[DC_SYSFS_END] = {
	[DC_SYSFS_IIN_THERMAL]= {
		sc_adsp_sysfs_get_iin_thermal, 
		sc_adsp_sysfs_set_iin_thermal},
	[DC_SYSFS_IIN_THERMAL_ICHG_CONTROL] = {
		sc_adsp_sysfs_get_iin_limit_ichg_control, 
		sc_adsp_sysfs_set_iin_limit_ichg_control},
	[DC_SYSFS_ICHG_CONTROL_ENABLE] = {
		sc_adsp_sysfs_get_ichg_control_enable,
		sc_adsp_sysfs_set_ichg_control_enable},
	[DC_SYSFS_ADAPTER_DETECT] = {
		sc_adsp_sysfs_get_adapter_detect,
		NULL},
	[DC_SYSFS_IADAPT] = {
		sc_adsp_sysfs_get_iadapt,
		NULL},
	[DC_SYSFS_FULL_PATH_RESISTANCE] = {
		sc_adsp_sysfs_get_full_path_resistance,
		NULL},
	[DC_SYSFS_DIRECT_CHARGE_SUCC] = {
		sc_adsp_sysfs_get_direct_charge_succ,
		NULL},
	[DC_SYSFS_SET_RESISTANCE_THRESHOLD] = {
		NULL,
		sc_adsp_sysfs_set_resistance_threshold},
	[DC_SYSFS_SET_CHARGETYPE_PRIORITY] = {
		NULL,
		sc_adsp_sysfs_set_chargetype_priority},
	[DC_SYSFS_THERMAL_REASON] = {
		sc_adsp_sysfs_get_thermal_reason,
		sc_adsp_sysfs_set_thermal_reason},
	[DC_SYSFS_AF] = {
		sc_adsp_sysfs_get_antifake,
		sc_adsp_sysfs_set_antifake},
	[DC_SYSFS_MULTI_SC_CUR] = {
		sc_adsp_sysfs_get_multi_sc_cur,
		NULL},
	[DC_SYSFS_SC_STATE] = {
		sc_adsp_sysfs_get_sc_state,
		NULL},
	[DC_SYSFS_DUMMY_VBAT] = {
		NULL,
		NULL},
};

#ifdef CONFIG_SYSFS
static ssize_t sc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t sc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info sc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_IIN_THERMAL, iin_thermal),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_IIN_THERMAL_ICHG_CONTROL, iin_thermal_ichg_control),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_ICHG_CONTROL_ENABLE, ichg_control_enable),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_THERMAL_REASON, thermal_reason),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_ADAPTER_DETECT, adaptor_detect),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_IADAPT, iadapt),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_FULL_PATH_RESISTANCE, full_path_resistance),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_DIRECT_CHARGE_SUCC, direct_charge_succ),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_SET_RESISTANCE_THRESHOLD, set_resistance_threshold),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_SET_CHARGETYPE_PRIORITY, set_chargetype_priority),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_AF, af),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_MULTI_SC_CUR, multi_sc_cur),
	power_sysfs_attr_ro(sc, 0444, DC_SYSFS_SC_STATE, sc_state),
	power_sysfs_attr_rw(sc, 0644, DC_SYSFS_DUMMY_VBAT, dummy_vbat),
};

#define SC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(sc_sysfs_field_tbl)

static struct attribute *sc_sysfs_attrs[SC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group sc_sysfs_attr_group = {
	.attrs = sc_sysfs_attrs,
};

static ssize_t sc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct dc_sysfs_ops *ops = NULL;
	struct dc_device_adsp *di = g_sc_adsp_di;
	int len;

	if (!di || !buf)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		sc_sysfs_field_tbl, SC_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	ops = &dc_sysfs_ops_tbl[info->name];
	if (!ops->get_property)
		return 0;

	len = ops->get_property(buf);
	return len;
}

static ssize_t sc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct dc_sysfs_ops *ops = NULL;
	struct dc_device_adsp *di = g_sc_adsp_di;
	int ret;

	if (!di || !buf)
		return -EINVAL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		sc_sysfs_field_tbl, SC_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	ops = &dc_sysfs_ops_tbl[info->name];
	if (!ops->set_property)
		return -EINVAL;

	ret = ops->set_property(buf);
	if (ret)
		return -EINVAL;
	return count;
}

static void sc_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(sc_sysfs_attrs,
		sc_sysfs_field_tbl, SC_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "direct_charger_sc",
		dev, &sc_sysfs_attr_group);
}

static void sc_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "direct_charger_sc",
		dev, &sc_sysfs_attr_group);
}
#else
static inline void sc_sysfs_create_group(struct device *dev)
{
}

static inline void sc_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static struct power_if_ops adsp_sc_if_ops = {
	.set_enable_charger = sc_adsp_set_enable_charger,
	.get_enable_charger = sc_adsp_get_enable_charger,
	.set_iin_limit = sc_adsp_set_iin_thermal,
	.get_iin_limit = sc_adsp_get_iin_thermal,
	.set_iin_thermal = sc_adsp_set_iin_thermal_index,
	.set_iin_thermal_all = sc_adsp_set_iin_thermal,
	.set_ichg_ratio = sc_adsp_set_ichg_ratio,
	.get_ichg_ratio = sc_adsp_get_ichg_ratio,
	.set_vterm_dec = sc_adsp_set_vterm_dec,
	.get_vterm_dec = sc_adsp_get_vterm_dec,
	.get_rt_test_time = sc_adsp_get_rt_test_time,
	.get_rt_test_result = sc_adsp_get_rt_test_result,
	.get_hota_iin_limit = sc_adsp_get_hota_iin_limit,
	.get_startup_iin_limit = sc_adsp_get_startup_iin_limit,
	.get_ibus = sc_adsp_get_ibus,
	.get_vbus = sc_adsp_get_vbus,
	.type_name = "sc",
};

static struct power_if_ops adsp_mainsc_if_ops = {
	.set_enable_charger = sc_adsp_mainsc_set_enable_charger,
	.get_enable_charger = sc_adsp_mainsc_get_enable_charger,
	.get_rt_test_time = sc_adsp_mainsc_get_rt_test_time,
	.get_rt_test_result = sc_adsp_mainsc_get_rt_test_result,
	.type_name = "mainsc",
};

static struct power_if_ops adsp_auxsc_if_ops = {
	.set_enable_charger = sc_adsp_auxsc_set_enable_charger,
	.get_enable_charger = sc_adsp_auxsc_get_enable_charger,
	.get_rt_test_time = sc_adsp_auxsc_get_rt_test_time,
	.get_rt_test_result = sc_adsp_auxsc_get_rt_test_result,
	.type_name = "auxsc",
};

static int sc_adsp_power_if_ops_register(struct dc_device_adsp *di)
{
	int ret;
	struct dc_sc_config *conf = &di->dc_sc_conf;

	ret = power_if_ops_register(&adsp_sc_if_ops);
	if (conf->multi_ic_mode_para.support_multi_ic) {
		hwlog_info("support_multi_ic\n");
		power_if_ops_register(&adsp_mainsc_if_ops);
		power_if_ops_register(&adsp_auxsc_if_ops);
	}
	return ret;
}

static void dc_sc_dts_sync_callback(void *dev_data)
{
	struct dc_device_adsp *di = (struct dc_device_adsp *)dev_data;

	if (!di)
		return;

	schedule_delayed_work(&di->sync_work, 0);
}

static void dc_sc_dts_glink_sync_work(struct work_struct *work)
{
	int ret;
	struct dc_device_adsp *di = container_of(work, struct dc_device_adsp, sync_work.work);

	if (!di)
		return;

	hwlog_info("sync dc sc dts to adsp\n");
	ret = hihonor_oem_glink_oem_update_config(DC_SC_CONFIG_ID, &di->dc_sc_conf, sizeof(di->dc_sc_conf));
	if (ret) {
		hwlog_err("sync dc sc dts to adsp fail\n");
		goto next;
	}

	hwlog_info("sync dc sysfs state to adsp\n");
	ret = hihonor_oem_glink_oem_update_config(DC_SC_SYSFS_DATA_ID, &di->sysfs_state, sizeof(di->sysfs_state));
	if (ret) {
		hwlog_err("sync dc sysfs state to adsp fail\n");
		goto next;
	}
	return;

next:
	schedule_delayed_work(&di->sync_work, msecs_to_jiffies(DC_SC_DTS_SYNC_INTERVAL));
}

static void dc_sc_adapter_handle_notify_event(void *dev_data, u32 notification, void *data)
{
	struct dc_device_adsp *di = (struct dc_device_adsp *)dev_data;

	if (!di)
		return;

	if (notification == OEM_NOTIFY_ADAPTER_INFO && data) {
		memcpy(&di->adap_info, data, sizeof(di->adap_info));
		adapter_get_device_info(ADAPTER_PROTOCOL_ADSP_SCP);
	} else if (notification == OEM_NOTIFY_DC_PLUG_OUT) {
		memset(&di->adap_info, 0, sizeof(di->adap_info));
		adapter_get_device_info(ADAPTER_PROTOCOL_ADSP_SCP);
	}
}

static struct hihonor_glink_ops dc_sc_dts_glink_ops = {
	.sync_data = dc_sc_dts_sync_callback,
	.notify_event = dc_sc_adapter_handle_notify_event
};

static int dc_sc_protocol_get_device_info(struct adapter_device_info *info)
{
	struct dc_device_adsp *di = g_sc_adsp_di;

	if (!info || !di)
		return -1;

	memcpy(info, &di->adap_info, sizeof(struct adapter_device_info));
	hwlog_info("get_device_info\n");
	return 0;
}


static struct adapter_protocol_ops adapter_protocol_adsp_scp_ops = {
	.type_name = "adsp_scp",
	.get_device_info = dc_sc_protocol_get_device_info,
};

#ifdef CONFIG_SYSFS
static ssize_t dc_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t dc_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info dc_mmi_sysfs_field_tbl[] = {
	power_sysfs_attr_ro(dc_mmi, 0444, DC_MMI_SYSFS_TIMEOUT, timeout),
	power_sysfs_attr_ro(dc_mmi, 0444, DC_MMI_SYSFS_LVC_RESULT, lvc_result),
	power_sysfs_attr_ro(dc_mmi, 0444, DC_MMI_SYSFS_SC_RESULT, sc_result),
	power_sysfs_attr_ro(dc_mmi, 0444, DC_MMI_SYSFS_HSC_RESULT, hsc_result),
	power_sysfs_attr_wo(dc_mmi, 0200, DC_MMI_SYSFS_TEST_STATUS, test_status),
};

#define DC_MMI_SYSFS_ATTRS_SIZE  ARRAY_SIZE(dc_mmi_sysfs_field_tbl)

static struct attribute *dc_mmi_sysfs_attrs[DC_MMI_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group dc_mmi_sysfs_attr_group = {
	.name = "dc_mmi",
	.attrs = dc_mmi_sysfs_attrs,
};

static ssize_t dc_mmi_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct dc_device_adsp *di = g_sc_adsp_di;
	struct power_sysfs_attr_info *info = NULL;
	int test_result;

	info = power_sysfs_lookup_attr(attr->attr.name, dc_mmi_sysfs_field_tbl,
		DC_MMI_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_MMI_SYSFS_LVC_RESULT:
		return scnprintf(buf, PAGE_SIZE, "%d\n", 0);
	case DC_MMI_SYSFS_SC_RESULT:
		dc_glink_get_mmi_test_result(&test_result);
		return scnprintf(buf, PAGE_SIZE, "%d\n", test_result);
	case DC_MMI_SYSFS_HSC_RESULT:
		return scnprintf(buf, PAGE_SIZE, "%d\n", 0);
	case DC_MMI_SYSFS_TIMEOUT:
		return scnprintf(buf, PAGE_SIZE, "%d\n", di->dc_sc_conf.mmi_para.timeout);
	default:
		break;
	}

	return 0;
}

static ssize_t dc_mmi_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	int val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name, dc_mmi_sysfs_field_tbl,
		DC_MMI_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case DC_MMI_SYSFS_TEST_STATUS:
		/* val=1 dc_mmi test start, val=0 dc_mmi test end */
		if (kstrtoint(buf, 10, &val) < 0 || (val < 0) ||
			(val > 1))
			return -EINVAL;
		dc_glink_set_mmi_test_flag(val);
		break;
	default:
		break;
	}

	return count;
}

static int dc_mmi_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(dc_mmi_sysfs_attrs, dc_mmi_sysfs_field_tbl,
		DC_MMI_SYSFS_ATTRS_SIZE);
	return sysfs_create_group(&dev->kobj, &dc_mmi_sysfs_attr_group);
}

static void dc_mmi_sysfs_remove_group(struct device *dev)
{
	sysfs_remove_group(&dev->kobj, &dc_mmi_sysfs_attr_group);
}
#else
static int dc_mmi_sysfs_create_group(struct device *dev)
{
	return 0;
}

static void dc_mmi_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

static void dc_mmi_init(struct device *dev)
{
	int ret;

	hwlog_info("dc_mmi_init\n");
	ret = dc_mmi_sysfs_create_group(dev);
	if (ret)
		return;

	hwlog_info("dc_mmi_init succ\n");
	return;
}

static void dc_mmi_exit(struct device *dev)
{
	if (!dev)
		return;

	dc_mmi_sysfs_remove_group(dev);
}

static struct power_test_ops g_dc_mmi_ops = {
	.name = "dc_mmi_adsp",
	.init = dc_mmi_init,
	.exit = dc_mmi_exit,
};

static void dc_sc_adsp_init_para(struct dc_device_adsp *di)
{
	int i;
	struct dc_sc_config *conf = &di->dc_sc_conf;
	struct dc_sysfs_state *state = &di->sysfs_state;

	for (i = 0; i < DC_TEMP_LEVEL; i++) {
		if (conf->temp_para[i].temp_cur_max > di->iin_thermal_default)
			di->iin_thermal_default = conf->temp_para[i].temp_cur_max;
	}

	state->sysfs_enable_charger = DC_SC_ENABLE_CHARGE;
	state->sysfs_mainsc_enable_charger = DC_SC_ENABLE_CHARGE;
	state->sysfs_auxsc_enable_charger = DC_SC_ENABLE_CHARGE;
	state->ichg_ratio = DC_SC_DEFAULT_ICHG_RATIO;
	state->vterm_dec = DC_SC_DEFAULT_VTERM_DEC;
}

static int dc_sc_adsp_probe(struct platform_device *pdev)
{
	int ret;
	struct dc_device_adsp *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	hwlog_info("%s %d\n", __func__, __LINE__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	g_sc_adsp_di = di;

	ret = adsp_dc_dts_parse(np, &di->dc_sc_conf);
	if (ret)
		goto fail_free_mem;

	dc_sc_adsp_init_para(di);

	dc_sc_dts_glink_ops.dev_data = di;
	ret = hihonor_oem_glink_ops_register(&dc_sc_dts_glink_ops);
	if (ret) {
		hwlog_err("%s fail to register glink ops\n", __func__);
		goto fail_free_mem;
	}

	INIT_DELAYED_WORK(&di->sync_work, dc_sc_dts_glink_sync_work);

	sc_sysfs_create_group(di->dev);
	ret = sc_adsp_power_if_ops_register(di);
	if (ret < 0)
		goto fail_create_link;

	ret = adapter_protocol_ops_register(&adapter_protocol_adsp_scp_ops);
	if (ret < 0)
		goto fail_create_link;

	ret = power_test_ops_register(&g_dc_mmi_ops);
	if (ret < 0)
		goto fail_create_link;

	power_dbg_ops_register("adsp_dc_sc_send_conf", di,
		NULL,
		(power_dbg_store)adsp_dc_sc_send_conf_store);

	return 0;

fail_create_link:
	sc_sysfs_remove_group(di->dev);

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_sc_adsp_di = NULL;

	return ret;
}

static int dc_sc_adsp_remove(struct platform_device *pdev)
{
	struct dc_device_adsp *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	devm_kfree(&pdev->dev, di);
	return 0;
}

static const struct of_device_id dc_sc_adsp_match_table[] = {
	{
		.compatible = "honor,direct_charger_sc_adsp",
		.data = NULL,
	},
	{},
};


static struct platform_driver dc_sc_adsp_driver = {
	.probe = dc_sc_adsp_probe,
	.remove = dc_sc_adsp_remove,
	.driver = {
		.name = "honor,direct_charger_sc_adsp",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(dc_sc_adsp_match_table),
	},
};

static int __init dc_sc_adsp_init(void)
{
	return platform_driver_register(&dc_sc_adsp_driver);
}

static void __exit dc_sc_adsp_exit(void)
{
	platform_driver_unregister(&dc_sc_adsp_driver);
}

device_initcall_sync(dc_sc_adsp_init);
module_exit(dc_sc_adsp_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charge sc adsp module driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
