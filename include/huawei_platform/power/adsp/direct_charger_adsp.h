/*
 * direct_charger_adsp.h
 *
 * direct charger adsp driver
 *
 * Copyright (c) 2021-2021 Hornor Technologies Co., Ltd.
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

#ifndef _DIRECT_CHARGER_ADSP_H_
#define _DIRECT_CHARGER_ADSP_H_

#include <linux/module.h>
#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/jiffies.h>
#include <linux/hrtimer.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/pm_wakeup.h>
#include <dsm/dsm_pub.h>
#include <chipset_common/hwpower/adapter_protocol.h>
#include <chipset_common/hwpower/power_dsm.h>
#include <log/hw_log.h>
#include <chipset_common/hwpower/power_ui_ne.h>
#include <chipset_common/hwpower/power_dts.h>

#define MIN_CURRENT_FOR_RES_DETECT    800
#define DC_SINGLEIC_CURRENT_LIMIT     8000
#define DC_MULTI_IC_IBAT_TH           4000
#define DC_CURRENT_OFFSET             300
#define DC_VBAT_COMP_PARA_MAX         12
#define DC_THERMAL_REASON_SIZE        16
#define DC_SC_DTS_SYNC_INTERVAL       1000
#define DC_MAX_RESISTANCE             10000

#define DC_MMI_DFLT_EX_PROT           1
#define DC_MMI_DFLT_TIMEOUT           10

#define DC_SC_CUR_LEN                 64
#define DC_SC_DEFAULT_ICHG_RATIO      100
#define DC_SC_DEFAULT_VTERM_DEC       0

#define DC_SC_ENABLE_CHARGE           1

#define CHARGE_IC_MAX_NUM       2
#define CHARGE_IC_MAIN          BIT(0)
#define CHARGE_IC_AUX           BIT(1)
#define CHARGE_MULTI_IC         (CHARGE_IC_MAIN | CHARGE_IC_AUX)

enum dc_ic_type {
	CHARGE_IC_TYPE_MAIN = 0,
	CHARGE_IC_TYPE_AUX,
	CHARGE_IC_TYPE_MAX,
};

enum dc_vbat_comp_info {
	DC_IC_ID,
	DC_IC_MODE,
	DC_VBAT_COMP_VALUE,
	DC_VBAT_COMP_TOTAL,
};

/*
 * define temprature threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_TEMP_LEVEL           5

enum dc_temp_info {
	DC_TEMP_MIN = 0,
	DC_TEMP_MAX,
	DC_TEMP_CUR_MAX,
	DC_TEMP_TOTAL,
};

struct dc_temp_para {
	int temp_min;
	int temp_max;
	int temp_cur_max;
};

#define DC_TEMP_CV_LEVEL        8

enum dc_temp_cv_info {
	DC_TEMP_CV_MIN,
	DC_TEMP_CV_MAX,
	DC_TEMP_CV_CUR,
	DC_TEMP_CV_TOTOL,
};

struct dc_temp_cv_para {
	int temp_min;
	int temp_max;
	int cv_curr;
};

/*
 * define resistance threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_RESIST_LEVEL         5

enum dc_resist_info {
	DC_RESIST_MIN = 0,
	DC_RESIST_MAX,
	DC_RESIST_CUR_MAX,
	DC_RESIST_TOTAL,
};

struct dc_resist_para {
	int resist_min;
	int resist_max;
	int resist_cur_max;
};

/*
 * define multistage (cc)constant current and (cv)constant voltage
 * support up to 5 parameters list on dts
 */
#define DC_VOLT_LEVEL           8

enum dc_volt_info {
	DC_PARA_VOL_TH = 0,
	DC_PARA_CUR_TH_HIGH,
	DC_PARA_CUR_TH_LOW,
	DC_PARA_VOLT_TOTAL,
};

struct dc_volt_para {
	int vol_th;
	int cur_th_high;
	int cur_th_low;
};

/*
 * define adapter current threshold at different voltages
 * support up to 6 parameters list on dts
 */
#define DC_ADP_CUR_LEVEL       6

enum dc_adp_cur_info {
	DC_ADP_VOL_MIN,
	DC_ADP_VOL_MAX,
	DC_ADP_CUR_TH,
	DC_ADP_TOTAL,
};

struct dc_adp_cur_para {
	int vol_min;
	int vol_max;
	int cur_th;
};

