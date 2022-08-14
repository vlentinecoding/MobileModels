/*
 * hihonor_usb_glink.c
 *
 * hihonor usb glink driver
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
#include <chipset_common/hwpower/power_event_ne.h>
#include <huawei_platform/usb/switch/usbswitch_common.h>
#include <huawei_platform/usb/hw_pd_dev.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/huawei_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>

#define HWLOG_TAG hihonor_usb_glink
HWLOG_REGIST();
#define HIHONOR_USB_GLINK_VBUS_ONLY_TIME 2000
#define HIHONOR_USB_GLINK_CHECK_VBUS_TIME 1000
#define HIHONOR_USB_GLINK_CHECK_VBUS_RETRY 20
#define HIHONOR_USB_GLINK_VBUS_MV 4500

/* check wire plugin out when wireless reverse charge */
#define HIHONOR_USB_GLINK_CHECK_PULLOUT_TIME 3000
#define HIHONOR_USB_GLINK_UNATTACKED_SNK     1
#define HIHONOR_USB_GLINK_UNATTACKED_SRC     4


struct hihonor_usb_glink_dev_info {
	struct device *dev;
	bool cc_insert;
	bool vbus_insert;
	struct delayed_work vbus_only_work;
	struct delayed_work check_vbus_work;
	struct delayed_work check_cc_wlcr_work;
};

static struct hihonor_usb_glink_dev_info *dev_info;

int hihonor_usb_glink_get_cable_type()
{
	int type = -1;

	if (hihonor_oem_glink_oem_get_prop(CHARGER_OEM_CABLE_TYPE, (void *)&type, sizeof(type))) {
		hwlog_err("glink get prop CHARGER_OEM_CABLE_TYPE fail\n");
		return -1;
	}

	hwlog_info("glink get prop CHARGER_OEM_CABLE_TYPE %s\n", type ? "non std" : "std");
	return type;
}

int hihonor_usb_glink_check_cc_vbus_short()
{
	int type = 0;

	hwlog_info("glink check cc vbus short\n");

	if (hihonor_oem_glink_oem_get_prop(CHARGER_OEM_CHECK_CC_VBUS_SHORT, (void *)&type, sizeof(type))) {
		hwlog_err("glink get prop CHARGER_OEM_CHECK_CC_VBUS_SHORT fail\n");
		return 0;
	}

	hwlog_info("glink get prop CHARGER_OEM_CHECK_CC_VBUS_SHORT %s\n", type ? "true" : "false");
	return type;
}

void hihonor_usb_glink_set_cc_insert(bool insert)
{
	if (!dev_info)
		return;
	dev_info->cc_insert = insert;
}

int hihonor_usb_glink_get_typec_sm_status()
{
	int typec_sm_status = -1;

	if (hihonor_oem_glink_oem_get_prop(CHARGER_OEM_TYPEC_SM_STATUS, (void *)&typec_sm_status, sizeof(typec_sm_status))) {
		hwlog_err("glink get prop CHARGER_OEM_TYPEC_SM_STATUS fail\n");
		return -1;
	}

	hwlog_info("glink get prop CHARGER_OEM_TYPEC_SM_STATUS %d\n", typec_sm_status);
	return typec_sm_status;
}

static void hihonor_usb_glink_vbus_only_work(struct work_struct *work)
{
	struct hihonor_usb_glink_dev_info *info = container_of(work, struct hihonor_usb_glink_dev_info, vbus_only_work.work);

	if (!info)
		return;
	if (info->cc_insert)
		return;

	if (!wireless_charge_is_pwr_good() || wireless_is_in_tx_mode()) {
		if (info->vbus_insert) {
			hwlog_info("%s: OEM_NOTIFY_VBUS_ON\n", __func__);
			power_event_notify(POWER_NT_CONNECT, POWER_NE_USB_CONNECT, NULL);
			power_event_notify(POWER_NT_CHARGING, POWER_NE_START_CHARGING, NULL);
			wireless_charge_wired_vbus_connect_handler();
			usbswitch_common_chg_type_det(true);
			if (wireless_is_in_tx_mode())
				schedule_delayed_work(&info->check_cc_wlcr_work, msecs_to_jiffies(HIHONOR_USB_GLINK_CHECK_PULLOUT_TIME));
		} else {
			hwlog_info("%s: OEM_NOTIFY_VBUS_OFF\n", __func__);
			power_event_notify(POWER_NT_CONNECT, POWER_NE_USB_DISCONNECT, NULL);
			power_event_notify(POWER_NT_CHARGING, POWER_NE_STOP_CHARGING, NULL);
			wireless_charge_wired_vbus_disconnect_handler();
			usbswitch_common_chg_type_det(false);
		}
	} else {
		hwlog_info("%s: wireless_charge_is_pwr_good\n", __func__);
	}
}

