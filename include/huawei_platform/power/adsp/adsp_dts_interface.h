/*
 * adsp_dts_interface.h
 *
 * adsp dts interface
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

#ifndef _ADSP_DTS_INTERFACE_H_
#define _ADSP_DTS_INTERFACE_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

#ifdef CONFIG_ADSP_DTS
int adsp_dts_glink_set_dc_para(void *para, size_t size);
int adsp_dts_glink_set_charger_para(void *para, size_t size);
#else
static inline int adsp_dts_glink_set_dc_para(void *para, size_t size)
{
	return -1;
}

static inline int adsp_dts_glink_set_charger_para(void *para, size_t size)
{
	return -1;
}

#endif /* CONFIG_ADSP_DTS */

#endif /* _ADSP_DTS_INTERFACE_H_ */

