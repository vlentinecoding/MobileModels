/*
 * charge_dmd_monitor.c
 *
 * charge dmd monitor interface
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

#include <linux/string.h>
#include <linux/power_supply.h>
#include <linux/power/huawei_battery.h>
#include <linux/power/huawei_charger.h>
#include <log/hw_log.h>
#include <huawei_platform/power/huawei_charger_adaptor.h>
#include <huawei_platform/power/charge_dmd_monitor.h>
#include <huawei_platform/power/common_module/power_platform.h>
#include <chipset_common/hwpower/coul_interface.h>

#define HWLOG_TAG charge_dmd_monitor
HWLOG_REGIST();

static bool dsm_batt_full_early_report(char *buf);
static bool check_batt_bad_curr_sensor(char *buf);

static void batt_info_dump(char *ext_buf, unsigned int buf_len)
{
	char buf[DSM_BAT_INFO_BUF_LEN] = {0};
	int vbus = power_platform_get_vbus_voltage();
	int batt_vol = power_platform_get_battery_voltage();
	int cap = power_platform_get_battery_ui_capacity();
	bool batt_present = power_platform_is_battery_exit();
	int batt_temp = power_platform_get_battery_temperature();
	int charger_type = power_platform_get_charger_type();

	if (!ext_buf)
		return;

	/* WARNNING: if extend, do not exceed max buf length */
	snprintf(buf, DSM_BAT_INFO_BUF_LEN - 1,
		"VBUS:%d,VBAT:%d,cap:%d,exist:%d,batt_temp:%d,chg_type:%d\n",
		vbus, batt_vol, cap, batt_present, batt_temp, charger_type);
	if (strlen(buf) > buf_len) {
		hwlog_err("%s buf len exceed\n", __func__);
		strncat(ext_buf, buf, buf_len);
		return;
	}
	strncat(ext_buf, buf, strlen(buf));
}

static bool check_batt_not_exist(char *buf)
{
	bool is_bat_exist = power_platform_is_battery_exit();

	if (!is_bat_exist) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt is not exist\n");
		return true;
	}

	return false;
}

static bool check_batt_temp(bool check_high)
{
	int batt_temp;
	bool check_result = false;

	batt_temp = power_platform_get_battery_temperature();
	if (check_high)
		check_result = (batt_temp >= BAT_TEMP_HIGH_LIMINT);
	else
		check_result = (batt_temp < BATTERY_TEMP_MIN_LIMINT);

	return check_result;
}

static bool check_batt_low_temp(char *buf)
{
	if (check_batt_temp(false)) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt temp overlow\n");
		return true;
	}
	return false;
}

static bool check_batt_high_temp(char *buf)
{
	if (check_batt_temp(true)) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt temp overhigh\n");
		return true;
	}
	return false;
}

static bool check_batt_volt(bool check_high)
{
	int vbatt;
	bool check_result = false;
	int check_cnt;

	vbatt = power_platform_get_battery_voltage();
	if (check_high) {
		if (vbatt > BAT_VOLT_MAX_LIMINT) {
			for (check_cnt = 0; check_cnt < (MAX_CONFIRM_CNT - 1); check_cnt++) {
				/* wait 100ms get vbat agsin */
				msleep(100);
				vbatt += battery_get_bat_voltage();
			}
			vbatt /= MAX_CONFIRM_CNT;
			 /* get averaged value */
		}
		check_result = (vbatt > BAT_VOLT_MAX_LIMINT);
	} else {
		check_result = (vbatt < BAT_VOLT_MIN_LIMINT);
	}

	if (check_result)
		hwlog_info("%s:vbat=%d\n", __func__, vbatt);

	return check_result;
}

static bool check_batt_volt_overhigh(char *buf)
{
	if (check_batt_volt(true)) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt volt overhigh\n");
		return true;
	}
	return false;
}

static bool check_batt_volt_overlow(char *buf)
{
	if (check_batt_volt(false)) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt volt overlow\n");
		return true;
	}
	return false;
}

