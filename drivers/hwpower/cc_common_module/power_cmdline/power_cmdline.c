/*
 * power_cmdline.c
 *
 * cmdline parse interface for power module
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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
#include <chipset_common/hwpower/power_printk.h>

#define HWLOG_TAG power_cmdline
HWLOG_REGIST();

#define BAT_SN_TYPE_MAX_LEN 4
#define BAT_SN_TYPE_LEN 2

static bool g_init_flag;
static bool g_factory_mode_flag;
static bool g_recovery_mode_flag;
static bool g_erecovery_mode_flag;
static bool g_powerdown_charging_flag;
static char g_bat_sn_type[BAT_SN_TYPE_MAX_LEN] = {0};

#ifdef CONFIG_HUAWEI_POWER_DEBUG
bool power_cmdline_is_root_mode(void)
{
	return true;
}
#else
bool power_cmdline_is_root_mode(void)
{
	return false;
}
#endif /* CONFIG_HUAWEI_POWER_DEBUG */

bool power_cmdline_is_factory_mode(void)
{
	if (g_init_flag)
		return g_factory_mode_flag;

	if (strstr(saved_command_line, "androidboot.swtype=factory"))
		g_factory_mode_flag = true;
	else
		g_factory_mode_flag = false;

	return g_factory_mode_flag;
}

bool power_cmdline_is_recovery_mode(void)
{
	if (g_init_flag)
		return g_recovery_mode_flag;

	if (strstr(saved_command_line, "enter_recovery=1"))
		g_recovery_mode_flag = true;
	else
		g_recovery_mode_flag = false;

	return g_recovery_mode_flag;
}

bool power_cmdline_is_erecovery_mode(void)
{
	if (g_init_flag)
		return g_erecovery_mode_flag;

	if (strstr(saved_command_line, "enter_erecovery=1"))
		g_erecovery_mode_flag = true;
	else
		g_erecovery_mode_flag = false;

	return g_erecovery_mode_flag;
}

bool power_cmdline_is_powerdown_charging_mode(void)
{
	if (g_init_flag)
		return g_powerdown_charging_flag;

	if (strstr(saved_command_line, "androidboot.mode=charger"))
		g_powerdown_charging_flag = true;
	else
		g_powerdown_charging_flag = false;

	return g_powerdown_charging_flag;
}

int power_cmdline_get_battery_info(char *name, unsigned int name_size)
{
	char *bat_info = NULL;
	unsigned int len;

	if (!name || name_size < BAT_SN_TYPE_MAX_LEN)
		return 0;

	if (g_init_flag) {
		strlcpy(name, g_bat_sn_type, name_size);
		return strlen(g_bat_sn_type);
	}

	bat_info = strstr(saved_command_line, "battery_info=");
	if (!bat_info)
		return 0;
	bat_info += strlen("battery_info=");
	len = strlen(bat_info);
	if (len < BAT_SN_TYPE_LEN)
		return 0;

	g_bat_sn_type[1] = *bat_info;
	g_bat_sn_type[0] = *(bat_info + 1);
	g_bat_sn_type[2] = '\0';
	strlcpy(name, g_bat_sn_type, name_size);

	return strlen(g_bat_sn_type);
}

static int __init power_cmdline_init(void)
{
	unsigned char type[BAT_SN_TYPE_MAX_LEN] = {0};
	(void)power_cmdline_is_factory_mode();
	(void)power_cmdline_is_recovery_mode();
	(void)power_cmdline_is_erecovery_mode();
	(void)power_cmdline_is_powerdown_charging_mode();
	(void)power_cmdline_get_battery_info(type, BAT_SN_TYPE_MAX_LEN);
	g_init_flag = true;

	return 0;
}

static void __exit power_cmdline_exit(void)
{
}

subsys_initcall(power_cmdline_init);
module_exit(power_cmdline_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("cmdline for power module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
