/*
 * rt9426a.c
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
#include <huawei_platform/power/adsp/fuelgauge_ic/rt9426a.h>

#define HWLOG_TAG adsp_rt9426a
HWLOG_REGIST();

#define MAX_BATT_NAME				32
#define NTC_PARA_LEVEL				20
#define RT9426A_DTSI_VER_SIZE		2
#define RT9426A_SOC_OFFSET_SIZE		2
#define MAX_SOC_OFFSET_DATA_LEN		2
#define RT9426A_ICC_THRESHOLD_SIZE	3
#define RT9426A_IEOC_SETTING_SIZE	4

#define RT9426A_OFFSET_INTERPLO_SIZE	2
#define RT9426A_EXTREG_TABLE_SIZE		14
#define RT9426A_EXTREG_TABLE_LENTH		16
#define RT9426A_DT_OFFSET_PARA_SIZE		3
#define RT9426A_DT_EXTREG_PARA_SIZE 	3

#define RT9426A_OCV_ROW_SIZE		10
#define RT9426A_OCV_COL_SIZE		8
#define RT9426A_MAX_AGING_STAGES	5
#define RT9426A_OCV_TABEL_SIZE		80

#define RT9426A_REG_CNTL			0x00
#define RT9426A_REG_RSVD			0x02
#define RT9426A_REG_CURR			0x04
#define RT9426A_REG_TEMP			0x06
#define RT9426A_REG_VBAT			0x08
#define RT9426A_REG_FLAG1			0x0A
#define RT9426A_REG_FLAG2			0x0C
#define RT9426A_REG_DEVICE_ID		0x0E
#define RT9426A_REG_RM				0x10
#define RT9426A_REG_FCC				0x12
#define RT9426A_REG_AI				0x14
#define RT9426A_REG_BCCOMP			0x1A
#define RT9426A_REG_DUMMY			0x1E
#define RT9426A_REG_INTT			0x28
#define RT9426A_REG_CYC				0x2A
#define RT9426A_REG_SOC				0x2C
#define RT9426A_REG_SOH				0x2E
#define RT9426A_REG_FLAG3			0x30
#define RT9426A_REG_IRQ				0x36
#define RT9426A_REG_ADV				0x3A
#define RT9426A_REG_DC				0x3C
#define RT9426A_REG_BDCNTL			0x3E
#define RT9426A_REG_SWINDOW1		0x40
#define RT9426A_REG_SWINDOW2		0x42
#define RT9426A_REG_SWINDOW3		0x44
#define RT9426A_REG_SWINDOW4		0x46
#define RT9426A_REG_SWINDOW5		0x48
#define RT9426A_REG_SWINDOW6		0x4A
#define RT9426A_REG_SWINDOW7		0x4C
#define RT9426A_REG_SWINDOW8		0x4E
#define RT9426A_REG_SWINDOW9		0x50
#define RT9426A_REG_OCV				0x62
#define RT9426A_REG_AV				0x64
#define RT9426A_REG_AT				0x66
#define RT9426A_REG_TOTAL_CHKSUM	0x68
#define RT9426A_REG_RSVD2			0x7C

#define RT9426A_VOLT_SOURCE_NONE	0
#define RT9426A_VOLT_SOURCE_VBAT	1
#define RT9426A_VOLT_SOURCE_OCV		2
#define RT9426A_VOLT_SOURCE_AV		3
#define RT9426A_TEMP_SOURCE_NONE	0
#define RT9426A_TEMP_SOURCE_TEMP	1
#define RT9426A_TEMP_SOURCE_INIT	2
#define RT9426A_TEMP_SOURCE_AT		3
#define RT9426A_CURR_SOURCE_NONE	0
#define RT9426A_CURR_SOURCE_CURR	1
#define RT9426A_CURR_SOURCE_AI		2

#define NTC_PARA_LEVEL				20

#define RT9426A_DESIGN_FCC_VAL		2000
#define RT9426A_FC_VTH_DEFAULT_VAL	0x0078
#define RT9426A_GAIN_DEFAULT_VAL	128
#define RT9426A_GAIN_BASE_VAL		512
#define RT9426A_COUL_DEFAULT_VAL	1000000
#define RT9426A_COUL_OFFSET_VAL     1000
#define RT9426A_GAIN_DEFAULT_VAL	128
#define RT9426A_GAIN_BASE_VAL		512
#define RT9426A_TBATICAL_MIN_A		752000
#define RT9426A_TBATICAL_MAX_A		1246000
#define RT9426A_TBATCAL_MIN_A		752000
#define RT9426A_TBATCAL_MAX_A		1246000
#define RT9426A_TBATICAL_MIN_B      (-20000)
#define RT9426A_TBATICAL_MAX_B      20000


enum comp_offset_typs {
	FG_COMP = 0,
	SOC_OFFSET,
	EXTREG_UPDATE,
};

enum ntc_temp_compensation_para_info {
	NTC_PARA_ICHG = 0,
	NTC_PARA_VALUE,
	NTC_PARA_TOTAL,
};

struct compensation_para {
	int refer;
	int comp_value;
};

struct data_point {
	union {
		int x;
		int voltage;
		int soc;
	};
	union {
		int y;
		int temperature;
	};
	union {
		int z;
		int curr;
	};
	union {
		int w;
		int offset;
	};
};

struct soc_offset_table {
	int soc_voltnr;
	int tempnr;
	struct data_point soc_offset_data[MAX_SOC_OFFSET_DATA_LEN];
};

struct fg_extreg_table {
	u8 data[RT9426A_EXTREG_TABLE_LENTH];
};

struct dt_offset_params {
	int data[RT9426A_DT_OFFSET_PARA_SIZE];
};

struct dt_extreg_params {
	int edata[RT9426A_DT_EXTREG_PARA_SIZE];
};

struct fg_fcc_aging_params {
	u32 fcc;
	u32 fc_vth;
	unsigned int ocv_table[RT9426A_OCV_ROW_SIZE][RT9426A_OCV_COL_SIZE];
};

struct rt9426a_para {
	char batt_name[MAX_BATT_NAME];
	int force_use_aux_cali_para;
	int ntc_compensation_is;
	struct compensation_para ntc_temp_compensation_para[NTC_PARA_LEVEL];

	u32 dtsi_version[RT9426A_DTSI_VER_SIZE];
	u32 para_version;
	int soc_offset_size[RT9426A_SOC_OFFSET_SIZE];
	struct soc_offset_table soc_offset;
	int offset_interpolation_order[RT9426A_OFFSET_INTERPLO_SIZE];
	struct fg_extreg_table extreg_table[RT9426A_EXTREG_TABLE_SIZE];
	int battery_type;

	u32 temp_source;
	u32 volt_source;
	u32 curr_source;

	/* current scaling in unit of 0.01 Ohm */
	u32 rs_ic_setting;
	u32 rs_schematic;
	int smooth_soc_en;

	/* update ieoc setting by dtsi for icc_sts = 0~4 */
	u32 icc_threshold[RT9426A_ICC_THRESHOLD_SIZE];
	u32 ieoc_setting[RT9426A_IEOC_SETTING_SIZE];

	/* calibration para */
	int volt_gain;
	int curr_gain;
	int curr_offset;

	/* fcc aging para */
	struct fg_fcc_aging_params aging_para;
};

