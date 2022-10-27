/*
 * adsp_battery.h
 *
 * driver for adsp battery fuel gauge
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

#ifndef _ADSP_BATTERY_H_
#define _ADSP_BATTERY_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>

#define MAX_BATT_NAME			32
#define MAX_FUELGAUGE_NAME		32
#define MAX_SE_INSTANCE_NUM		16
#define RATIO_1K				1000

#define ADSP_BATTERY_SYNC_WORK_INTERVAL        1000
#define ADSP_BATTERY_CHARGE_UPDATE_INTERVAL    10000
#define ADSP_BATTERY_DISCHARGE_UPDATE_INTERVAL 30000
#define ADSP_BATTERY_LOW_VOLT_UPDATE_INTERVAL  2000
#define ADSP_BATTERY_UPDATE_WORK_LOW_TEMP      (-100)
#define ADSP_BATTERY_LOW_TEMP_UPDATE_INTERVAL  10000

/* battery ui capacity */
#define BUC_SOC_CALIBRATION_PARA_LEVEL      2
#define BUC_CAPACITY_DIVISOR                100

/* battery fault */
#define BAT_FAULT_NORMAL_CUTOFF_VOL  3150
#define BAT_FAULT_SLEEP_CUTOFF_VOL   3350
#define BAT_FAULT_CUTOFF_VOL_OFFSET  10
#define BAT_FAULT_CUTOFF_VOL_FILTERS 3

struct battery_model_info {
	u32 voltage_max_design;
	u32 charge_full_design;
	u32 id_voltage;
	char brand[MAX_BATT_NAME];
};

struct battery_fault_info {
	int vol_cutoff_normal;
	int vol_cutoff_sleep;
	int vol_cutoff_low_temp;
	int vol_cutoff_filter_cnt;
};

struct bat_ui_soc_calibration_para {
	int soc;
	int volt;
};

#define BUC_WORK_INTERVAL_LEVEL   8
enum bat_ui_capacity_interval {
	BUC_WORK_INTERVAL_MIN_SOC = 0,
	BUC_WORK_INTERVAL_MAX_SOC,
	BUC_WORK_INTERVAL_VALUE,
	BUC_WORK_INTERVAL_TOTAL,
};

struct bat_ui_cap_interval_para {
	int min_soc;
	int max_soc;
	int interval;
};

struct battery_ui_capacity_info {
	int soc_at_term;
	int vth_correct_en;
	struct bat_ui_soc_calibration_para vth_soc_calibration_data[BUC_SOC_CALIBRATION_PARA_LEVEL];
	struct bat_ui_cap_interval_para charging_interval_para[BUC_WORK_INTERVAL_LEVEL];
	struct bat_ui_cap_interval_para discharging_interval_para[BUC_WORK_INTERVAL_LEVEL];
};

struct adsp_battery_psy_info
{
	int exist;
	int ui_capacity;
	int capacity_level;
	int temp_now;
	int cycle_count;
	int fcc;
	int voltage_now;
	int current_now;
	int capacity_rm;
	int health;
};

struct adsp_battery_soc_decimal
{
	int rep_soc;
	int round_soc;
	int base;
};

struct adsp_battery_device {
	struct device *dev;
	struct delayed_work sync_work;
	struct delayed_work update_work;
	struct battery_model_info batt_model_info;
	struct battery_fault_info batt_fault_info;
	struct battery_ui_capacity_info batt_ui_cap_info;
	struct notifier_block event_nb;
	struct adsp_battery_psy_info psy_info;
	struct adsp_battery_soc_decimal soc_decimal;
	int soc_decimal_update;
	int charge_status;
	int last_capacity;
};

enum battery_dts_type {
	ADSP_BATTERY_DTS_BEGIN = 0,
	ADSP_BATTERY_DTS_FAULT = ADSP_BATTERY_DTS_BEGIN,
	ADSP_BATTERY_DTS_UI_CAPACITY,
	ADSP_BATTERY_DTS_END
};

#ifdef CONFIG_ADSP_BATTERY
int adsp_battery_ui_soc_get_filter_sum(int base);

void adsp_battery_ui_soc_sync_filter(int rep_soc, int round_soc, int base);

void adsp_battery_restart_update_work(void);

void adsp_battery_cancle_update_work(void);
#else
static inline int adsp_battery_ui_soc_get_filter_sum(int base)
{
	return -1;
}

static inline void adsp_battery_ui_soc_sync_filter(int rep_soc, int round_soc, int base) {}

static inline void adsp_battery_restart_update_work(void) {}

static inline void adsp_battery_cancle_update_work(void) {}
#endif /* CONFIG_ADSP_BATTERY */

#endif /* _ADSP_BATTERY_H_ */

