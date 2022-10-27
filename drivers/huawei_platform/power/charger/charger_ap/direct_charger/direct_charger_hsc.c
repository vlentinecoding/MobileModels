/*
 * direct_charger_sc.c
 *
 * direct charger with sc (switch cap) driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
//#include <mt-plat/mtk_battery.h>
//#include <mt-plat/charger_type.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_common.h>
#include <chipset_common/hwpower/power_interface.h>
#include <chipset_common/hwpower/power_devices_info.h>
#include <chipset_common/hwpower/adapter_protocol.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <huawei_platform/power/hihonor_charger_glink.h>
#include <chipset_common/hwpower/power_dsm.h>
#include <chipset_common/hwpower/power_cmdline.h>
#ifdef CONFIG_HUAWEI_USB_SHORT_CIRCUIT_PROTECT
#include <huawei_platform/power/uscp/usb_short_circuit_protect.h>
#endif
#include <chipset_common/hwpower/direct_charge/direct_charge_debug.h>
#include <huawei_platform/power/huawei_charger_adaptor.h>

#define HWLOG_TAG direct_charge_hsc
HWLOG_REGIST();

static struct direct_charge_device *g_hsc_di;
static struct direct_charge_volt_para_group g_hsc_volt_para[DC_VOLT_GROUP_MAX];

struct atomic_notifier_head hsc_fault_notifier_list;
ATOMIC_NOTIFIER_HEAD(hsc_fault_notifier_list);

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
static void hsc_get_devices_info_by_type(struct direct_charge_device *di,
	int type, int dev_check_type)
{
	int ret;
	struct power_devices_info_data *power_dev_info = NULL;

	if (!di)
		return;

	ret = dc_ic_init(HSC_MODE, type);
	di->ls_id = dc_get_ic_id(HSC_MODE, type);
	ret |= dc_ic_exit(HSC_MODE, type);
	if ((di->ls_id < DC_DEVICE_ID_BEGIN) || (di->ls_id >= DC_DEVICE_ID_END))
		di->ls_id = DC_DEVICE_ID_END;
	di->ls_name = dc_get_device_name(di->ls_id);
	hwlog_info("switchcap id=%d, %s\n", di->ls_id, di->ls_name);
	if (ret == 0)
		set_hw_dev_flag(dev_check_type);

	power_dev_info = power_devices_info_register();
	if (power_dev_info) {
		power_dev_info->dev_name = di->ls_name;
		power_dev_info->dev_id = di->ls_id;
		power_dev_info->ver_id = 0;
	}
}

static void hsc_get_devices_info(struct direct_charge_device *di)
{
	hsc_get_devices_info_by_type(di, CHARGE_IC_MAIN, DEV_I2C_SWITCHCAP);
	if (di->multi_ic_mode_para.support_multi_ic)
		hsc_get_devices_info_by_type(di, CHARGE_IC_AUX, DEV_I2C_SC_AUX);
}
#else
static void hsc_get_devices_info(struct direct_charge_device *di)
{
}
#endif /* CONFIG_HUAWEI_HW_DEV_DCT */

void hsc_set_ignore_full_res(bool flag)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!di)
		return;

	di->ignore_full_path_res = flag;
}
static int hsc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -1;

	ret = hsc_set_disable_flags((val ?
		DC_CLEAR_DISABLE_FLAGS : DC_SET_DISABLE_FLAGS),
		DC_DISABLE_SYS_NODE);
	hwlog_info("set enable_charger=%d\n", di->sysfs_enable_charger);
	return ret;
}

static int hsc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_enable_charger;
	return 0;
}

static int mainhsc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -1;

	di->sysfs_mainsc_enable_charger = val;
	hwlog_info("set mainsc enable_charger=%d\n",
		di->sysfs_mainsc_enable_charger);
	return 0;
}

static int mainhsc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_mainsc_enable_charger;
	return 0;
}

static int auxhsc_set_enable_charger(unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	/* must be 0 or 1, 0: disable, 1: enable */
	if ((val < 0) || (val > 1))
		return -1;

	di->sysfs_auxsc_enable_charger = val;
	hwlog_info("set auxsc enable_charger=%d\n",
		di->sysfs_auxsc_enable_charger);
	return 0;
}

static int auxhsc_get_enable_charger(unsigned int *val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->sysfs_auxsc_enable_charger;
	return 0;
}

