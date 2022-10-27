/*
 * huawei_charger_common.c
 *
 * common interface for huawei charger driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include <log/hw_log.h>
#include <linux/power/huawei_charger.h>
#include <huawei_platform/power/huawei_charger_common.h>
#include <huawei_platform/power/hihonor_charger_glink.h>

#define HWLOG_TAG huawei_charger_common
HWLOG_REGIST();

int charge_enable_powerpath(bool path_enable)
{
	return 0;
}

int charger_set_input_current(u32 uA)
{
	hihonor_charger_glink_set_input_current(uA / 1000);
	return 0;
}
