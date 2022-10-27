/*
 * rt9426a.h
 *
 * rt9426a interface
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

#ifndef _RT9426A_H_
#define _RT9426A_H_

#ifdef CONFIG_ADSP_BATTERY
int rt9426a_parse_para(struct device_node *np, 
	const char *batt_model_name, void **fuel_para, int *para_size);
int rt9426a_parse_aging_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size);
#else
static inline int rt9426a_parse_para(struct device_node *np, 
	const char *batt_model_name, void **fuel_para, int *para_size)
{
	return 0;
}

static inline int rt9426a_parse_aging_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size)
{
	return 0;
}
#endif /* CONFIG_ADSP_BATTERY */
#endif /* _RT9426A_H_ */