struct rt9426a_cali_info {
	int para_type;
	int fg_curr;
	int fg_vol;
	int fg_curr_gain;
	int fg_curr_offset;
	int fg_vol_gain;
	int fg_vol_offset;
};

static void new_vgcomp_soc_offset_datas(int type,
	struct rt9426a_para *pdata, int size_x, int size_y, int size_z)
{
	switch (type) {
	case SOC_OFFSET:
		memset(pdata->soc_offset.soc_offset_data, 0, 
			sizeof(pdata->soc_offset.soc_offset_data));
		pdata->soc_offset.soc_voltnr = size_x;
		pdata->soc_offset.tempnr = size_y;
		break;
	default:
		WARN_ON(1);
	}
}

static int rt9426a_parse_battery_para(struct device_node *child_node, struct rt9426a_para *pdata)
{
#ifdef CONFIG_OF
	int ret;
	int i;
	int j;
	int sizes[RT9426A_SOC_OFFSET_SIZE + 1] = { 0 };
	struct dt_offset_params *offset_params;
	char prop_name[64] = {0};
	u32 fcc[RT9426A_MAX_AGING_STAGES];
	u32 fc_vth[RT9426A_MAX_AGING_STAGES];
	unsigned int ocv_table[RT9426A_MAX_AGING_STAGES][RT9426A_OCV_ROW_SIZE][RT9426A_OCV_COL_SIZE];

	if (!child_node || !pdata)
		return -EINVAL;

	ret = of_property_read_u32_array(child_node, "rt,dtsi_version",
		pdata->dtsi_version, RT9426A_DTSI_VER_SIZE);
	if (ret < 0)
		pdata->dtsi_version[0] = pdata->dtsi_version[1] = 0;
	hwlog_info("dtsi_version: %u, %u\n", pdata->dtsi_version[0], pdata->dtsi_version[1]);

	ret = of_property_read_u32(child_node, "rt,para_version",
		&pdata->para_version);
	if (ret < 0)
		pdata->para_version = 0;
	hwlog_info("para_version: %u\n", pdata->para_version);

	ret = of_property_read_u32(child_node, "rt,battery_type",
		&pdata->battery_type);
	if (ret < 0) {
		hwlog_err("uset default battery_type 4350mV, EDV=3200mV\n");
		pdata->battery_type = 4352;
	}
	hwlog_info("battery_type: %u\n", pdata->battery_type);

	ret = of_property_read_u32(child_node, "rt,volt_source",
		&pdata->volt_source);
	if (ret < 0)
		pdata->volt_source = RT9426A_REG_AV;

	if (pdata->volt_source == RT9426A_VOLT_SOURCE_NONE) {
		pdata->volt_source = 0;
	} else if (pdata->volt_source == RT9426A_VOLT_SOURCE_VBAT) {
		pdata->volt_source = RT9426A_REG_VBAT;
	} else if (pdata->volt_source == RT9426A_VOLT_SOURCE_OCV) {
		pdata->volt_source = RT9426A_REG_OCV;
	} else if (pdata->volt_source == RT9426A_VOLT_SOURCE_AV) {
		pdata->volt_source = RT9426A_REG_AV;
	} else {
		hwlog_err("pdata->volt_source is out of range, use 3\n");
		pdata->volt_source = RT9426A_REG_AV;
	}
	hwlog_info("volt_source: %u\n", pdata->volt_source);

	ret = of_property_read_u32(child_node, "rt,temp_source",
		&pdata->temp_source);
	if (ret < 0)
		pdata->temp_source = 0;

	if (pdata->temp_source == RT9426A_TEMP_SOURCE_NONE) {
		pdata->temp_source = 0;
	} else if (pdata->temp_source == RT9426A_TEMP_SOURCE_TEMP) {
		pdata->temp_source = RT9426A_REG_TEMP;
	} else if (pdata->temp_source == RT9426A_TEMP_SOURCE_INIT) {
		pdata->temp_source = RT9426A_REG_INTT;
	} else if (pdata->temp_source == RT9426A_TEMP_SOURCE_AT) {
		pdata->temp_source = RT9426A_REG_AT;
	} else {
		hwlog_err("pdata->temp_source is out of range, use 0\n");
		pdata->temp_source = 0;
	}
	hwlog_info("temp_source: %u\n", pdata->temp_source);

	ret = of_property_read_u32(child_node, "rt,curr_source",
		&pdata->curr_source);
	if (ret < 0)
		pdata->curr_source = 0;

	if (pdata->curr_source == RT9426A_CURR_SOURCE_NONE) {
		pdata->curr_source = 0;
	} else if (pdata->curr_source == RT9426A_CURR_SOURCE_CURR) {
		pdata->curr_source = RT9426A_REG_CURR;
	} else if (pdata->curr_source == RT9426A_CURR_SOURCE_AI) {
		pdata->curr_source = RT9426A_REG_AI;
	} else {
		hwlog_err("pdata->curr_source is out of range, use 2\n");
		pdata->curr_source = RT9426A_REG_AI;
	}
	hwlog_info("curr_source: %u\n", pdata->curr_source);

	ret = of_property_read_u32_array(child_node,
		"rt,offset_interpolation_order",
		pdata->offset_interpolation_order,
		RT9426A_OFFSET_INTERPLO_SIZE);
	if (ret < 0)
		pdata->offset_interpolation_order[0] =
			pdata->offset_interpolation_order[1] = 2;
	hwlog_info("offset_interpolation_order: %u, %u\n", 
		pdata->offset_interpolation_order[0], pdata->offset_interpolation_order[1]);

	sizes[0] = sizes[1] = 0;
	ret = of_property_read_u32_array(child_node, "rt,soc_offset_size",
		sizes, RT9426A_SOC_OFFSET_SIZE);
	if (ret < 0)
		hwlog_err("Can't get prop soc_offset_size(%d)\n", ret);

	new_vgcomp_soc_offset_datas(SOC_OFFSET, pdata, sizes[0],
		sizes[1], 0);
	offset_params = kzalloc(sizes[0] * sizes[1] *
		sizeof(struct dt_offset_params), GFP_KERNEL);
	if (!offset_params)
		return -1;

	of_property_read_u32_array(child_node, "rt,soc_offset_data",
		(u32 *)offset_params, sizes[0] * sizes[1] *
		(RT9426A_SOC_OFFSET_SIZE + 1));
	for (j = 0; j < sizes[0] * sizes[1]; j++) {
		pdata->soc_offset.soc_offset_data[j].x =
			offset_params[j].data[0];
		pdata->soc_offset.soc_offset_data[j].y =
			offset_params[j].data[1];
		pdata->soc_offset.soc_offset_data[j].offset =
			offset_params[j].data[2];
		hwlog_info("soc_offset_size[%d]: %d, %d, %d\n", j,
			pdata->soc_offset.soc_offset_data[j].x,
			pdata->soc_offset.soc_offset_data[j].y,
			pdata->soc_offset.soc_offset_data[j].offset);
	}
	kfree(offset_params);
	/*  Read Ext. Reg Table for RT9426A  */
	ret = of_property_read_u8_array(child_node, "rt,fg_extreg_table",
		(u8 *)pdata->extreg_table, RT9426A_EXTREG_TABLE_SIZE * RT9426A_EXTREG_TABLE_LENTH);
	if (ret < 0) {
		hwlog_err("no ocv table property\n");
		for (i = 0; i < RT9426A_EXTREG_TABLE_SIZE; i++)
			for (j = 0; j < RT9426A_EXTREG_TABLE_LENTH; j++)
				pdata->extreg_table[i].data[j] = 0;
	}
	hwlog_info("fg_extreg_table: \n");
	for (i = 0; i < RT9426A_EXTREG_TABLE_SIZE; i++) {
		for (j = 0; j < RT9426A_EXTREG_TABLE_LENTH; j++)
			hwlog_info("%u, ", pdata->extreg_table[i].data[j]);
		hwlog_info("\n");
	}

	/* parse fcc array by 5 element */
	ret = of_property_read_u32_array(child_node, "rt,fcc", fcc, RT9426A_MAX_AGING_STAGES);
	if (ret < 0) {
		hwlog_err("no FCC property, use defaut 2000\n");
		for (i = 0; i < RT9426A_MAX_AGING_STAGES; i++)
			fcc[i] = RT9426A_DESIGN_FCC_VAL;
	}

	/* parse fc_vth array by 5 element */
	ret = of_property_read_u32_array(child_node, "rt,fg_fc_vth", fc_vth, RT9426A_MAX_AGING_STAGES);
	if (ret < 0) {
		hwlog_err("no fc_vth property, use default 4200mV\n");
		for (i = 0; i < RT9426A_MAX_AGING_STAGES; i++)
			fc_vth[i] = RT9426A_FC_VTH_DEFAULT_VAL;
	}
	/* for smooth_soc */
	of_property_read_u32(child_node, "rt,smooth_soc_en",
		&pdata->smooth_soc_en);
	hwlog_info("smooth_soc_en = %d\n", pdata->smooth_soc_en);

	of_property_read_u32(child_node, "rt,rs_ic_setting",
		&pdata->rs_ic_setting);
	of_property_read_u32(child_node, "rt,rs_schematic",
		&pdata->rs_schematic);
	hwlog_info("rs_ic_setting = %d\n", pdata->rs_ic_setting);
	hwlog_info("rs_schematic = %d\n", pdata->rs_schematic);

	/* for update ieoc setting */
	ret = of_property_read_u32_array(child_node, "rt,icc_threshold",
		pdata->icc_threshold, RT9426A_ICC_THRESHOLD_SIZE);
	if (ret < 0) {
		hwlog_err("no icc threshold property reset to 0\n");
		for (i = 0; i < RT9426A_ICC_THRESHOLD_SIZE; i++)
			pdata->icc_threshold[i] = 0;
	}
	hwlog_info("icc_threshold: ");
	for (i = 0; i < RT9426A_ICC_THRESHOLD_SIZE; i++)
		hwlog_info("%u, ", pdata->icc_threshold[i]);
	hwlog_info("\n");
	
	ret = of_property_read_u32_array(child_node, "rt,ieoc_setting",
		pdata->ieoc_setting, RT9426A_IEOC_SETTING_SIZE);
	if (ret < 0) {
		hwlog_err("no ieoc setting property, reset to 0\n");
		for (i = 0; i < RT9426A_IEOC_SETTING_SIZE; i++)
			pdata->ieoc_setting[i] = 0;
	}
	hwlog_info("ieoc_setting: ");
	for (i = 0; i < RT9426A_IEOC_SETTING_SIZE; i++)
		hwlog_info("%u, ", pdata->ieoc_setting[i]);
	hwlog_info("\n");

	/* parse ocv_table array by 80x5 element */
	for (i = 0; i < RT9426A_MAX_AGING_STAGES; i++) {
		snprintf(prop_name, 64, "rt,fg_ocv_table%d", i);
		ret = of_property_read_u32_array(child_node, prop_name,
			(u32 *)ocv_table + i * RT9426A_OCV_TABEL_SIZE, RT9426A_OCV_TABEL_SIZE);
		if (ret < 0)
			memset32((u32 *)ocv_table + i * RT9426A_OCV_TABEL_SIZE, 0, RT9426A_OCV_TABEL_SIZE);
	}

	/* select first stage */
	pdata->aging_para.fcc = fcc[0];
	hwlog_info("fcc: %u\n", pdata->aging_para.fcc);
	pdata->aging_para.fc_vth = fc_vth[0];
	hwlog_info("fc_vth: %u\n", pdata->aging_para.fc_vth);
	hwlog_info("ocv_table: \n");
	for (i = 0; i < RT9426A_OCV_ROW_SIZE; i++) {
		for (j = 0; j < RT9426A_OCV_COL_SIZE; j++) {
			pdata->aging_para.ocv_table[i][j] = ocv_table[0][i][j];
			hwlog_info("%u, ", pdata->aging_para.ocv_table[i][j]);
		}
		hwlog_info("\n");
	}
#endif /* CONFIG_OF */
	return 0;
}

