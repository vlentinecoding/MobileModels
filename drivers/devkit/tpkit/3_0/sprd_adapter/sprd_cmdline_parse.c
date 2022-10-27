/*
 * Honor Touchscreen Driver
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
#include <linux/kernel.h>
#include <linux/string.h>
#include <honor_platform/log/hw_log.h>

#define HWLOG_TAG TS_KIT
HWLOG_REGIST();

#define CMDLINE_KEY_SWTYPE "androidboot.swtype"
#define SWTYPE_FACTORY "factory"
#define CMDLINE_KEY_RECOVERY "enter_recovery"
#define CMDLINE_KEY_BOOTMODE "androidboot.mode"
#define BOOTMODE_CHARGER "charger"

static int g_factory_flag;
static unsigned int g_recovery_flag;
static unsigned int g_pd_charge_flag;

static int __init early_parse_swtype_cmdline(char *swtype)
{
	if (swtype == NULL)
		return 0;
	if (strncmp(swtype, SWTYPE_FACTORY, strlen(SWTYPE_FACTORY)) == 0)
		g_factory_flag = 1;
	hwlog_info("swtype: %s, g_factory_flag: %d\n", swtype, g_factory_flag);
	return 0;
}

early_param(CMDLINE_KEY_SWTYPE, early_parse_swtype_cmdline);

int sprd_is_factory(void)
{
	return g_factory_flag;
}

static int __init early_parse_recovery_cmdline(char *recovery)
{
	if (recovery == NULL)
		return 0;
	if (strncmp(recovery, "1", 1) == 0)
		g_recovery_flag = 1;
	hwlog_info("%s=%u\n", CMDLINE_KEY_RECOVERY, g_recovery_flag);
	return 0;
}

early_param(CMDLINE_KEY_RECOVERY, early_parse_recovery_cmdline);

unsigned int sprd_get_recovery_flag(void)
{
	return g_recovery_flag;
}

static int __init early_parse_bootmode_cmdline(char *bootmode)
{
	if (bootmode == NULL)
		return 0;
	if (strncmp(bootmode, BOOTMODE_CHARGER, strlen(BOOTMODE_CHARGER)) == 0)
		g_pd_charge_flag = 1;
	hwlog_info("mode=%s, %u\n", bootmode, g_pd_charge_flag);
	return 0;
}

unsigned int sprd_get_pd_charge_flag(void)
{
	return g_pd_charge_flag;
}

early_param(CMDLINE_KEY_BOOTMODE, early_parse_bootmode_cmdline);

