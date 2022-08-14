/*
 * charge_dmd_monitor.h
 *
 * charge dmd monitor driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#ifndef _CHARGE_DMD_MONITOR_H_
#define _CHARGE_DMD_MONITOR_H_

#include <chipset_common/hwpower/power_dsm.h>

#define DSM_BAT_INFO_BUF_LEN     512
#define BAT_TEMP_HIGH_LIMINT     68
#define BATTERY_TEMP_MIN_LIMINT  (-20)
#define BAT_VOLT_MAX_LIMINT      4550000
#define BAT_VOLT_MIN_LIMINT      2800000
#define MAX_CONFIRM_CNT          3
#define BATTERY_CURRENT_DIVI     10
#define BAT_CHR_CURRENT          15000
#define INCHG_OCP_COUNT          3
#define BAT_TEMP_OUT_RANGE_HIGH  50
#define BAT_TEMP_OUT_RANGE_LOW   0
#define BAT_TEMP_WARM_LIMINT     45
#define DMS_REPORT_BUF_LEN       1024
#define BAT_CHG_FULL_SOC_LIMINT  95
#define CURRENT_OFFSET           10
#define DSM_CHECK_TIMES          5
#define BATT_CAPACITY_REDUCE_TH  80
#define CAPACITY_FULL            100

enum normal_check_dsm_info {
	BATT_TERMINATE_TOO_EARLY,
	BATT_BAD_CURR_SENSOR,
	CHARGE_BATT_CAPACITY,
};

struct dsm_info {
	int error_no;
	bool notify_enable;
	bool notify_always;
	void (*dump)(char *buf, unsigned int buf_len);
	bool (*check_error)(char *buf);
};

void reset_batt_dsm_notify_en(void);
void charging_check_error_info(void);
void normal_check_error_info(void);
#endif /* _CHARGE_DMD_MONITOR_H_ */
