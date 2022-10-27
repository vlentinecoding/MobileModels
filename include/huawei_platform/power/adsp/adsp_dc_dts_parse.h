/*
 * adsp_dc_dts_parse.h
 *
 * adsp dts parse interface for direct charge
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

#ifndef _ADSP_DC_DTS_PARSE_H_
#define _ADSP_DC_DTS_PARSE_H_

#include <huawei_platform/power/adsp/direct_charger_adsp.h>

#ifdef CONFIG_ADSP_DIRECT_CHARGER
int adsp_dc_dts_parse(struct device_node *np, struct dc_sc_config *conf);
#else
static inline int adsp_dc_dts_parse(struct device_node *np, struct dc_sc_config *conf)
{
	return -1;
}
#endif /* CONFIG_ADSP_DIRECT_CHARGER */

#endif /* _ADSP_DC_DTS_PARSE_H_ */