static bool check_batt_not_terminate(char *buf)
{
	int vbat;
	enum charger_type chg_type;

	vbat = power_platform_get_battery_voltage();
	chg_type = power_platform_get_charger_type();
	if ((vbat > BAT_VOLT_MAX_LIMINT) && (chg_type != POWER_SUPPLY_TYPE_UNKNOWN)) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt not terminate\n");
		return true;
	}

	return false;
}

static bool check_curr_overhigh(bool check_chg, int *cnt)
{
	bool check_flag = false;
	int bat_current;
	enum charger_type chg_type;

	chg_type = power_platform_get_charger_type();
	if (check_chg)
		check_flag = (chg_type != POWER_SUPPLY_TYPE_UNKNOWN);
	else
		check_flag = (chg_type == POWER_SUPPLY_TYPE_UNKNOWN);

	bat_current = power_platform_get_battery_current();
	bat_current /= BATTERY_CURRENT_DIVI;
	if (check_flag && (abs(bat_current) > BAT_CHR_CURRENT)) {
		(*cnt)++;
		if (*cnt >= INCHG_OCP_COUNT) {
			*cnt = 0;
			return true;
		}
	} else {
		*cnt = 0;
	}

	return false;
}

static bool check_charge_curr_overhigh(char *buf)
{
	static int curr_over_cnt;

	if (check_curr_overhigh(true, &curr_over_cnt)) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt cur is overhigh\n");
		return true;
	}
	return false;
}

static bool check_batt_temp_out_range(char *buf)
{
	int batt_temp = power_platform_get_battery_temperature();
	if ((batt_temp > BAT_TEMP_OUT_RANGE_HIGH) ||
		(batt_temp < BAT_TEMP_OUT_RANGE_LOW)) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt temp is out of range\n");
		return true;
	}
	return false;
}

static bool check_batt_high_temp_warm(char *buf)
{
	int batt_temp = power_platform_get_battery_temperature();

	if (batt_temp >= BAT_TEMP_WARM_LIMINT) {
		snprintf(buf, DMS_REPORT_BUF_LEN, "batt temp is high\n");
		return true;
	}
	return false;
}

static struct dsm_info g_batt_dsm_array[] = {
	{
		ERROR_BATT_NOT_EXIST,// 920001003
		true,
		true,
		batt_info_dump,
		check_batt_not_exist
	},
	{
		ERROR_BATT_TEMP_LOW,// 920001004
		true,
		true,
		batt_info_dump,
		check_batt_low_temp
	},
	{
		ERROR_BATT_VOLT_HIGH,// 920001005
		true,
		true,
		batt_info_dump,
		check_batt_volt_overhigh
	},
	{
		ERROR_BATT_VOLT_LOW,// 920001006
		true,
		true,
		batt_info_dump,
		check_batt_volt_overlow
	},
	{
		ERROR_BATT_NOT_TERMINATE,// 920001008
		true,
		true,
		batt_info_dump,
		check_batt_not_terminate
	},
	{
		ERROR_CHARGE_CURR_OVERHIGH,// 920001012
		true,
		true,
		batt_info_dump,
		check_charge_curr_overhigh
	},
	{
		ERROR_CHARGE_TEMP_FAULT,// 920001015
		true,
		true,
		batt_info_dump,
		check_batt_temp_out_range
	},
	{
		ERROR_CHARGE_BATT_TEMP_SHUTDOWN,// 920001016
		true,
		true,
		batt_info_dump,
		check_batt_high_temp
	},
	{
		ERROR_CHARGE_VBAT_OVP,// 920001036
		true,
		true,
		batt_info_dump,
		check_batt_volt_overhigh
	},
	{
		ERROR_CHARGE_TEMP_WARM,// 920001070
		true,
		true,
		batt_info_dump,
		check_batt_high_temp_warm
	},
};

