/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hw_pogopin_otg_id.c
 *
 * pogopin id driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/delay.h>
#include <linux/of_address.h>
#include <linux/regulator/consumer.h>
#include <linux/clk.h>
#include <linux/of_gpio.h>
#include <chipset_common/hwpower/common_module/power_delay.h>
#include <chipset_common/hwpower/common_module/power_dts.h>
#include <huawei_platform/usb/hw_pogopin.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pogopin_otg_id.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <chipset_common/hwpower/common_module/power_pinctrl.h>
#include <linux/extcon-provider.h>
#include <huawei_platform/hwpower/common_module/power_platform.h>
#include <linux/soc/qcom/fsa4480-i2c.h>

/* include platform head-file */
#if defined(CONFIG_DEC_USB)
#include "dwc_otg_dec.h"
#include "dwc_otg_cil.h"
#endif

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG hw_pogopin_id
HWLOG_REGIST();

static struct pogopin_otg_id_dev *g_pogopin_otg_id_di;

static void pogopin_otg_in_handle_work(struct pogopin_otg_id_dev *di)
{
	if (!di)
		return;

	hwlog_info("pogopin otg insert cutoff cc,wait data and power switch\n");
	/* set usb mode to micro usb mode */
	pogopin_set_usb_mode(POGOPIN_MICROB_OTG_MODE);
	pogopin_5pin_set_pogo_status(POGO_OTG);
	pogopin_5pin_otg_in_switch_from_typec();
	(void)power_msleep(DT_MSLEEP_100MS, 0, NULL);
	pogopin_event_notify(POGOPIN_PLUG_IN_OTG);
}

static void pogopin_otg_out_handle_work(struct pogopin_otg_id_dev *di)
{
	if (!di)
		return;

	hwlog_info("pogopin otg out,switch to typec, wait\n");
	/* set usb mode to typec usb mode */
	pogopin_set_usb_mode(POGOPIN_TYPEC_MODE);
	pogopin_event_notify(POGOPIN_PLUG_OUT_OTG);
	pogopin_5pin_remove_switch_to_typec();
}

static int pogopin_otg_status_check_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct pogopin_otg_id_dev *di = g_pogopin_otg_id_di;

	if (!di) {
		hwlog_err("di is null\n");
		return NOTIFY_OK;
	}

	switch (event) {
	case POGOPIN_CHARGER_OUT_COMPLETE:
		if (gpio_get_value(di->gpio) == LOW)
			schedule_delayed_work(&di->otg_intb_work,
				POGOPIN_DELAYED_50MS);
		break;
	default:
		break;
	}

	return NOTIFY_OK;
}

static void pogopin_otg_id_intb_work(struct work_struct *work)
{
	int gpio_value;
	struct pogopin_otg_id_dev *di = g_pogopin_otg_id_di;
	static bool pogo_otg_trigger = false;

	if (!work || !di || pogopin_is_charging()) {
		hwlog_err("work or di is null or pogopin is charging\n");
		return;
	}

	gpio_value = gpio_get_value(di->gpio);
	hwlog_info("%s gpio_value = %d\n", __func__, gpio_value);

	if (di->pogo_otg_gpio_status == gpio_value)
		return;

	di->pogo_otg_gpio_status = gpio_value;

	if (gpio_value == LOW) {
		pogo_otg_trigger = true;
		pogopin_otg_in_handle_work(di);
	} else {
		/* ignore otg plug out event when undetect otg plug in event */
		if (!pogo_otg_trigger) {
			hwlog_err("%s:otg insert error, do nothing\n",
				__func__);
			return;
		}
		pogo_otg_trigger = false;
		pogopin_otg_out_handle_work(di);
	}
}

static irqreturn_t pogopin_otg_id_irq_handle(int irq, void *dev_id)
{
	int gpio_value;
	struct pogopin_otg_id_dev *di = g_pogopin_otg_id_di;

	if (!di) {
		hwlog_err("di is null\n");
		return IRQ_HANDLED;
	}

	disable_irq_nosync(di->irq);
	gpio_value = gpio_get_value(di->gpio);
	hwlog_info("%s gpio_value:%d\n", __func__, gpio_value);
	if (gpio_value == LOW)
		irq_set_irq_type(di->irq,IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND);
	else
		irq_set_irq_type(di->irq,IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND);

	enable_irq(di->irq);
	schedule_delayed_work(&di->otg_intb_work, POGOPIN_DELAYED_50MS);

	return IRQ_HANDLED;
}

static int pogopin_otg_id_parse_dts(struct pogopin_otg_id_dev *di,
	struct device_node *np)
{
	if (!di)
		return -EINVAL;

