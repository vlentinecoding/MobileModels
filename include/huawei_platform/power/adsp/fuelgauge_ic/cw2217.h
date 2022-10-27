/*
 * cw2217.h
 *
 * cw2217 interface
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

#ifndef _FG_CW2217_H_
#define _FG_CW2217_H_

#include <huawei_platform/power/adsp/fuelgauge_ic/fg_common.h>

#define DESIGN_CAPACITY              4000
#define SIZE_BATINFO                 80

struct cw2217_para {
	unsigned int design_capacity;
	int fg_para_version;
	int rsense;
	int ntc_compensation_is;
	struct compensation_para ntc_temp_compensation_para[NTC_PARA_LEVEL];
	unsigned int coefficient;
	unsigned char cw_image_data[SIZE_BATINFO];
};

#ifdef CONFIG_ADSP_BATTERY
int cw2217_parse_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size);
int cw2217_parse_aging_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size);
#else
static inline int cw2217_parse_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size)
{
	return 0;
}

static inline int cw2217_parse_aging_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size)
{
	return 0;
}
#endif /* CONFIG_ADSP_BATTERY */
#endif /* _FG_CW2217_H_ */