static void rt9426a_parse_batt_ntc(struct device_node *np, struct rt9426a_para *pdata)
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
		hwlog_err("temp is too long use only front %d paras\n",
			array_len);
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

static int rt9426a_calibration_para_invalid(int c_gain, int v_gain, int c_offset)
{
	return ((c_gain < RT9426A_TBATICAL_MIN_A) ||
		(c_gain > RT9426A_TBATICAL_MAX_A) ||
		(v_gain < RT9426A_TBATCAL_MIN_A) ||
		(v_gain > RT9426A_TBATCAL_MAX_A) ||
		(c_offset < RT9426A_TBATICAL_MIN_B) ||
		(c_offset > RT9426A_TBATICAL_MAX_B));
}

static int rt9426a_get_data_from_int(int val)
{
	return RT9426A_GAIN_DEFAULT_VAL + (((s64)(val) * RT9426A_GAIN_BASE_VAL) /
		RT9426A_COUL_DEFAULT_VAL - RT9426A_GAIN_BASE_VAL);
}

static int rt9426a_get_offset_from_int(struct rt9426a_para *pdata, int val)
{
	if (pdata->rs_ic_setting) {
		hwlog_err("%s %d \n", __func__, pdata->rs_ic_setting);
		return RT9426A_GAIN_DEFAULT_VAL;
	}

	return RT9426A_GAIN_DEFAULT_VAL +
		((s64)(val) * pdata->rs_schematic / pdata->rs_ic_setting) / RT9426A_COUL_OFFSET_VAL;
}

