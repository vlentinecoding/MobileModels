/*
 * usb_analog_hs_mos.c
 *
 * usb analog headset mos-switch driver
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
 */

#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/power_supply.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/usb/typec.h>
#include <log/hw_log.h>
#include "usb_analog_hs_internal.h"

#define HWLOG_TAG usb_analog_hs
HWLOG_REGIST();

#define MIC_SWITCH_DELAY_MS    0
#define GPIO_LEVEL_HIGH        1
#define GPIO_LEVEL_LOW         0
#define LR_CHANNEL_VOLTAGE     2960000

struct usb_analog_hs_mos_priv {
	int gpio_type;
	int gpio_mic_gnd;     // high: sbu1->mic, sbu2->gnd
	int gpio_mos_switch;  // high: mos to hs, low: mos to usb
	int gpio_lr_ex_switch;  //high: switch to hs, low: switch to usb
	int mic_switch_delay; // unit: ms
	struct device *dev;
	struct regulator *mos_switch_ldo;
	struct device_node *of_node;
	struct device_node *switch_node;
	struct notifier_block switch_nb;
};

static void usb_analog_hs_mos_uninit_gpios(struct usb_analog_hs_mos_priv *priv)
{
	if (priv->gpio_mic_gnd > 0)
		gpio_free(priv->gpio_mic_gnd);

	if (priv->gpio_mos_switch > 0)
		gpio_free(priv->gpio_mos_switch);

	if (priv->gpio_lr_ex_switch > 0)
		gpio_free(priv->gpio_lr_ex_switch);
}

static int usb_analog_hs_mos_init_gpios(struct usb_analog_hs_mos_priv *priv)
{
	usb_analog_hs_get_property_gpio(priv->of_node,
		&priv->gpio_mic_gnd, GPIO_LEVEL_LOW, "switch_mic_gnd");
	usb_analog_hs_get_property_gpio(priv->of_node,
		&priv->gpio_mos_switch, GPIO_LEVEL_LOW, "switch_mos_hs");
	usb_analog_hs_get_property_gpio(priv->of_node,
		&priv->gpio_lr_ex_switch, GPIO_LEVEL_LOW, "switch_lr_ex");

	return 0;
}

static int usb_analog_hs_mos_init_regulator(struct usb_analog_hs_mos_priv *priv)
{
	int ret;

	if (priv->gpio_mos_switch > 0) {
		hwlog_info("%s: use gpio to switch mos\n", __func__);
		usb_analog_hs_set_gpio_value(priv->gpio_type, priv->gpio_mos_switch, GPIO_LEVEL_LOW);
		return 0;
	}

	priv->mos_switch_ldo = devm_regulator_get(priv->dev, "switch_lr_channel");
	if (IS_ERR(priv->mos_switch_ldo)) {
		hwlog_err("%s: switch_lr_channel regulator get failed\n", __func__);
		return -ENOENT;
	}

	ret = regulator_get_voltage(priv->mos_switch_ldo);
	hwlog_info("%s: mos_switch_ldo voltage is %d(default %d)\n",
		__func__, ret, LR_CHANNEL_VOLTAGE);
	return 0;
}

static int usb_analog_hs_mos_power_on(struct usb_analog_hs_mos_priv *priv)
{
	int is_enabled = 0;

	if (priv->gpio_lr_ex_switch > 0)
		usb_analog_hs_set_gpio_value(priv->gpio_type, priv->gpio_lr_ex_switch, GPIO_LEVEL_HIGH);

	if (priv->gpio_mos_switch > 0) {
		usb_analog_hs_set_gpio_value(priv->gpio_type, priv->gpio_mos_switch, GPIO_LEVEL_HIGH);
		return 0;
	}

	if (!priv->mos_switch_ldo) {
		hwlog_err("%s: mos_switch_ldo is null\n", __func__);
		return -EINVAL;
	}

	is_enabled = regulator_is_enabled(priv->mos_switch_ldo);
	if ((is_enabled == 0) && (regulator_enable(priv->mos_switch_ldo) < 0)) {
		hwlog_err("%s: mos_switch_ldo regulator enable failed\n", __func__);
		return -EFAULT;
	}

	hwlog_info("%s: mos_switch_ldo power on\n", __func__);
	return 0;
}