static int hsc_set_iin_limit(unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;
	int index;
	int cur_low;
	unsigned int idx;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val is too large: %u, tuned as default\n", val);
		val = di->iin_thermal_default;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -1;
	}

	index = di->stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;
	idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	if (val == 0)
		di->sysfs_iin_thermal_array[idx] = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal_array[idx] = cur_low;
	else
		di->sysfs_iin_thermal_array[idx] = val;

	hwlog_info("set input current: %u, limit current: %d, current channel type: %u\n",
		val, di->sysfs_iin_thermal_array[idx], idx);
	return 0;
}

static int hsc_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;
	int index;
	int cur_low;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val is too large: %u, tuned as default\n", val);
		val = di->iin_thermal_default;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -1;
	}

	index = di->stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

	if (val == 0)
		di->sysfs_iin_thermal_array[idx] = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal_array[idx] = cur_low;
	else
		di->sysfs_iin_thermal_array[idx] = val;
	di->sysfs_iin_thermal = di->sysfs_iin_thermal_array[idx];

	hwlog_info("set input current: %u, limit current: %d, channel type: %u\n",
		val, di->sysfs_iin_thermal_array[idx], idx);
	return 0;
}

static int hsc_set_iin_limit_ichg_control(unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;
	int index;
	int cur_low;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (val > di->iin_thermal_default) {
		hwlog_err("val is too large: %u, tuned as default\n", val);
		return -1;
	}

	if ((di->stage_size < 1) || (di->stage_size > DC_VOLT_LEVEL)) {
		hwlog_err("invalid stage_size=%d\n", di->stage_size);
		return -1;
	}

	index = di->stage_size - 1;
	cur_low = di->orig_volt_para_p[0].volt_info[index].cur_th_low;

	if (val == 0)
		di->sysfs_iin_thermal_ichg_control = di->iin_thermal_default;
	else if (val < cur_low)
		di->sysfs_iin_thermal_ichg_control = cur_low;
	else
		di->sysfs_iin_thermal_ichg_control = val;

	hwlog_info("ichg_control set input current: %u, limit current: %d\n",
		val, di->sysfs_iin_thermal_ichg_control);
	return 0;
}

static int hsc_get_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_hsc_di;
	int idx;

	if (!di || !val) {
		hwlog_err("di or val is null\n");
		return -1;
	}

	idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
	*val = di->sysfs_iin_thermal_array[idx];
	return 0;
}

static int hsc_set_iin_thermal(unsigned int index, unsigned int iin_thermal_value)
{
	if (index >= DC_CHANNEL_TYPE_END) {
		hwlog_err("error index: %u, out of boundary\n", index);
		return -1;
	}
	return hsc_set_iin_limit_array(index, iin_thermal_value);
}

static int hsc_set_iin_thermal_all(unsigned int value)
{
	int i;

	for (i = DC_CHANNEL_TYPE_BEGIN; i < DC_CHANNEL_TYPE_END; i++) {
		if (hsc_set_iin_limit_array(i, value))
			return -1;
	}
	return 0;
}

static int hsc_set_ichg_ratio(unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->ichg_ratio = val;
	hwlog_info("set ichg_ratio=%d\n", val);
	return 0;
}

static int hsc_set_vterm_dec(unsigned int val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	di->vterm_dec = val;
	hwlog_info("set vterm_dec=%d\n", val);
	return 0;
}

static int hsc_get_rt_test_time_by_mode(unsigned int *val, int mode)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->rt_test_para[mode].rt_test_time;
	return 0;
}

static int hsc_get_rt_test_time(unsigned int *val)
{
	return hsc_get_rt_test_time_by_mode(val, DC_NORMAL_MODE);
}

static int mainhsc_get_rt_test_time(unsigned int *val)
{
	return hsc_get_rt_test_time_by_mode(val, DC_CHAN1_MODE);
}

static int auxhsc_get_rt_test_time(unsigned int *val)
{
	return hsc_get_rt_test_time_by_mode(val, DC_CHAN2_MODE);
}

static int hsc_get_rt_test_result_by_mode(unsigned int *val, int mode)
{
	int iin_thermal_th;
	struct direct_charge_device *di = g_hsc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	iin_thermal_th = di->rt_test_para[mode].rt_curr_th + 500; /* margin is 500mA */
	if (pd_dpm_get_cc_moisture_status() || di->bat_temp_err_flag ||
		di->rt_test_para[mode].rt_test_result ||
		(di->sysfs_enable_charger == 0) ||
		((direct_charge_get_stage_status() == DC_STAGE_CHARGING) &&
		(di->sysfs_iin_thermal < iin_thermal_th)) ||
		direct_charge_check_charge_done())
		*val = 0; /* 0: succ */
	else
		*val = 1; /* 1: fail */

	return 0;
}

