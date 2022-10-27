/*
 * cps4057_dts.c
 *
 * cps4057 dts driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "cps4057.h"

#define HWLOG_TAG wireless_cps4057_dts
HWLOG_REGIST();

static const char * const g_cps4057_rx_fod_summary[] = {
	[RX_FOD_5V_DEFAULT] = "rx_fod_5v_default",
	[RX_FOD_9V_CP62] = "rx_fod_9v_cp62",
	[RX_FOD_9V_CP61] = "rx_fod_9v_cp61",
	[RX_FOD_15V_CP62] = "rx_fod_15v_cp62",
	[RX_FOD_15V_CP61] = "rx_fod_15v_cp61",
};

static void cps4057_parse_tx_fod(struct device_node *np,
	struct cps4057_dev_info *di)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th0", (u32 *)&di->tx_fod.ploss_th0,
		CPS4057_TX_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th1", (u32 *)&di->tx_fod.ploss_th1,
		CPS4057_TX_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th2", (u32 *)&di->tx_fod.ploss_th2,
		CPS4057_TX_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_th3", (u32 *)&di->tx_fod.ploss_th3,
		CPS4057_TX_PLOSS_TH3_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th0", (u32 *)&di->tx_fod.hp_ploss_th0,
		CPS4057_TX_HP_PLOSS_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th1", (u32 *)&di->tx_fod.hp_ploss_th1,
		CPS4057_TX_HP_PLOSS_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_ploss_th2", (u32 *)&di->tx_fod.hp_ploss_th2,
		CPS4057_TX_HP_PLOSS_TH2_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_fod_cur_th0", (u32 *)&di->tx_fod.hp_cur_th0,
		CPS4057_TX_HP_FOD_CUR_TH0_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_hp_fod_cur_th1", (u32 *)&di->tx_fod.hp_cur_th1,
		CPS4057_TX_HP_FOD_CUR_TH1_VAL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"tx_ploss_cnt", (u32 *)&di->tx_fod.ploss_cnt,
		CPS4057_TX_PLOSS_CNT_VAL);
}

static int cps4057_parse_rx_fod(struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_5v", di->rx_fod_5v, CPS4057_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_9v", di->rx_fod_9v, CPS4057_RX_FOD_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_fod_15v", di->rx_fod_15v, CPS4057_RX_FOD_LEN))
		return -EINVAL;

	return 0;
}

static int cps4057_parse_rx_fod_para_info(struct device_node *np,
	struct cps4057_dev_info *di)
{
	int index;

	for (index = RX_FOD_BEGIN; index < RX_FOD_END; index++) {
		if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		g_cps4057_rx_fod_summary[index], di->rx_fod_para_info[index].rx_fod, CPS4057_RX_FOD_LEN))
			return -EINVAL;
		di->rx_fod_para_info[index].tx_fod_index = g_cps4057_rx_fod_summary[index];
	}
	return 0;
}


static int cps4057_parse_rx_fod_para_group(struct device_node *np,
	struct cps4057_dev_info *di)
{
	int array_len, row, col, i, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"fod_para_group", CPS4057_FOD_PARA_MULTI_TX, TX_INFO_TOTAL);
	hwlog_info("%s: array_len=%d\n", __func__, array_len);

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "fod_para_group", i, &tmp_string))
			return -EINVAL;

		row = i / TX_INFO_TOTAL;
		col = i % TX_INFO_TOTAL;

		switch (col) {
		case TX_VERSION:
			if (kstrtoint(tmp_string, POWER_BASE_HEX, &idata))
				return -EINVAL;
			if (idata < CPS4057_TX_VERSION_BASE) {
				hwlog_err("invalid tx version=%d\n", idata);
				return -EINVAL;
			}
			di->multi_tx[row].tx_version = idata;
			break;
		case RX_FOD_5V:
			strncpy(di->multi_tx[row].rx_fod_5v,
				tmp_string, CPS4057_FOD_PARA_STRING_LENGTH - 1);
			break;
		case RX_FOD_9V:
			strncpy(di->multi_tx[row].rx_fod_9v,
				tmp_string, CPS4057_FOD_PARA_STRING_LENGTH - 1);
			break;
		case RX_FOD_15:
			strncpy(di->multi_tx[row].rx_fod_15v,
				tmp_string, CPS4057_FOD_PARA_STRING_LENGTH - 1);
			break;
		default:
			break;
		}
	}

	for (i = 0; i < array_len / TX_INFO_TOTAL; i++) {
		hwlog_info("%s:0x%x, %s, %s, %s\n", __func__, di->multi_tx[i].tx_version,
			di->multi_tx[i].rx_fod_5v, di->multi_tx[i].rx_fod_9v, di->multi_tx[i].rx_fod_15v);
	}
	return 0;
}

static int cps4057_parse_ldo_cfg(struct device_node *np,
	struct cps4057_dev_info *di)
{
	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_5v", di->rx_ldo_cfg_5v, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_9v", di->rx_ldo_cfg_9v, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	if (power_dts_read_u8_array(power_dts_tag(HWLOG_TAG), np,
		"rx_ldo_cfg_sc", di->rx_ldo_cfg_sc, CPS4057_RX_LDO_CFG_LEN))
		return -EINVAL;

	return 0;
}

int cps4057_parse_dts(struct device_node *np, struct cps4057_dev_info *di)
{
	int ret;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"rx_ss_good_lth", (u32 *)&di->rx_ss_good_lth,
		CPS4057_RX_SS_MAX);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_en_valid_val", (u32 *)&di->gpio_en_valid_val, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"full_bridge_ith", (u32 *)&di->full_bridge_ith,
		CPS4057_FULL_BRIDGE_ITH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
			"support_multi_tx",(u32 *)&di->support_multi_tx, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"irq_no_suspend",(u32 *)&di->irq_no_suspend, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"product_isolation",(u32 *)&di->product_isolation, 0);
	cps4057_parse_tx_fod(np, di);

	if (!di->support_multi_tx)
		ret = cps4057_parse_rx_fod(np, di);
	else {
		ret = cps4057_parse_rx_fod_para_info(np, di);
		ret |= cps4057_parse_rx_fod_para_group(np, di);
	}
	if (ret) {
		hwlog_err("parse_dts: parse rx_fod para failed\n");
		return ret;
	}

	ret = cps4057_parse_ldo_cfg(np, di);
	if (ret) {
		hwlog_err("parse_dts: parse ldo cfg failed\n");
		return ret;
	}
	return 0;
}
