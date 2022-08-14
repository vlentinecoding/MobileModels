/*
 * usb_analog_hs.h
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

#ifndef USB_ANALOG_HS_H
#define USB_ANALOG_HS_H

#include <linux/of.h>
#include <linux/notifier.h>
#include <linux/soc/qcom/fsa4480-i2c.h>

#ifdef CONFIG_USB_ANALOG_HS_MODULE
#define CONFIG_USB_ANALOG_HS
#endif

#ifdef CONFIG_USB_ANALOG_HS_MOS_MODULE
#define CONFIG_USB_ANALOG_HS_MOS
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

enum usb_analog_hs_gpio_type {
	USB_ANALOG_HS_GPIO_SOC           = 0,
	USB_ANALOG_HS_GPIO_CODEC         = 1,
};

struct usb_analog_hs_ops {
	int (*switch_event)(struct device_node *node, enum fsa_function event);
};

#ifdef CONFIG_USB_ANALOG_HS
int usb_analog_hs_ops_register(struct device_node *node, struct usb_analog_hs_ops *ops);

int usb_analog_hs_switch_event(struct device_node *node, enum fsa_function event);
int usb_analog_hs_reg_notifier(struct notifier_block *nb, struct device_node *node);
int usb_analog_hs_unreg_notifier(struct notifier_block *nb, struct device_node *node);

int fsa4480_switch_event_qcom(struct device_node *node, enum fsa_function event);
int fsa4480_reg_notifier_qcom(struct notifier_block *nb, struct device_node *node);
int fsa4480_unreg_notifier_qcom(struct notifier_block *nb, struct device_node *node);
#else
static inline int usb_analog_hs_ops_register(struct device_node *node,
	struct usb_analog_hs_ops *ops)
{
	return 0;
}

static inline int usb_analog_hs_switch_event(struct device_node *node, enum fsa_function event)
{
	return 0;
}

static inline int usb_analog_hs_reg_notifier(struct notifier_block *nb, struct device_node *node)
{
	return 0;
}

static inline int usb_analog_hs_unreg_notifier(struct notifier_block *nb, struct device_node *node)
{
	return 0;
}
#endif // !CONFIG_USB_ANALOG_HS

#endif // USB_ANALOG_HS_H
