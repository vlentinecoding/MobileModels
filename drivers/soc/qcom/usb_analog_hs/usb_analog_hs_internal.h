/*
 * usb_analog_hs_internal.h
 *
 * usb analog headset internal driver
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

#ifndef USB_ANALOG_HS_INTERNAL_H
#define USB_ANALOG_HS_INTERNAL_H

#include <linux/of.h>
#include <linux/soc/qcom/usb_analog_hs.h>

int usb_analog_hs_get_property_value(struct device_node *node,
	const char *prop_name, int default_value);
bool usb_analog_hs_get_property_bool(struct device_node *node,
	const char *prop_name, bool default_setting);
int usb_analog_hs_get_property_gpio(struct device_node *node,
	int *gpio_index, int out_value, const char *gpio_name);
int usb_analog_hs_get_gpio_value(int type, int gpio);
void usb_analog_hs_set_gpio_value(int type, int gpio, int value);

#endif // USB_ANALOG_HS_INTERNAL_H
