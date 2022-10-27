/*
 * adsp_dts_interface.c
 *
 * adsp dts interface
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

#include <log/hw_log.h>
#include <huawei_platform/power/adsp/adsp_dts_interface.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>

#define HWLOG_TAG adsp_dts_interface
HWLOG_REGIST();

int adsp_dts_glink_set_dc_para(void *para, size_t size)
{
	hwlog_info("glink set dc para, size: %d\n", size);

	if (hihonor_oem_glink_oem_update_config(DC_SC_CONFIG_ID, para, size)) {
		hwlog_err("glink set dc para fail\n");
		//return -1;
	}

	return 0;
}

int adsp_dts_glink_set_charger_para(void *para, size_t size)
{
	
	hwlog_info("glink set charger para, size: %d\n", size);

	if (hihonor_oem_glink_oem_update_config(BUCK_CHARGER_ID, para, size)) {
		hwlog_err("glink set charger para fail\n");
		//return -1;
	}

	return 0;
}

