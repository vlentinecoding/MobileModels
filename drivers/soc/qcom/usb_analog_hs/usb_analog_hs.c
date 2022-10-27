/*
 * usb_analog_hs.c
 *
 * usb analog headset driver
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

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/power_supply.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/mutex.h>
#include <linux/usb/typec.h>
#include <linux/usb/ucsi_glink.h>
#include <linux/iio/consumer.h>
#include <log/hw_log.h>
#include "usb_analog_hs_internal.h"

#define HWLOG_TAG usb_analog_hs
HWLOG_REGIST();

struct usb_analog_hs_priv {
	struct device *dev;
	struct device_node *fsa_switch_node;
	struct device_node *switch_node;
	struct usb_analog_hs_ops *switch_ops;

	u32 use_powersupply;
	struct notifier_block usb_nb;
	struct power_supply *usb_psy;
	struct iio_channel *iio_ch;
	atomic_t usbc_mode;

	struct work_struct analog_hs_work;
	struct blocking_notifier_head hs_notifier;
	struct mutex notification_lock;
};

int usb_analog_hs_get_property_value(struct device_node *node,
	const char *prop_name, int default_value)
{
	int value = 0;
	int ret;

	ret = of_property_read_u32(node, prop_name, &value);
	if (ret < 0)
		value = default_value;

	hwlog_info("%s: %s is %d\n", __func__, prop_name, value);
	return value;
}
EXPORT_SYMBOL(usb_analog_hs_get_property_value);

bool usb_analog_hs_get_property_bool(struct device_node *node,
	const char *prop_name, bool default_setting)
{
	bool setting = false;

	if (!of_find_property(node, prop_name, NULL))
		setting = default_setting;
	else
		setting = of_property_read_bool(node, prop_name);

	hwlog_info("%s: %s is %d\n", __func__, prop_name, setting);
	return setting;
}
EXPORT_SYMBOL(usb_analog_hs_get_property_bool);

int usb_analog_hs_get_property_gpio(struct device_node *node,
	int *gpio_index, int out_value, const char *gpio_name)
{
	int gpio;
	int ret;

	gpio = of_get_named_gpio(node, gpio_name, 0);
	*gpio_index = gpio;
	if (gpio < 0) {
		hwlog_info("%s: looking up %s prop in node %s, not existed %d\n",
			__func__, node->full_name, gpio_name, gpio);
		return 0; // maybe no need gpio, do not return error
	}

	if (!gpio_is_valid(gpio)) {
		hwlog_err("%s: invalid gpio %s\n", __func__, gpio_name);
		return -ENOENT;
	}

	ret = gpio_request(gpio, gpio_name);
	if (ret < 0) {
		hwlog_err("%s: request gpio %s failed %d\n", __func__, gpio_name, ret);
		return ret;
	}

	ret = gpio_direction_output(gpio, out_value);
	if (ret < 0)
		hwlog_err("%s: set output %s failed %d\n", __func__, gpio_name, ret);
	return ret;
}
EXPORT_SYMBOL(usb_analog_hs_get_property_gpio);

int usb_analog_hs_get_gpio_value(int type, int gpio)
{
	if (gpio <= 0)
		return -ENODEV;

	if (type == USB_ANALOG_HS_GPIO_CODEC)
		return gpio_get_value_cansleep(gpio);

	return gpio_get_value(gpio);
}
EXPORT_SYMBOL(usb_analog_hs_get_gpio_value);

void usb_analog_hs_set_gpio_value(int type, int gpio, int value)
{
	if (gpio <= 0)
		return;

	if (type == USB_ANALOG_HS_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}
EXPORT_SYMBOL(usb_analog_hs_set_gpio_value);

int usb_analog_hs_ops_register(struct device_node *node, struct usb_analog_hs_ops *ops)
{
	struct platform_device *pdev = NULL;
	struct usb_analog_hs_priv *priv = NULL;

	if (!node || !ops)
		return -EINVAL;

	pdev = of_find_device_by_node(node);
	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv)
		return -EINVAL;

	priv->switch_ops = ops;
	hwlog_info("%s: register switch_ops success\n", __func__);
	return 0;
}
EXPORT_SYMBOL(usb_analog_hs_ops_register);

static int usb_analog_hs_event_changed_psupply(struct usb_analog_hs_priv *priv,
	unsigned long evt, void *ptr)
{
	UNUSED(evt);
	UNUSED(ptr);
	hwlog_debug("%s: queueing analog_hs_work\n", __func__);
	pm_stay_awake(priv->dev);
	queue_work(system_freezable_wq, &priv->analog_hs_work);
	return 0;
}

static int usb_analog_hs_event_changed_ucsi(struct usb_analog_hs_priv *priv,
	unsigned long evt, void *ptr)
{
	enum typec_accessory acc = ((struct ucsi_glink_constat_info *)ptr)->acc;

	UNUSED(evt);
	hwlog_info("%s: usb event received, mode %d(%ld) expected %d\n",
		__func__, acc, priv->usbc_mode.counter, TYPEC_ACCESSORY_AUDIO);

	switch (acc) {
	case TYPEC_ACCESSORY_AUDIO:
	case TYPEC_ACCESSORY_NONE:
		// filter notifications received before
		if (atomic_read(&(priv->usbc_mode)) == acc)
			break;
		atomic_set(&(priv->usbc_mode), acc);

		hwlog_debug("%s: queueing analog_hs_work\n", __func__);
		pm_stay_awake(priv->dev);
		queue_work(system_freezable_wq, &priv->analog_hs_work);
		break;
	default:
		break;
	}

	return 0;
}

static int usb_analog_hs_event_changed(struct notifier_block *nb, unsigned long evt, void *ptr)
{
	struct usb_analog_hs_priv *priv = container_of(nb, struct usb_analog_hs_priv, usb_nb);

	if (!priv || !priv->dev)
		return -EINVAL;

	if (priv->use_powersupply)
		return usb_analog_hs_event_changed_psupply(priv, evt, ptr);

	return usb_analog_hs_event_changed_ucsi(priv, evt, ptr);
}

extern int pd_dpm_get_analog_hs_state(void);
static int usb_analog_hs_setup_switches_psupply(struct usb_analog_hs_priv *priv)
{
	union power_supply_propval mode;
	int ret = 0;

	mode.intval = pd_dpm_get_analog_hs_state();
	mutex_lock(&priv->notification_lock);
	// get latest mode again within locked context
	if (ret < 0) {
		hwlog_err("%s: unable to read usbc_mode %d\n", __func__, ret);
		goto done;
	}

	hwlog_debug("%s: setting active %d, mode intval %d\n",
		__func__, mode.intval != TYPEC_ACCESSORY_NONE, mode.intval);

	if (atomic_read(&(priv->usbc_mode)) == mode.intval)
		goto done;
	atomic_set(&(priv->usbc_mode), mode.intval);

	switch (mode.intval) {
	case TYPEC_ACCESSORY_AUDIO:
	case TYPEC_ACCESSORY_NONE:
		blocking_notifier_call_chain(&priv->hs_notifier, mode.intval, NULL);
		break;
	default:
		break;
	}

done:
	mutex_unlock(&priv->notification_lock);
	return ret;
}

static int usb_analog_hs_setup_switches_ucsi(struct usb_analog_hs_priv *priv)
{
	int mode;

	mutex_lock(&priv->notification_lock);
	// get latest mode again within locked context
	mode = atomic_read(&(priv->usbc_mode));
	hwlog_debug("%s: setting active %d, mode 0x%X\n",
		__func__, mode != TYPEC_ACCESSORY_NONE, mode);

	switch (mode) {
	case TYPEC_ACCESSORY_AUDIO:
	case TYPEC_ACCESSORY_NONE:
		blocking_notifier_call_chain(&priv->hs_notifier, mode, NULL);
		break;
	default:
		break;
	}

	mutex_unlock(&priv->notification_lock);
	return 0;
}

static int usb_analog_hs_setup_switches(struct usb_analog_hs_priv *priv)
{
	if (priv->use_powersupply)
		return usb_analog_hs_setup_switches_psupply(priv);

	return usb_analog_hs_setup_switches_ucsi(priv);
}

int usb_analog_hs_reg_notifier(struct notifier_block *nb, struct device_node *node)
{
	struct platform_device *pdev = NULL;
	struct usb_analog_hs_priv *priv = NULL;
	int ret;

	if (!nb || !node)
		return -EINVAL;

	pdev = of_find_device_by_node(node);
	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv || !priv->dev)
		return -EINVAL;

	if (priv->fsa_switch_node)
		return fsa4480_reg_notifier_qcom(nb, priv->fsa_switch_node);

	ret = blocking_notifier_chain_register(&priv->hs_notifier, nb);
	if (ret < 0)
		return ret;

	// as part of the init sequence check if there is a connected USB C analog adapter
	hwlog_info("%s: verify if USB adapter is already inserted\n", __func__);
	if (priv->use_powersupply)
		atomic_set(&(priv->usbc_mode), 0);
	ret = usb_analog_hs_setup_switches(priv);
	return ret;
}
EXPORT_SYMBOL(usb_analog_hs_reg_notifier);

int fsa4480_reg_notifier(struct notifier_block *nb, struct device_node *node)
{
	hwlog_info("%s: enter\n", __func__);
	return usb_analog_hs_reg_notifier(nb, node);
}
EXPORT_SYMBOL(fsa4480_reg_notifier);

int usb_analog_hs_unreg_notifier(struct notifier_block *nb, struct device_node *node)
{
	struct platform_device *pdev = NULL;
	struct usb_analog_hs_priv *priv = NULL;
	int ret;

	if (!nb || !node)
		return -EINVAL;

	pdev = of_find_device_by_node(node);
	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv)
		return -EINVAL;

	if (priv->fsa_switch_node)
		return fsa4480_unreg_notifier_qcom(nb, priv->fsa_switch_node);

	mutex_lock(&priv->notification_lock);
	ret = blocking_notifier_chain_unregister(&priv->hs_notifier, nb);
	mutex_unlock(&priv->notification_lock);
	return ret;
}
EXPORT_SYMBOL(usb_analog_hs_unreg_notifier);

int fsa4480_unreg_notifier(struct notifier_block *nb, struct device_node *node)
{
	return usb_analog_hs_unreg_notifier(nb, node);
}
EXPORT_SYMBOL(fsa4480_unreg_notifier);

int usb_analog_hs_switch_event(struct device_node *node, enum fsa_function event)
{
	struct platform_device *pdev = NULL;
	struct usb_analog_hs_priv *priv = NULL;

	if (!node)
		return -EINVAL;

	pdev = of_find_device_by_node(node);
	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv)
		return -EINVAL;

	if (priv->fsa_switch_node)
		return fsa4480_switch_event_qcom(priv->fsa_switch_node, event);

	if (!priv->switch_ops || !priv->switch_ops->switch_event)
		return -EINVAL;

	return priv->switch_ops->switch_event(priv->switch_node, event);
}
EXPORT_SYMBOL(usb_analog_hs_switch_event);

int fsa4480_switch_event(struct device_node *node, enum fsa_function event)
{
	return usb_analog_hs_switch_event(node, event);
}
EXPORT_SYMBOL(fsa4480_switch_event);

static void usb_analog_hs_work_fn(struct work_struct *work)
{
	struct usb_analog_hs_priv *priv = container_of(work,
		struct usb_analog_hs_priv, analog_hs_work);

	if (!priv || !priv->dev)
		return;

	usb_analog_hs_setup_switches(priv);
	pm_relax(priv->dev);
}

static int usb_analog_hs_probe(struct platform_device *pdev)
{
	struct usb_analog_hs_priv *priv = NULL;
	const char *fsa_switch_str = "qcom,dp-aux-switch";
	const char *switch_str = "honor,analog-hs-switch";
	u32 use_powersupply = 0;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;
	platform_set_drvdata(pdev, priv);

	priv->fsa_switch_node = of_parse_phandle(pdev->dev.of_node, fsa_switch_str, 0);
	if (!priv->fsa_switch_node) {
		hwlog_info("%s: not use fsa switch\n", __func__);
	} else {
		hwlog_info("%s: use fsa switch\n", __func__);
		return 0;
	}

	priv->switch_node = of_parse_phandle(pdev->dev.of_node, switch_str, 0);
	if (!priv->switch_node) {
		hwlog_err("%s: not find analog_hs switch\n", __func__);
		return -EINVAL;
	}
	hwlog_info("%s: find analog_hs switch\n", __func__);

	priv->usb_nb.notifier_call = usb_analog_hs_event_changed;
	priv->usb_nb.priority = 0;
	ret = of_property_read_u32(priv->dev->of_node, "qcom,use-power-supply", &use_powersupply);
	if (ret < 0 || use_powersupply == 0) {
		priv->use_powersupply = 0;
		ret = register_ucsi_glink_notifier(&priv->usb_nb);
		if (ret < 0) {
			hwlog_err("%s: ucsi glink reg failed %d\n", __func__, ret);
			goto err_data;
		}
		hwlog_info("%s: ucsi glink reg success\n", __func__);
	} else {
		priv->use_powersupply = 1;
		priv->usb_psy = power_supply_get_by_name("usb");
		if (!priv->usb_psy) {
			ret = -EPROBE_DEFER;
			hwlog_info("%s: could not get USB psy info %d\n", __func__, ret);
			goto err_data;
		}

		priv->iio_ch = iio_channel_get(priv->dev, "typec_mode");
		if (!priv->iio_ch) {
			hwlog_err("%s: iio_channel_get failed for typec_mode\n", __func__);
			goto err_supply;
		}

		ret = power_supply_reg_notifier(&priv->usb_nb);
		if (ret < 0) {
			hwlog_err("%s: power supply reg failed %d\n", __func__, ret);
			goto err_supply;
		}
	}

	mutex_init(&priv->notification_lock);
	INIT_WORK(&priv->analog_hs_work, usb_analog_hs_work_fn);

	priv->hs_notifier.rwsem =
		(struct rw_semaphore)__RWSEM_INITIALIZER((priv->hs_notifier).rwsem);
	priv->hs_notifier.head = NULL;

	hwlog_info("%s: probe success\n", __func__);
	return 0;

err_supply:
	power_supply_put(priv->usb_psy);
err_data:
	devm_kfree(&pdev->dev, priv);
	return ret;
}

static int usb_analog_hs_remove(struct platform_device *pdev)
{
	struct usb_analog_hs_priv *priv = NULL;

	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv)
		return -EINVAL;

	if (priv->fsa_switch_node) {
		dev_set_drvdata(&pdev->dev, NULL);
		return 0;
	}

	if (priv->use_powersupply) {
		power_supply_unreg_notifier(&priv->usb_nb);
		power_supply_put(priv->usb_psy);
	} else {
		unregister_ucsi_glink_notifier(&priv->usb_nb);
	}

	cancel_work_sync(&priv->analog_hs_work);
	pm_relax(priv->dev);
	mutex_destroy(&priv->notification_lock);
	dev_set_drvdata(&pdev->dev, NULL);
	return 0;
}

static const struct of_device_id usb_analog_hs_dt_match[] = {
	{ .compatible = "honor,usb_analog_hs", },
	{}
};
MODULE_DEVICE_TABLE(of, usb_analog_hs_dt_match);

static struct platform_driver usb_analog_hs_driver = {
	.driver = {
		.name           = "usb_analog_hs",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(usb_analog_hs_dt_match),
	},
	.probe  = usb_analog_hs_probe,
	.remove = usb_analog_hs_remove,
};

static int __init usb_analog_hs_init(void)
{
	int ret;

	ret = platform_driver_register(&usb_analog_hs_driver);
	if (ret)
		hwlog_err("%s: register driver failed %d\n", __func__, ret);

	return ret;
}
module_init(usb_analog_hs_init);

static void __exit usb_analog_hs_exit(void)
{
	platform_driver_unregister(&usb_analog_hs_driver);
}
module_exit(usb_analog_hs_exit);

MODULE_DESCRIPTION("usb analog hs driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
