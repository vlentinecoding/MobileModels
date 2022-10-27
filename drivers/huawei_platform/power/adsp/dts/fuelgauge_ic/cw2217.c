/*
 * cw2217.c
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <log/hw_log.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/coul_interface.h>
#include <chipset_common/hwpower/coul_calibration.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/power_dts.h>
#include <huawei_platform/power/adsp/fuelgauge_ic/cw2217.h>

#define HWLOG_TAG adsp_cw2217
HWLOG_REGIST();

#define DEFAULT_CALI_PARA 1000000

static unsigned int get_coefficient_from_flash(void)
{
	unsigned int coefficient = 0;
	if (coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_A, &coefficient)) {
		return DEFAULT_CALI_PARA;
	}

	if (coefficient) {
		hwlog_info("coefficient = %d\n", coefficient);
		return coefficient;
	} else {
		return DEFAULT_CALI_PARA;
	}
}

static int cw2217_parse_fg_para(struct device_node *np, const char *batt_model_name, struct cw2217_para *pdata)
{
	int ret;
	const char *battery_name = NULL;
	struct device_node *child_node = NULL;
	struct device_node *default_node = NULL;

	for_each_child_of_node(np, child_node) {
		if (power_dts_read_string(power_dts_tag(HWLOG_TAG),
			child_node, "batt_name", &battery_name)) {
			hwlog_info("childnode without batt_name property");
			continue;
		}
		if (!battery_name)
			continue;
		if (!default_node)
			default_node = child_node;
		hwlog_info("search battery data, battery_name: %s\n", battery_name);
		if (!batt_model_name || !strcmp(battery_name, batt_model_name))
			break;
	}

	if (!child_node) {
		if (default_node) {
			hwlog_info("cannt match childnode, use first\n");
			child_node = default_node;
		} else {
			hwlog_info("cannt find any childnode, use father\n");
			child_node = np;
		}
	}

	(void)power_dts_read_u32(power_dts_tag("FG_CW2217"), child_node,
		"fg_para_version", (u32 *)&pdata->fg_para_version, 0xFF);
	hwlog_info("fg_para_version = 0x%x\n", pdata->fg_para_version);

	ret = power_dts_read_u8_array(power_dts_tag("FG_CW2217"), child_node,
		"cw_image_data", pdata->cw_image_data, SIZE_BATINFO);
	if (ret) {
		hwlog_err("cw_image_data read failed\n");
		return -EINVAL;
	}

	return 0;
}

static void cw2217_parse_batt_ntc(struct device_node *np, struct cw2217_para *pdata)
{
	int array_len;
	int i;
	long idata = 0;
	const char *string = NULL;
	int ret;

	if (!np)
		return;
	if (of_property_read_u32(np, "ntc_compensation_is",
		&(pdata->ntc_compensation_is))) {
		hwlog_info("get ntc_compensation_is failed\n");
		return;
	}
	array_len = of_property_count_strings(np, "ntc_temp_compensation_para");
	if ((array_len <= 0) || (array_len % NTC_PARA_TOTAL != 0)) {
		hwlog_err("ntc is invaild,please check ntc_temp_para number\n");
		return;
	}
	if (array_len > NTC_PARA_LEVEL * NTC_PARA_TOTAL) {
		array_len = NTC_PARA_LEVEL * NTC_PARA_TOTAL;
		hwlog_err("temp is too long use only front %d paras\n", array_len);
		return;
	}

	for (i = 0; i < array_len; i++) {
		ret = of_property_read_string_index(np,
			"ntc_temp_compensation_para", i, &string);
		if (ret) {
			hwlog_err("get ntc_temp_compensation_para failed\n");
			return;
		}
		/* 10 means decimalism */
		ret = kstrtol(string, 10, &idata);
		if (ret)
			break;

		switch (i % NTC_PARA_TOTAL) {
		case NTC_PARA_ICHG:
			pdata->ntc_temp_compensation_para[i / NTC_PARA_TOTAL]
				.refer = idata;
			break;
		case NTC_PARA_VALUE:
			pdata->ntc_temp_compensation_para[i / NTC_PARA_TOTAL]
				.comp_value = idata;
			break;
		default:
			hwlog_err("ntc_temp_compensation_para get failed\n");
		}
		hwlog_info("ntc_temp_compensation_para[%d][%d] = %ld\n",
			i / (NTC_PARA_TOTAL), i % (NTC_PARA_TOTAL), idata);
	}
}

int cw2217_parse_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size)
{
#ifdef CONFIG_OF
	int ret;
	struct cw2217_para *pdata = NULL;

	if (!np || !fuel_para || !para_size)
		return -EINVAL;

	pdata = kzalloc(sizeof(struct cw2217_para), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	ret = of_property_read_u32(np, "design_capacity", &pdata->design_capacity);
	if (ret < 0) {
		hwlog_err("get design capacity fail!\n");
		pdata->design_capacity = DESIGN_CAPACITY;
	}
	ret = of_property_read_u32(np, "rsense", &pdata->rsense);
	if (ret < 0 || pdata->rsense == 0) {
		hwlog_err("get rsense fail!\n");
		pdata->rsense = 100; /* default is 1 Milliohm, set 100 to support accuracy 0.01 */
	}
	pdata->coefficient = get_coefficient_from_flash();
	cw2217_parse_batt_ntc(np, pdata);
	ret = cw2217_parse_fg_para(np, batt_model_name, pdata);
	if (ret) {
		hwlog_err("parse_dts: parse fg_para failed\n");
		return ret;
	}

	*fuel_para = pdata;
	*para_size = sizeof(*pdata);
#endif /* CONFIG_OF */
	return 0;
}

