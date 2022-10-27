/*
 * adsp_dc_dts_parse.c
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

#include <log/hw_log.h>
#include <huawei_platform/power/adsp/direct_charger_adsp.h>
#include <chipset_common/hwpower/power_common.h>
#include <chipset_common/hwpower/power_cmdline.h>
#include <chipset_common/hwpower/power_printk.h>
#include <chipset_common/hwpower/power_dts.h>

#define HWLOG_TAG adsp_dc_dts_parse
HWLOG_REGIST();

#define POWER_BASE_DEC 10

static void dc_parse_gain_current_para(struct device_node *np,
	struct dc_sc_config *conf)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gain_curr_10v2a", &conf->gain_curr_10v2a, 0);
}

static void dc_parse_adp_cur_para(struct device_node *np,
	struct dc_adp_cur_para *data, const char *name)
{
	int row, col, len;
	int idata[DC_ADP_CUR_LEVEL * DC_ADP_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, idata, DC_ADP_CUR_LEVEL, DC_ADP_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_ADP_TOTAL; row++) {
		col = row * DC_ADP_TOTAL + DC_ADP_VOL_MIN;
		data[row].vol_min = idata[col];
		col = row * DC_ADP_TOTAL + DC_ADP_VOL_MAX;
		data[row].vol_max = idata[col];
		col = row * DC_ADP_TOTAL + DC_ADP_CUR_TH;
		data[row].cur_th = idata[col];
	}
}

static void dc_parse_adapter_antifake_para(struct device_node *np, struct dc_sc_config *conf)
{
	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_check_enable",
		&conf->adp_antifake_enable, 0))
		return;

	if (conf->adp_antifake_enable != 1) {
		conf->adp_antifake_enable = 0;
		return;
	}

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_antifake_key_index",
		&conf->adp_antifake_key_index, 1);

	/* set public key as default key in factory mode */
	if (power_cmdline_is_factory_mode())
		conf->adp_antifake_key_index = 1;

	hwlog_info("adp_antifake_key_index=%d\n",
		conf->adp_antifake_key_index);
}

static int dc_parse_volt_para(struct device_node *np, struct dc_sc_config *conf, int group)
{
	int i, row, col, len;
	int idata[DC_VOLT_LEVEL * DC_PARA_VOLT_TOTAL] = { 0 };
	int j = group;
	char *volt_para = NULL;

	volt_para = conf->orig_volt_para[j].bat_info.volt_para_index;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		volt_para, idata, DC_VOLT_LEVEL, DC_PARA_VOLT_TOTAL);
	if (len < 0)
		return -EINVAL;

	conf->orig_volt_para[j].stage_size = len / DC_PARA_VOLT_TOTAL;
	hwlog_info("%s stage_size=%d\n", volt_para,
		conf->orig_volt_para[j].stage_size);

	for (row = 0; row < len / DC_PARA_VOLT_TOTAL; row++) {
		col = row * DC_PARA_VOLT_TOTAL + DC_PARA_VOL_TH;
		conf->orig_volt_para[j].volt_info[row].vol_th = idata[col];
		col = row * DC_PARA_VOLT_TOTAL + DC_PARA_CUR_TH_HIGH;
		conf->orig_volt_para[j].volt_info[row].cur_th_high = idata[col];
		col = row * DC_PARA_VOLT_TOTAL + DC_PARA_CUR_TH_LOW;
		conf->orig_volt_para[j].volt_info[row].cur_th_low = idata[col];
	}

	conf->orig_volt_para[j].bat_info.parse_ok = 1;

	for (i = 0; i < conf->orig_volt_para[j].stage_size; i++)
		hwlog_info("%s[%d]=%d %d %d\n", volt_para, i,
			conf->orig_volt_para[j].volt_info[i].vol_th,
			conf->orig_volt_para[j].volt_info[i].cur_th_high,
			conf->orig_volt_para[j].volt_info[i].cur_th_low);

	return 0;
}

static void dc_parse_group_volt_para(struct device_node *np, struct dc_sc_config *conf)
{
	int i;

	for (i = 0; i < conf->stage_group_size; i++) {
		if (dc_parse_volt_para(np, conf, i))
			return;
	}
}