static void hihonor_usb_glink_check_vbus_work(struct work_struct *work)
{
	struct hihonor_usb_glink_dev_info *info = container_of(work, struct hihonor_usb_glink_dev_info, check_vbus_work.work);
	batt_mngr_get_buck_info buck_info = {0};
	int ret;
	static int retry = 0;

	if (retry >= HIHONOR_USB_GLINK_CHECK_VBUS_RETRY)
		return;

	if (!info)
		return;

	if (info->vbus_insert) {
		hwlog_info("%s: vbus already insert\n", __func__);
		return;
	}

	ret = hihonor_oem_glink_oem_get_prop(CHARGER_OEM_BUCK_INFO, &buck_info, sizeof(buck_info));
	if ((ret != 0) || (buck_info.buck_vbus < HIHONOR_USB_GLINK_VBUS_MV)) {
		retry++;
		schedule_delayed_work(&info->check_vbus_work, msecs_to_jiffies(HIHONOR_USB_GLINK_CHECK_VBUS_TIME));
		return;
	}

	schedule_delayed_work(&info->vbus_only_work, msecs_to_jiffies(HIHONOR_USB_GLINK_VBUS_ONLY_TIME));
}

static void hihonor_usb_glink_check_cc_wlcr_work(struct work_struct *work)
{
	int typec_sm_status = hihonor_usb_glink_get_typec_sm_status();
	hwlog_info("%s: typec_sm_status = 0x%x\n", __func__, typec_sm_status);
	if ((typec_sm_status == HIHONOR_USB_GLINK_UNATTACKED_SNK) || (typec_sm_status == HIHONOR_USB_GLINK_UNATTACKED_SRC)) {
		power_event_notify(POWER_NT_CONNECT, POWER_NE_USB_DISCONNECT, NULL);
		power_event_notify(POWER_NT_CHARGING, POWER_NE_STOP_CHARGING, NULL);
		wireless_charge_wired_vbus_disconnect_handler();
		usbswitch_common_chg_type_det(false);
	}
}

static void hihonor_usb_glink_notification(void *dev_data, u32 notification)
{
	struct hihonor_usb_glink_dev_info *info = (struct hihonor_usb_glink_dev_info *)dev_data;

	if (!info)
		return;

	if (notification == OEM_NOTIFY_VBUS_ON) {
		info->vbus_insert = true;
		schedule_delayed_work(&info->vbus_only_work, msecs_to_jiffies(HIHONOR_USB_GLINK_VBUS_ONLY_TIME));
	} else if (notification == OEM_NOTIFY_VBUS_OFF) {
		info->vbus_insert = false;
		if (delayed_work_pending(&info->vbus_only_work))
			cancel_delayed_work_sync(&info->vbus_only_work);
		schedule_delayed_work(&info->vbus_only_work, msecs_to_jiffies(0));
	} else if (notification == OEM_NOTIFY_QUICK_ICON) {
		charge_send_icon_uevent(ICON_TYPE_QUICK);
	}

	return;
}

static struct hihonor_glink_ops usb_glink_ops = {
	.notify_event = hihonor_usb_glink_notification,
};

static int hihonor_usb_glink_probe(struct platform_device *pdev)
{
	struct hihonor_usb_glink_dev_info *info = NULL;
	struct device *dev = &pdev->dev;

	hwlog_info("%s enter\n", __func__);

	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	usb_glink_ops.dev_data = info;
	(void)hihonor_oem_glink_ops_register(&usb_glink_ops);

	info->dev = dev;
	platform_set_drvdata(pdev, info);
	dev_info = info;

	INIT_DELAYED_WORK(&info->vbus_only_work, hihonor_usb_glink_vbus_only_work);
	INIT_DELAYED_WORK(&info->check_vbus_work, hihonor_usb_glink_check_vbus_work);
	INIT_DELAYED_WORK(&info->check_cc_wlcr_work, hihonor_usb_glink_check_cc_wlcr_work);
	schedule_delayed_work(&info->check_vbus_work, msecs_to_jiffies(HIHONOR_USB_GLINK_CHECK_VBUS_TIME));
	return 0;
}

static int hihonor_usb_glink_remove(struct platform_device *pdev)
{
	struct hihonor_usb_glink_dev_info *info = platform_get_drvdata(pdev);

	if (!info)
		return -EINVAL;

	cancel_delayed_work_sync(&info->vbus_only_work);
	cancel_delayed_work_sync(&info->check_vbus_work);
	cancel_delayed_work_sync(&info->check_cc_wlcr_work);
	(void)hihonor_oem_glink_ops_unregister(&usb_glink_ops);
	kfree(info);
	return 0;
}

static const struct of_device_id hihonor_usb_glink_match_table[] = {
	{ .compatible = "hihonor-usb-glink" },
	{},
};

static struct platform_driver hihonor_usb_glink_driver = {
	.driver = {
		.name = "hihonor-usb-glink",
		.of_match_table = hihonor_usb_glink_match_table,
	},
	.probe = hihonor_usb_glink_probe,
	.remove = hihonor_usb_glink_remove,
};

module_platform_driver(hihonor_usb_glink_driver);

MODULE_DESCRIPTION("hihonor usb Glink driver");
MODULE_LICENSE("GPL v2");