static int hsc_get_rt_test_result(unsigned int *val)
{
	return hsc_get_rt_test_result_by_mode(val, DC_NORMAL_MODE);
}

static int mainhsc_get_rt_test_result(unsigned int *val)
{
	return hsc_get_rt_test_result_by_mode(val, DC_CHAN1_MODE);
}

static int auxhsc_get_rt_test_result(unsigned int *val)
{
	return hsc_get_rt_test_result_by_mode(val, DC_CHAN2_MODE);
}

static int hsc_get_hota_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->hota_iin_limit;
	return 0;
}

static int hsc_get_startup_iin_limit(unsigned int *val)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	*val = di->startup_iin_limit;
	return 0;
}

static int hsc_get_multi_cur(char *buf, unsigned int len)
{
	int ret;
	int i = 0;
	char temp_buf[DC_HSC_CUR_LEN] = {0};
	struct direct_charge_device *di = g_hsc_di;

	if (!power_cmdline_is_factory_mode())
		return 0;

	if (!di || (di->multi_ic_mode_para.support_multi_ic == 0)) {
		hwlog_err("di is null or not support multi sc\n");
		return 0;
	}

	ret = snprintf(buf, len, "%d,%d", di->curr_info.channel_num,
		di->curr_info.ibat_max);
	while ((ret > 0) && (i < di->curr_info.channel_num)) {
		memset(temp_buf, 0, sizeof(temp_buf));
		ret += snprintf(temp_buf, DC_HSC_CUR_LEN,
			"\r\n^MULTICHARGER:%s,%d,%d,%d,%d",
			di->curr_info.ic_name[i], di->curr_info.ibus[i],
			di->curr_info.vout[i], di->curr_info.vbat[i],
			di->curr_info.tbat[i]);
		strncat(buf, temp_buf, strlen(temp_buf));
		i++;
	}

	return ret;
}


static int hsc_get_ibus(int *ibus)
{
	int ret;

	if (!ibus)
		return -1;

	ret = dc_get_ic_ibus(HSC_MODE, CHARGE_MULTI_IC, ibus);
	if (ret || ibus < 0)
		return -1;
	return 0;
}

static int hsc_get_vbus(int *vbus)
{
	struct direct_charge_device *di = g_hsc_di;

	if (!di || di->ls_vbus < 0 || !vbus)
		return -1;

	*vbus = di->ls_vbus;
	return 0;
}

static struct power_if_ops hsc_if_ops = {
	.set_enable_charger = hsc_set_enable_charger,
	.get_enable_charger = hsc_get_enable_charger,
	.set_iin_limit = hsc_set_iin_limit,
	.get_iin_limit = hsc_get_iin_limit,
	.set_iin_thermal = hsc_set_iin_thermal,
	.set_iin_thermal_all = hsc_set_iin_thermal_all,
	.set_ichg_ratio = hsc_set_ichg_ratio,
	.set_vterm_dec = hsc_set_vterm_dec,
	.get_rt_test_time = hsc_get_rt_test_time,
	.get_rt_test_result = hsc_get_rt_test_result,
	.get_hota_iin_limit = hsc_get_hota_iin_limit,
	.get_startup_iin_limit = hsc_get_startup_iin_limit,
	.get_ibus = hsc_get_ibus,
	.get_vbus = hsc_get_vbus,
	.type_name = "hsc",
};

static struct power_if_ops mainhsc_if_ops = {
	.set_enable_charger = mainhsc_set_enable_charger,
	.get_enable_charger = mainhsc_get_enable_charger,
	.get_rt_test_time = mainhsc_get_rt_test_time,
	.get_rt_test_result = mainhsc_get_rt_test_result,
	.type_name = "mainhsc",
};

static struct power_if_ops auxhsc_if_ops = {
	.set_enable_charger = auxhsc_set_enable_charger,
	.get_enable_charger = auxhsc_get_enable_charger,
	.get_rt_test_time = auxhsc_get_rt_test_time,
	.get_rt_test_result = auxhsc_get_rt_test_result,
	.type_name = "auxhsc",
};

