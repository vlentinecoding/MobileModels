/*
 * wireless_charge_psy.c
 *
 * wireless charge driver, function as power supply
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

#include <linux/platform_device.h>
#include <linux/power_supply.h>
#include <log/hw_log.h>
#include <linux/power/huawei_charger.h>
#include <huawei_platform/power/huawei_charger_common.h>
#include <chipset_common/hwpower/power_ui_ne.h>
#include <chipset_common/hwpower/power_event_ne.h>
//#include <mt-plat/charger_type.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/huawei_charger_adaptor.h>

#define HWLOG_TAG wireless_charge_psy
HWLOG_REGIST();

static int wlc_psy_online_changed(bool online)
{
	static struct power_supply *psy = NULL;
	union power_supply_propval pval;

	if (!psy) {
		psy = power_supply_get_by_name("Wireless");
		if (!psy) {
			hwlog_err("%s: get power supply failed\n", __func__);
			return -EINVAL;
		}
	}

	pval.intval = online;

	return power_supply_set_property(psy, POWER_SUPPLY_PROP_ONLINE, &pval);
}

int wlc_psy_chg_type_changed(bool online)
{
	static struct power_supply *psy = NULL;
	union power_supply_propval pval;
	enum charger_type chg_type;

	if (!psy) {
		psy = power_supply_get_by_name("usb");
		if (!psy) {
			hwlog_err("%s: get power supply failed\n", __func__);
			return -EINVAL;
		}
	}

	chg_type = mt_get_charger_type();
	if (!online && (chg_type != CHARGER_TYPE_WIRELESS)) {
		hwlog_err("%s: charger_type=%d\n", __func__, chg_type);
		return 0;
	}

	if (online) {
		pval.intval = CHARGER_TYPE_WIRELESS;
		(void)charge_enable_powerpath(true);
	} else {
		pval.intval = CHARGER_REMOVED;
		(void)charge_enable_powerpath(false);
	}

	return power_supply_set_property(psy,
		POWER_SUPPLY_PROP_REAL_TYPE, &pval);
}

int wlc_handle_sink_event(bool sink_flag)
{
	int ret = 0;
	if (sink_flag) {
		charge_send_icon_uevent(ICON_TYPE_WIRELESS_NORMAL);
		power_event_notify(POWER_NT_CONNECT, POWER_NE_WIRELESS_CONNECT, NULL);
		power_event_notify(POWER_NT_CHARGING, POWER_NE_START_CHARGING, NULL);
	} else {
		power_event_notify(POWER_NT_CONNECT, POWER_NE_WIRELESS_DISCONNECT, NULL);
		if (wireless_charge_get_wired_channel_state() != WIRED_CHANNEL_ON) {
			charge_send_icon_uevent(ICON_TYPE_INVALID);
			power_event_notify(POWER_NT_CHARGING, POWER_NE_STOP_CHARGING, NULL);
		}
	}

	if (wlc_psy_online_changed(sink_flag) < 0) {
		ret = -EINVAL;
		hwlog_err("%s: report psy online failed\n", __func__);
	}

	if (wlc_psy_chg_type_changed(sink_flag) < 0) {
		ret = -EINVAL;
		hwlog_err("%s: report psy chg_type failed\n", __func__);
	}
	return ret;
}

static int wireless_get_property(struct power_supply *psy,
	enum power_supply_property psp, union power_supply_propval *val)
{
	struct wireless_charge_device_info *di = power_supply_get_drvdata(psy);

	if (!di) {
		pr_notice("%s: no wlc chg data\n", __func__);
		return -EINVAL;
	}

	switch (psp) {
	case POWER_SUPPLY_PROP_ONLINE:
		val->intval = di->online;
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static enum power_supply_property g_wireless_properties[] = {
	POWER_SUPPLY_PROP_ONLINE,
};

static int wireless_set_property(struct power_supply *psy,
	enum power_supply_property prop,
	const union power_supply_propval *pval)
{
	struct wireless_charge_device_info *di = power_supply_get_drvdata(psy);

	pr_info("%s\n", __func__);

	if (!di) {
		pr_notice("%s: no wlc chg data\n", __func__);
		return -EINVAL;
	}

	switch (prop) {
	case POWER_SUPPLY_PROP_ONLINE:
		di->online = pval->intval;
		return 0;
	default:
		return -EINVAL;
	}

	return 0;
}

static int wls_psy_prop_is_writeable(struct power_supply *psy,
	enum power_supply_property prop)
{
	switch (prop) {
		case POWER_SUPPLY_PROP_ONLINE:
			return 1;
		default:
			break;
	}
	return 0;
}

static const struct power_supply_desc g_wireless_desc = {
	.name = "Wireless",
	.type = POWER_SUPPLY_TYPE_WIRELESS,
	.properties = g_wireless_properties,
	.num_properties = ARRAY_SIZE(g_wireless_properties),
	.get_property = wireless_get_property,
	.set_property = wireless_set_property,
	.property_is_writeable = wls_psy_prop_is_writeable,
};

int wlc_power_supply_register(struct platform_device *pdev, struct wireless_charge_device_info *di)
{
	struct power_supply *psy = NULL;
	if(!pdev || !di) {
		hwlog_err("%s:invalid pdev or di\n", __func__);
		return PTR_ERR(psy);
	}
	di->wlc_cfg.drv_data = di;

	psy = power_supply_register(&pdev->dev, &g_wireless_desc, &di->wlc_cfg);
	if (IS_ERR(psy)) {
		hwlog_err("power_supply_register failed\n");
		return PTR_ERR(psy);
	}

	return 0;
}