static void dc_parse_temp_cv_para(struct device_node *np, struct dc_sc_config *conf)
{
	int row, col, len;
	int idata[DC_TEMP_CV_LEVEL * DC_TEMP_CV_TOTOL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"temp_cv_para", idata, DC_TEMP_CV_LEVEL, DC_TEMP_CV_TOTOL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_TEMP_CV_TOTOL; row++) {
		col = row * DC_TEMP_CV_TOTOL + DC_TEMP_CV_MIN;
		conf->temp_cv_para[row].temp_min = idata[col];
		col = row * DC_TEMP_CV_TOTOL + DC_TEMP_CV_MAX;
		conf->temp_cv_para[row].temp_max = idata[col];
		col = row * DC_TEMP_CV_TOTOL + DC_TEMP_CV_CUR;
		conf->temp_cv_para[row].cv_curr = idata[col];
	}
}

static void dc_parse_bat_para(struct device_node *np, struct dc_sc_config *conf)
{
	int i, row, col, array_len, idata;
	const char *tmp_string = NULL;

	array_len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"bat_para", DC_VOLT_GROUP_MAX, DC_PARA_BAT_TOTAL);
	if (array_len < 0) {
		conf->stage_group_size = 1;
		 /* default temp_high is 45 centigrade */
		conf->orig_volt_para[0].bat_info.temp_high = 45;
		 /* default temp_low is 10 centigrade */
		conf->orig_volt_para[0].bat_info.temp_low = 10;
		strncpy(conf->orig_volt_para[0].bat_info.batid,
			"default", DC_BAT_BRAND_LEN_MAX - 1);
		strncpy(conf->orig_volt_para[0].bat_info.volt_para_index,
			"volt_para", DC_VOLT_NODE_LEN_MAX - 1);
		return;
	}

	conf->stage_group_size = array_len / DC_PARA_BAT_TOTAL;

	for (i = 0; i < array_len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
			np, "bat_para", i, &tmp_string))
			return;

		row = i / DC_PARA_BAT_TOTAL;
		col = i % DC_PARA_BAT_TOTAL;

		switch (col) {
		case DC_PARA_BAT_ID:
			strncpy(conf->orig_volt_para[row].bat_info.batid,
				tmp_string, DC_BAT_BRAND_LEN_MAX - 1);
			break;
		case DC_PARA_TEMP_LOW:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return;

			/* must be (0, 50) centigrade */
			if (idata < 0 || idata > 50) {
				hwlog_err("invalid temp_low=%d\n", idata);
				return;
			}

			conf->orig_volt_para[row].bat_info.temp_low = idata;
			break;
		case DC_PARA_TEMP_HIGH:
			if (kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				return;

			/* must be (0, 50) centigrade */
			if (idata < 0 || idata > 50) {
				hwlog_err("invalid temp_high=%d\n", idata);
				return;
			}

			conf->orig_volt_para[row].bat_info.temp_high = idata;
			break;
		case DC_PARA_INDEX:
			strncpy(conf->orig_volt_para[row].bat_info.volt_para_index,
				tmp_string, DC_VOLT_NODE_LEN_MAX - 1);
			break;
		default:
			break;
		}
	}

	for (i = 0; i < conf->stage_group_size; i++)
		hwlog_info("bat_para[%d]=%d %d %s\n", i,
			conf->orig_volt_para[i].bat_info.temp_low,
			conf->orig_volt_para[i].bat_info.temp_high,
			conf->orig_volt_para[i].bat_info.volt_para_index);
}

static void dc_parse_resist_para(struct device_node *np,
	struct dc_resist_para *data, const char *name)
{
	int row, col, len;
	int idata[DC_RESIST_LEVEL * DC_RESIST_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, idata, DC_RESIST_LEVEL, DC_RESIST_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_RESIST_TOTAL; row++) {
		col = row * DC_RESIST_TOTAL + DC_RESIST_MIN;
		data[row].resist_min = idata[col];
		col = row * DC_RESIST_TOTAL + DC_RESIST_MAX;
		data[row].resist_max = idata[col];
		col = row * DC_RESIST_TOTAL + DC_RESIST_CUR_MAX;
		data[row].resist_cur_max = idata[col];
	}
}

