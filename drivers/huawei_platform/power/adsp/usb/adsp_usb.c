/*
 * adsp_usb.c
 *
 * adsp usb driver
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
#include <linux/power_supply.h>
#include <log/hw_log.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_supply_interface.h>
#include <huawei_platform/power/adsp/adsp_dts_interface.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/power_dts.h>

#define HWLOG_TAG adsp_usb
HWLOG_REGIST();

#define DEFAULT_USB_ICL_CUR_UA         2300000
#define ADSP_USB_UPDATE_WORK_INTERVAL  100



struct adsp_usb_info {
	int usb_icl_max_uA;
	int online;
};

struct adsp_usb_device {
	struct device *dev;
	struct adsp_usb_info usb_info;
	struct delayed_work update_work;
};


static enum power_supply_property capital_usb_props[] = {
	POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT,
};

static int capital_usb_psy_get_prop(struct power_supply *psy,
		enum power_supply_property prop,
		union power_supply_propval *pval)
{
	struct adsp_usb_device *di = power_supply_get_drvdata(psy);
	int rc;

	if (!di)
		return -1;

	pval->intval = -ENODATA;
	rc = power_supply_get_property_value("usb", prop, pval);

	switch (prop) {
	case POWER_SUPPLY_PROP_INPUT_CURRENT_LIMIT:
		if (pval->intval <= 0) {
			hwlog_info("use default usb_icl_max\n");
			pval->intval = di->usb_info.usb_icl_max_uA;
		}
		break;
	default:
		break;
	}

	return rc;
}

static const struct power_supply_desc capital_usb_psy_desc = {
	.name = "USB",
	.type = POWER_SUPPLY_TYPE_USB,
	.properties = capital_usb_props,
	.num_properties = ARRAY_SIZE(capital_usb_props),
	.get_property = capital_usb_psy_get_prop,
};

static void adsp_usb_init_psy(struct adsp_usb_device *di)
{
	struct power_supply_config psy_cfg = {};

	psy_cfg.drv_data = di;
	psy_cfg.of_node = di->dev->of_node;
	devm_power_supply_register(di->dev, &capital_usb_psy_desc, &psy_cfg);
}

static void adsp_usb_event_notify_callback(void *dev_data, u32 notification, void *data)
{
	struct adsp_usb_device *di = (struct adsp_usb_device *)dev_data;

	switch (notification)
	{
	case OEM_NOTIFY_USB_ONLINE:
		if (!data)
			return;
		di->usb_info.online = *(int *)data;
		schedule_delayed_work(&di->update_work, 0);
		break;
	default:
		return;
	}
}

static struct hihonor_glink_ops adsp_usb_glink_ops = {
	.notify_event = adsp_usb_event_notify_callback,
};

static void adsp_usb_update_work(struct work_struct *work)
{
	struct adsp_usb_device *di = container_of(work, struct adsp_usb_device, update_work.work);
	struct power_supply *psy = NULL;
	union power_supply_propval propval;

	psy = power_supply_get_by_name("usb");
	if (!psy) {
		hwlog_err("usb psy is not ready\n");
		goto restart_work;
	}

	propval.intval = di->usb_info.online;
	power_supply_set_property(psy, POWER_SUPPLY_PROP_ONLINE, &propval);
	return;

restart_work:
	schedule_delayed_work(&di->update_work, msecs_to_jiffies(ADSP_USB_UPDATE_WORK_INTERVAL));
}

static int adsp_usb_parse_dts(struct device_node *np, struct adsp_usb_device *di)
{
	struct adsp_usb_info *info = NULL;

	if (!np || !di)
		return -1;

	info = &di->usb_info;
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"usb-icl-max-in-uA", &info->usb_icl_max_uA, DEFAULT_USB_ICL_CUR_UA);

	return 0;
}

static int adsp_usb_probe(struct platform_device *pdev)
{
	struct adsp_usb_device *di = NULL;
	struct device_node *np = NULL;
	int ret;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	hwlog_info("%s %d\n", __func__, __LINE__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;

	adsp_usb_parse_dts(np, di);
	adsp_usb_glink_ops.dev_data = di;
	ret = hihonor_oem_glink_ops_register(&adsp_usb_glink_ops);
	if (ret) {
		hwlog_err("%s fail to register glink ops\n", __func__);
		goto fail_free_mem;
	}
	adsp_usb_init_psy(di);
	INIT_DELAYED_WORK(&di->update_work, adsp_usb_update_work);
	return 0;

fail_free_mem:
	hwlog_err("adsp_usb_probe err\n");
	devm_kfree(&pdev->dev, di);
	return ret;
}

static int adsp_usb_remove(struct platform_device *pdev)
{
	struct adsp_usb_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	devm_kfree(&pdev->dev, di);
	return 0;
}

static int adsp_usb_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int adsp_usb_resume(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id adsp_usb_match_table[] = {
	{
		.compatible = "honor,adsp_usb",
		.data = NULL,
	},
	{},
};

static struct platform_driver adsp_usb_driver = {
	.probe = adsp_usb_probe,
	.remove = adsp_usb_remove,
#ifdef CONFIG_PM
	.suspend = adsp_usb_suspend,
	.resume = adsp_usb_resume,
#endif
	.driver = {
		.name = "honor,adsp_usb",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_usb_match_table),
	},
};

static int __init adsp_usb_init(void)
{
	return platform_driver_register(&adsp_usb_driver);
}

static void __exit adsp_usb_exit(void)
{
	platform_driver_unregister(&adsp_usb_driver);
}

device_initcall_sync(adsp_usb_init);
module_exit(adsp_usb_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adsp usb module driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");