static void rt9426a_init_cali_para(struct device_node *np, struct rt9426a_para *pdata)
{
	int c_a = 0;
	int v_a = 0;
	int c_offset = 0;

	pdata->curr_gain = RT9426A_GAIN_DEFAULT_VAL;
	pdata->volt_gain = RT9426A_GAIN_DEFAULT_VAL;
	pdata->curr_offset = RT9426A_GAIN_DEFAULT_VAL;

	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_A, &c_a);
	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_VOL_A, &v_a);
	coul_cali_get_para(COUL_CALI_MODE_AUX, COUL_CALI_PARA_CUR_B, &c_offset);

	pdata->force_use_aux_cali_para =
		of_property_read_bool(np, "force_use_aux_cali_para");

	if (pdata->force_use_aux_cali_para && (c_a != 0) && (v_a != 0)) {
		hwlog_info("force_use_aux_cali_para\n");

		if (c_a < RT9426A_TBATICAL_MIN_A)
			c_a = RT9426A_TBATICAL_MIN_A;
		else if (c_a > RT9426A_TBATICAL_MAX_A)
			c_a = RT9426A_TBATICAL_MAX_A;

		if (c_offset < RT9426A_TBATICAL_MIN_B || c_offset > RT9426A_TBATICAL_MAX_B)
			c_offset = 0;
		if (v_a < RT9426A_TBATCAL_MIN_A)
			v_a = RT9426A_TBATCAL_MIN_A;
		else if (v_a > RT9426A_TBATCAL_MAX_A)
			v_a = RT9426A_TBATCAL_MAX_A;
	}

	if (rt9426a_calibration_para_invalid(c_a, v_a, c_offset)) {
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_CUR_A, &c_a);
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_VOL_A, &v_a);
		coul_cali_get_para(COUL_CALI_MODE_MAIN, COUL_CALI_PARA_CUR_B, &c_offset);
		if (rt9426a_calibration_para_invalid(c_a, v_a, c_offset))
			return;
	}

	pdata->curr_gain = rt9426a_get_data_from_int(c_a);
	pdata->volt_gain = rt9426a_get_data_from_int(v_a);
	pdata->curr_offset = rt9426a_get_offset_from_int(pdata, c_offset);

	hwlog_info("c_gain %d, v_gain %d, c_offset %d\n", pdata->curr_gain, pdata->volt_gain, pdata->curr_offset);
}