static void hsc_power_if_ops_register(struct direct_charge_device *di)
{
	power_if_ops_register(&hsc_if_ops);
	if (di->multi_ic_mode_para.support_multi_ic) {
		power_if_ops_register(&mainhsc_if_ops);
		power_if_ops_register(&auxhsc_if_ops);
	}
}

int hsc_get_di(struct direct_charge_device **di)
{
	if (!g_hsc_di || !di) {
		hwlog_err("g_hsc_di or di is null\n");
		return -1;
	}

	*di = g_hsc_di;

	return 0;
}

void hsc_get_fault_notifier(struct atomic_notifier_head **notifier)
{
	if (!notifier) {
		hwlog_err("notifier is null\n");
		return;
	}

	*notifier = &hsc_fault_notifier_list;
}
int hsc_set_disable_flags(int val, int type)
{
	int i;
	unsigned int disable;
	struct direct_charge_device *di = g_hsc_di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	if (type < DC_DISABLE_BEGIN || type >= DC_DISABLE_END) {
		hwlog_err("invalid disable_type=%d\n", type);
		return -1;
	}

	disable = 0;
	di->sysfs_disable_charger[type] = val;
	for (i = 0; i < DC_DISABLE_END; i++)
		disable |= di->sysfs_disable_charger[i];
	di->sysfs_enable_charger = !disable;

	hwlog_info("set_disable_flag val=%d, type=%d, disable=%d\n",
		val, type, disable);
	return 0;
}