/* define rt test time parameters */
enum dc_rt_test_info {
	DC_RT_CURR_TH,
	DC_RT_TEST_TIME,
	DC_RT_TEST_INFO_TOTAL,
};

enum dc_rt_test_mode {
	DC_NORMAL_MODE,
	DC_CHAN1_MODE, /* channel1: single main sc mode */
	DC_CHAN2_MODE, /* channel2: single aux sc mode */
	DC_MODE_TOTAL,
};

struct dc_rt_test_para {
	u32 rt_curr_th;
	u32 rt_test_time;
};

/*
 * define voltage parameters of different batteries
 * at different temperature threshold
 * support up to 8 parameters list on dts
 */
#define DC_VOLT_GROUP_MAX       8
#define DC_BAT_BRAND_LEN_MAX    16
#define DC_VOLT_NODE_LEN_MAX    16

enum direct_charge_bat_info {
	DC_PARA_BAT_ID = 0,
	DC_PARA_TEMP_LOW,
	DC_PARA_TEMP_HIGH,
	DC_PARA_INDEX,
	DC_PARA_BAT_TOTAL,
};

struct dc_bat_para {
	int temp_low;
	int temp_high;
	int parse_ok;
	char batid[DC_BAT_BRAND_LEN_MAX];
	char volt_para_index[DC_VOLT_NODE_LEN_MAX];
};

/*
 * define dc time threshold with maximum current
 * support up to 5 parameters list on dts
 */
#define DC_TIME_PARA_LEVEL               5

enum dc_time_info {
	DC_TIME_INFO_TIME_TH,
	DC_TIME_INFO_IBAT_MAX,
	DC_TIME_INFO_MAX,
};

struct dc_time_para {
	int time_th;
	int ibat_max;
};

/*
* define dc first cc charge time with max power
* support up to 7 parameters list on dts
*/
#define DC_MAX_POWER_TIME_PARA_LEVEL    20

enum dc_max_power_time_info {
	DC_ADAPTER_TYPE,
	DC_MAX_POWER_TIME,
	DC_MAX_POWER_LIMIT_CURRENT,
	DC_MAX_POWER_PARA_MAX,
};

struct dc_max_power_time_para {
	int adatper_type;
	int max_power_time;
	int limit_current;
};

struct dc_volt_para_group {
	struct dc_volt_para volt_info[DC_VOLT_LEVEL];
	struct dc_bat_para bat_info;
	int stage_size;
};

enum dc_iin_thermal_channel_type {
	DC_CHANNEL_TYPE_BEGIN = 0,
	DC_SINGLE_CHANNEL = DC_CHANNEL_TYPE_BEGIN,
	DC_DUAL_CHANNEL,
	DC_CHANNEL_TYPE_END,
};

#define DC_VSTEP_PARA_LEVEL    4

enum dc_vstep_info {
	DC_VSTEP_INFO_CURR_GAP,
	DC_VSTEP_INFO_VSTEP,
	DC_VSTEP_INFO_MAX,
};

struct dc_vstep_para {
	int curr_gap;
	int vstep;
};

#define MULTI_IC_CURR_RATIO_PARA_LEVEL   5

enum multi_ic_current_ratio_info {
	MULTI_IC_CURR_RATIO_ERR_CHECK_CNT,
	MULTI_IC_CURR_RATIO_MIN,
	MULTI_IC_CURR_RATIO_MAX,
	MULTI_IC_CURR_RATIO_DMD_LEVEL,
	MULTI_IC_CURR_RATIO_LIMIT_CURRENT,
	MULTI_IC_CURR_RATIO_ERR_MAX,
};

struct multi_ic_curr_ratio_para {
	u32 error_cnt;
	u32 current_ratio_min;
	u32 current_ratio_max;
	u32 dmd_level;
	int limit_current;
};

#define MULTI_IC_VBAT_ERROR_PARA_LEVEL     5

enum multi_ic_vbat_error_info {
	MULTI_IC_VBAT_ERROR_CHECK_CNT,
	MULTI_IC_VBAT_ERROR_DELTA,
	MULTI_IC_VBAT_ERROR_DMD_LEVEL,
	MULTI_IC_VBAT_ERROR_LIMIT_CURRENT,
	MULTI_IC_VBAT_ERROR_MAX,
};

struct multi_ic_vbat_error_para {
	u32 error_cnt;
	u32 vbat_error;
	u32 dmd_level;
	u32 limit_current;
};

#define MULTI_IC_TBAT_ERROR_PARA_LEVEL   5

