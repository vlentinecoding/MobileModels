/*
 * hihonor_charger_glink.c
 *
 * hihonor charger glink driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <linux/slab.h>
#include <linux/kernel.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/power_printk.h>

#define HWLOG_TAG hihonor_charger_glink
HWLOG_REGIST();

int hihonor_charger_glink_enable_hiz(int enable)
{
	hwlog_info("glink set hiz enable %d\n", enable);

	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_HIZ_EN, &enable, sizeof(enable))) {
		hwlog_err("glink enable hiz fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_enable_charge(int enable)
{
	hwlog_info("glink set charge enable %d\n", enable);

	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_CHARGE_EN, &enable, sizeof(enable))) {
		hwlog_err("glink enable charge fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_set_input_current(int input_current)
{
	hwlog_info("glink set input_current %d\n", input_current);

	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_INPUT_CURRENT, &input_current, sizeof(input_current))) {
		hwlog_err("glink input_current fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_set_sdp_input_current(int input_current)
{
	hwlog_info("glink set sdp_input_current %d\n", input_current);

	input_current *= CURRENT_MATOUA;
	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_SDP_INPUT_CURRENT, &input_current, sizeof(input_current))) {
		hwlog_err("glink enable sdp_input_current fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_set_charge_current(int charge_current)
{
	hwlog_info("glink set charge_current %d\n", charge_current);

	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_CHARGE_CURRENT, &charge_current, sizeof(charge_current))) {
		hwlog_err("glink enable charge_input_current fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_set_charger_type(int charger_type)
{
	hwlog_info("glink set charger type %d\n", charger_type);

	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_CHARGER_TYPE, &charger_type, sizeof(charger_type))) {
		hwlog_err("glink set charger type fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_enable_wlc_src(int wlc_src)
{
	hwlog_info("glink enable wireless power supply source %d\n", wlc_src);

	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_WLC_SRC, &wlc_src, sizeof(wlc_src))) {
		hwlog_err("glink enable wireless power supply source fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_enable_identify_insert(int enable)
{
	hwlog_info("glink enable identify insert %d\n", enable);
	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_INSERT_IDENTIFY, &enable, sizeof(enable))) {
		hwlog_err("glink enable identify insert fail\n");
		return -1;
	}

	return 0;
}

int hihonor_charger_glink_enable_boost5v(int enable)
{
	hwlog_info("glink enable boost5v %d\n", enable);
	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_BOOST5V, &enable, sizeof(enable))) {
		hwlog_err("glink enable boost5v fail\n");
		return -1;
	}
	return 0;
}

int hihonor_charger_glink_enable_usbsuspend_collapse(int enable)
{
	hwlog_info("glink enable usbsuspend collapse %d\n", enable);
	if (hihonor_oem_glink_oem_set_prop(CHARGER_OEM_SUSPEND_COLLAPSE_EN, &enable, sizeof(enable))) {
		hwlog_err("glink enable usbsuspend collapse fail\n");
		return -1;
	}
	return 0;
}