static void hsc_fault_work(struct work_struct *work)
{
	char buf[POWER_DSM_BUF_SIZE_0256] = { 0 };
	char reg_info[POWER_DSM_BUF_SIZE_0128] = { 0 };
	struct direct_charge_device *di = NULL;
	struct nty_data *data = NULL;

	if (!work) {
		hwlog_err("work is null\n");
		return;
	}

	di = container_of(work, struct direct_charge_device, fault_work);
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	data = di->fault_data;
	direct_charge_set_stop_charging_flag(1);

	if (data)
		snprintf(reg_info, sizeof(reg_info),
			"sc charge_fault=%d, addr=0x%x, event1=0x%x, event2=0x%x\n",
			di->charge_fault, data->addr,
			data->event1, data->event2);
	else
		snprintf(reg_info, sizeof(reg_info),
			"sc charge_fault=%d, addr=0x0, event1=0x0, event2=0x0\n",
			di->charge_fault);

	dc_fill_eh_buf(di->dsm_buff, sizeof(di->dsm_buff),
		DC_EH_HAPPEN_SC_FAULT, reg_info);

	switch (di->charge_fault) {
	case DC_FAULT_VBUS_OVP:
		hwlog_err("vbus ovp happened\n");
		snprintf(buf, sizeof(buf), "vbus ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_VBUS_OVP, buf);
		break;
	case DC_FAULT_TSBAT_OTP:
		hwlog_err("tsbat otp happened\n");
		snprintf(buf, sizeof(buf), "tsbat otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TSBAT_OTP, buf);
		break;
	case DC_FAULT_TSBUS_OTP:
		hwlog_err("tsbus otp happened\n");
		snprintf(buf, sizeof(buf), "tsbus otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TSBUS_OTP, buf);
		break;
	case DC_FAULT_TDIE_OTP:
		hwlog_err("tdie otp happened\n");
		snprintf(buf, sizeof(buf), "tdie otp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_TDIE_OTP, buf);
		break;
	case DC_FAULT_VDROP_OVP:
		hwlog_err("vdrop ovp happened\n");
		snprintf(buf, sizeof(buf), "vdrop ovp happened, ic_addr:%d, reg_val:%d, "
			"ic_role:%d\n", data->addr, data->event5, data->ic_role);
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_HSC_FAULT_VDROP_OVP, buf);
		break;
	case DC_FAULT_AC_OVP:
		hwlog_err("AC ovp happened\n");
		snprintf(buf, sizeof(buf), "ac ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_AC_OVP, buf);
		break;
	case DC_FAULT_VBAT_OVP:
		hwlog_err("vbat ovp happened\n");
		snprintf(buf, sizeof(buf), "vbat ovp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_VBAT_OVP, buf);
		break;
	case DC_FAULT_IBAT_OCP:
		hwlog_err("ibat ocp happened\n");
		snprintf(buf, sizeof(buf), "ibat ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_IBAT_OCP, buf);
		break;
	case DC_FAULT_IBUS_OCP:
		hwlog_err("ibus ocp happened\n");
		snprintf(buf, sizeof(buf), "ibus ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_IBUS_OCP, buf);
		break;
	case DC_FAULT_CONV_OCP:
		hwlog_err("conv ocp happened\n");
		snprintf(buf, sizeof(buf), "conv ocp happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_SC_FAULT_CONV_OCP, buf);
		chg_set_adaptor_test_result(TYPE_HSC, 3);
		di->sc_conv_ocp_count++;
		break;
	case DC_FAULT_LTC7820:
		hwlog_err("ltc7820 chip error happened\n");
		snprintf(buf, sizeof(buf), "ltc7820 chip error happened\n");
		strncat(buf, reg_info, strlen(reg_info));
		break;
	case DC_FAULT_INA231:
		hwlog_err("ina231 interrupt happened\n");
		dc_ic_enable(SC_MODE, di->cur_mode, DC_IC_DISABLE);
		dc_close_aux_wired_channel();
		break;
	case DC_FAULT_CC_SHORT:
		hwlog_err("typec cc vbus short happened\n");
		break;
	case DC_FAULT_I2C_ERROR:
		hwlog_err("ic %s i2c error happend\n", data->ic_name);
		snprintf(buf, sizeof(buf), "dc i2c error happend, ic_name:%s, ic_addr:%d,"
			"ic_role:%d\n", data->ic_name, data->addr, data->ic_role);
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC, ERROR_CHARGE_I2C_RW, buf);
		break;
	case DC_FAULT_IBUS_OCP_PEAK:
		hwlog_err("ibus ocp peak happend\n");
		snprintf(buf, sizeof(buf), "dc ibus ocp peak happened, ic_addr:%d, reg_val:%d "
			"ic_role:%d\n", data->addr, data->event5, data->ic_role);
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_HSC_FAULT_IBUS_OCP_PEAK, buf);
		break;
	case DC_FAULT_IQ6Q8_OCP_PEAK:
		hwlog_err("IQ6Q8 ocp peak happend\n");
		snprintf(buf, sizeof(buf), "dc IQ6Q8 peak happened, ic_addr:%d, reg_val:%d "
			"ic_role:%d\n", data->addr, data->event5, data->ic_role);
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_HSC_FAULT_IQ6Q7_OCP_PEAK, buf);
		break;
	case DC_FAULT_VDROP_MIN:
		hwlog_err("vdrop min happend\n");
		snprintf(buf, sizeof(buf), "dc vdrop min happened, ic_addr:%d, reg_val:%d "
			"ic_role:%d\n", data->addr, data->event5, data->ic_role);
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_HSC_FAULT_VDROP_MIN, buf);
		break;
	case DC_FAULT_IBUS_UCP:
		hwlog_err("ibus ucp happend\n");
		snprintf(buf, sizeof(buf), "ibus ucp happened, ic_addr:%d, reg_val:%d "
			"ic_role:%d\n", data->addr, data->event6, data->ic_role);
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_HSC_FAULT_IBUS_UCP, buf);
		break;
	case DC_FAULT_IBUS_RCP:
		hwlog_err("ibus rcp happend\n");
		snprintf(buf, sizeof(buf), "ibus rcp happened, ic_addr:%d, reg_val:%d "
			"ic_role:%d\n", data->addr, data->event6, data->ic_role);
		power_dsm_dmd_report(POWER_DSM_DIRECT_CHARGE_SC,
			DSM_DIRECT_CHARGE_HSC_FAULT_IBUS_RCP, buf);
		break;
	default:
		hwlog_err("unknown fault: %d happened\n", di->charge_fault);
		break;
	}
}

#ifdef CONFIG_SYSFS
static ssize_t hsc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t hsc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info hsc_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_IIN_THERMAL, iin_thermal),
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_IIN_THERMAL_ICHG_CONTROL, iin_thermal_ichg_control),
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_ICHG_CONTROL_ENABLE, ichg_control_enable),
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_THERMAL_REASON, thermal_reason),
	power_sysfs_attr_ro(hsc, 0444, DC_SYSFS_ADAPTER_DETECT, adaptor_detect),
	power_sysfs_attr_ro(hsc, 0444, DC_SYSFS_IADAPT, iadapt),
	power_sysfs_attr_ro(hsc, 0444, DC_SYSFS_FULL_PATH_RESISTANCE, full_path_resistance),
	power_sysfs_attr_ro(hsc, 0444, DC_SYSFS_DIRECT_CHARGE_SUCC, direct_charge_succ),
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_SET_RESISTANCE_THRESHOLD, set_resistance_threshold),
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_SET_CHARGETYPE_PRIORITY, set_chargetype_priority),
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_AF, af),
	power_sysfs_attr_ro(hsc, 0444, DC_SYSFS_MULTI_SC_CUR, multi_hsc_cur),
	power_sysfs_attr_ro(hsc, 0444, DC_SYSFS_SC_STATE, hsc_state),
	power_sysfs_attr_rw(hsc, 0644, DC_SYSFS_DUMMY_VBAT, dummy_vbat),
};