enum multi_ic_tbat_error_info {
	MULTI_IC_TBAT_ERROR_CHECK_CNT,
	MULTI_IC_TBAT_ERROR_DELTA,
	MULTI_IC_TBAT_ERROR_DMD_LEVEL,
	MULTI_IC_TBAT_ERROR_LIMIT_CURRENT,
	MULTI_IC_TBAT_ERROR_MAX,
};

struct multi_ic_tbat_error_para {
	u32 error_cnt;
	u32 tbat_error;
	u32 dmd_level;
	int limit_current;
};


struct multi_ic_check_para {
	struct multi_ic_curr_ratio_para curr_ratio[MULTI_IC_CURR_RATIO_PARA_LEVEL];
	struct multi_ic_vbat_error_para vbat_error[MULTI_IC_VBAT_ERROR_PARA_LEVEL];
	struct multi_ic_tbat_error_para tbat_error[MULTI_IC_TBAT_ERROR_PARA_LEVEL];
};

struct multi_ic_check_mode_para {
	int support_multi_ic;
	int ibat_comb;
	int curr_offset;
	int single_ic_ibat_th;
	u32 support_select_temp;
	int multi_ic_ibat_th;
};

struct dc_vbat_comp_para {
	int ic_id;
	int ic_mode;
	int vbat_comp;
};

struct dc_comp_para {
	struct dc_vbat_comp_para vbat_comp_para[DC_VBAT_COMP_PARA_MAX];
	int vbat_comp_group_size;
};

enum dc_mmi_test_para {
	DC_MMI_PARA_TIMEOUT,
	DC_MMI_PARA_EXPT_PORT,
	DC_MMI_PARA_MULTI_SC_TEST,
	DC_MMI_PARA_IBAT_TH,
	DC_MMI_PARA_IBAT_TIMEOUT,
	DC_MMI_PARA_MAX,
};

struct dc_mmi_para {
	int timeout;
	u32 expt_prot;
	int multi_sc_test;
	int ibat_th;
	int ibat_timeout;
};

struct dc_sc_config
{
	struct dc_volt_para_group orig_volt_para[DC_VOLT_GROUP_MAX];
	struct dc_temp_para temp_para[DC_TEMP_LEVEL];
	struct dc_temp_cv_para temp_cv_para[DC_TEMP_CV_LEVEL];
	struct dc_resist_para nonstd_resist_para[DC_RESIST_LEVEL];
	struct dc_resist_para std_resist_para[DC_RESIST_LEVEL];
	struct dc_resist_para second_resist_para[DC_RESIST_LEVEL];
	struct dc_resist_para ctc_resist_para[DC_RESIST_LEVEL];
	struct dc_adp_cur_para adp_10v2p25a[DC_ADP_CUR_LEVEL];
	struct dc_adp_cur_para adp_10v2p25a_car[DC_ADP_CUR_LEVEL];
	struct dc_adp_cur_para adp_qtr_a_10v2p25a[DC_ADP_CUR_LEVEL];
	struct dc_adp_cur_para adp_qtr_c_20v3a[DC_ADP_CUR_LEVEL];   
	struct dc_adp_cur_para adp_10v4a[DC_ADP_CUR_LEVEL];
	struct dc_time_para time_para[DC_TIME_PARA_LEVEL];
	struct dc_max_power_time_para max_power_time[DC_MAX_POWER_TIME_PARA_LEVEL];
	struct dc_vstep_para vstep_para[DC_VSTEP_PARA_LEVEL];
	struct dc_rt_test_para rt_test_para[DC_MODE_TOTAL];
	struct multi_ic_check_para multi_ic_check_info;
	struct multi_ic_check_mode_para multi_ic_mode_para;
	struct dc_comp_para comp_para;
	struct dc_mmi_para mmi_para;
	int stage_group_size;
	int local_mode;
	int volt_ratio;
	int std_cable_full_path_res_max;
	int nonstd_cable_full_path_res_max;
	int second_path_res_report_th;
	int second_resist_check_en;
	int scp_cable_detect_enable;
	u32 cc_cable_detect_enable;
	u32 cc_unsafe_sc_enable;
	u32 sbu_unsafe_sc_enable;
	int max_current_for_nonstd_cable;
	int max_current_for_ctc_cable;
	u32 is_send_cable_type;
	u32 low_temp_hysteresis;
	u32 orig_low_temp_hysteresis;
	u32 high_temp_hysteresis;
	u32 orig_high_temp_hysteresis;
	int init_delt_vset;
	int init_adapter_vset;
	int delta_err;
	int delta_err_10v2p25a;
	int delta_err_10v4a;
	int vstep;
	int vol_err_th;
	int adaptor_leakage_current_th;
	u32 first_cc_stage_timer_in_min;
	u32 adp_antifake_key_index;
	u32 adp_antifake_enable;
	u32 gain_curr_10v2a;
	u32 gain_curr_10v2p25a;
	u32 startup_iin_limit;
	u32 hota_iin_limit;
	int max_adapter_vset;
	int max_tadapt;
	int max_tls;
	int path_ibus_th;
	int ibat_abnormal_th;
	int max_dc_bat_vol;
	int min_dc_bat_vol;
	int super_ico_current;
	int ui_max_pwr;
	int product_max_pwr;
	int sc_to_bulk_delay_hiz;
};

