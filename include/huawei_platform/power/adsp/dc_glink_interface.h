/*
 * dc_glink_interface.h
 *
 * glink interface for direct charge
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

#ifndef _DC_GLINK_INTERFACE_H_
#define _DC_GLINK_INTERFACE_H_

#include <huawei_platform/power/adsp/direct_charger_adsp.h>

#ifdef CONFIG_ADSP_DIRECT_CHARGER
int dc_glink_set_enable_charge(int enable);
int dc_glink_mainsc_set_enable_charge(int enable);
int dc_glink_auxsc_set_enable_charge(int enable);
int dc_glink_set_iin_thermal(int *iin_thermal_array, int array_size);
int dc_glink_get_adaptor_detect(int *adaptor_detect);
int dc_glink_get_iadaptor(int *iadaptor);
int dc_glink_get_multi_cur(struct dc_charge_info *info);
int dc_glink_get_vbus(int *vbus);
int dc_glink_get_cur_mode(int *cur_mode);
int dc_glink_set_vterm_dec(unsigned int vterm_dec);
int dc_glink_set_ichg_ratio(unsigned int ichg_ratio);
int dc_glink_get_rt_test_result(int *result);
int dc_glink_set_mmi_test_flag(int test_flag);
int dc_glink_get_mmi_test_result(int *result);

#else
static inline int dc_glink_set_enable_charge(int enable)
{
	return 0;
}

static inline int dc_glink_mainsc_set_enable_charge(int enable)
{
	return 0;
}

static inline int dc_glink_auxsc_set_enable_charge(int enable)
{
	return 0;
}

static inline int dc_glink_set_iin_thermal(int *iin_thermal_array, int array_size)
{
	return 0;
}

static inline int dc_glink_get_adaptor_detect(int *adaptor_detect)
{
	return 0;
}

static inline int dc_glink_get_iadaptor(int *iadaptor)
{
	return 0;
}

static inline int dc_glink_get_multi_cur(struct dc_charge_info *info)
{
	return 0;
}

static inline int dc_glink_get_vbus(int *vbus)
{
	return 0;
}

static inline int dc_glink_get_cur_mode(int *cur_mode)
{
	return 0;
}

static inline int dc_glink_set_vterm_dec(unsigned int vterm_dec)
{
	return 0;
}

static inline int dc_glink_set_ichg_ratio(unsigned int ichg_ratio)
{
	return 0;
}

static inline int dc_glink_get_rt_test_result(int *result)
{
	return 0;
}

static inline int dc_glink_set_mmi_test_flag(int test_flag)
{
	return 0;
}

static inline int dc_glink_get_mmi_test_result(int *result)
{
	return 0;
}

#endif /* CONFIG_ADSP_DIRECT_CHARGER */

#endif /* _DC_GLINK_INTERFACE_H_ */
