/*
 * huawei_charger_adaaptor.c
 *
 * huawei charger adaaptor for power module
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

#include <huawei_platform/power/huawei_charger_adaptor.h>
#include <linux/kernel.h>
#include <linux/power_supply.h>
#include <linux/power/huawei_charger.h>
#include <linux/power/huawei_battery.h>
#include <linux/power/charger-manager.h>
#include <linux/notifier.h>
#include <log/hw_log.h>
#include <chipset_common/hwpower/power_ui_ne.h>
#include <chipset_common/hwpower/power_event_ne.h>
#include <chipset_common/hwpower/power_supply_interface.h>
#include <huawei_platform/power/hihonor_charger_glink.h>
#ifndef CONFIG_PMIC_AP_CHARGER
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#endif

#define HWLOG_TAG honor_charger_adaptor
HWLOG_REGIST();

#define DEFAULT_CAP              50
#define DEFAULT_VOLTAGE          4000
#define MV_TO_UV                 1000


int huawei_get_vbat_max(void)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->chg_psy, POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN, &val);
	if (rc < 0) {
		hwlog_err("%s get voltage max fail\n", __func__);
		return DEFAULT_FASTCHG_MAX_VOLTAGE;
	}
	return val.intval / MV_TO_UV;
}

int huawei_battery_capacity(void)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->batt_psy, POWER_SUPPLY_PROP_CAPACITY, &val);
	if (rc < 0) {
		hwlog_err("%s get voltage max fail\n", __func__);
		return DEFAULT_CAP;
	}
	return val.intval;
}

int get_reset_adapter(void)
{
	return 0;
}

void charge_send_uevent(int input_events)
{
}

void charge_request_charge_monitor(void)
{
}

int converse_usb_type(int val)
{
	int type;

	switch (val) {
	case POWER_SUPPLY_TYPE_UNKNOWN:
		type = CHARGER_REMOVED;
		break;
	case POWER_SUPPLY_TYPE_USB_CDP:
		type = CHARGER_TYPE_BC_USB;
		break;
	case POWER_SUPPLY_TYPE_USB:
		type = CHARGER_TYPE_USB;
		break;
	case POWER_SUPPLY_TYPE_USB_DCP:
		type = CHARGER_TYPE_STANDARD;
		break;
	case POWER_SUPPLY_TYPE_USB_FLOAT:
		type = CHARGER_TYPE_NON_STANDARD;
		break;
	case POWER_SUPPLY_TYPE_WIRELESS:
		type = CHARGER_TYPE_WIRELESS;
		break;
	case POWER_SUPPLY_TYPE_FCP:
		type = CHARGER_TYPE_FCP;
		break;
	default:
		type = CHARGER_REMOVED;
		break;
	}
     return type;
}

enum charger_type mt_get_charger_type(void)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->usb_psy, POWER_SUPPLY_PROP_REAL_TYPE, &val);
	if (rc < 0) {
		hwlog_err("%s get chg type fail\n", __func__);
		return 0;
	}
	return val.intval;
}

char *huawei_get_battery_type(void)
{
	union power_supply_propval val;

	if (power_supply_get_property_value("battery",
		POWER_SUPPLY_PROP_BRAND, &val))
		return "default";
	return (char *)val.strval; 
}

void charge_send_icon_uevent(int icon_type)
{
	power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
	power_supply_sync_changed("battery");
	power_supply_sync_changed("Battery");
}

int charger_manager_notifier(struct charger_manager *info, int event)
{
	return 0;
}

void wired_connect_send_icon_uevent(int icon_type)
{
	hwlog_info("%s enter,icon_type=%d\n", __func__, icon_type);

	charge_send_icon_uevent(icon_type);
	power_supply_sync_changed("battery");
}

void wired_disconnect_send_icon_uevent(void)
{
	charge_send_icon_uevent(ICON_TYPE_INVALID);
}

int get_charger_vbus_vol(void)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->usb_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (rc < 0) {
		hwlog_err("%s get vbus vol fail\n", __func__);
		return 0;
	}
	return val.intval;
}

signed int battery_get_bat_voltage(void)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->batt_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (rc < 0) {
		hwlog_err("%s get vbus vol fail\n", __func__);
		return DEFAULT_VOLTAGE;
	}
	return val.intval;
}

int charge_enable_force_sleep(bool enable)
{
	return 0;
}

int charge_enable_hz(bool hz_enable)
{
#ifdef CONFIG_PMIC_AP_CHARGER
	union power_supply_propval val = {0};
#else
	int enable;
#endif
	int rc;
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

#ifdef CONFIG_PMIC_AP_CHARGER
	val.intval = hz_enable;
	rc = set_prop_to_psy(di->batt_psy, POWER_SUPPLY_PROP_HIZ_MODE, &val);
#else
	enable = hz_enable ? 1 : 0;
	rc = hihonor_charger_glink_enable_hiz(enable);
#endif
	if (rc < 0)
		hwlog_err("%s enable hz fail\n", __func__);
	return rc;
}

int get_charge_done_type(void)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->batt_psy, POWER_SUPPLY_PROP_CHARGE_DONE, &val);
	if (rc < 0) {
		hwlog_err("%s charge done fail\n", __func__);
		return 0;
	}
	return val.intval;
}

static int first_check;
int get_first_insert(void)
{
	return first_check;
}

void set_first_insert(int flag)
{
	pr_info("set insert flag %d\n", flag);
	first_check = flag;
}

int charger_dev_get_vbus(u32 *vbus)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->usb_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
	if (rc < 0) {
		hwlog_err("%s get vbus vol fail\n", __func__);
		return -1;
	}
	*vbus = val.intval / MV_TO_UV;
	return 0;
}

int battery_get_vbus(void)
{
	return 0;
}

int charger_dev_get_chg_state(u32 *pg_state)
{
	return 0;
}

int charger_dev_get_ibus(u32 *ibus)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	rc = get_prop_from_psy(di->usb_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_NOW, &val);
	if (rc < 0) {
		hwlog_err("%s get vbus vol fail\n", __func__);
		return -1;
	}
	*ibus = val.intval;
	return 0;
}

signed int battery_get_bat_current(void)
{
	int rc;
	union power_supply_propval val = {0, };
	struct charge_device_info *di = NULL;

	di = get_charger_device_info();
	if (!di) {
		hwlog_err("%s g_di is null\n", __func__);
		return 0;
	}

	if (!di->batt_psy)
		di->batt_psy = power_supply_get_by_name("battery");

	rc = get_prop_from_psy(di->batt_psy, POWER_SUPPLY_PROP_CURRENT_NOW, &val);
	if (rc < 0) {
		hwlog_err("%s get vbus vol fail\n", __func__);
		return 0;
	}
	return val.intval;
}

u32 get_charger_ibus_curr(void)
{
	u32 ibus_curr = 0;
#ifndef CONFIG_PMIC_AP_CHARGER
	batt_mngr_get_buck_info buck_info = {0};
#endif

	(void)charger_dev_get_ibus(&ibus_curr);
	hwlog_info("%s get ibus = %u\n", __func__, ibus_curr);
#ifndef CONFIG_PMIC_AP_CHARGER
	if (ibus_curr == 0 &&
		!hihonor_oem_glink_oem_get_prop(CHARGER_OEM_BUCK_INFO,
			&buck_info, sizeof(buck_info))) {
		hwlog_info("%s get ibus from glink = %u\n", __func__, buck_info.buck_ibus);
		return buck_info.buck_ibus;
	}
#endif
	return ibus_curr / MV_TO_UV;
}

int charger_dev_set_mivr(u32 uV)
{
	return 0;
}

int charger_dev_set_vbus_vset(u32 uv)
{
	return 0;
}

int charge_enable_eoc(bool eoc_enable)
{
	return 0;
}

void reset_cur_delay_eoc_count(void)
{
}

void Charger_Detect_Init(void)
{
}

void Charger_Detect_Release(void)
{
}

static unsigned int g_charge_time;
unsigned int get_charging_time(void)
{
	return g_charge_time;
}

enum charger_type huawei_get_charger_type(void)
{
	return mt_get_charger_type();
}