static int usb_analog_hs_mos_power_off(struct usb_analog_hs_mos_priv *priv)
{
	int is_enabled = 0;

	if (priv->gpio_lr_ex_switch > 0)
		usb_analog_hs_set_gpio_value(priv->gpio_type, priv->gpio_lr_ex_switch, GPIO_LEVEL_LOW);

	if (priv->gpio_mos_switch > 0) {
		usb_analog_hs_set_gpio_value(priv->gpio_type, priv->gpio_mos_switch, GPIO_LEVEL_LOW);
		return 0;
	}

	if (!priv->mos_switch_ldo) {
		hwlog_err("%s: mos_switch_ldo is null\n", __func__);
		return -EINVAL;
	}

	is_enabled = regulator_is_enabled(priv->mos_switch_ldo);
	if ((is_enabled > 0) && (regulator_disable(priv->mos_switch_ldo) < 0)) {
		hwlog_err("%s: mos_switch_ldo regulator disable failed\n", __func__);
		return -EFAULT;
	}

	if (usb_analog_hs_get_gpio_value(priv->gpio_type, priv->gpio_mic_gnd) == GPIO_LEVEL_HIGH)
		usb_analog_hs_set_gpio_value(priv->gpio_type, priv->gpio_mic_gnd, GPIO_LEVEL_LOW);

	hwlog_info("%s: mos_switch_ldo power off\n", __func__);
	return 0;
}

static int usb_analog_hs_mos_init_dts_settings(struct usb_analog_hs_mos_priv *priv)
{
	int ret;

	priv->mic_switch_delay = usb_analog_hs_get_property_value(priv->of_node,
		"mic_switch_delay", MIC_SWITCH_DELAY_MS);
	priv->gpio_type = usb_analog_hs_get_property_value(priv->of_node,
		"gpio_type", USB_ANALOG_HS_GPIO_SOC);

	ret = usb_analog_hs_mos_init_gpios(priv);
	if (ret < 0) {
		hwlog_err("%s: parse gpios failed %d\n", __func__, ret);
		return ret;
	}

	ret = usb_analog_hs_mos_init_regulator(priv);
	if (ret < 0) {
		hwlog_err("%s: parse regulator failed %d\n", __func__, ret);
		goto err_gpio;
	}

	// init mic-gnd-gpio and power state
	usb_analog_hs_mos_power_off(priv);
	return 0;

err_gpio:
	usb_analog_hs_mos_uninit_gpios(priv);
	return ret;
}

static int usb_analog_hs_mos_mic_gnd_switch(struct usb_analog_hs_mos_priv *priv)
{
	int gpio_mic_gnd;

	gpio_mic_gnd = usb_analog_hs_get_gpio_value(priv->gpio_type, priv->gpio_mic_gnd);
	hwlog_info("%s: current gpio_mic_gnd is %d\n", __func__, gpio_mic_gnd);

	gpio_mic_gnd = (gpio_mic_gnd == GPIO_LEVEL_HIGH) ? GPIO_LEVEL_LOW : GPIO_LEVEL_HIGH;
	usb_analog_hs_set_gpio_value(priv->gpio_type, priv->gpio_mic_gnd, gpio_mic_gnd);
	if (priv->mic_switch_delay > 0)
		msleep(priv->mic_switch_delay);

	gpio_mic_gnd = usb_analog_hs_get_gpio_value(priv->gpio_type, priv->gpio_mic_gnd);
	hwlog_info("%s: gpio_mic_gnd change to %d\n", __func__, gpio_mic_gnd);
	return 0;
}

static int usb_analog_hs_mos_switch_event(struct device_node *node, enum fsa_function event)
{
	struct platform_device *pdev = NULL;
	struct usb_analog_hs_mos_priv *priv = NULL;
	int ret = 0;

	if (!node)
		return -EINVAL;

	pdev = of_find_device_by_node(node);
	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv || !priv->dev)
		return -EINVAL;

	hwlog_info("%s: mos_switch event %d\n", __func__, event);
	switch (event) {
		case FSA_MIC_GND_SWAP:
			ret = usb_analog_hs_mos_mic_gnd_switch(priv);
			break;
		default:
			break;
	}

	return ret;
}