#define HSC_SYSFS_ATTRS_SIZE  ARRAY_SIZE(hsc_sysfs_field_tbl)

static struct attribute *hsc_sysfs_attrs[HSC_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group hsc_sysfs_attr_group = {
	.attrs = hsc_sysfs_attrs,
};

static ssize_t hsc_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	enum charger_type type = mt_get_charger_type();
	int chg_state = direct_charge_in_charging_stage();
	int ret;
	int len;
	int idx;

	info = power_sysfs_lookup_attr(attr->attr.name,
		hsc_sysfs_field_tbl, HSC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_iin_thermal_array[idx]);
		break;
	case DC_SYSFS_IIN_THERMAL_ICHG_CONTROL:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_iin_thermal_ichg_control);
		break;
	case DC_SYSFS_ICHG_CONTROL_ENABLE:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->ichg_control_enable);
		break;
	case DC_SYSFS_THERMAL_REASON:
		len = snprintf(buf, PAGE_SIZE, "%s\n", di->thermal_reason);
		break;
	case DC_SYSFS_ADAPTER_DETECT:
		ret = ADAPTER_DETECT_FAIL;
		if (adapter_get_protocol_register_state(ADAPTER_PROTOCOL_SCP)) {
			len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
			break;
		}

		if ((type == CHARGER_TYPE_STANDARD) ||
			(type == CHARGER_REMOVED &&
			chg_state == DC_IN_CHARGING_STAGE)) {
			if (di->adapter_detect_flag == DC_ADAPTER_DETECT)
				ret = 0;
			else
				ret = ADAPTER_DETECT_FAIL;
		}

		len = snprintf(buf, PAGE_SIZE, "%d\n", ret);
		break;
	case DC_SYSFS_IADAPT:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->iadapt);
		break;
	case DC_SYSFS_FULL_PATH_RESISTANCE:
		len = snprintf(buf, PAGE_SIZE, "%d\n",
			di->full_path_resistance);
		break;
	case DC_SYSFS_DIRECT_CHARGE_SUCC:
		len = snprintf(buf, PAGE_SIZE, "%d\n", di->dc_succ_flag);
		break;
	case DC_SYSFS_AF:
		len = direct_charge_get_antifake_data(di, buf, DC_AF_KEY_LEN);
		break;
	case DC_SYSFS_MULTI_SC_CUR:
		len = hsc_get_multi_cur(buf, PAGE_SIZE);
		break;
	case DC_SYSFS_SC_STATE:
		idx = (di->cur_mode == CHARGE_MULTI_IC) ? DC_DUAL_CHANNEL : DC_SINGLE_CHANNEL;
		len = snprintf(buf, PAGE_SIZE, "%d\n", idx);
		break;
	case DC_SYSFS_DUMMY_VBAT:
		len = snprintf (buf, PAGE_SIZE, "%d\n", di->dummy_vbat);
		break;
	default:
		len = 0;
		break;
	}

	return len;
}

