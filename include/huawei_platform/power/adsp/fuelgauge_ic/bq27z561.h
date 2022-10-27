/*
 * bq27z561.h
 *
 * bq27z561 interface
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

#ifndef _FG_BQ27Z561_H_
#define _FG_BQ27Z561_H_

#include <huawei_platform/power/adsp/fuelgauge_ic/fg_common.h>

#define BQFS_IMAGE_SIZE                  7000
#define FG_PARA_INVALID_VER              0xFF


struct bq27z561_para {
	int fcc_th[2];
	int qmax_th[2];
	int bqfs_image_size;
	unsigned char bqfs_image_data[BQFS_IMAGE_SIZE];
	int fg_para_version;
	int c_gain;
	int v_gain;
	int ntc_compensation_is;
	struct compensation_para ntc_temp_compensation_para[NTC_PARA_LEVEL];
};

#ifdef CONFIG_ADSP_DTS
int bq27z561_parse_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size);
int bq27z561_parse_aging_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size);
#else
static inline int bq27z561_parse_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size)
{
	return 0;
}

static inline int bq27z561_parse_aging_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size)
{
	return 0;
}
#endif /* CONFIG_ADSP_DTS */
#endif /* _FG_BQ27Z561_H_ */