static void dc_parse_temp_para(struct device_node *np, struct dc_sc_config *conf)
{
	int row, col, len;
	int idata[DC_TEMP_LEVEL * DC_TEMP_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"temp_para", idata, DC_TEMP_LEVEL, DC_TEMP_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_TEMP_TOTAL; row++) {
		col = row * DC_TEMP_TOTAL + DC_TEMP_MIN;
		conf->temp_para[row].temp_min = idata[col];
		col = row * DC_TEMP_TOTAL + DC_TEMP_MAX;
		conf->temp_para[row].temp_max = idata[col];
		col = row * DC_TEMP_TOTAL + DC_TEMP_CUR_MAX;
		conf->temp_para[row].temp_cur_max = idata[col];
	}
}

static int dc_mmi_parse_dts(struct device_node *np, struct dc_sc_config *conf)
{
	u32 tmp_para[DC_MMI_PARA_MAX] = { 0 };

	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"dc_mmi_para", tmp_para, DC_MMI_PARA_MAX)) {
		tmp_para[DC_MMI_PARA_TIMEOUT] = DC_MMI_DFLT_TIMEOUT;
		tmp_para[DC_MMI_PARA_EXPT_PORT] = DC_MMI_DFLT_EX_PROT;
	}
	conf->mmi_para.timeout = (int)tmp_para[DC_MMI_PARA_TIMEOUT];
	conf->mmi_para.expt_prot = tmp_para[DC_MMI_PARA_EXPT_PORT];
	conf->mmi_para.multi_sc_test = (int)tmp_para[DC_MMI_PARA_MULTI_SC_TEST];
	conf->mmi_para.ibat_th = (int)tmp_para[DC_MMI_PARA_IBAT_TH];
	conf->mmi_para.ibat_timeout = (int)tmp_para[DC_MMI_PARA_IBAT_TIMEOUT];
	hwlog_info("[dc_mmi_para] timeout:%ds, expt_prot:%d, multi_sc_test=%d, ibat_th=%d, ibat_timeout=%d\n",
		conf->mmi_para.timeout, conf->mmi_para.expt_prot,
		conf->mmi_para.multi_sc_test, conf->mmi_para.ibat_th,
		conf->mmi_para.ibat_timeout);

	return 0;
}

static void dc_parse_rt_test_para(struct device_node *np, struct dc_sc_config *conf)
{
	int row, col, len;
	int idata[DC_MODE_TOTAL * DC_RT_TEST_INFO_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"rt_test_para", idata, DC_MODE_TOTAL, DC_RT_TEST_INFO_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_RT_TEST_INFO_TOTAL; row++) {
		col = row * DC_RT_TEST_INFO_TOTAL + DC_RT_CURR_TH;
		conf->rt_test_para[row].rt_curr_th = idata[col];
		col = row * DC_RT_TEST_INFO_TOTAL + DC_RT_TEST_TIME;
		conf->rt_test_para[row].rt_test_time = idata[col];
	}
}