int rt9426a_parse_para(struct device_node *np, 
	const char *batt_model_name, void **fuel_para, int *para_size)
{
#ifdef CONFIG_OF
	struct rt9426a_para *pdata = NULL;
	const char *battery_name = NULL;
	struct device_node *child_node = NULL;

	if (!np || !fuel_para || !para_size)
		return -EINVAL;

	pdata = kzalloc(sizeof(struct rt9426a_para), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	for_each_child_of_node(np, child_node) {
		if (power_dts_read_string(power_dts_tag(HWLOG_TAG),
			child_node, "batt_name", &battery_name)) {
			hwlog_info("childnode without batt_name property");
			continue;
		}
		if (!battery_name)
			continue;
		hwlog_info("search battery data, battery_name: %s\n",
			battery_name);
		if (!batt_model_name || !strcmp(battery_name, batt_model_name))
			break;
	}

	if (!child_node) {
		hwlog_err("cannt match batt model: %s\n", batt_model_name);
		return -EINVAL;
	}

	if (!pdata)
		return -EINVAL;

	snprintf(pdata->batt_name, MAX_BATT_NAME, "%s", batt_model_name);
	rt9426a_parse_battery_para(child_node, pdata);
	rt9426a_parse_batt_ntc(np, pdata);

	rt9426a_init_cali_para(np, pdata);

	*fuel_para = pdata;
	*para_size = sizeof(struct rt9426a_para);

#endif /* CONFIG_OF */
	return 0;
}

int rt9426a_parse_aging_para(struct device_node *np, 
	const char *batt_model_name, void **fuel_para, int *para_size)
{
	return 0;
}

static int rt9426a_get_calibration_curr(int *val, void *dev_data)
{
	struct rt9426a_cali_info info;

	info.para_type = COUL_CALI_PARA_CUR;
	if (hihonor_oem_glink_oem_get_prop(BATTERY_OEM_FG_CALI_INFO, &info, sizeof(info))) {
		hwlog_err("%s, get cali info fail\n", __func__);
		return -1;
	}
	*val = info.fg_curr;
	return 0;
}

static int rt9426a_get_calibration_vol(int *val, void *dev_data)
{
	struct rt9426a_cali_info info;

	info.para_type = COUL_CALI_PARA_VOL;
	if (hihonor_oem_glink_oem_get_prop(BATTERY_OEM_FG_CALI_INFO, &info, sizeof(info))) {
		hwlog_err("%s, get cali info fail\n", __func__);
		return -1;
	}
	*val = info.fg_vol;
	return 0;
}

static int rt9426a_set_current_gain(unsigned int val, void *dev_data)
{
	struct rt9426a_cali_info info = {
		.para_type = COUL_CALI_PARA_CUR_A,
		.fg_curr_gain = val,
	};

	if (hihonor_oem_glink_oem_set_prop(BATTERY_OEM_FG_CALI_INFO, &info, sizeof(info))) {
		hwlog_err("%s, set cali info fail\n", __func__);
		return -1;
	}
	return 0;
}

static int rt9426a_set_current_offset(int val, void *dev_data)
{
	struct rt9426a_cali_info info = {
		.para_type = COUL_CALI_PARA_CUR_B,
		.fg_curr_gain = val,
	};

	if (hihonor_oem_glink_oem_set_prop(BATTERY_OEM_FG_CALI_INFO, &info, sizeof(info))) {
		hwlog_err("%s, set cali info fail\n", __func__);
		return -1;
	}
	return 0;
}

static int rt9426a_set_voltage_gain(unsigned int val, void *dev_data)
{
	struct rt9426a_cali_info info = {
		.para_type = COUL_CALI_PARA_VOL_A,
		.fg_curr_gain = val,
	};

	if (hihonor_oem_glink_oem_set_prop(BATTERY_OEM_FG_CALI_INFO, &info, sizeof(info))) {
		hwlog_err("%s, set cali info fail\n", __func__);
		return -1;
	}
	return 0;
}

static int rt9426a_enable_cali_mode(int enable, void *dev_data)
{
	if (hihonor_oem_glink_oem_set_prop(BATTERY_OEM_FG_CALI_MODE, &enable, sizeof(enable))) {
		hwlog_err("%s, set cali mode fail\n", __func__);
		return -1;
	}
	return 0;
}

struct coul_cali_ops rt9426a_cali_ops = {
	.dev_name = "aux",
	.get_current = rt9426a_get_calibration_curr,
	.get_voltage = rt9426a_get_calibration_vol,
	.set_current_gain = rt9426a_set_current_gain,
	.set_current_offset = rt9426a_set_current_offset,
	.set_voltage_gain = rt9426a_set_voltage_gain,
	.set_cali_mode = rt9426a_enable_cali_mode,
};