struct dc_charge_info
{
    int succ_flag;
    const char *ic_name[CHARGE_IC_MAX_NUM];
    int channel_num;
    int ibat_max;
    int ibus[CHARGE_IC_MAX_NUM];
    int vbat[CHARGE_IC_MAX_NUM];
    int vout[CHARGE_IC_MAX_NUM];
    int tbat[CHARGE_IC_MAX_NUM];
};

/*
 * define sysfs type with direct charge
 * DC is simplified identifier with direct-charge
 */
enum dc_sysfs_type {
	DC_SYSFS_BEGIN = 0,
	DC_SYSFS_IIN_THERMAL = DC_SYSFS_BEGIN,
	DC_SYSFS_IIN_THERMAL_ICHG_CONTROL,
	DC_SYSFS_ICHG_CONTROL_ENABLE,
	DC_SYSFS_ADAPTER_DETECT,
	DC_SYSFS_IADAPT,
	DC_SYSFS_FULL_PATH_RESISTANCE,
	DC_SYSFS_DIRECT_CHARGE_SUCC,
	DC_SYSFS_SET_RESISTANCE_THRESHOLD,
	DC_SYSFS_SET_CHARGETYPE_PRIORITY,
	DC_SYSFS_THERMAL_REASON,
	DC_SYSFS_AF,
	DC_SYSFS_MULTI_SC_CUR,
	DC_SYSFS_SC_STATE,
	DC_SYSFS_DUMMY_VBAT,
	DC_SYSFS_END,
};

enum dc_mmi_sysfs_type {
	DC_MMI_SYSFS_BEGIN = 0,
	DC_MMI_SYSFS_TIMEOUT = DC_MMI_SYSFS_BEGIN,
	DC_MMI_SYSFS_LVC_RESULT,
	DC_MMI_SYSFS_SC_RESULT,
	DC_MMI_SYSFS_HSC_RESULT,
	DC_MMI_SYSFS_TEST_STATUS,
	DC_MMI_SYSFS_END,
};

/*
  * define disable type with direct charge
  * DC is simplified identifier with direct-charge
  */
enum direct_charge_disable_type {
	DC_DISABLE_BEGIN = 0,
	DC_DISABLE_SYS_NODE = DC_DISABLE_BEGIN,
	DC_DISABLE_FATAL_ISC_TYPE,
	DC_DISABLE_WIRELESS_TX,
	DC_DISABLE_BATT_CERTIFICATION_TYPE,
	DC_DISABLE_END,
};

struct dc_sysfs_ops {
	int (*get_property)(char *buf);
	int (*set_property)(const char *buf);
};

struct dc_sysfs_state
{
	int sysfs_iin_thermal_array[DC_CHANNEL_TYPE_END];
	unsigned int ignore_full_path_res;
	int sysfs_enable_charger;
	int sysfs_mainsc_enable_charger;
	int sysfs_auxsc_enable_charger;
	int vterm_dec;
	int ichg_ratio;
};

struct dc_device_adsp {
	struct device *dev;
	struct dc_sc_config dc_sc_conf;
	struct delayed_work sync_work;
	struct dc_sysfs_state sysfs_state;
	struct adapter_device_info adap_info;
	int iin_thermal_default;
	int sysfs_iin_thermal;
	int sysfs_iin_thermal_ichg_control;
	char thermal_reason[DC_THERMAL_REASON_SIZE];
	int ichg_control_enable;
	u32 dummy_vbat;
	struct dc_charge_info charge_info;
	u32 rt_test_result[DC_MODE_TOTAL];
};

#endif /* _DIRECT_CHARGER_ADSP_H_ */
