/*
 * huawei_battery.h
 *
 * huawei battery driver
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

#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/usb/otg.h>
#include <linux/io.h>
#include <linux/gpio.h>
#include <linux/power_supply.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sysfs.h>
#include <linux/power/huawei_charger.h>
#include <linux/i2c.h>

#ifndef _HUAWEI_BATTERY_H_
#define _HUAWEI_BATTERY_H_

#define BUFFER_SIZE                     30
#define CHG_OVP_REG_NUM                 3
#define CHG_DIS_REG_NUM                 3
#define DEFAULT_FASTCHG_MAX_CURRENT     1000
#define DEFAULT_FASTCHG_MAX_VOLTAGE     4400
#define DEFAULT_MAX_VOLTAGE             4400000
#define DEFAULT_FASTCHG_WARM_CURRENT    900
#define DEFAULT_FASTCHG_WARM_VOLTAGE    4100
#define DEFAULT_FASTCHG_COOL_CURRENT    1100
#define DEFAULT_FASTCHG_COOL_VOLTAGE    4400
#define DEFAULT_USB_ICL_CURRENT         2300
#define DEFAULT_CUSTOM_COOL_LOW_TH      0
#define DEFAULT_CUSTOM_TEMP_CHARGE_LOW  10
#define DEFAULT_CUSTOM_TEMP_CHARGE_HIGH 45
#define DEFAULT_CUSTOM_COOL_UP_TH       50
#define DEFAULT_CUSTOM_COOL_CURRENT     300
/* 1A/2A charger adaptive algo */
#define BATTERY_IBUS_MARGIN_MA          2000
#define BATTERY_IBUS_CONFIRM_COUNT      6
#define BATTERY_IBUS_WORK_DELAY_TIME    1000
#define BATTERY_IBUS_WORK_START_TIME    3000
#define IINLIM_1000                     1000
#define IINLIM_1500                     1500
#define IINLIM_2000                     2000
#define BATT_HEALTH_TEMP_HOT            600
#define BATT_HEALTH_TEMP_WARM           480
#define BATT_HEALTH_TEMP_COOL           100
#define BATT_HEALTH_TEMP_COLD           (-200)
#define EMPTY_SOC                       0
#define FACTORY_SOC_LOW                 6
#define FULL_SOC                        100
#define DEFAULT_BATT_TEMP               250

#define SPMI_ADDR_MASK                  0xffff
#define SPMI_SID_OFFSET                 16
#define SPMI_SID_MASK                   0xf

#define MV_TO_UV                        1000
#define MA_TO_UA                        1000
#define SPMI_BUS                        1
#define I2C_BUS                         2

struct spmi_bus_info {
	struct spmi_controller *spmi_ctrl;
};

struct i2c_bus_info {
	struct i2c_adapter *i2c_adapter;
	unsigned short i2c_addr;
	unsigned short i2c_flags;
};

struct ops_param {
	u8 sid;
	u16 addr;
	int count;
	u8 *val;
};

struct huawei_battery_info;
typedef int (*reg_ops)(struct huawei_battery_info *, struct ops_param *, bool);

struct huawei_battery_info {
	struct device *dev;
	struct power_supply *batt_psy;
	enum power_supply_property *batt_props;
	int  num_props;

	struct power_supply *bk_batt_psy;
	struct power_supply *bms_psy;
	struct power_supply *usb_psy;

	int bus_type;
	bool support_parallel_charger;
	struct device *bk_battery_dev;
	struct spmi_bus_info spmi_info;
	struct regmap *regmap;

	struct i2c_bus_info i2c_info;
	struct i2c_bus_info second_i2c_info;

	/* register operation */
	reg_ops battery_reg_read;
	reg_ops battery_reg_write;
	reg_ops battery_second_reg_read;
	reg_ops battery_second_reg_write;

	/* chargelog dump regs */
	u32 *dump_regs;
	int dump_regs_num;
	u32 *parallel_dump_regs;
	int parallel_regs_num;

	/* voteable */
	struct votable *usb_icl_votable;
	struct votable *fcc_votable;
	struct votable *vbat_votable;
	struct votable *bch_votable;

	int fastchg_max_current;
	int fastchg_max_voltage;
	int fastchg_warm_current;
	int fastchg_warm_voltage;
	int fastchg_cool_current;
	int fastchg_cool_voltage;
	int usb_icl_max;
	int thermal_limit_current;
	int current_adjusted_ma;
	bool ibus_detect_disable;
	int ibus_detecting;
	int customize_cool_lower_limit;
	int customize_cool_upper_limit;
	int fastchg_current_customize_cool;
	int charger_batt_capacity_mah;
	struct wakeup_source *chg_ws;
	struct wakeup_source *dmd_ws;
	int dmd_monitor_work_interval;

	/* delayed work */
	struct delayed_work ibus_detect_work;
	struct delayed_work jeita_work;
	struct delayed_work monitor_charging_work;
	struct delayed_work sync_voltage_work;
	struct delayed_work monitor_dmd_work;

	/* jeita solution */
	bool jeita_use_interrupt;
	bool jeita_hw_curr_limit;
	bool jeita_hw_volt_limit;
	bool jeita_hw_chg_dis;
	bool disable_huawei_func;
	int jeita_chg_dis_reg;
	int jeita_chg_dis_mask;
	int jeita_chg_dis_value;
	/* fake soc */
	int fake_soc;
	int chr_type;
	int enable_hv_charging;
	int charger_thread_polling;
	int use_third_coul;
};

int get_prop_from_psy(struct power_supply *psy, enum power_supply_property prop,
	union power_supply_propval *val);
int set_prop_to_psy(struct power_supply *psy, enum power_supply_property prop,
	const union power_supply_propval *val);
int battery_get_soc(void);
int battery_get_bat_avg_current(void);
int battery_get_present(void);
#endif /* _HUAWEI_BATTERY_H_ */
