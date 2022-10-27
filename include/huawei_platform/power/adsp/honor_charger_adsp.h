/*
 * honor_charger_adsp.h
 *
 * honor charger adsp driver
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

#include <linux/version.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/notifier.h>
#include <linux/power_supply.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_dsm.h>
#include <chipset_common/hwpower/power_event_ne.h>
#include <chipset_common/hwpower/power_ui_ne.h>
#include <huawei_platform/power/huawei_charger_common.h>

#ifndef _HONOR_CHARGER_ADSP_
#define _HONOR_CHARGER_ADSP_

#define DEFAULT_IIN_CURRENT      1000
#define MAX_CURRENT              2500
#define MIN_CURRENT              100
#define HLTHERM_CURRENT          2000
#define NO_CHG_TEMP_LOW          0
#define NO_CHG_TEMP_HIGH         500
#define BATT_EXIST_TEMP_LOW     (-400)
#define DEFAULT_NORMAL_TEMP      250
#define RUNN_TEST_TEMP           250
#define DEFAULT_IIN_THL          2300
#define DEFAULT_ITERM            200
#define DEFAULT_VTERM            4400000
#define RATIO_1K                 1000

#define MAX_SIZE                1024
#define CHARGELOG_SIZE          2048
#define ADAPTOR_NAME_SIZE       32
#define TMEP_BUF_LEN            64

#define INPUT_NUMBER_BASE       10

#define CHARGE_TEMP_PARA_MAX_NUM  8
#define CHARGE_DTS_SYNC_INTERVAL  1000

#define CHARGE_DSM_BUFF_MAX_SIZE  240

enum charge_temp_para_info {
	CHARGE_TEMP_PARA_TEMP_MIN = 0,
	CHARGE_TEMP_PARA_TEMP_MAX,
	CHARGE_TEMP_PARA_MAX_CUR,
	CHARGE_TEMP_PARA_VTERM,
	CHARGE_TEMP_PARA_ITERM,
	CHARGE_TEMP_PARA_MAX,
};

enum charge_sysfs_type {
	CHARGE_SYSFS_IIN_RT_CURRENT = 0,
	CHARGE_SYSFS_IBUS,
	CHARGE_SYSFS_VBUS,
	CHARGE_SYSFS_HIZ,
	CHARGE_SYSFS_CHARGE_TYPE,
	CHARGE_SYSFS_FCP_SUPPORT,
	CHARGE_SYSFS_UPDATE_VOLT_NOW,
	CHARGE_SYSFS_END,
};

enum charger_type {
	CHARGER_TYPE_USB = 0,      /* SDP */
	CHARGER_TYPE_BC_USB,       /* CDP */
	CHARGER_TYPE_NON_STANDARD, /* UNKNOW */
	CHARGER_TYPE_STANDARD,     /* DCP */
	CHARGER_TYPE_FCP,          /* FCP */
	CHARGER_REMOVED,           /* not connected */
	USB_EVENT_OTG_ID,
	CHARGER_TYPE_VR,           /* VR charger */
	CHARGER_TYPE_TYPEC,        /* PD charger */
	CHARGER_TYPE_PD,           /* PD charger */
	CHARGER_TYPE_SCP,          /* SCP charger */
	CHARGER_TYPE_WIRELESS,     /* wireless charger */
	CHARGER_TYPE_POGOPIN,      /* pogopin charger */
};

enum iin_thermal_charge_type {
	IIN_THERMAL_CHARGE_TYPE_BEGIN = 0,
	IIN_THERMAL_WCURRENT_5V = IIN_THERMAL_CHARGE_TYPE_BEGIN,
	IIN_THERMAL_WCURRENT_9V,
	IIN_THERMAL_WLCURRENT_5V,
	IIN_THERMAL_WLCURRENT_9V,
	IIN_THERMAL_CHARGE_TYPE_END,
};

struct charger_temp_para {
    int temp_min;
	int temp_max;
	u32 max_current;
	u32 vterm;
	u32 iterm;
};

struct charger_config {
    struct charger_temp_para chg_temp_para[CHARGE_TEMP_PARA_MAX_NUM];
    u32 startup_iin_limit;
    u32 hota_iin_limit;
    u32 en_eoc_max_delay;
};

struct charge_sysfs_data
{
	int iin_rt_curr;
	int hiz_mode;
	int ibus;
	int vbus;
	unsigned int iin_thl_array[IIN_THERMAL_CHARGE_TYPE_END];
	unsigned int charge_enable;
	char *reg_value;
	char *reg_head;
	struct mutex dump_reg_lock;
	struct mutex dump_reg_head_lock;
};

struct charger_sysfs_ops {
	int (*get_property)(char *buf);
	int (*set_property)(const char *buf);
};

/*adaptor protocol test*/
enum adaptor_type {
	TYPE_SCP,
	TYPE_FCP,
	TYPE_PD,
	TYPE_SC,
	TYPE_HSC,
	TYPE_END,
};

enum test_state {
	DETECT_FAIL = 0,
	DETECT_SUCC = 1,
	PROTOCOL_FINISH_SUCC = 2,
};

struct adaptor_test_info{
	int charger_type;
	char adaptor_name[ADAPTOR_NAME_SIZE];
	int result;
};

struct charge_dsm_info
{
	int dsm_type;
	int dsm_no;
	char dsm_buff[CHARGE_DSM_BUFF_MAX_SIZE];
};

struct charge_device_info {
	struct device *dev;
	struct delayed_work sync_work;
	struct charge_sysfs_data sysfs_data;
	struct charger_config chg_config;
	struct power_supply *batt_psy;
	struct power_supply *usb_psy;
	u32 startup_iin_limit;
	u32 hota_iin_limit;
};

#endif /* _HONOR_CHARGER_ADSP_ */