static struct usb_analog_hs_ops g_usb_analog_hs_ops = {
	.switch_event = usb_analog_hs_mos_switch_event,
};

static int usb_analog_hs_mos_switch_callback(struct notifier_block *nb,
	unsigned long mode, void *ptr)
{
	struct usb_analog_hs_mos_priv *priv = container_of(nb,
		struct usb_analog_hs_mos_priv, switch_nb);

	if (!priv)
		return -EINVAL;

	hwlog_info("%s: usb typec mode = %lu\n", __func__, mode);
	switch (mode) {
	case TYPEC_ACCESSORY_AUDIO:
		usb_analog_hs_mos_power_on(priv);
		break;
	case TYPEC_ACCESSORY_NONE:
		usb_analog_hs_mos_power_off(priv);
		break;
	default:
		break;
	}
	return 0;
}

static int usb_analog_hs_mos_probe(struct platform_device *pdev)
{
	struct usb_analog_hs_mos_priv *priv = NULL;
	const char *switch_str = "honor,analog-hs-switch";
	struct device_node *switch_node = NULL;
	int ret;

	switch_node = of_parse_phandle(pdev->dev.of_node, switch_str, 0);
	if (!switch_node) {
		hwlog_err("%s: find analog_hs switch failed\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s: find analog_hs switch\n", __func__);

	if (usb_analog_hs_ops_register(switch_node, &g_usb_analog_hs_ops) < 0) {
		hwlog_info("%s: need to wait for probe to complete\n", __func__);
		return -EPROBE_DEFER;
	}

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;
	platform_set_drvdata(pdev, priv);
	priv->of_node = pdev->dev.of_node;
	priv->switch_node = switch_node;

	ret = usb_analog_hs_mos_init_dts_settings(priv);
	if (ret < 0)
		goto err_data;

	priv->switch_nb.notifier_call = usb_analog_hs_mos_switch_callback;
	// The higher the value, the higher the priority, here set to 1.
	priv->switch_nb.priority = 1;
	ret = usb_analog_hs_reg_notifier(&priv->switch_nb, switch_node);
	if (ret < 0) {
		hwlog_err("%s: usb_analog_hs notifier reg failed %d\n", __func__, ret);
		goto err_dts;
	}

	hwlog_info("%s: probe success\n", __func__);
	return 0;

err_dts:
	usb_analog_hs_mos_uninit_gpios(priv);
err_data:
	devm_kfree(&pdev->dev, priv);
	return ret;
}

static int usb_analog_hs_mos_remove(struct platform_device *pdev)
{
	struct usb_analog_hs_mos_priv *priv = NULL;

	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv)
		return -EINVAL;

	usb_analog_hs_unreg_notifier(&priv->switch_nb, priv->switch_node);
	usb_analog_hs_mos_uninit_gpios(priv);
	dev_set_drvdata(&pdev->dev, NULL);
	return 0;
}

static const struct of_device_id usb_analog_hs_mos_of_match[] = {
	{ .compatible = "honor,usb_analog_hs_mos", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_analog_hs_mos_of_match);

static struct platform_driver usb_analog_hs_mos_driver = {
	.driver = {
		.name   = "usb_analog_hs_mos",
		.owner  = THIS_MODULE,
		.of_match_table = usb_analog_hs_mos_of_match,
	},
	.probe  = usb_analog_hs_mos_probe,
	.remove = usb_analog_hs_mos_remove,
};

static int __init usb_analog_hs_mos_init(void)
{
	return platform_driver_register(&usb_analog_hs_mos_driver);
}
module_init(usb_analog_hs_mos_init);

static void __exit usb_analog_hs_mos_exit(void)
{
	platform_driver_unregister(&usb_analog_hs_mos_driver);
}
module_exit(usb_analog_hs_mos_exit);

MODULE_DESCRIPTION("usb analog hs mos driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