	di->gpio = of_get_named_gpio(np, "pogo_otg_int_gpio", 0);
	if (!gpio_is_valid(di->gpio)) {
		hwlog_err("gpio is not valid\n");
		return -EINVAL;
	}

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"otg_adc_channel", &di->otg_adc_channel, 0))
		return -EINVAL;

	return 0;
}

static int pogopin_otg_id_irq_init(struct pogopin_otg_id_dev *di)
{
	int ret = -EINVAL;
	int gpio_value;

	if (!di)
		return -EINVAL;

	di->irq = gpio_to_irq(di->gpio);
	if (di->irq < 0) {
		hwlog_err("gpio map to irq fail\n");
		return ret;
	}

	gpio_value = gpio_get_value(di->gpio);
	hwlog_info("%s gpio:%d\n", __func__, gpio_value);
	if (gpio_value == LOW)
		ret = request_irq(di->irq, pogopin_otg_id_irq_handle,
			IRQF_TRIGGER_RISING | IRQF_NO_SUSPEND | IRQF_ONESHOT,
			"otg_gpio_irq", NULL);
	else
		ret = request_irq(di->irq, pogopin_otg_id_irq_handle,
			IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND | IRQF_ONESHOT,
			"otg_gpio_irq", NULL);
	if (ret < 0)
		hwlog_err("gpio irq request fail\n");
	else
		di->otg_irq_enabled = TRUE;

	return ret;
}

static int pogopin_otg_id_probe(struct platform_device *pdev)
{
	int ret;
	struct device_node *np = NULL;
	struct device *dev = NULL;
	struct pogopin_otg_id_dev *di = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_pogopin_otg_id_di = di;
	platform_set_drvdata(pdev, di);
	di->pdev = pdev;
	dev = &pdev->dev;
	np = dev->of_node;

	di->pogo_otg_gpio_status = -1;
	(void)power_pinctrl_config(&(pdev->dev), "pinctrl-names", 1);
	ret = pogopin_otg_id_parse_dts(di, np);
	if (ret != 0) {
		hwlog_err("fail to parse dts");
		goto fail_parse_dts;
	}

	di->pogopin_otg_status_check_nb.notifier_call =
		pogopin_otg_status_check_notifier_call;
	ret = pogopin_event_notifier_register(&di->pogopin_otg_status_check_nb);
	if (ret < 0) {
		hwlog_err("pogopin otg_notifier register failed\n");
		goto fail_parse_dts;
	}

	ret = gpio_request(di->gpio, "otg_gpio_irq");
	if (ret < 0) {
		hwlog_err("gpio request fail\n");
		goto fail_request_gpio;
	}

	ret = gpio_direction_input(di->gpio);
	if (ret < 0) {
		hwlog_err("gpio set input fail\n");
		goto fail_set_gpio_direction;
	}
	msleep(5); /* sleep 5 ms */
	ret = pogopin_otg_id_irq_init(di);
	if (ret != 0) {
		hwlog_err("pogopin_otg_id_irq_init fail\n");
		goto fail_request_irq;
	}

	INIT_DELAYED_WORK(&di->otg_intb_work, pogopin_otg_id_intb_work);
	ret = gpio_get_value(di->gpio);
	if (ret == 0)
		schedule_delayed_work(&di->otg_intb_work, OTG_DELAYED_5000MS);

	return 0;
fail_request_irq:
fail_set_gpio_direction:
	gpio_free(di->gpio);
fail_parse_dts:
fail_request_gpio:
	pogopin_event_notifier_unregister(&di->pogopin_otg_status_check_nb);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	return ret;
}

static int pogopin_otg_id_remove(struct platform_device *pdev)
{
	struct pogopin_otg_id_dev *di = g_pogopin_otg_id_di;

	if (!di)
		return -ENODEV;

	free_irq(di->irq, pdev);
	gpio_free(di->gpio);
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, di);
	g_pogopin_otg_id_di = NULL;

	return 0;
}

static const struct of_device_id pogopin_otg_id_of_match[] = {
	{
		.compatible = "huawei,pogopin-otg-by-id",
	},
	{},
};

static struct platform_driver pogopin_otg_id_drv = {
	.probe = pogopin_otg_id_probe,
	.remove = pogopin_otg_id_remove,
	.driver = {
		.owner = THIS_MODULE,
		.name = "pogo_otg_id",
		.of_match_table = pogopin_otg_id_of_match,
	},
};

static int __init pogopin_otg_id_init(void)
{
	return platform_driver_register(&pogopin_otg_id_drv);
}

static void __exit pogopin_otg_id_exit(void)
{
	platform_driver_unregister(&pogopin_otg_id_drv);
}

late_initcall(pogopin_otg_id_init);
module_exit(pogopin_otg_id_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("POGOPIN OTG connection/disconnection driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
