/*
 * dc_glink_interface.c
 *
 * direct charger glink interface
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
#include <linux/slab.h>
#include <linux/kernel.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/power_printk.h>
#include <huawei_platform/power/adsp/dc_glink_interface.h>

#define HWLOG_TAG dc_glink_interface
HWLOG_REGIST();

int dc_glink_set_enable_charge(int enable)
{
	hwlog_info("dc glink set dc charger enable %d\n", enable);

	if (hihonor_oem_glink_oem_set_prop(DIRECT_CHARGE_OEM_CHARGE_EN, 
		&enable, sizeof(enable))) {
		hwlog_err("dc glink set dc charger enable fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_mainsc_set_enable_charge(int enable)
{
	hwlog_info("dc glink set dc mainsc charger enable %d\n", enable);

	if (hihonor_oem_glink_oem_set_prop(DIRECT_CHARGE_OEM_MAINSC_CHARGE_EN, 
		&enable, sizeof(enable))) {
		hwlog_err("dc glink set dc mainsc charger enable fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_auxsc_set_enable_charge(int enable)
{
	hwlog_info("dc glink set dc auxsc charger enable %d\n", enable);

	if (hihonor_oem_glink_oem_set_prop(DIRECT_CHARGE_OEM_AUXSC_CHARGE_EN, 
		&enable, sizeof(enable))) {
		hwlog_err("dc glink set dc auxsc charger enable fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_set_iin_thermal(int *iin_thermal_array, int array_size)
{
	hwlog_info("dc glink set iin thermal array\n");

	if (hihonor_oem_glink_oem_set_prop(DIRECT_CHARGE_OEM_IIN_THERMAL, 
		iin_thermal_array, sizeof(int) * array_size)) {
		hwlog_err("dc glink set iin thermal array fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_get_adaptor_detect(int *adaptor_detect)
{
	hwlog_info("dc glink get adaptor_detect\n");

	if (hihonor_oem_glink_oem_get_prop(DIRECT_CHARGE_OEM_ADAPOR_DETECT, 
		adaptor_detect, sizeof(int))) {
		hwlog_err("dc glink get adaptor_detect fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_get_iadaptor(int *iadaptor)
{
	hwlog_info("dc glink get iadaptor\n");

	if (hihonor_oem_glink_oem_get_prop(DIRECT_CHARGE_OEM_IADAPTOR, 
		iadaptor, sizeof(int))) {
		hwlog_err("dc glink get iadaptor fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_get_multi_cur(struct dc_charge_info *info)
{
	hwlog_info("dc glink get multi cur info\n");

	if (hihonor_oem_glink_oem_get_prop(DIRECT_CHARGE_OEM_MULTI_CUR, 
		info, sizeof(struct dc_charge_info))) {
		hwlog_err("dc glink get multi cur info fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_get_vbus(int *vbus)
{
	hwlog_info("dc glink get vbus\n");

	if (hihonor_oem_glink_oem_get_prop(DIRECT_CHARGE_OEM_VBUS, 
		vbus, sizeof(int))) {
		hwlog_err("dc glink get vbus fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_get_cur_mode(int *cur_mode)
{
	hwlog_info("dc glink get cur mode\n");

	if (hihonor_oem_glink_oem_get_prop(DIRECT_CHARGE_OEM_CUR_MODE, 
		cur_mode, sizeof(int))) {
		hwlog_err("dc glink get cur mode fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_set_vterm_dec(unsigned int vterm_dec)
{
	hwlog_info("dc glink set vterm_dec\n");

	if (hihonor_oem_glink_oem_set_prop(DIRECT_CHARGE_OEM_VTERM_DEC, 
		&vterm_dec, sizeof(unsigned int))) {
		hwlog_err("dc glink set vterm_dec fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_set_ichg_ratio(unsigned int ichg_ratio)
{
	hwlog_info("dc glink set ichg_ratio\n");

	if (hihonor_oem_glink_oem_set_prop(DIRECT_CHARGE_OEM_ICHG_RATIO, 
		&ichg_ratio, sizeof(unsigned int))) {
		hwlog_err("dc glink set ichg_ratio fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_get_rt_test_result(int *result)
{
	hwlog_info("dc glink get dc rt_test_result\n");

	if (hihonor_oem_glink_oem_get_prop(DIRECT_CHARGE_OEM_RT_TEST_INFO, 
		result, sizeof(int) * DC_MODE_TOTAL)) {
		hwlog_err("dc glink get dc rt_test_result fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_set_mmi_test_flag(int test_flag)
{
	hwlog_info("dc glink set dc mmi test flag\n");
	if (hihonor_oem_glink_oem_set_prop(DIRECT_CHARGE_OEM_MMI_TEST_FLAG,
		&test_flag, sizeof(int))) {
		hwlog_err("dc glink set dc mmi test flag fail\n");
		return -1;
	}

	return 0;
}

int dc_glink_get_mmi_test_result(int *result)
{
	hwlog_info("dc glink get dc mmi test result\n");
	if (hihonor_oem_glink_oem_get_prop(DIRECT_CHARGE_OEM_MMI_RESULT,
		result, sizeof(int))) {
		hwlog_err("dc glink get dc mmi test result fail\n");
		return -1;
	}

	return 0;
}
