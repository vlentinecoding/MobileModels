/*
 * hihonor_usb_glink.h
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

#ifndef __HIHONOR_USB_GLINK
#define __HIHONOR_USB_GLINK

#ifdef CONFIG_HIHONOR_OEM_GLINK
int hihonor_usb_glink_get_cable_type(void);
void hihonor_usb_glink_set_cc_insert(bool insert);
int hihonor_usb_glink_check_cc_vbus_short(void);
int hihonor_usb_glink_get_typec_sm_status(void);

#else
int hihonor_usb_glink_get_cable_type(void)
{
	return -1;
}

void hihonor_usb_glink_set_cc_insert(bool insert)
{
	return;
}
int hihonor_usb_glink_check_cc_vbus_short(void)
{
	return 0;
}
#endif /* CONFIG_HIHONOR_OEM_GLINK */

#endif /* __HIHONOR_USB_GLINK */