static void dc_parse_basic_para(struct device_node *np, struct dc_sc_config *conf)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dc_volt_ratio", (u32 *)&conf->volt_ratio, 1);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"init_adapter_vset", (u32 *)&conf->init_adapter_vset,
		4400); /* default is 4400mv */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"init_delt_vset", (u32 *)&conf->init_delt_vset,
		300); /* default is 300mv */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ui_max_pwr", (u32 *)&conf->ui_max_pwr, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"product_max_pwr", (u32 *)&conf->product_max_pwr, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"second_resist_check_en", (u32 *)&conf->second_resist_check_en, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"standard_cable_full_path_res_max",
		(u32 *)&conf->std_cable_full_path_res_max, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"nonstd_cable_full_path_res_max",
		(u32 *)&conf->nonstd_cable_full_path_res_max, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_current_for_none_standard_cable",
		(u32 *)&conf->max_current_for_nonstd_cable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_current_for_ctc_cable",
		(u32 *)&conf->max_current_for_ctc_cable,
		conf->max_current_for_nonstd_cable);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"second_path_res_report_th",
		(u32 *)&conf->second_path_res_report_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"super_ico_current", (u32 *)&conf->super_ico_current,
		4000); /* default is 4000ma */
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"is_send_cable_type", &conf->is_send_cable_type, 1);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ibat_abnormal_th", (u32 *)&conf->ibat_abnormal_th, 0);
	if (power_cmdline_is_factory_mode())
		conf->ibat_abnormal_th = 0;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"first_cc_stage_timer_in_min",
		&conf->first_cc_stage_timer_in_min, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_err_th", (u32 *)&conf->vol_err_th, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"adaptor_leakage_current_th",
		(u32 *)&conf->adaptor_leakage_current_th, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_dc_bat_vol", (u32 *)&conf->max_dc_bat_vol, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"min_dc_bat_vol", (u32 *)&conf->min_dc_bat_vol, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vstep", (u32 *)&conf->vstep, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err", (u32 *)&conf->delta_err, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err_10v2p25a", (u32 *)&conf->delta_err_10v2p25a,
		(u32)conf->delta_err);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"delta_err_10v4a", (u32 *)&conf->delta_err_10v4a,
		(u32)conf->delta_err);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_cable_detect_enable", &conf->cc_cable_detect_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"low_temp_hysteresis", &conf->orig_low_temp_hysteresis, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"high_temp_hysteresis", &conf->orig_high_temp_hysteresis, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"startup_iin_limit", &conf->startup_iin_limit, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"hota_iin_limit", &conf->hota_iin_limit, conf->startup_iin_limit);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_adaptor_vset", (u32 *)&conf->max_adapter_vset, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tadapt", (u32 *)&conf->max_tadapt, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"max_tls", (u32 *)&conf->max_tls, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"cc_unsafe_sc_enable", (u32 *)&conf->cc_unsafe_sc_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sbu_unsafe_sc_enable", (u32 *)&conf->sbu_unsafe_sc_enable, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"path_ibus_th", (u32 *)&conf->path_ibus_th, MIN_CURRENT_FOR_RES_DETECT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sc_to_bulk_delay_hiz", (u32 *)&conf->sc_to_bulk_delay_hiz, 0);
}

static void multi_ic_parse_current_ratio_para(struct device_node *np,
	struct multi_ic_check_para *info)
{
	int row, col, len;
	int data[MULTI_IC_CURR_RATIO_PARA_LEVEL * MULTI_IC_CURR_RATIO_ERR_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"current_ratio", data, MULTI_IC_CURR_RATIO_PARA_LEVEL,
		MULTI_IC_CURR_RATIO_ERR_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / MULTI_IC_CURR_RATIO_ERR_MAX; row++) {
		col = row * MULTI_IC_CURR_RATIO_ERR_MAX + MULTI_IC_CURR_RATIO_ERR_CHECK_CNT;
		info->curr_ratio[row].error_cnt = data[col];
		col = row * MULTI_IC_CURR_RATIO_ERR_MAX + MULTI_IC_CURR_RATIO_MIN;
		info->curr_ratio[row].current_ratio_min = data[col];
		col = row * MULTI_IC_CURR_RATIO_ERR_MAX + MULTI_IC_CURR_RATIO_MAX;
		info->curr_ratio[row].current_ratio_max = data[col];
		col = row * MULTI_IC_CURR_RATIO_ERR_MAX + MULTI_IC_CURR_RATIO_DMD_LEVEL;
		info->curr_ratio[row].dmd_level = data[col];
		col = row * MULTI_IC_CURR_RATIO_ERR_MAX + MULTI_IC_CURR_RATIO_LIMIT_CURRENT;
		info->curr_ratio[row].limit_current = data[col];
	}
}

static void multi_ic_parse_vbat_error_para(struct device_node *np,
	struct multi_ic_check_para *info)
{
	int row, col, len;
	int data[MULTI_IC_VBAT_ERROR_PARA_LEVEL * MULTI_IC_VBAT_ERROR_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"vbat_error", data, MULTI_IC_VBAT_ERROR_PARA_LEVEL,
		MULTI_IC_VBAT_ERROR_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / MULTI_IC_VBAT_ERROR_MAX; row++) {
		col = row * MULTI_IC_VBAT_ERROR_MAX + MULTI_IC_VBAT_ERROR_CHECK_CNT;
		info->vbat_error[row].error_cnt = data[col];
		col = row * MULTI_IC_VBAT_ERROR_MAX + MULTI_IC_VBAT_ERROR_DELTA;
		info->vbat_error[row].vbat_error = data[col];
		col = row * MULTI_IC_VBAT_ERROR_MAX + MULTI_IC_VBAT_ERROR_DMD_LEVEL;
		info->vbat_error[row].dmd_level = data[col];
		col = row * MULTI_IC_VBAT_ERROR_MAX + MULTI_IC_VBAT_ERROR_LIMIT_CURRENT;
		info->vbat_error[row].limit_current = data[col];
	}
}