static ssize_t hsc_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct power_sysfs_attr_info *info = NULL;
	struct direct_charge_device *di = dev_get_drvdata(dev);
	long val = 0;

	info = power_sysfs_lookup_attr(attr->attr.name,
		hsc_sysfs_field_tbl, HSC_SYSFS_ATTRS_SIZE);
	if (!info || !di)
		return -EINVAL;

	switch (info->name) {
	case DC_SYSFS_IIN_THERMAL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;

		hsc_set_iin_limit((unsigned int)val);
		break;
	case DC_SYSFS_IIN_THERMAL_ICHG_CONTROL:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		hsc_set_iin_limit_ichg_control((unsigned int)val);
		break;
	case DC_SYSFS_ICHG_CONTROL_ENABLE:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		di->ichg_control_enable = val;
		break;
	case DC_SYSFS_THERMAL_REASON:
		if (strlen(buf) >= DC_THERMAL_REASON_SIZE)
			return -EINVAL;
		snprintf(di->thermal_reason, strlen(buf), "%s", buf);
		sysfs_notify(&di->dev->kobj, NULL, "thermal_reason");
		hwlog_info("THERMAL set reason = %s, buf = %s\n", di->thermal_reason, buf);
		break;
	case DC_SYSFS_SET_RESISTANCE_THRESHOLD:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > DC_MAX_RESISTANCE))
			return -EINVAL;

		hwlog_info("set resistance_threshold=%ld\n", val);

		di->std_cable_full_path_res_max = val;
		di->nonstd_cable_full_path_res_max = val;
		di->ctc_cable_full_path_res_max = val;
		di->ignore_full_path_res = true;
		break;
	case DC_SYSFS_SET_CHARGETYPE_PRIORITY:
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) ||
			(val < 0) || (val > DC_MAX_RESISTANCE))
			return -EINVAL;

		hwlog_info("set chargertype_priority=%ld\n", val);

		if (val == DC_CHARGE_TYPE_HSC)
			direct_charge_set_local_mode(OR_SET, HSC_MODE);
		else if (val == DC_CHARGE_TYPE_LVC)
			direct_charge_set_local_mode(AND_SET, LVC_MODE);
		else
			hwlog_info("invalid chargertype priority\n");
		break;
	case DC_SYSFS_AF:
		/* must be 0 or 1 */
		if ((kstrtol(buf, POWER_BASE_DEC, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;

		direct_charge_set_antifake_result(di, val);
		break;
	case DC_SYSFS_DUMMY_VBAT:
		if (kstrtol(buf, POWER_BASE_DEC, &val) < 0)
			return -EINVAL;
		hwlog_err("dc set mainhsc and auxhsc dummy vbat is %u\n", (unsigned int)val);
		di->dummy_vbat =  (unsigned int)val;
		break;
	default:
		break;
	}

	return count;
}

static void hsc_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(hsc_sysfs_attrs,
		hsc_sysfs_field_tbl, HSC_SYSFS_ATTRS_SIZE);
	power_sysfs_create_link_group("hw_power", "charger", "direct_charger_hsc",
		dev, &hsc_sysfs_attr_group);
}

static void hsc_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "direct_charger_hsc",
		dev, &hsc_sysfs_attr_group);
}
#else
static inline void hsc_sysfs_create_group(struct device *dev)
{
}

static inline void hsc_sysfs_remove_group(struct device *dev)
{
}
#endif /* CONFIG_SYSFS */

/* get manufacture RT start or end flag */
static int get_runtest_stop_charge_flag(void)
{
	const char *pstr = NULL;

	pstr = strstr(saved_command_line, "RT_type=RT F");
	if (!pstr) {
		hwlog_info("No fastboot RT_type info\n");
		return false;
	}

	return true;
}

static void hsc_init_parameters(struct direct_charge_device *di)
{
	int i;

	di->sysfs_enable_charger = 1;
	if (power_cmdline_is_factory_mode() && get_runtest_stop_charge_flag()) {
		di->sysfs_enable_charger = 0;
		hwlog_info("rt disable charge\n");
	}
	di->dc_volt_ratio = 4;
	di->sysfs_mainsc_enable_charger = 1;
	di->sysfs_auxsc_enable_charger = 1;
	di->dc_stage = DC_STAGE_DEFAULT;
	di->sysfs_iin_thermal = di->iin_thermal_default;
	di->sysfs_iin_thermal_array[DC_SINGLE_CHANNEL] = di->iin_thermal_default;
	di->sysfs_iin_thermal_array[DC_DUAL_CHANNEL] = di->iin_thermal_default;
	di->max_adapter_iset = di->iin_thermal_default;
	di->sysfs_iin_thermal_ichg_control = di->iin_thermal_default;
	di->ichg_control_enable = 0;
	di->dc_succ_flag = DC_ERROR;
	di->scp_stop_charging_complete_flag = 1;
	di->dc_err_report_flag = FALSE;
	di->bat_temp_err_flag = false;
	di->sc_conv_ocp_count = 0;
	di->ignore_full_path_res = false;
	di->cur_mode = CHARGE_IC_MAIN;
	di->local_mode = HSC_MODE;
	di->force_single_path_flag = false;
	for (i = 0; i < DC_MODE_TOTAL; i++)
		di->rt_test_para[i].rt_test_result = false;
	di->cc_safe = true;
}

