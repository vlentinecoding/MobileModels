/*
 * hihonor_charger_glink.c
 *
 * hihonor charger glink driver
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

#ifndef _HIHONOR_CHARGER_GLINK_
#define _HIHONOR_CHARGER_GLINK_

#ifdef CONFIG_HIHONOR_OEM_GLINK
int hihonor_charger_glink_enable_hiz(int enable);
int hihonor_charger_glink_enable_charge(int enable);
int hihonor_charger_glink_set_input_current(int input_current);
int hihonor_charger_glink_set_sdp_input_current(int input_current);
int hihonor_charger_glink_set_charge_current(int input_current);
int hihonor_charger_glink_set_charger_type(int charger_type);
int hihonor_charger_glink_enable_wlc_src(int wlc_src);
int hihonor_charger_glink_enable_identify_insert(int enable);
int hihonor_charger_glink_enable_boost5v(int enable);
int hihonor_charger_glink_enable_usbsuspend_collapse(int enable);
#else
static inline int hihonor_charger_glink_enable_hiz(int enable)
{
	return -1;
}

static inline int hihonor_charger_glink_enable_charge(int enable)
{
	return -1;
}

static inline int hihonor_charger_glink_set_input_current(int input_current)
{
	return -1;
}

static inline int hihonor_charger_glink_set_sdp_input_current(int input_current)
{
	return -1;
}

static inline int hihonor_charger_glink_set_charge_current(int input_current)
{
	return -1;
}

static inline int hihonor_charger_glink_set_charger_type(int charger_type)
{
	return -1;
}

static inline int hihonor_charger_glink_enable_wlc_src(int wlc_src)
{
	return -1;
}

static inline int hihonor_charger_glink_enable_identify_insert(int enable)
{
	return -1;
}

static inline int hihonor_charger_glink_enable_boost5v(int enable)
{
       return -1;
}

static inline int hihonor_charger_glink_enable_usbsuspend_collapse(int enable)
{
	return -1;
}

#endif /* CONFIG_HIHONOR_OEM_GLINK */
#endif /* _HIHONOR_CHARGER_GLINK_ */