static void multi_ic_parse_tbat_error_para(struct device_node *np,
	struct multi_ic_check_para *info)
{
	int row, col, len;
	int data[MULTI_IC_TBAT_ERROR_PARA_LEVEL * MULTI_IC_TBAT_ERROR_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"tbat_error", data, MULTI_IC_TBAT_ERROR_PARA_LEVEL,
		MULTI_IC_TBAT_ERROR_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / MULTI_IC_TBAT_ERROR_MAX; row++) {
		col = row * MULTI_IC_TBAT_ERROR_MAX + MULTI_IC_TBAT_ERROR_CHECK_CNT;
		info->tbat_error[row].error_cnt = data[col];
		col = row * MULTI_IC_TBAT_ERROR_MAX + MULTI_IC_TBAT_ERROR_DELTA;
		info->tbat_error[row].tbat_error = data[col];
		col = row * MULTI_IC_TBAT_ERROR_MAX + MULTI_IC_TBAT_ERROR_DMD_LEVEL;
		info->tbat_error[row].dmd_level = data[col];
		col = row * MULTI_IC_TBAT_ERROR_MAX + MULTI_IC_TBAT_ERROR_LIMIT_CURRENT;
		info->tbat_error[row].limit_current = data[col];
	}
}

static void multi_ic_parse_check_para(struct device_node *np,
	struct multi_ic_check_para *info)
{
	if (!np || !info)
		return;

	multi_ic_parse_current_ratio_para(np, info);
	multi_ic_parse_vbat_error_para(np, info);
	multi_ic_parse_tbat_error_para(np, info);
}

static void dc_parse_multi_ic_para(struct device_node *np, struct dc_sc_config *conf)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_multi_ic", (u32 *)&conf->multi_ic_mode_para.support_multi_ic, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"multi_ic_ibat_th", 
		(u32 *)&conf->multi_ic_mode_para.multi_ic_ibat_th, DC_MULTI_IC_IBAT_TH);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
 		"ibat_comb", (u32 *)&conf->multi_ic_mode_para.ibat_comb, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"single_ic_ibat_th", (u32 *)&conf->multi_ic_mode_para.single_ic_ibat_th,
		DC_SINGLEIC_CURRENT_LIMIT);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"curr_offset", (u32 *)&conf->multi_ic_mode_para.curr_offset, DC_CURRENT_OFFSET);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"support_select_temp", &conf->multi_ic_mode_para.support_select_temp, 0);

	multi_ic_parse_check_para(np, &conf->multi_ic_check_info);
}

static void dc_parse_time_para(struct device_node *np, struct dc_sc_config *conf)
{
	int row, col, len;
	int idata[DC_TIME_PARA_LEVEL * DC_TIME_INFO_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"time_para", idata, DC_TIME_PARA_LEVEL, DC_TIME_INFO_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_TIME_INFO_MAX; row++) {
		col = row * DC_TIME_INFO_MAX + DC_TIME_INFO_TIME_TH;
		conf->time_para[row].time_th = idata[col];
		col = row * DC_TIME_INFO_MAX + DC_TIME_INFO_IBAT_MAX;
		conf->time_para[row].ibat_max = idata[col];
	}
}

static void dc_parse_vstep_para(struct device_node *np, struct dc_sc_config *conf)
{
	int row, col, len;
	int idata[DC_VSTEP_PARA_LEVEL * DC_VSTEP_INFO_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"vstep_para", idata, DC_VSTEP_PARA_LEVEL, DC_VSTEP_INFO_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_VSTEP_INFO_MAX; row++) {
		col = row * DC_VSTEP_INFO_MAX + DC_VSTEP_INFO_CURR_GAP;
		conf->vstep_para[row].curr_gap = idata[col];
		col = row * DC_VSTEP_INFO_MAX + DC_VSTEP_INFO_VSTEP;
		conf->vstep_para[row].vstep = idata[col];
	}
}

