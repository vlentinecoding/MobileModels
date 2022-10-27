/*
 * adsp_ui_event.c
 *
 * adsp ui_event driver
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
#include <linux/kthread.h>
#include <log/hw_log.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_ui_ne.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_event_ne.h>
#include <chipset_common/hwpower/power_supply_interface.h>

#define HWLOG_TAG adsp_ui_event
HWLOG_REGIST();

#define ADSP_UI_MAX_POWER_TH   40000

static LIST_HEAD(g_ui_event_listhead);

typedef void (*callback_func_type)(void *dev_data);

struct adsp_ui_event_device {
	struct device *dev;
	struct mutex list_lock;
	wait_queue_head_t wait_que;
	bool event_is_ready;
	unsigned int ui_max_power;
};

struct adsp_ui_event_list_node {
	struct list_head node;
	int event;
	unsigned int data[MAX_OEM_PROPERTY_DATA_SIZE];
};

struct super_icon_type_data
{
    unsigned int cable_type;
    unsigned int max_power;
};

static void adsp_ui_event_add_event(struct adsp_ui_event_device *di, unsigned int event, void *data)
{
	struct adsp_ui_event_list_node *entry = NULL;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return;

	entry->event = event;
	memcpy(entry->data, data, MAX_OEM_PROPERTY_DATA_SIZE);

	mutex_lock(&di->list_lock);
	list_add_tail(&entry->node, &g_ui_event_listhead);
	mutex_unlock(&di->list_lock);

	return;
}

static void wakeup_event_thread(struct adsp_ui_event_device *di)
{
	if (!di)
		return;

	di->event_is_ready = true;
	wake_up(&di->wait_que);
}

static void adsp_ui_event_notify_callback(void *dev_data, u32 notification, void *data)
{
	struct adsp_ui_event_device *di = (struct adsp_ui_event_device *)dev_data;

	if (!di)
		return;

	adsp_ui_event_add_event(di, notification, data);
	if (!di->event_is_ready) {
		wakeup_event_thread(di);
	}
}

static void adsp_ui_event_handle_event(unsigned int evnet, void *data, struct adsp_ui_event_device *di)
{
	int icon_type = ICON_TYPE_INVALID;
	struct power_supply *psy = NULL;
	union power_supply_propval propval;
	struct super_icon_type_data *icon_data = NULL;

	psy = power_supply_get_by_name("usb");

	switch (evnet)
	{
	case OEM_NOTIFY_START_CHARGING:
		power_event_notify(POWER_NT_CONNECT, POWER_NE_USB_CONNECT, NULL);
		power_event_notify(POWER_NT_CHARGING, POWER_NE_START_CHARGING, NULL);
		if (psy) {
			propval.intval = 1;
			power_supply_set_property(psy, POWER_SUPPLY_PROP_ONLINE, &propval);
		}
		break;
	case OEM_NOTIFY_STOP_CHARGING:
		power_event_notify(POWER_NT_CHARGING, POWER_NE_STOP_CHARGING, NULL);
		break;
	case OEM_NOTIFY_DC_PLUG_OUT:
		power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
		power_event_notify(POWER_NT_CHARGING, POWER_NE_STOP_CHARGING, NULL);
		power_event_notify(POWER_NT_CONNECT, POWER_NE_USB_DISCONNECT, NULL);
		di->ui_max_power = 0;
		power_ui_event_notify(POWER_UI_NE_MAX_POWER, &di->ui_max_power);
		if (psy) {
			propval.intval = 0;
			power_supply_set_property(psy, POWER_SUPPLY_PROP_ONLINE, &propval);
		}
		break;
	case OEM_NOTIFY_ICON_TYPE_QUICK:
		icon_type = ICON_TYPE_QUICK;
		power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
		break;
	case OEM_NOTIFY_ICON_TYPE_SUPER:
		if (!data)
			return;
		icon_type = ICON_TYPE_SUPER;
		icon_data = (struct super_icon_type_data *)data;
		di->ui_max_power = icon_data->max_power;
		power_ui_event_notify(POWER_UI_NE_ICON_TYPE, &icon_type);
		power_ui_event_notify(POWER_UI_NE_CABLE_TYPE, &icon_data->cable_type);
		if (di->ui_max_power >= ADSP_UI_MAX_POWER_TH)
			power_ui_event_notify(POWER_UI_NE_MAX_POWER, &di->ui_max_power);
		break;
	case OEM_NOTIFY_DC_SOC_DECIMAL:
		power_event_notify(POWER_NT_SOC_DECIMAL, POWER_NE_SOC_DECIMAL_DC,
			&di->ui_max_power);
		break;
	default:
		return;
	}

	power_supply_sync_changed("battery");
	power_supply_sync_changed("Battery");
}

static int adsp_ui_event_routine_thread(void *arg)
{
	struct adsp_ui_event_device *di = arg;
	struct adsp_ui_event_list_node *entry = NULL;

	if (!di)
		return -1;

	while (1) {
		wait_event(di->wait_que, (di->event_is_ready == true));
		while (!list_empty(&g_ui_event_listhead)) {
			entry = list_entry(g_ui_event_listhead.next, struct adsp_ui_event_list_node, node);
			adsp_ui_event_handle_event(entry->event, entry->data, di);
			mutex_lock(&di->list_lock);
			list_del(&entry->node);
			kfree(entry);
			mutex_unlock(&di->list_lock);
		}
		di->event_is_ready = false;
	}

	return 0;
}

static struct hihonor_glink_ops adsp_ui_event_glink_ops = {
	.notify_event = adsp_ui_event_notify_callback,
};

static int adsp_ui_event_probe(struct platform_device *pdev)
{
	int ret;
	struct adsp_ui_event_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	hwlog_info("%s %d\n", __func__, __LINE__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	mutex_init(&di->list_lock);
	init_waitqueue_head(&di->wait_que);

	platform_set_drvdata(pdev, di);
	adsp_ui_event_glink_ops.dev_data = di;
	ret = hihonor_oem_glink_ops_register(&adsp_ui_event_glink_ops);
	if (ret) {
		hwlog_err("%s fail to register glink ops\n", __func__);
		goto fail_free_mem;
	}

	kthread_run(adsp_ui_event_routine_thread, di, "adsp_ui_event_thread");
	return 0;

fail_free_mem:
 	hwlog_err("adsp_ui_event_probe err\n");
 	devm_kfree(&pdev->dev, di);
 	return ret;
}

static int adsp_ui_event_remove(struct platform_device *pdev)
{
	struct adsp_ui_event_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	devm_kfree(&pdev->dev, di);
	return 0;
}

#ifdef CONFIG_PM
static int adsp_ui_event_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct adsp_ui_event_device *di = platform_get_drvdata(pdev);
	if(!di) {
		hwlog_info("adsp_ui_event_device is null\n");
		return 0;
	}

	return 0;
}

static int adsp_ui_event_resume(struct platform_device *pdev)
{
	struct adsp_ui_event_device *di = platform_get_drvdata(pdev);
	if(!di) {
		hwlog_info("adsp_ui_event_device is null\n");
		return 0;
	}

	wakeup_event_thread(di);
	return 0;
}
#endif

static const struct of_device_id adsp_ui_event_match_table[] = {
	{
		.compatible = "honor,adsp_ui_event",
		.data = NULL,
	},
	{},
};

static struct platform_driver adsp_ui_event_driver = {
	.probe = adsp_ui_event_probe,
	.remove = adsp_ui_event_remove,
#ifdef CONFIG_PM
	.suspend = adsp_ui_event_suspend,
	.resume = adsp_ui_event_resume,
#endif
	.driver = {
		.name = "honor,adsp_ui_event",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_ui_event_match_table),
	},
};

static int __init adsp_ui_event_init(void)
{
	return platform_driver_register(&adsp_ui_event_driver);
}

static void __exit adsp_ui_event_exit(void)
{
	platform_driver_unregister(&adsp_ui_event_driver);
}

device_initcall_sync(adsp_ui_event_init);
module_exit(adsp_ui_event_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adsp ui_event module driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