static struct dsm_info g_normal_batt_dsm_array[] = {
	{
		ERROR_BATT_TERMINATE_TOO_EARLY,// 920001007
		true,
		true,
		batt_info_dump,
		dsm_batt_full_early_report
	},
	{
		ERROR_BATT_BAD_CURR_SENSOR,// 920001009
		true,
		true,
		batt_info_dump,
		check_batt_bad_curr_sensor
	},
};

static bool dsm_batt_full_early_report(char *buf)
{
	int bat_soc = power_platform_get_battery_capacity();
	bool chrg_done;

	if (!g_normal_batt_dsm_array[BATT_TERMINATE_TOO_EARLY].notify_enable ||
		(bat_soc > BAT_CHG_FULL_SOC_LIMINT))
		return  false;

	check_buck_is_charge_done(&chrg_done);
	hwlog_info("%s,chrg_done:%d\n", __func__, chrg_done);
	if (!chrg_done)
		return false;

	snprintf(buf, DMS_REPORT_BUF_LEN,
		"Battery cutoff early. soc=%d\n", bat_soc);

	return true;
}

static bool check_batt_bad_curr_sensor(char *buf)
{
	int charger_type = power_platform_get_charger_type();
	int ibat = power_platform_get_battery_current();
	static int times = 0;

	if (!g_normal_batt_dsm_array[BATT_BAD_CURR_SENSOR].notify_enable)
		return false;

	if ((charger_type == CHARGER_REMOVED) && (ibat > CURRENT_OFFSET))
		times ++;
	else
		times = 0;

	if (times == DSM_CHECK_TIMES) {
		times = 0;
		snprintf(buf, DMS_REPORT_BUF_LEN,
		"when charger is removed, current = %dmA\n", ibat);
		return true;
	}

	return false;
}

/* excute when charger plug out */
void reset_batt_dsm_notify_en(void)
{
	int i;
	unsigned int array_len;

	array_len = ARRAY_SIZE(g_batt_dsm_array);
	for (i = 0; i < array_len; ++i) {
		if (g_batt_dsm_array[i].notify_always == true)
			g_batt_dsm_array[i].notify_enable = true;
	}
	array_len = ARRAY_SIZE(g_normal_batt_dsm_array);
	for (i = 0; i < array_len; ++i) {
		if (g_normal_batt_dsm_array[i].notify_always == true)
			g_normal_batt_dsm_array[i].notify_enable = true;
	}
	return;
}

static void bat_check_error_func(struct dsm_info *batt_dsm_array,
	unsigned int array_len, bool dmd_report_once)
{
	int i;
	int ret;
	char buf[DMS_REPORT_BUF_LEN + 1] = {0};

	if (!coul_interface_is_coul_ready(COUL_TYPE_MAIN))
		return;

	for (i = 0; i < array_len; ++i) {
		/* every cycle reset the buffer */
		memset(buf, 0, sizeof(buf));
		if ((batt_dsm_array[i].notify_enable == false) ||
			(batt_dsm_array[i].check_error(buf) == false))
			continue;
		if (batt_dsm_array[i].dump)
			batt_dsm_array[i].dump(buf, DMS_REPORT_BUF_LEN);
		/* print the final buffer */
		ret = power_dsm_dmd_report(POWER_DSM_BATTERY,
			batt_dsm_array[i].error_no, buf);
		if (!ret) {
			hwlog_err("power_dsm_dmd_report error\n");
			continue;
		}
		/* dmd report one time in normal check procession */
		if (dmd_report_once)
			batt_dsm_array[i].notify_enable = false;
	}
	return;
}

void charging_check_error_info(void)
{
	unsigned int array_len = ARRAY_SIZE(g_batt_dsm_array);

	bat_check_error_func(g_batt_dsm_array, array_len, false);
}

void normal_check_error_info(void)
{
	unsigned int array_len = ARRAY_SIZE(g_normal_batt_dsm_array);

	bat_check_error_func(g_normal_batt_dsm_array, array_len, true);
}