static void dc_adapter_max_power_time_para(struct device_node *np, struct dc_sc_config *conf)
{
	int len, row, col;
	int idata[DC_MAX_POWER_TIME_PARA_LEVEL * DC_MAX_POWER_PARA_MAX] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"adapter_max_power_time_para", idata, DC_MAX_POWER_TIME_PARA_LEVEL,
		DC_MAX_POWER_PARA_MAX);
	if (len < 0)
		return;

	for (row = 0; row < len / DC_MAX_POWER_PARA_MAX; row++) {
		col = row * DC_MAX_POWER_PARA_MAX + DC_ADAPTER_TYPE;
		conf->max_power_time[row].adatper_type = idata[col];
		col = row * DC_MAX_POWER_PARA_MAX + DC_MAX_POWER_TIME;
		conf->max_power_time[row].max_power_time = idata[col];
		col = row * DC_MAX_POWER_PARA_MAX + DC_MAX_POWER_LIMIT_CURRENT;
		conf->max_power_time[row].limit_current = idata[col];
	}
}

static void dc_parse_vbat_comp_para(struct device_node *np,
	struct dc_comp_para *info)
{
	int i, j, len, ret;
	u32 para[DC_VBAT_COMP_PARA_MAX * DC_VBAT_COMP_TOTAL] = { 0 };

	len = power_dts_read_u32_count(power_dts_tag(HWLOG_TAG), np,
 		"vbat_comp_para", DC_VBAT_COMP_PARA_MAX, DC_VBAT_COMP_TOTAL);
	if (len < 0) {
		info->vbat_comp_group_size = 0;
		return;
	}

	ret = power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"vbat_comp_para", para, len);
	if (ret < 0) {
		info->vbat_comp_group_size = 0;
		return;
	}

	info->vbat_comp_group_size = len / DC_VBAT_COMP_TOTAL;

	for (i = 0; i < info->vbat_comp_group_size; i++) {
		j = DC_VBAT_COMP_TOTAL * i;
		info->vbat_comp_para[i].ic_id = para[j + DC_IC_ID];
		info->vbat_comp_para[i].ic_mode = para[j + DC_IC_MODE];
		info->vbat_comp_para[i].vbat_comp = para[j + DC_VBAT_COMP_VALUE];
	}

	for (i = 0; i < info->vbat_comp_group_size; i++)
		hwlog_info("ic_id=%d,ic_mode=%d,vbat_comp_value=%d\n",
			info->vbat_comp_para[i].ic_id,
			info->vbat_comp_para[i].ic_mode,
			info->vbat_comp_para[i].vbat_comp);
}

int adsp_dc_dts_parse(struct device_node *np, struct dc_sc_config *conf)
{
	if (!np || !conf) {
		hwlog_err("np or conf is null\n");
		return -EINVAL;
	}

	hwlog_info("dc_dts_parse\n");

	dc_parse_basic_para(np, conf);
	dc_parse_temp_para(np, conf);
	dc_parse_resist_para(np, conf->std_resist_para, "std_resist_para");
	dc_parse_resist_para(np, conf->second_resist_para, "second_resist_para");
	dc_parse_resist_para(np, conf->nonstd_resist_para, "resist_para");
	dc_parse_resist_para(np, conf->ctc_resist_para, "ctc_resist_para");
	dc_parse_bat_para(np, conf);
	dc_parse_group_volt_para(np, conf);
	dc_parse_temp_cv_para(np, conf);
	dc_parse_adapter_antifake_para(np, conf);
	dc_parse_gain_current_para(np, conf);
	dc_parse_adp_cur_para(np, conf->adp_10v2p25a, "10v2p25a_cur_para");
	dc_parse_adp_cur_para(np, conf->adp_10v2p25a_car, "10v2p25a_car_cur_para");
	dc_parse_adp_cur_para(np, conf->adp_qtr_a_10v2p25a, "adp_qtr_a_10v2p25a_cur_para");
	dc_parse_adp_cur_para(np, conf->adp_qtr_c_20v3a, "adp_qtr_c_20v3a_cur_para");
	dc_parse_adp_cur_para(np, conf->adp_10v4a, "10v4a_cur_para");
	dc_parse_multi_ic_para(np, conf);
	dc_parse_time_para(np, conf);
	dc_parse_vstep_para(np, conf);
	dc_adapter_max_power_time_para(np, conf);
	dc_parse_rt_test_para(np, conf);
	dc_mmi_parse_dts(np, conf);
	dc_parse_vbat_comp_para(np, &conf->comp_para);

	return 0;
}