/*
 * bq27z561.c
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
#include <huawei_platform/power/adsp/fuelgauge_ic/bq27z561.h>

#define HWLOG_TAG adsp_bq27z561
HWLOG_REGIST();

#define BQ27Z561_CALI_PRECIE            1000000
#define BQ27Z561_TBATICAL_MIN_A         752000
#define BQ27Z561_TBATICAL_MAX_A         1246000
#define BQ27Z561_TBATCAL_MIN_A          752000
#define BQ27Z561_TBATCAL_MAX_A          1246000

static int bq27z561_parse_th(struct device_node *np, struct bq27z561_para *pdata)
{
	int ret;

	if (!np || !pdata)
		return -1;

	ret = of_property_read_u32_array(np, "fcc_th", pdata->fcc_th, 2);
	if (ret < 0) {
		pdata->fcc_th[0] = 2000;
		pdata->fcc_th[1] = 6000;
	}

	ret = of_property_read_u32_array(np, "qmax_th", pdata->qmax_th, 2);
	if (ret < 0) {
		pdata->qmax_th[0] = 2000;
		pdata->qmax_th[1] = 6000;
	}

	hwlog_err("fcc_th[0]=%d, fcc_th[1]=%d, qmax_th[0]=%d, qmax_th[1]=%d\n",
		pdata->fcc_th[0], pdata->fcc_th[1], pdata->qmax_th[0], pdata->qmax_th[1]);
	return 0;
}

static int bq27z561_parse_fg_para(struct device_node *np, const char *batt_model_name,
	struct bq27z561_para *pdata)
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

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), child_node,
		"fg_para_version", (u32 *)&pdata->fg_para_version,
		FG_PARA_INVALID_VER);
	hwlog_info("fg_para_version = 0x%x\n", pdata->fg_para_version);

	pdata->bqfs_image_size = of_property_count_u8_elems(child_node, "bqfs_image_data");
	if (pdata->bqfs_image_size <= 0)
		return -EINVAL;

	hwlog_info("bqfs_image_size = %d\n", pdata->bqfs_image_size);

	ret = of_property_read_u8_array(child_node, "bqfs_image_data",
		pdata->bqfs_image_data, pdata->bqfs_image_size);

	return ret;
}

static void bq27z561_parse_batt_ntc(struct device_node *np,
	struct bq27z561_para *pdata)
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

static int bq27z561_calibration_para_invalid(int c_gain, int v_gain)
{
	return ((c_gain < BQ27Z561_TBATICAL_MIN_A) ||
		(c_gain > BQ27Z561_TBATICAL_MAX_A) ||
		(v_gain < BQ27Z561_TBATCAL_MIN_A) ||
		(v_gain > BQ27Z561_TBATCAL_MAX_A));
}

static void bq27z561_init_calibration_para(struct bq27z561_para *pdata)
{
	int c_a = BQ27Z561_CALI_PRECIE;
	int v_a = BQ27Z561_CALI_PRECIE;

	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_A, &c_a);
	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_VOL_A, &v_a);

	if (bq27z561_calibration_para_invalid(c_a, v_a)) {
		coul_cali_get_para(COUL_CALI_MODE_MAIN,
			COUL_CALI_PARA_CUR_A, &c_a);
		coul_cali_get_para(COUL_CALI_MODE_MAIN,
			COUL_CALI_PARA_VOL_A, &v_a);
		if (bq27z561_calibration_para_invalid(c_a, v_a)) {
			c_a = BQ27Z561_CALI_PRECIE;
			v_a = BQ27Z561_CALI_PRECIE;
			hwlog_info("all paras invalid use default val\n");
			goto update;
		}
	}

	hwlog_info("c_a %d, v_a %d\n", c_a, v_a);
update:
	pdata->c_gain = c_a;
	pdata->v_gain = v_a;
}


int bq27z561_parse_para(struct device_node *np,
	const char *batt_model_name, void **fuel_para, int *para_size)
{
#ifdef CONFIG_OF
	int ret;
	struct bq27z561_para *pdata = NULL;

	if (!np || !fuel_para || !para_size)
		return -EINVAL;

	pdata = kzalloc(sizeof(struct bq27z561_para), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	bq27z561_parse_batt_ntc(np, pdata);
	bq27z561_parse_th(np, pdata);
	ret = bq27z561_parse_fg_para(np, batt_model_name, pdata);
	if (ret) {
		hwlog_err("parse_dts: parse fg_para failed\n");
		return ret;
	}
	bq27z561_init_calibration_para(pdata);

	*fuel_para = pdata;
	*para_size = sizeof(struct bq27z561_para);

#endif /* CONFIG_OF */
	return 0;
}