static void hsc_init_mulit_ic_check_info(struct direct_charge_device *di)
{
	di->multi_ic_check_info.limit_current = di->iin_thermal_default;
	di->multi_ic_check_info.ibat_th = MULTI_IC_INFO_IBAT_TH_DEFAULT;
}

static void hiz_mode_set_work(struct work_struct *work)
{
        hwlog_info("%s disable hiz mode\n", __func__);
        charge_enable_hz(HIZ_MODE_DISABLE);
	hihonor_charger_glink_enable_charge(1);
}

static int hsc_probe(struct platform_device *pdev)
{
	int ret;
	struct direct_charge_device *di = NULL;
	struct device_node *np = NULL;
	struct direct_charge_pps_ops *l_pps_ops = NULL;
	struct direct_charge_cd_ops *l_cd_ops = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	di->orig_volt_para_p = g_hsc_volt_para;

	ret = dc_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	direct_charge_get_g_pps_ops(&l_pps_ops);
	direct_charge_get_g_cd_ops(&l_cd_ops);
	di->pps_ops = l_pps_ops;
	di->cd_ops = l_cd_ops;
	hsc_init_parameters(di);
	hsc_init_mulit_ic_check_info(di);
	direct_charge_set_local_mode(OR_SET, HSC_MODE);

	platform_set_drvdata(pdev, di);

	di->charging_wq = create_singlethread_workqueue("hsc_charging_wq");
	di->kick_wtd_wq = create_singlethread_workqueue("hsc_wtd_wq");

	wakeup_source_init(&di->charging_lock, "hsc_wakelock");
	INIT_WORK(&di->calc_thld_work, direct_charge_calc_thld_work);
	INIT_WORK(&di->control_work, direct_charge_control_work);
	INIT_WORK(&di->fault_work, hsc_fault_work);
	INIT_WORK(&di->kick_wtd_work, direct_charge_kick_wtd_work);
	INIT_DELAYED_WORK(&di->hiz_mode_work, hiz_mode_set_work);

	hrtimer_init(&di->calc_thld_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->calc_thld_timer.function = direct_charge_calc_thld_timer_func;
	hrtimer_init(&di->control_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->control_timer.function = direct_charge_control_timer_func;
	hrtimer_init(&di->kick_wtd_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	di->kick_wtd_timer.function = direct_charge_kick_wtd_timer_func;

	init_completion(&di->dc_af_completion);

	hsc_sysfs_create_group(di->dev);
	g_hsc_di = di;
	direct_charge_set_di(di);

	di->fault_nb.notifier_call = direct_charge_fault_notifier_call;
	ret = atomic_notifier_chain_register(&hsc_fault_notifier_list,
		&di->fault_nb);
	if (ret < 0) {
		hwlog_err("register notifier failed\n");
		goto fail_create_link;
	}

	hsc_get_devices_info(di);
	hsc_power_if_ops_register(di);
	hsc_dbg_register(di);
	hwlog_info("%s %d\n", __func__, __LINE__);

	return 0;

fail_create_link:
	hsc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_hsc_di = NULL;

	return ret;
}

static int hsc_remove(struct platform_device *pdev)
{
	struct direct_charge_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	hsc_sysfs_remove_group(di->dev);
	wakeup_source_trash(&di->charging_lock);
	devm_kfree(&pdev->dev, di);
	g_hsc_di = NULL;

	return 0;
}

static const struct of_device_id hsc_match_table[] = {
	{
		.compatible = "direct_charger_hsc",
		.data = NULL,
	},
	{},
};

static struct platform_driver hsc_driver = {
	.probe = hsc_probe,
	.remove = hsc_remove,
	.driver = {
		.name = "direct_charger_hsc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hsc_match_table),
	},
};

static int __init hsc_init(void)
{
	return platform_driver_register(&hsc_driver);
}

static void __exit hsc_exit(void)
{
	platform_driver_unregister(&hsc_driver);
}

late_initcall(hsc_init);
module_exit(hsc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("direct charger with switch cap module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
