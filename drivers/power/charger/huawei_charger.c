/*
 * huawei_charger.c
 *
 * huawei charger driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#include <linux/notifier.h>
#include <linux/mutex.h>
#include <linux/spmi.h>
#include <linux/sysfs.h>
#include <linux/kernel.h>
#include <linux/power/huawei_charger.h>
#include <linux/power/huawei_battery.h>
#include <huawei_platform/power/direct_charger/direct_charger.h>
#include <linux/version.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <huawei_platform/power/huawei_charger_adaptor.h>
#include <chipset_common/hwpower/power_interface.h>
#include <huawei_platform/power/huawei_charger.h>
#include <chipset_common/hwpower/power_supply_interface.h>
#include <chipset_common/hwpower/power_log.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_cmdline.h>
#include <chipset_common/hwpower/power_common.h>
#include <huawei_platform/power/hihonor_charger_glink.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_fg_glink.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#define HWLOG_TAG honor_charger
HWLOG_REGIST();

#define DEFAULT_ITERM            200
#define DEFAULT_VTERM            4400000
#define DEFAULT_FCP_TEST_DELAY   6000
#define DEFAULT_IIN_CURRENT      1000
#define MAX_CURRENT              3000
#define MIN_CURRENT              100
#define CHARGE_DSM_BUF_SIZE      4096
#define DEC_BASE                 10
#define INPUT_EVENT_ID_MIN       0
#define INPUT_EVENT_ID_MAX       3000
#define CHARGE_INFO_BUF_SIZE     30
#define DEFAULT_IIN_RT           1
#define DEFAULT_HIZ_MODE         0
#define DEFAULT_CHRG_CONFIG      1
#define DEFAULT_FACTORY_DIAG     1
#define UPDATE_VOLT              1
#define EN_HIZ                   1
#define DIS_HIZ                  0
#define EN_CHG                   1
#define DIS_CHG                  0
#define EN_WDT                   1
#define DIS_WDT                  0
#define INVALID_LIMIT_CUR        0
#define NAME                     "charger"
#define NO_CHG_TEMP_LOW          0
#define NO_CHG_TEMP_HIGH         500
#define BATT_EXIST_TEMP_LOW     (-400)
#define DEFAULT_NORMAL_TEMP      250
#define RUNN_TEST_TEMP	250
#define DEFAULT_IIN_THL 2300
#define FCP_READ_DELAY  100
#define INPUT_NUMBER_BASE 10
#define MV_TO_UV    1000
#define MA_TO_UA    1000
#define RT_TEST_TEMP_TH 450
#define LIMIT_CURRENT_TOO_LOW    1

#ifdef CONFIG_HLTHERM_RUNTEST
#define HLTHERM_CURRENT 2000
#endif

/* adaptor test macro */
#define TMEP_BUF_LEN                        10
#define POSTFIX_LEN                         3
#define INVALID_RET_VAL                     (-1)
#define ADAPTOR_TEST_START                  1
#define MIN_ADAPTOR_TEST_INS_NUM            0
#define MAX_ADAPTOR_TEST_INS_NUM            5
#define MAX_EVENT_COUNT                     16
#define EVENT_QUEUE_UNIT                    MAX_EVENT_COUNT
#define WEAKSOURCE_CNT                      10
#define ADAPTOR_STR_LEN                     10

#ifndef strict_strtol
#define strict_strtol kstrtol
#endif

#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0)
struct timespec64 current_kernel_time(void)
{
	struct timespec64 now;

	ktime_get_coarse_real_ts64(&now);
	return now;
}

struct timespec64 current_kernel_time64(void)
{
	struct timespec64 now;

	ktime_get_coarse_real_ts64(&now);
	return now;
}
#endif

static void wakeup_source_drop(struct wakeup_source *ws)
{
 	if (!ws)
 		return;

 	__pm_relax(ws);
}

static void wakeup_source_prepare(struct wakeup_source *ws, const char *name)
{
 	if (ws) {
 		memset(ws, 0, sizeof(*ws));
 		ws->name = name;
	}
}

void wakeup_source_init(struct wakeup_source *ws, const char *name)
{
	wakeup_source_prepare(ws, name);
	wakeup_source_add(ws);
}

void wakeup_source_trash(struct wakeup_source *ws)
{
	wakeup_source_remove(ws);
	wakeup_source_drop(ws);
}

static BLOCKING_NOTIFIER_HEAD(charge_manager_notifier_head);

int usb_charger_register_notifier(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	return blocking_notifier_chain_register(&charge_manager_notifier_head, nb);
}

int usb_charger_notifier_unregister(struct notifier_block *nb)
{
	if (!nb)
		return -EINVAL;

	return blocking_notifier_chain_unregister(&charge_manager_notifier_head, nb);
}

/* adaptor test result */
struct adaptor_test_attr adptor_test_tbl[] = {
	{TYPE_SCP, "SCP", DETECT_FAIL},
	{TYPE_FCP, "FCP", DETECT_FAIL},
	{TYPE_PD, "PD", DETECT_FAIL},
	{TYPE_SC, "SC", DETECT_FAIL},
	{TYPE_HSC, "HSC", DETECT_FAIL},
};

extern int usb_charger_register_notifier(struct notifier_block *nb);
extern int usb_charger_notifier_unregister(struct notifier_block *nb);

#ifdef CONFIG_HUAWEI_DSM
static struct dsm_client *dsm_chargemonitor_dclient = NULL;
static struct dsm_dev dsm_charge_monitor =
{
	.name = "dsm_charge_monitor",
	.fops = NULL,
	.buff_size = 4096, /* dsm buffer for charger is 4096 bytes */
};
#endif

struct class *power_class = NULL;
struct device *charge_dev = NULL;
static struct charge_device_info *g_charger_device_para = NULL;
static struct kobject *g_sysfs_poll;
int g_basp_learned_fcc = -1;

struct charge_device_info *get_charger_device_info(void)
{
	if (!g_charger_device_para)
		return NULL;
	return g_charger_device_para;
}

static int set_property_on_psy(struct power_supply *psy,
	enum power_supply_property prop, int value)
{
	int ret;
	union power_supply_propval val = {0, };

	val.intval = value;
	ret = set_prop_to_psy(psy, prop, &val);
	if (ret)
		pr_err("psy does not allow set prop %d ret = %d\n",
			prop, ret);
	return ret;
}

static int get_property_from_psy(struct power_supply *psy,
		enum power_supply_property prop)
{
	int ret;
	union power_supply_propval val = {0, };

	ret = get_prop_from_psy(psy, prop, &val);
	if (ret) {
		pr_err("psy doesn't support reading prop %d ret = %d\n",
			prop, ret);
		return ret;
	}
	return val.intval;
}

void chg_set_adaptor_test_result(enum adaptor_name charger_type, enum test_state result)
{
	int i;
	int adt_test_tbl_len = sizeof(adptor_test_tbl) / (sizeof(adptor_test_tbl[0]));

	for (i = 0; i < adt_test_tbl_len; i++) {
		if (adptor_test_tbl[i].charger_type == charger_type) {
			adptor_test_tbl[i].result = result;
			break;
		}
	}
	if (i == adt_test_tbl_len)
		pr_err("adaptor type is out of range\n");
}

#ifdef CONFIG_DIRECT_CHARGER
static int chg_get_adaptor_test_result(char *buf)
{
	int i;
	int adt_test_tbl_len;
	int succ_char_sum = 0; /* init the return val to 0 */
	int real_num_read = 0; /* init the number to write to 0 */
	char temp_buf[TMEP_BUF_LEN] = { 0 }; /* init temp buffer to null */
	unsigned int local_mode;

	if (!buf)
		return INVALID_RET_VAL;

	local_mode = direct_charge_get_local_mode();
	adt_test_tbl_len = sizeof(adptor_test_tbl) / (sizeof(adptor_test_tbl[0]));
	for (i = 0; i < adt_test_tbl_len; i++) {
		if (!(local_mode & SC_MODE) && (adptor_test_tbl[i].charger_type == TYPE_SC))
			continue;
		if (i != adt_test_tbl_len - 1) {
			succ_char_sum += snprintf(temp_buf, TMEP_BUF_LEN, "%s:%d,",
				adptor_test_tbl[i].adaptor_str_name, adptor_test_tbl[i].result);
			strncat(buf, temp_buf, strlen(temp_buf));
		} else {
			succ_char_sum += snprintf(temp_buf, TMEP_BUF_LEN, "%s:%d\n",
				adptor_test_tbl[i].adaptor_str_name, adptor_test_tbl[i].result);
			strncat(buf, temp_buf, strlen(temp_buf));
		}
		real_num_read += (strlen(adptor_test_tbl[i].adaptor_str_name) + POSTFIX_LEN);
		if (succ_char_sum != real_num_read) {
			succ_char_sum = INVALID_RET_VAL;
			break;
		}
	}
	buf[strlen(buf) - 1] = '\n';
	pr_info("succ_writen_char = %d, real_to_write = %d, buf = %s\n",
		succ_char_sum, real_num_read, buf);

	return succ_char_sum;
}
#endif

static void clear_adaptor_test_result(void)
{
	int i;
	int adt_test_tbl_len = sizeof(adptor_test_tbl) / (sizeof(adptor_test_tbl[0]));

	for (i = 0; i < adt_test_tbl_len; i++)
		adptor_test_tbl[i].result = DETECT_FAIL;

	return;
}

void get_log_info_from_psy(struct power_supply *psy,
			   enum power_supply_property prop, char *buf)
{
	int rc = 0;
	union power_supply_propval val = {0, };

    if (!psy) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return;
    }

	val.strval = buf;
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
	rc = psy->get_property(psy, prop, &val);
#else
	rc = psy->desc->get_property(psy, prop, &val);
#endif
	if (rc) {
		pr_err("psy does not allow get prop %d rc = %d\n", prop, rc);
	}
}

static bool is_usb_psy_available(struct charge_device_info *di)
{
	if (!di) {
		pr_err("%s: charge_device is not ready! fatal error\n", __func__);
		return false;
	}

	if (di->usb_psy)
		return true;

	di->usb_psy = power_supply_get_by_name("usb");
	if (!di->usb_psy)
		return false;

	return true;
}

static bool is_pc_port_psy_available(struct charge_device_info *di)
{
	if (!di) {
		pr_err("%s: charge_device is not ready! fatal error\n", __func__);
		return false;
	}

	if (di->pc_port_psy)
		return true;

	di->pc_port_psy = power_supply_get_by_name(di->usb_psy_name);
	if (!di->pc_port_psy)
		return false;

	return true;
}

static bool is_battery_psy_available(struct charge_device_info *di)
{
	if (!di) {
		pr_err("%s: charge_device is not ready! fatal error\n", __func__);
		return false;
	}

	if (di->batt_psy)
		return true;

	di->batt_psy = power_supply_get_by_name("battery");
	if (!di->batt_psy)
		return false;

	return true;
}

static bool is_chg_psy_available(struct charge_device_info *di)
{
	if (!di) {
		pr_err("%s: charge_device is not ready! fatal error\n", __func__);
		return false;
	}

	if (di->chg_psy)
		return true;

	di->chg_psy = power_supply_get_by_name("huawei_charge");
	if (!di->chg_psy)
		return false;

	return true;
}

#ifdef CONFIG_PMIC_AP_CHARGER
static bool is_bms_psy_available(struct charge_device_info *di)
{
	if (!di) {
		pr_err("%s: charge_device is not ready! fatal error\n", __func__);
		return false;
	}

	if (di->bms_psy)
		return true;

	di->bms_psy = power_supply_get_by_name(di->bms_psy_name);
	if (!di->bms_psy)
		return false;

	return true;
}
#endif

static bool is_main_psy_available(struct charge_device_info *di)
{
	if (!di) {
		pr_err("%s: charge_device is not ready! fatal error\n", __func__);
		return false;
	}

	if (di->main_psy)
		return true;

	di->main_psy = power_supply_get_by_name(di->main_psy_name);
	if (!di->main_psy)
		return false;

	return true;
}

static bool is_bk_battery_psy_available(struct charge_device_info *di)
{
	if (!di) {
		pr_err("%s: charge_device is not ready! fatal error\n", __func__);
		return false;
	}

	if (di->bk_batt_psy)
		return true;

	di->bk_batt_psy = power_supply_get_by_name("bk_battery");
	if (!di->bk_batt_psy)
		return false;

	return true;
}

static int charger_set_constant_voltage(u32 uv)
{
	if (!is_chg_psy_available(g_charger_device_para))
		return -1;

	return set_property_on_psy(g_charger_device_para->chg_psy, POWER_SUPPLY_PROP_VOLTAGE_MAX, uv);
}

static ssize_t get_poll_charge_start_event(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct charge_device_info *chip = g_charger_device_para;

	if (!dev || !attr || !buf) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return -EINVAL;
	}

	if (chip)
		return snprintf(buf, PAGE_SIZE, "%d\n", chip->input_event);
	else
		return 0;
}

#define POLL_CHARGE_EVENT_MAX 3000
static ssize_t set_poll_charge_event(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct charge_device_info *chip = g_charger_device_para;
	long val = 0;

	if (!dev || !attr || !buf) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return -EINVAL;
	}

	if (chip) {
		if ((kstrtol(buf, DEC_BASE, &val) < 0) ||
			(val < INPUT_EVENT_ID_MIN) ||
			(val > INPUT_EVENT_ID_MAX)) {
			return -EINVAL;
		}
		chip->input_event = val;
		sysfs_notify(g_sysfs_poll, NULL, "poll_charge_start_event");
	}
	return count;
}
static DEVICE_ATTR(poll_charge_start_event, 0644,
	get_poll_charge_start_event, set_poll_charge_event);

static ssize_t get_poll_charge_done_event(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return -EINVAL;
	}
	return sprintf(buf, "%d\n", g_basp_learned_fcc);
}
static DEVICE_ATTR(poll_charge_done_event, 0644,
	get_poll_charge_done_event, NULL);

static ssize_t get_poll_fcc_learn_event(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return -EINVAL;
	}
	return sprintf(buf, "%d\n", g_basp_learned_fcc);
}
static DEVICE_ATTR(poll_fcc_learn_event, 0644,
	get_poll_fcc_learn_event, NULL);

static int charge_event_poll_register(struct device *dev)
{
	int ret;

	if (!dev) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return -EINVAL;
	}

	ret = sysfs_create_file(&dev->kobj,
		&dev_attr_poll_charge_start_event.attr);
	if (ret) {
		pr_err("fail to create poll node for %s\n", dev->kobj.name);
		return ret;
	}
	ret = sysfs_create_file(&dev->kobj,
		&dev_attr_poll_charge_done_event.attr);
	if (ret) {
		pr_err("fail to create poll node for %s\n", dev->kobj.name);
		return ret;
	}
	ret = sysfs_create_file(&dev->kobj,
		&dev_attr_poll_fcc_learn_event.attr);
	if (ret) {
		pr_err("fail to create poll node for %s\n", dev->kobj.name);
		return ret;
	}

	g_sysfs_poll = &dev->kobj;
	return ret;
}

void cap_learning_event_done_notify(void)
{
	struct charge_device_info *chip = g_charger_device_para;

	if (!chip) {
		pr_info("charger device is not init, do nothing!\n");
		return;
	}

	pr_info("fg cap learning event notify!\n");
	if (g_sysfs_poll)
		sysfs_notify(g_sysfs_poll, NULL, "basp_learned_fcc");
}

void wireless_connect_send_icon_uevent(int icon_type)
{
	charge_send_icon_uevent(icon_type);
	power_supply_sync_changed("battery");
	power_event_notify(POWER_NT_CHARGING, POWER_NE_START_CHARGING, NULL);
}

void cap_charge_done_event_notify(void)
{
	struct charge_device_info *chip = g_charger_device_para;

	if (!chip) {
		pr_info("charger device is not init, do nothing\n");
		return;
	}

	pr_info("charging done event notify\n");
	if (g_sysfs_poll)
		sysfs_notify(g_sysfs_poll, NULL, "poll_basp_done_event");
}

void charge_event_notify(unsigned int event)
{
	struct charge_device_info *chip = g_charger_device_para;

	if (!chip) {
		pr_info("smb device is not init, do nothing!\n");
		return;
	}
	/* avoid notify charge stop event,
	 * continuously without charger inserted
	 */
	if ((chip->input_event != event) || (event == SMB_START_CHARGING)) {
		chip->input_event = event;
		if (g_sysfs_poll)
			sysfs_notify(g_sysfs_poll, NULL,
				"poll_charge_start_event");
	}
}

static int get_iin_thermal_charge_type(void)
{
	int vol;
	int type;
	int conver_type;
	struct charge_device_info *di = g_charger_device_para;

	if (!di)
		return IIN_THERMAL_WCURRENT_5V;

	vol = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW);
	type = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_REAL_TYPE);
	conver_type = converse_usb_type(type);

	if (conver_type == CHARGER_TYPE_WIRELESS)
		return (vol / UV_TO_MV / MV_TO_V < ADAPTER_7V) ?
			IIN_THERMAL_WLCURRENT_5V : IIN_THERMAL_WLCURRENT_9V;

	return (vol / UV_TO_MV / MV_TO_V < ADAPTER_7V) ?
		IIN_THERMAL_WCURRENT_5V : IIN_THERMAL_WCURRENT_9V;
}

static void update_iin_thermal(struct charge_device_info *di)
{
	int idx;

	if (!di || !is_chg_psy_available(di)) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return;
	}

	idx = get_iin_thermal_charge_type();
	di->sysfs_data.iin_thl = di->sysfs_data.iin_thl_array[idx];
	set_property_on_psy(di->chg_psy, POWER_SUPPLY_PROP_IIN_THERMAL,
		di->sysfs_data.iin_thl);
	pr_info("update iin_thermal current: %d, type: %d",
		di->sysfs_data.iin_thl, idx);
}

static void smb_update_status(struct charge_device_info *di)
{
	unsigned int events = SMB_STOP_CHARGING;
	int charging_enabled;
	int battery_present;

	if (!di || !is_battery_psy_available(di)) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return;
	}

	charging_enabled = get_property_from_psy(di->batt_psy,
					POWER_SUPPLY_PROP_CHARGING_ENABLED);
	battery_present = get_property_from_psy(di->batt_psy,
					POWER_SUPPLY_PROP_PRESENT);
	if (!battery_present) {
		events = SMB_STOP_CHARGING;
	}
	if (!events) {
		if (charging_enabled && battery_present) {
			events = SMB_START_CHARGING;
		}
	}
	charge_event_notify(events);
	update_iin_thermal(di);
}

static void otg_type_work(struct work_struct *work)
{
	struct charge_device_info *chip = NULL;
	int otg_type;

	if (!work) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return;
	}

	chip = container_of(work, struct charge_device_info,
		otg_type_work.work);
	if (!chip) {
		pr_err("%s: Cannot get chip, fatal error\n", __func__);
		return;
	}
	otg_type = chip->otg_type;
	if (otg_type == 0) {
		pr_err("%s: otg_type do not need process\n", __func__);
		return;
	}
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	if (hihonor_oem_glink_oem_set_prop(USB_TYPEC_OEM_OTG_TYPE,
		&otg_type, sizeof(otg_type))) {
		schedule_delayed_work(&chip->otg_type_work, msecs_to_jiffies(10));
		return;
	}
#endif
	pr_err("%s: set otg type %d\n", __func__, otg_type);
}

static void smb_charger_work(struct work_struct *work)
{
	struct charge_device_info *chip = NULL; 

    if (!work) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return;
    }

    chip = container_of(work,
                struct charge_device_info, smb_charger_work.work);
    if (!chip) {
        pr_err("%s: Cannot get chip, fatal error\n", __func__);
        return;
    }

	smb_update_status(chip);
	schedule_delayed_work(&chip->smb_charger_work,
				msecs_to_jiffies(QPNP_SMBCHARGER_TIMEOUT));
}

int huawei_handle_charger_event(unsigned long event)
{
	struct charge_device_info *di = NULL;
    /* give a chance for bootup with usb present case */
	static int smb_charge_work_flag = 1;

	di = g_charger_device_para;
	if(NULL == di) {
		pr_err(" honor_handle_charger_event charge ic  is unregister !\n");
		return -EINVAL;
	}
	if (event && smb_charge_work_flag) {
		charge_event_notify(SMB_START_CHARGING);
		schedule_delayed_work(&di->smb_charger_work, msecs_to_jiffies(0));
		smb_charge_work_flag = 0;
	}
	if (!event) {
		charge_event_notify(SMB_STOP_CHARGING);
		cancel_delayed_work_sync(&di->smb_charger_work);
		smb_charge_work_flag = 1;
	}
	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
static bool is_batt_temp_normal(int temp)
{
	if (((temp > BATT_EXIST_TEMP_LOW) && (temp <= NO_CHG_TEMP_LOW)) ||
		(temp >= NO_CHG_TEMP_HIGH))
		return false;
	return true;
}
#endif

static int huawei_charger_set_runningtest(struct charge_device_info *di, int val)
{
	int iin_rt;

	if (!di || !is_battery_psy_available(di)) {
		pr_err("charge_device is not ready! cannot set runningtest\n");
		return -EINVAL;
	}

	set_property_on_psy(di->batt_psy, POWER_SUPPLY_PROP_CHARGING_ENABLED, !!val);
	iin_rt = get_property_from_psy(di->batt_psy, POWER_SUPPLY_PROP_CHARGING_ENABLED);
	di->sysfs_data.iin_rt = iin_rt;
	pr_info("[honor_charger_set_runningtest]:iin_rt = %d\n",val);
	return 0 ;
}

static int huawei_charger_set_rt_current(struct charge_device_info *di, int val)
{
	int iin_rt_curr;

	if (!di || !is_chg_psy_available(di)) {
		pr_err("charge_device is not ready! cannot set runningtest\n");
		return -EINVAL;
	}

    if (0 == val){
        iin_rt_curr = get_property_from_psy(di->chg_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_MAX);
		pr_info("honor_charger_set_rt_current  rt_curr =%d\n",iin_rt_curr);
    	}
    else if ((val <= MIN_CURRENT)&&(val > 0))
#ifndef CONFIG_HLTHERM_RUNTEST
        iin_rt_curr = MIN_CURRENT;
#else
        iin_rt_curr = HLTHERM_CURRENT;
#endif
    else
        iin_rt_curr = val;

    if (hihonor_charger_glink_set_input_current(iin_rt_curr))
        set_property_on_psy(di->chg_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, iin_rt_curr);

    di->sysfs_data.iin_rt_curr= iin_rt_curr;
    pr_info("[honor_charger_set_rt_current]:set iin_rt_curr %d\n",iin_rt_curr);
    return 0 ;
}

static int huawei_charger_set_hz_mode(struct charge_device_info *di, int val)
{
	int hiz_mode;

        if (!hihonor_charger_glink_enable_hiz(val)) {
                di->sysfs_data.hiz_mode = val;
                pr_info("[honor_charger_set_hz_mode]:set hiz_mode %d\n",val);
                return 0;
        }

	if (!di || !is_battery_psy_available(di)) {
		pr_err("charge_device is not ready! cannot set runningtest\n");
		return -EINVAL;
	}
    hiz_mode = !!val;
    set_property_on_psy(di->batt_psy, POWER_SUPPLY_PROP_HIZ_MODE,hiz_mode);
    di->sysfs_data.hiz_mode = hiz_mode;
    pr_info("[honor_charger_set_hz_mode]:set hiz_mode %d\n",hiz_mode);
    return 0 ;
}

static int huawei_charger_set_adaptor_change(struct charge_device_info *di, int val)
{
	if (!di || !is_battery_psy_available(di)) {
		pr_err("charge_device is not ready! cannot set factory diag\n");
		return -EINVAL;
	}
	set_property_on_psy(di->batt_psy, POWER_SUPPLY_PROP_ADAPTOR_VOLTAGE, val);
        pr_info("[honor_charger_set_adaptor_change]:set adpator volt %d\n",val);
	return 0;
}

#define CHARGE_SYSFS_FIELD(_name, n, m, store)	\
{	\
	.attr = __ATTR(_name, m, charge_sysfs_show, store),	\
	.name = CHARGE_SYSFS_##n,	\
}

#define CHARGE_SYSFS_FIELD_RW(_name, n)	\
	CHARGE_SYSFS_FIELD(_name, n, S_IWUSR | S_IRUGO,	\
		charge_sysfs_store)

#define CHARGE_SYSFS_FIELD_RO(_name, n)	\
	CHARGE_SYSFS_FIELD(_name, n, S_IRUGO, NULL)

static ssize_t charge_sysfs_show(struct device *dev,
 				 struct device_attribute *attr, char *buf);
static ssize_t charge_sysfs_store(struct device *dev,
				  struct device_attribute *attr, const char *buf, size_t count);

struct charge_sysfs_field_info
{
	char name;
	struct device_attribute    attr;
};


static struct charge_sysfs_field_info charge_sysfs_field_tbl[] =
{
	CHARGE_SYSFS_FIELD_RW(iin_thermal,       IIN_THERMAL),
	CHARGE_SYSFS_FIELD_RW(iin_runningtest,    IIN_RUNNINGTEST),
	CHARGE_SYSFS_FIELD_RW(iin_rt_current,   IIN_RT_CURRENT),
	CHARGE_SYSFS_FIELD_RW(enable_hiz, HIZ),

	CHARGE_SYSFS_FIELD_RW(shutdown_watchdog, WATCHDOG_DISABLE),
	CHARGE_SYSFS_FIELD_RW(enable_charger,    ENABLE_CHARGER),
	CHARGE_SYSFS_FIELD_RW(factory_diag,    FACTORY_DIAG_CHARGER),
	CHARGE_SYSFS_FIELD_RO(chargelog_head,    CHARGELOG_HEAD),
	CHARGE_SYSFS_FIELD_RO(chargelog,    CHARGELOG),
	CHARGE_SYSFS_FIELD_RW(update_volt_now,    UPDATE_VOLT_NOW),
	CHARGE_SYSFS_FIELD_RO(ibus, IBUS),
	CHARGE_SYSFS_FIELD_RO(vbus, VBUS),
	CHARGE_SYSFS_FIELD_RO(chargerType, CHARGE_TYPE),
	CHARGE_SYSFS_FIELD_RO(charge_term_volt_design,
		CHARGE_TERM_VOLT_DESIGN),
	CHARGE_SYSFS_FIELD_RO(charge_term_curr_design,
		CHARGE_TERM_CURR_DESIGN),
	CHARGE_SYSFS_FIELD_RO(charge_term_volt_setting,
		CHARGE_TERM_VOLT_SETTING),
	CHARGE_SYSFS_FIELD_RO(charge_term_curr_setting,
		CHARGE_TERM_CURR_SETTING),
	CHARGE_SYSFS_FIELD_RW(regulation_voltage, REGULATION_VOLTAGE),
	CHARGE_SYSFS_FIELD_RO(fcp_support, FCP_SUPPORT),
	CHARGE_SYSFS_FIELD_RW(adaptor_voltage, ADAPTOR_VOLTAGE),
	CHARGE_SYSFS_FIELD_RW(adaptor_test, ADAPTOR_TEST),
	CHARGE_SYSFS_FIELD_RW(thermal_reason, THERMAL_REASON),
	CHARGE_SYSFS_FIELD_RW(vterm_dec, VTERM_DEC),
	CHARGE_SYSFS_FIELD_RW(ichg_ratio, ICHG_RATIO),
	CHARGE_SYSFS_FIELD_RO(charger_online, CHARGER_ONLINE),
};

static struct attribute *charge_sysfs_attrs[ARRAY_SIZE(charge_sysfs_field_tbl) + 1];

static const struct attribute_group charge_sysfs_attr_group =
{
	.attrs = charge_sysfs_attrs,
};

 /* initialize charge_sysfs_attrs[] for charge attribute */
static void charge_sysfs_init_attrs(void)
{
	int i = 0, limit = ARRAY_SIZE(charge_sysfs_field_tbl);

	for (i = 0; i < limit; i++) {
		charge_sysfs_attrs[i] = &charge_sysfs_field_tbl[i].attr.attr;
	}

	charge_sysfs_attrs[limit] = NULL; /* Has additional entry for this */
}

/*
 * get the current device_attribute from charge_sysfs_field_tbl
 * by attr's name
 */
static struct charge_sysfs_field_info *charge_sysfs_field_lookup(const char *name)
{
	int i = 0, limit = ARRAY_SIZE(charge_sysfs_field_tbl);

    if (!name) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return NULL;
    }

	for (i = 0; i < limit; i++) {
		if (!strcmp(name, charge_sysfs_field_tbl[i].attr.attr.name)) {
			break;
		}
	}

	if (i >= limit)	{
		return NULL;
	}

	return &charge_sysfs_field_tbl[i];
}

int get_loginfo_int(struct power_supply *psy, int propery)
{
	int rc = 0;
	union power_supply_propval ret = {0, };

	if (!psy) {
		pr_err("get input source power supply node failed!\n");
		return -EINVAL;
	}
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4, 4, 0))
	rc = psy->get_property(psy, propery, &ret);
#else
	rc = psy->desc->get_property(psy, propery, &ret);
#endif
	if (rc) {
		//pr_err("Couldn't get type rc = %d\n", rc);
		ret.intval = -EINVAL;
	}

	return ret.intval;
}
EXPORT_SYMBOL_GPL(get_loginfo_int);

void strncat_length_protect(char *dest, char *src)
{
	int str_length = 0;

	if (NULL == dest || NULL == src) {
		pr_err("the dest or src is NULL");
		return;
	}
	if (strlen(dest) >= CHARGELOG_SIZE) {
		pr_err("strncat dest is full!\n");
		return;
	}

	str_length = min(CHARGELOG_SIZE - strlen(dest), strlen(src));
	if (str_length > 0) {
		strncat(dest, src, str_length);
	}
}
EXPORT_SYMBOL_GPL(strncat_length_protect);

#ifdef CONFIG_PMIC_AP_CHARGER
static void conver_usbtype(int val, char *p_str)
{
	if (NULL == p_str) {
		pr_err("the p_str is NULL\n");
		return;
	}

	switch (val) {
	case POWER_SUPPLY_TYPE_UNKNOWN:
		strncpy(p_str, "UNKNOWN", sizeof("UNKNOWN"));
		break;
	case POWER_SUPPLY_TYPE_USB_CDP:
		strncpy(p_str, "CDP", sizeof("CDP"));
		break;
	case POWER_SUPPLY_TYPE_USB:
		strncpy(p_str, "USB", sizeof("USB"));
		break;
	case POWER_SUPPLY_TYPE_USB_DCP:
		strncpy(p_str, "DC", sizeof("DC"));
		break;
	case POWER_SUPPLY_TYPE_USB_HVDCP:
		strncpy(p_str, "HVDCP", sizeof("HVDCP"));
		break;
	case POWER_SUPPLY_TYPE_USB_HVDCP_3:
		strncpy(p_str, "HVDCP_3", sizeof("HVDCP_3"));
		break;
	default:
		strncpy(p_str, "UNSTANDARD", sizeof("UNSTANDARD"));
		break;
	}
}
#else
static void conver_usbtype(int val, char *p_str)
{
        if (NULL == p_str) {
		pr_err("the p_str is NULL\n");
		return;
	}

	switch (val) {
	case CHARGER_TYPE_NON_STANDARD:
		strncpy(p_str, "UNSTANDARD", sizeof("UNSTANDARD"));
		break;
	case CHARGER_TYPE_BC_USB:
		strncpy(p_str, "CDP", sizeof("CDP"));
		break;
	case CHARGER_TYPE_USB:
		strncpy(p_str, "USB", sizeof("USB"));
		break;
	case CHARGER_TYPE_STANDARD:
		strncpy(p_str, "DC", sizeof("DC"));
		break;
	case CHARGER_TYPE_TYPEC:
		strncpy(p_str, "TYPEC", sizeof("TYPEC"));
		break;
	case CHARGER_TYPE_PD:
		strncpy(p_str, "PD", sizeof("PD"));
		break;
	case CHARGER_TYPE_FCP:
		strncpy(p_str, "FCP", sizeof("FCP"));
		break;
	case CHARGER_TYPE_WIRELESS:
		strncpy(p_str, "WLS", sizeof("WLS"));
		break;
	case CHARGER_TYPE_SCP:
		strncpy(p_str, "SCP", sizeof("SCP"));
		break;
	case CHARGER_REMOVED:
		strncpy(p_str, "REMOVED", sizeof("REMOVED"));
		break;
	default:
		strncpy(p_str, "UNKNOWN", sizeof("UNKNOWN"));
		break;
	}
}
#endif /* CONFIG_PMIC_AP_CHARGER */

static void conver_charging_status(int val, char *p_str)
{
	if (NULL == p_str) {
		pr_err("the p_str is NULL\n");
		return;
	}

	switch (val) {
	case POWER_SUPPLY_STATUS_UNKNOWN:
		strncpy(p_str, "UNKNOWN", sizeof("UNKNOWN"));
		break;
	case POWER_SUPPLY_STATUS_CHARGING:
		strncpy(p_str, "CHARGING", sizeof("CHARGING"));
		break;
	case POWER_SUPPLY_STATUS_DISCHARGING:
		strncpy(p_str, "DISCHARGING", sizeof("DISCHARGING"));
		break;
	case POWER_SUPPLY_STATUS_NOT_CHARGING:
		strncpy(p_str, "NOTCHARGING", sizeof("NOTCHARGING"));
		break;
	case POWER_SUPPLY_STATUS_FULL:
		strncpy(p_str, "FULL", sizeof("FULL"));
		break;
	default:
		break;
	}
}

static void conver_charger_health(int val, char *p_str)
{
	if (NULL == p_str) {
		pr_err("the p_str is NULL\n");
		return;
	}

	switch (val) {
	case POWER_SUPPLY_HEALTH_OVERHEAT:
		strncpy(p_str, "OVERHEAT", sizeof("OVERHEAT"));
		break;
	case POWER_SUPPLY_HEALTH_COLD:
		strncpy(p_str, "COLD", sizeof("COLD"));
		break;
	case POWER_SUPPLY_HEALTH_WARM:
		strncpy(p_str, "WARM", sizeof("WARM"));
		break;
	case POWER_SUPPLY_HEALTH_COOL:
		strncpy(p_str, "COOL", sizeof("COOL"));
		break;
	case POWER_SUPPLY_HEALTH_GOOD:
		strncpy(p_str, "GOOD", sizeof("GOOD"));
		break;
	default:
		break;
	}
}

static int get_charger_online(void)
{
	int online = 0;
	int type = 0;

	if (is_usb_psy_available(g_charger_device_para))
		type = get_loginfo_int(g_charger_device_para->usb_psy, POWER_SUPPLY_PROP_REAL_TYPE);
	if (type != 0)
		online = 1;

	return online;
}

static bool charger_shutdown_flag;
static int __init early_parse_shutdown_flag(char *p)
{
	if (p) {
		if (!strcmp(p, "charger")) {
			charger_shutdown_flag = true;
		}
	}
	return 0;
}
early_param("androidboot.mode", early_parse_shutdown_flag);

static void get_charger_shutdown_flag(bool flag, char *p_str)
{
	if (NULL == p_str) {
		pr_err("the p_str is NULL\n");
		return;
	}
	if (flag) {
		strncpy(p_str, "OFF", sizeof("OFF"));
	} else {
		strncpy(p_str, "ON", sizeof("ON"));
	}
}

/* show the value for all charge device's node */
static ssize_t charge_sysfs_show(struct device *dev,
                                 struct device_attribute *attr, char *buf)
{
	int ret = 0;
	struct charge_sysfs_field_info *info = NULL;
	struct charge_device_info *di = NULL;
	int ibus = 0;
	unsigned int type = CHARGER_REMOVED;
#ifdef CONFIG_DIRECT_CHARGER
	unsigned int val;
#endif
	int vol;

    if (!dev || !attr || !buf) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return -EINVAL;
    }

    di = dev_get_drvdata(dev);
	info = charge_sysfs_field_lookup(attr->attr.name);
	if (!di || !info) {
 		return -EINVAL;
	}

	switch (info->name) {
	case CHARGE_SYSFS_IIN_THERMAL:
		ret = snprintf(buf, MAX_SIZE, "%u\n", di->sysfs_data.iin_thl);
		break;
	case CHARGE_SYSFS_IIN_RUNNINGTEST:
		ret = snprintf(buf, MAX_SIZE, "%u\n", di->sysfs_data.iin_rt);
		break;
	case CHARGE_SYSFS_IIN_RT_CURRENT:
		ret = snprintf(buf, MAX_SIZE, "%u\n", di->sysfs_data.iin_rt_curr);
		break;
	case CHARGE_SYSFS_ENABLE_CHARGER:
#ifndef CONFIG_HLTHERM_RUNTEST
		ret = snprintf(buf, MAX_SIZE, "%u\n", di->chrg_config);
		break;
#endif
	case CHARGE_SYSFS_WATCHDOG_DISABLE:
		return snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_data.wdt_disable);
	case CHARGE_SYSFS_FACTORY_DIAG_CHARGER:
		ret = snprintf(buf, MAX_SIZE, "%u\n", di->factory_diag);
		break;
	case CHARGE_SYSFS_CHARGELOG_HEAD:
		return power_log_common_operate(POWER_LOG_DUMP_LOG_HEAD,
			buf, PAGE_SIZE);
	case CHARGE_SYSFS_CHARGELOG:
		return power_log_common_operate(POWER_LOG_DUMP_LOG_CONTENT,
			buf, PAGE_SIZE);
	case CHARGE_SYSFS_UPDATE_VOLT_NOW:
        /* always return 1 when reading UPDATE_VOLT_NOW property */
		ret = snprintf(buf, MAX_SIZE, "%u\n", 1);
		break;
	case CHARGE_SYSFS_IBUS:
		if (is_battery_psy_available(di))
			ibus = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_NOW);
		return snprintf(buf, MAX_SIZE, "%d\n", ibus);
		break;
	case CHARGE_SYSFS_VBUS:
		vol = get_loginfo_int(di->usb_psy,
			POWER_SUPPLY_PROP_VOLTAGE_NOW);
		ret = snprintf(buf, MAX_SIZE, "%d\n", vol);
		break;
	case CHARGE_SYSFS_CHARGE_TYPE:
		if (is_usb_psy_available(di))
			type = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_REAL_TYPE);
		return snprintf(buf, PAGE_SIZE, "%d\n", type);
		break;
	case CHARGE_SYSFS_FCP_SUPPORT:
		return snprintf(buf, MAX_SIZE, "1\n");
		break;
	case CHARGE_SYSFS_HIZ:
		ret = snprintf(buf, MAX_SIZE, "%u\n", di->sysfs_data.hiz_mode);
		break;
	case CHARGE_SYSFS_CHARGE_TERM_VOLT_DESIGN:
		ret = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_data.vterm);
		break;
	case CHARGE_SYSFS_CHARGE_TERM_CURR_DESIGN:
		ret = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_data.iterm);
		break;
	case CHARGE_SYSFS_CHARGE_TERM_VOLT_SETTING:
		vol = get_loginfo_int(di->main_psy,
			POWER_SUPPLY_PROP_VOLTAGE_MAX);
		ret = snprintf(buf, PAGE_SIZE, "%d\n",
			((vol < di->sysfs_data.vterm) ? vol :
			di->sysfs_data.vterm));
		break;
	case CHARGE_SYSFS_CHARGE_TERM_CURR_SETTING:
		ret = snprintf(buf, PAGE_SIZE, "%d\n", di->sysfs_data.iterm);
		break;
#ifdef CONFIG_DIRECT_CHARGER
	case CHARGE_SYSFS_ADAPTOR_TEST:
		ret = chg_get_adaptor_test_result(buf);
		return ret;
	case CHARGE_SYSFS_VTERM_DEC:
		val = huawei_get_basp_vterm_dec();
		ret = snprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
	case CHARGE_SYSFS_ICHG_RATIO:
		val = huawei_get_basp_ichg_ratio();
		ret = snprintf(buf, PAGE_SIZE, "%d\n", val);
		break;
#endif
	case CHARGE_SYSFS_REGULATION_VOLTAGE:
		return snprintf(buf, PAGE_SIZE, "%u\n", di->sysfs_data.vterm_rt);
	case CHARGE_SYSFS_ADAPTOR_VOLTAGE:
		if (di->reset_adapter)
			return snprintf(buf, PAGE_SIZE, "%d\n", ADAPTER_5V);
		else
			return snprintf(buf, PAGE_SIZE, "%d\n", ADAPTER_9V);
		break;
	case CHARGE_SYSFS_CHARGER_ONLINE:
		return snprintf(buf, PAGE_SIZE, "%d\n", get_charger_online());
	default:
		pr_err("(%s)NODE ERR!!HAVE NO THIS NODE:(%d)\n", __func__, info->name);
		break;
	}

	return ret;
}

/* set the value for charge_data's node which is can be written */
static ssize_t charge_sysfs_store(struct device *dev, struct device_attribute *attr,
					const char *buf, size_t count)
{
	long val = 0;
	struct charge_sysfs_field_info *info = NULL;
	struct charge_device_info *di = NULL;
#ifdef CONFIG_DIRECT_CHARGER
	int ret;
#endif

    if (!dev || !attr || !buf) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return -EINVAL;
    }

    di = dev_get_drvdata(dev);
    if (!di) {
        pr_err("%s: Cannot get charge device info, fatal error\n", __func__);
        return -EINVAL;
    }

	info = charge_sysfs_field_lookup(attr->attr.name);
	if (!info)	{
		return -EINVAL;
	}

	switch (info->name) {
	/* hot current limit function*/
	case CHARGE_SYSFS_IIN_THERMAL:
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0)||(val < 0)||(val > MAX_CURRENT)) {
			return -EINVAL;
		}
		if (val <= LIMIT_CURRENT_TOO_LOW){
			val = get_property_from_psy(di->chg_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_MAX);
		}
		if (hihonor_charger_glink_set_charge_current(val) && is_chg_psy_available(di)) {
			set_property_on_psy(di->chg_psy, POWER_SUPPLY_PROP_IIN_THERMAL, val);
			di->sysfs_data.iin_thl = val;
			pr_info("THERMAL set input current = %ld\n", val);
		}
		di->sysfs_data.iin_thl = val;
		pr_info("THERMAL adsp set input current = %ld\n", val);
		break;
	/* running test charging/discharge function*/
	case CHARGE_SYSFS_IIN_RUNNINGTEST:
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0)||(val < 0)||(val > MAX_CURRENT)) {
			return -EINVAL;
		}
		pr_info("THERMAL set running test val = %ld\n", val);
		huawei_charger_set_runningtest(di, val);
		pr_info("THERMAL set running test iin_rt = %d\n", di->sysfs_data.iin_rt);
		break;
    /* running test charging/discharge function*/
	case CHARGE_SYSFS_IIN_RT_CURRENT:
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0)||(val < 0)||(val > MAX_CURRENT)) {
			return -EINVAL;
		}
		pr_info("THERMAL set rt test val = %ld\n", val);  
		huawei_charger_set_rt_current(di, val);
		pr_info("THERMAL set rt test iin_rt = %d\n", di->sysfs_data.iin_rt);
		break;
	/* charging/discharging function*/
	case CHARGE_SYSFS_ENABLE_CHARGER:
        /* enable charger input must be 0 or 1 */
#ifndef CONFIG_HLTHERM_RUNTEST
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0)||(val < 0) || (val > 1)) {
			return -EINVAL;
		}
		pr_info("ENABLE_CHARGER set enable charger val = %ld\n", val);
		if (is_battery_psy_available(di)) {
			set_property_on_psy(di->batt_psy, POWER_SUPPLY_PROP_CHARGING_ENABLED, !!val);
			di->chrg_config = val;
			pr_info("ENABLE_CHARGER set chrg_config = %d\n", di->chrg_config);
		}
		break;
#endif
	/* factory diag function*/
	case CHARGE_SYSFS_FACTORY_DIAG_CHARGER:
        /* factory diag valid input is 0 or 1 */
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0)||(val < 0) || (val > 1)) {
			return -EINVAL;
		}
		pr_info("ENABLE_CHARGER set factory diag val = %ld\n", val);
		if (is_battery_psy_available(di)) {
			set_property_on_psy(di->batt_psy, POWER_SUPPLY_PROP_CHARGING_ENABLED, !!val);
			di->factory_diag = val;
			pr_info("ENABLE_CHARGER set factory_diag = %d\n", di->factory_diag);
		}
		break;
	case CHARGE_SYSFS_UPDATE_VOLT_NOW:
        /* update volt now valid input is 1 */
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0) || (1 != val))
			return -EINVAL;
		break;
	case CHARGE_SYSFS_WATCHDOG_DISABLE:
		di->sysfs_data.wdt_disable = val;
		pr_info("RUNNINGTEST set wdt disable = %d\n",
			di->sysfs_data.wdt_disable);
		break;
	case CHARGE_SYSFS_HIZ:
        /* hiz valid input is 0 or 1 */
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0) || (val < 0) || (val > 1))
			return -EINVAL;
		pr_info("ENABLE_CHARGER set hz mode val = %ld\n", val);
		huawei_charger_set_hz_mode(di,val);
		break;
	case CHARGE_SYSFS_ADAPTOR_TEST:
		if ((strict_strtol(buf, 10, &val) < 0) || (val < MIN_ADAPTOR_TEST_INS_NUM) ||
			(val > MAX_ADAPTOR_TEST_INS_NUM))
			return -EINVAL;
		if (val == ADAPTOR_TEST_START) {
			pr_info("Reset adaptor test result to FAIL\n");
			clear_adaptor_test_result();
		}
		break;
	case CHARGE_SYSFS_REGULATION_VOLTAGE:
		if ((strict_strtol(buf, DEC_BASE, &val) < 0) || (val < 3200) || (val > 4400))
			return -EINVAL;
		di->sysfs_data.vterm_rt = val;
		ret = charger_set_constant_voltage(di->sysfs_data.vterm_rt * 1000);
		hwlog_info("RUNNINGTEST set terminal voltage = %d\n",
			   di->sysfs_data.vterm_rt);
		break;
	case CHARGE_SYSFS_ADAPTOR_VOLTAGE:
		if ((kstrtol(buf, INPUT_NUMBER_BASE, &val) < 0)||(val < 0)||(val > INPUT_NUMBER_BASE) ) {
			return -EINVAL;
		}
		if(ADAPTER_5V == val) {
			pr_info("Reset adaptor to 5V\n");
			di->reset_adapter = TRUE;
		} else {
			pr_info("Restore adaptor to 9V\n");
			di->reset_adapter = FALSE;
		}
		huawei_charger_set_adaptor_change(di,di->reset_adapter);
		break;
	case CHARGE_SYSFS_VTERM_DEC:
		huawei_set_basp_vterm_dec(val);
		break;
	case CHARGE_SYSFS_ICHG_RATIO:
		huawei_set_basp_ichg_ratio(val);
		break;
	default:
		pr_err("(%s)NODE ERR!!HAVE NO THIS NODE:(%d)\n", __func__, info->name);
		break;
	}
	return count;
}

static struct class *hw_power_class;
struct class *hw_power_get_class(void)
{
	if (NULL == hw_power_class) {
		hw_power_class = class_create(THIS_MODULE, "hw_power");
		if (IS_ERR(hw_power_class)) {
			pr_err("hw_power_class create fail");
			return NULL;
		}
	}
	return hw_power_class;
}
EXPORT_SYMBOL_GPL(hw_power_get_class);

static void ChargeSysfsCreateGroup(struct device *dev)
{
	charge_sysfs_init_attrs();
	power_sysfs_create_link_group("hw_power", "charger", "charge_data", dev, &charge_sysfs_attr_group);
}

static void ChargeSysfsRemoveGroup(struct device *dev)
{
	power_sysfs_remove_link_group("hw_power", "charger", "charge_data", dev, &charge_sysfs_attr_group);
}

static int charger_plug_notifier_call(struct notifier_block *nb, unsigned long event, void *data)
{
	if(huawei_handle_charger_event(event)){
		return NOTIFY_BAD;
	}
	pr_info("charger plug in or out\n");
	return NOTIFY_OK;
}
static struct charge_device_info *charge_device_info_alloc(void)
{
	struct charge_device_info *di;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di) {
		pr_err("alloc di failed\n");
		return NULL;
	}
	di->sysfs_data.reg_value = kzalloc(sizeof(char) * CHARGELOG_SIZE, GFP_KERNEL);
	if (!di->sysfs_data.reg_value) {
		pr_err("alloc reg_value failed\n");
		goto alloc_fail_1;
	}
	di->sysfs_data.reg_head = kzalloc(sizeof(char) * CHARGELOG_SIZE, GFP_KERNEL);
	if (!di->sysfs_data.reg_head) {
		pr_err("alloc reg_head failed\n");
		goto alloc_fail_2;
	}
	return di;

alloc_fail_2:
	kfree(di->sysfs_data.reg_value);
	di->sysfs_data.reg_value = NULL;
alloc_fail_1:
	kfree(di);
	return NULL;
}

static void charge_device_info_free(struct charge_device_info *di)
{
	if(di != NULL) {

		if(di->sysfs_data.reg_head != NULL) {
			kfree(di->sysfs_data.reg_head);
			di->sysfs_data.reg_head = NULL;
		}
		if(di->sysfs_data.reg_value != NULL) {
			kfree(di->sysfs_data.reg_value);
			di->sysfs_data.reg_value = NULL;
		}
		kfree(di);
	}
}

#if 0
static bool hw_charger_init_psy(struct charge_device_info *di)
{
	di->bk_batt_psy = power_supply_get_by_name("bk_battery");
	if (!di->bk_batt_psy) {
		pr_err("%s: bk_batt_psy is null\n", __func__);
		return false;
	}
	di->pc_port_psy = power_supply_get_by_name("pc_port");
	if (!di->pc_port_psy){
		pr_err("%s: pc_port_psy is null\n", __func__);
		return false;
	}

	di->batt_psy = power_supply_get_by_name("battery");
	if (!di->batt_psy){
		pr_err("%s: batt_psy is null\n", __func__);
		return false;
	}

	di->chg_psy = power_supply_get_by_name("huawei_charge");
	if (!di->chg_psy) {
		pr_err("%s: chg_psy is null\n", __func__);
		return false;
	}

	di->bms_psy = power_supply_get_by_name("bms");
	if (!di->bms_psy){
		pr_err("%s: bms_psy is null\n", __func__);
		return false;
	}

	di->main_psy = power_supply_get_by_name("main");
	if (!di->main_psy){
		pr_err("%s: main_psy is null\n", __func__);
		return false;
	}

	return true;
}
#endif

int battery_get_bat_temperature(void)
{
	int temp = 250;
	if (is_battery_psy_available(g_charger_device_para)) {
		temp = get_property_from_psy(g_charger_device_para->batt_psy,
			POWER_SUPPLY_PROP_TEMP);
	}
	return temp;
}

static int dcp_charger_enable_charge(struct charge_device_info *di, int val)
{
	if (!hihonor_charger_glink_enable_charge(val)) {
		di->sysfs_data.charge_enable = val;
		if (!is_usb_psy_available(g_charger_device_para) ||
			!get_property_from_psy(g_charger_device_para->usb_psy,
				POWER_SUPPLY_PROP_ONLINE)) {
			return 0;
		}

		val  ==  1 ? power_event_notify(POWER_NT_CHARGING, POWER_NE_START_CHARGING, NULL)
			: power_event_notify(POWER_NT_CHARGING, POWER_NE_STOP_CHARGING, NULL);
		return 0;
	}
#ifdef CONFIG_PMIC_AP_CHARGER
	if (is_battery_psy_available(g_charger_device_para)) {
		set_property_on_psy(g_charger_device_para->batt_psy, POWER_SUPPLY_PROP_CHARGING_ENABLED, !!val);
		di->sysfs_data.charge_enable = get_property_from_psy(g_charger_device_para->batt_psy,
			POWER_SUPPLY_PROP_CHARGING_ENABLED);
	}
#endif
	return 0;
}

static int dcp_charger_set_rt_current(struct charge_device_info *di, int val)
{
	int iin_rt_curr;

	if (!di || !(is_chg_psy_available(di))) {
		pr_err("charge_device is not ready\n");
		return -EINVAL;
	}
	/* 0&1:open limit 100:limit to 100 */
	if ((val == 0) || (val == 1)) {
		iin_rt_curr = get_property_from_psy(di->chg_psy,
			POWER_SUPPLY_PROP_INPUT_CURRENT_MAX);
		pr_info("%s rt_current =%d\n", __func__, iin_rt_curr);
	} else if ((val <= MIN_CURRENT) && (val > 1)) {
#ifndef CONFIG_HLTHERM_RUNTEST
					iin_rt_curr = MIN_CURRENT;
#else
					iin_rt_curr = HLTHERM_CURRENT;
#endif
	} else {
		iin_rt_curr = val;
	}

	if (hihonor_charger_glink_set_input_current(iin_rt_curr))
		set_property_on_psy(di->chg_psy,
			POWER_SUPPLY_PROP_INPUT_CURRENT_MAX, iin_rt_curr);

	di->sysfs_data.iin_rt_curr = iin_rt_curr;
	return 0;

}

static int dcp_set_enable_charger(unsigned int val)
{
	struct charge_device_info *di = g_charger_device_para;
	int ret;
	int batt_temp;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	if ((val < DIS_CHG) || (val > EN_CHG))
		return -1;

	hwlog_info("dcp: RUNNINGTEST chg stat = %d, set charge enable = %d\n",
		di->sysfs_data.charge_enable, val);

	batt_temp = battery_get_bat_temperature();

#ifndef CONFIG_HLTHERM_RUNTEST
	if ((val == EN_CHG) && !is_batt_temp_normal(batt_temp)) {
		hwlog_err("dcp: battery temp is %d, abandon enable_charge\n",
			batt_temp);
		return -1;
	}
#endif

	ret = dcp_charger_enable_charge(di, val);
	return ret;
}

static int dcp_get_enable_charger(unsigned int *val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->sysfs_data.charge_enable;
	return 0;
}

static int sdp_set_iin_limit(unsigned int val)
{
	struct charge_device_info *di = g_charger_device_para;

	if(!di || !(is_main_psy_available(di))) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	/* sdp current limit > 450mA */
	if (power_cmdline_is_factory_mode() && (val >= 450)) {
		if (!hihonor_charger_glink_set_sdp_input_current(val))
			return 0;

		set_property_on_psy(di->main_psy,
			POWER_SUPPLY_PROP_CURRENT_MAX, val);
		hwlog_info("set sdp ibus current is: %u\n", val);
	}
	return 0;
}

static int dcp_set_iin_limit(unsigned int val)
{
	struct charge_device_info *di = g_charger_device_para;
	int ret;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	ret = dcp_charger_set_rt_current(di, val);

	hwlog_info("set input current is:%d\n", val);

	return ret;
}

static int dcp_get_iin_limit(unsigned int *val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->sysfs_data.iin_rt_curr;

	return 0;
}

#ifndef CONFIG_HLTHERM_RUNTEST
static int dcp_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	/*
	 * set default iin when value is 0 or 1
	 * set 100mA for iin when value is between 2 and 100
	 */
	if ((val == 0) || (val == 1))
		di->sysfs_data.iin_thl_array[idx] = DEFAULT_IIN_THL;
	else if ((val > 1) && (val <= MIN_CURRENT))
		di->sysfs_data.iin_thl_array[idx] = MIN_CURRENT;
	else
		di->sysfs_data.iin_thl_array[idx] = val;

	hwlog_info("thermal send input current = %d, type: %u\n",
		di->sysfs_data.iin_thl_array[idx], idx);
	if (idx != get_iin_thermal_charge_type())
		return 0;

	di->sysfs_data.iin_thl = di->sysfs_data.iin_thl_array[idx];
	set_property_on_psy(di->chg_psy, POWER_SUPPLY_PROP_IIN_THERMAL,
		di->sysfs_data.iin_thl);
	hwlog_info("thermal set input current = %d\n", di->sysfs_data.iin_thl);
	return 0;
}
#else
static int dcp_set_iin_limit_array(unsigned int idx, unsigned int val)
{
	return 0;
}
#endif /* CONFIG_HLTHERM_RUNTEST */

static int dcp_set_iin_thermal_all(unsigned int value)
{
	int i;

	for (i = IIN_THERMAL_CHARGE_TYPE_BEGIN; i < IIN_THERMAL_CHARGE_TYPE_END; i++) {
		if (dcp_set_iin_limit_array(i, value))
			return -EINVAL;
	}
	return 0;
}

static int dcp_set_ichg_limit(unsigned int val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("di or core_data is null\n");
		return -EINVAL;
	}

	return 0;
}
static void huawei_battery_vbat_max(int *max)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di || !(is_chg_psy_available(di))) {
		hwlog_err("charge_device is not ready\n");
		*max = DEFAULT_VTERM;
		return;
	}

	*max = get_property_from_psy(di->chg_psy,
		POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN);

	hwlog_info("honor_battery_vbat_max max=%d\n", *max);
}

static void huawei_battery_set_vbat_max(int max)
{
	power_supply_set_int_property_value("huawei_charge", POWER_SUPPLY_PROP_VOLTAGE_MAX, max);
}

static int dcp_set_vterm_dec(unsigned int val)
{
	int vterm_max;
	int vterm_basp;
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("%s di or core_data is null\n", __func__);
		return -EINVAL;
	}

	if (!val)
		return 0;
	huawei_battery_vbat_max(&vterm_max);
	if (vterm_max < 0) {
		hwlog_err("get vterm_max=%d fail\n", vterm_max);
		vterm_max = DEFAULT_VTERM;
	}
	hihonor_glink_set_vterm_dec(val);
	val *= 1000;
	vterm_basp = vterm_max - val;
	vterm_basp /= 1000;
	huawei_battery_set_vbat_max(vterm_basp);
	hwlog_info("%s set charger terminal voltage is:%d\n",
		__func__, vterm_basp);
	return 0;
}

static int dcp_get_hota_iin_limit(unsigned int *val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->hota_iin_limit;
	return 0;
}

static int dcp_get_startup_iin_limit(unsigned int *val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->startup_iin_limit;
	return 0;
}

#ifdef CONFIG_FCP_CHARGER
static int fcp_get_rt_test_time(unsigned int *val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	*val = di->rt_test_time;
	return 0;
}
extern enum fcp_check_stage_type fcp_get_stage(void);

u32 get_rt_curr_th(void)
{
	struct charge_device_info *di = g_charger_device_para;
	if (!di) {
		hwlog_err("di is null\n");
		return 0;
	}
	return di->rt_curr_th;
}

void set_rt_test_result(bool flag)
{
	struct charge_device_info *di = g_charger_device_para;
	if (!di) {
		hwlog_err("di is null\n");
		return;
	}
	di->rt_test_succ = flag;
}

static int fcp_get_rt_test_result(unsigned int *val)
{
	int tbatt;
	int ibat;
	struct charge_device_info *di = g_charger_device_para;
	enum fcp_check_stage_type fcp_charge_flag = fcp_get_stage();

	if (!di) {
		hwlog_err("di is null\n");
		return -EINVAL;
	}

	tbatt = battery_get_bat_temperature();
	ibat = battery_get_bat_current() / 10;

	if ((di->sysfs_data.charge_enable == 0) ||
		(tbatt >= RT_TEST_TEMP_TH) ||
		di->rt_test_succ ||
		(fcp_charge_flag &&
		(ibat >= (int)di->rt_curr_th)))
		*val = 0; /* 0: succ */
	else
		*val = 1; /* 1: fail */

	di->rt_test_succ = false;
	return 0;
}
#endif

static int dcp_set_iin_thermal(unsigned int index, unsigned int value)
{
	if (index >= IIN_THERMAL_CHARGE_TYPE_END) {
		pr_err("error index: %u, out of boundary\n", index);
		return -EINVAL;
	}
	return dcp_set_iin_limit_array(index, value);
}

static int set_adapter_voltage(int val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!di || (val < 0)) {
		pr_err("di is null or val is invalid\n");
		return -EINVAL;
	}

	if (val == ADAPTER_5V)
		di->reset_adapter = TRUE;
	else
		di->reset_adapter = FALSE;

	huawei_charger_set_adaptor_change(di, di->reset_adapter);
	return 0;
}

static int get_adapter_voltage(int *val)
{
	struct charge_device_info *di = g_charger_device_para;

	if (!val || !di)
		return -EINVAL;
	if (di->reset_adapter)
		*val = ADAPTER_5V;
	else
		*val = ADAPTER_9V;
	return 0;
}

int adaptor_cfg_for_wltx_vbus(int vol, int cur)
{
	hwlog_info("adaptor_cfg for wireless_tx: vol=%d, cur=%d\n", vol, cur);
	if (!adaptor_cfg_for_normal_chg(vol, cur)) {
		hwlog_info("%s: cfg_normal_chg failed\n", __func__);
		return -EIO;
	}
	charger_vbus_init_handler(vol);
	return 0;
}

bool adaptor_cfg_for_normal_chg(int vol, int cur)
{
	struct adapter_init_data sid;

	sid.scp_mode_enable = 1;
	sid.vset_boundary = vol;
	sid.iset_boundary = cur;
	sid.init_voltage = vol;
	sid.watchdog_timer = 0; /* disable watchdog for normal charge */

	if (adapter_set_init_data(ADAPTER_PROTOCOL_SCP, &sid))
		return false;
	if (adapter_set_output_voltage(ADAPTER_PROTOCOL_SCP, vol))
		return false;
	if (adapter_set_output_current(ADAPTER_PROTOCOL_SCP, cur))
		return false;

	return true;
}

void charger_vbus_init_handler(int vbus)
{
	struct charge_device_info *di = g_charger_device_para;
	int vset = 0;
	if (!di)
		return;

	if (vbus >= ADAPTER_12V * MVOLT_PER_VOLT)
		vset = ADAPTER_12V;
	else if (vbus >= ADAPTER_9V * MVOLT_PER_VOLT)
		vset = ADAPTER_9V;
	else
		vset = ADAPTER_5V;
	hwlog_info("[vbus_init_handler] vbus:%dmV\n", vbus);
}

int charger_get_output_voltage(int *output_voltage)
{
	if (adapter_get_output_voltage(ADAPTER_PROTOCOL_SCP, output_voltage)) {
		hwlog_info("[charger_get_output_voltage] get vout failed\n");
		return -1;
	}
	else
		return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static void charger_glink_sync_work(void *dev_data)
{
	struct charge_device_info *di = (struct charge_device_info *)dev_data;

	if (!di)
		return;

	(void)hihonor_oem_glink_oem_set_prop(USB_TYPEC_OEM_OTG_TYPE, &(di->otg_type),
		sizeof(di->otg_type));
}
#endif

static struct power_if_ops sdp_if_ops = {
	.set_iin_limit = sdp_set_iin_limit,
	.type_name = "sdp",
};

static struct power_if_ops dcp_if_ops = {
	.set_enable_charger = dcp_set_enable_charger,
	.get_enable_charger = dcp_get_enable_charger,
	.set_iin_limit = dcp_set_iin_limit,
	.get_iin_limit = dcp_get_iin_limit,
	.set_ichg_limit = dcp_set_ichg_limit,
	.set_vterm_dec = dcp_set_vterm_dec,
	.get_hota_iin_limit = dcp_get_hota_iin_limit,
	.get_startup_iin_limit = dcp_get_startup_iin_limit,
	.set_iin_thermal = dcp_set_iin_thermal,
	.set_iin_thermal_all = dcp_set_iin_thermal_all,
	.set_adap_volt = set_adapter_voltage,
	.get_adap_volt = get_adapter_voltage,
	.type_name = "dcp",
};

static struct power_if_ops fcp_if_ops = {
	.get_rt_test_time = fcp_get_rt_test_time,
	.get_rt_test_result = fcp_get_rt_test_result,
	.type_name = "hvc",
};

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static struct hihonor_glink_ops charge_glink_ops = {
	.sync_data = charger_glink_sync_work,
};
#endif

static int honor_get_register_head(char *buffer, int size, void *dev_data)
{
	struct charge_device_info *di = (struct charge_device_info *)dev_data;

	if (!buffer || !di) {
		pr_err("%s di is null\n",__func__);
		return -1;
	}

	mutex_lock(&di->sysfs_data.dump_reg_head_lock);
	snprintf(buffer, MAX_SIZE, "online type      Vusb  iin_th ch_en   status    health   bat_on  temp   temp_r  vol    cur   cap   r_soc  vbus   ibus  %s  Mode  ",
	di->sysfs_data.reg_head);
	mutex_unlock(&di->sysfs_data.dump_reg_head_lock);

	return 0;
}

extern bool g_charger_shutdown_flag;
static int honor_value_dump(char *buffer, int size, void *dev_data)
{
	int online = 0, in_type = 0, ch_en = 0, status = 0;
	int health = 0, bat_present = 0, ret = 0;
	int real_temp = 0, temp = 0, vol = 0, cur = 0, capacity = 0, real_soc = 0, ibus = 0, usb_vol = 0;
	char cType[CHG_LOG_STR_LEN] = {0};
	char cStatus[CHG_LOG_STR_LEN] = {0};
	char cHealth[CHG_LOG_STR_LEN] = {0};
	char cOn[CHG_LOG_STR_LEN] = {0};
	batt_mngr_get_buck_info buck_info = {0};
	struct charge_device_info *di = (struct charge_device_info *)dev_data;

	if (!buffer || !di) {
		pr_err("%s di is null\n",__func__);
		return -1;
	}

	if (is_usb_psy_available(di)) {
		online = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_ONLINE);
		if (is_pc_port_psy_available(di))
			online = online || get_loginfo_int(di->pc_port_psy, POWER_SUPPLY_PROP_ONLINE);
		in_type = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_REAL_TYPE);
		conver_usbtype(in_type, cType);
		usb_vol = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW);
	}
	if (is_battery_psy_available(di)) {
#ifdef CONFIG_PMIC_AP_CHARGER
		ch_en = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_CHARGING_ENABLED);
#else
		ch_en = di->sysfs_data.charge_enable;
#endif
		status = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_STATUS);
		conver_charging_status(status, cStatus);
		health = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_HEALTH);
		conver_charger_health(health, cHealth);
		bat_present = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_PRESENT);
		temp = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_TEMP);
		vol = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_VOLTAGE_NOW);
		cur = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_CURRENT_NOW);
		capacity = get_loginfo_int(di->batt_psy, POWER_SUPPLY_PROP_CAPACITY);
#ifdef CONFIG_PMIC_AP_CHARGER
		ibus = get_loginfo_int(di->usb_psy, POWER_SUPPLY_PROP_INPUT_CURRENT_NOW);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
		ret = hihonor_oem_glink_oem_get_prop(CHARGER_OEM_BUCK_INFO, &buck_info, sizeof(buck_info));
		ibus = buck_info.buck_ibus;
		if(ret)
			pr_err("oem get ibus fail\n");
#endif
#endif
	}
	if (is_bk_battery_psy_available(di))
		real_temp = get_loginfo_int(di->bk_batt_psy, POWER_SUPPLY_PROP_TEMP);
#ifdef CONFIG_PMIC_AP_CHARGER
	if (is_bms_psy_available(di))
		real_soc = get_loginfo_int(di->bms_psy, POWER_SUPPLY_PROP_CAPACITY_RAW);
#endif
	get_charger_shutdown_flag(charger_shutdown_flag, cOn);
	mutex_lock(&di->sysfs_data.dump_reg_lock);
	ret = snprintf(buffer, MAX_SIZE, "%-7d%-10s%-7d%-7d%-7d%-10s%-10s%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%-7d%s %-7s",
		online, cType, usb_vol/1000, di->sysfs_data.iin_thl,
		ch_en, cStatus, cHealth, bat_present, temp, real_temp, vol/1000,
		cur, capacity, real_soc, buck_info.buck_vbus, ibus, di->sysfs_data.reg_value, cOn);
	mutex_unlock(&di->sysfs_data.dump_reg_lock);

	return 0;
}

static struct power_log_ops honor_charger_log_ops = {
	.dev_name = "buck_charger",
	.dump_log_head = honor_get_register_head,
	.dump_log_content = honor_value_dump,
};

static void parameter_init(struct charge_device_info *di)
{
	if (!di)
		return;

	di->sysfs_data.iin_thl = DEFAULT_IIN_THL;
	di->sysfs_data.iin_rt = DEFAULT_IIN_RT;
	di->sysfs_data.iin_rt_curr = DEFAULT_IIN_CURRENT;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WCURRENT_5V] = DEFAULT_IIN_THL;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WCURRENT_9V] = DEFAULT_IIN_THL;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WLCURRENT_5V] = DEFAULT_IIN_THL;
	di->sysfs_data.iin_thl_array[IIN_THERMAL_WLCURRENT_9V] = DEFAULT_IIN_THL;
	di->sysfs_data.hiz_mode = DEFAULT_HIZ_MODE;
	di->chrg_config = DEFAULT_CHRG_CONFIG;
	di->factory_diag = DEFAULT_FACTORY_DIAG;
	di->sysfs_data.charge_enable = DEFAULT_CHRG_CONFIG;
	di->sysfs_data.vterm_rt = DEFAULT_VTERM;
	di->reset_adapter = FALSE;
}

static void parse_charge_dts(struct charge_device_info *di)
{
	struct device_node *node = NULL;
	int ret = 0;

	parameter_init(di);
	if (!(di->dev) || !(di->dev->of_node)) {
		pr_err("%s: dev or of_node is null\n", __func__);
		return;
	}
	node = di->dev->of_node;
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"huawei,vterm", &(di->sysfs_data.vterm), DEFAULT_VTERM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"huawei,iterm", &(di->sysfs_data.iterm), DEFAULT_ITERM);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"huawei,fcp-test-delay", &(di->fcp_test_delay), DEFAULT_FCP_TEST_DELAY);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"startup_iin_limit", &(di->startup_iin_limit), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"hota_iin_limit", &(di->hota_iin_limit), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"en_eoc_max_delay", &(di->en_eoc_max_delay), 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), node,
		"honor,otg-type", &(di->otg_type), 0);

	ret = of_property_read_string(node, "honor,bms-psy-name", &di->bms_psy_name);
	if (ret) {
		di->bms_psy_name = "bms";
		pr_err("default bms nase set\n");
	}

	ret = of_property_read_string(node, "honor,main-psy-name", &di->main_psy_name);
	if (ret) {
		di->main_psy_name = "main";
		pr_err("default main nase set\n");
	}

	ret = of_property_read_string(node, "honor,usb-psy-name", &di->usb_psy_name);
	if (ret) {
		di->usb_psy_name = "pc_port";
		pr_err("default usb nase set\n");
	}
}
static int charge_probe(struct platform_device *pdev)
{
	int ret;
	struct charge_device_info *di;
	int usb_pre;

	if (!pdev) {
		pr_err("%s: invalid param, fatal error\n", __func__);
		return -EINVAL;
	}

	di = charge_device_info_alloc();
	if (!di) {
		pr_err("memory allocation failed.\n");
		return -ENOMEM;
	}

	INIT_DELAYED_WORK(&di->smb_charger_work, smb_charger_work);
	INIT_DELAYED_WORK(&di->otg_type_work, otg_type_work);
	di->dev = &(pdev->dev);
	dev_set_drvdata(&(pdev->dev), di);
	parse_charge_dts(di);

	schedule_delayed_work(&di->otg_type_work, 0);

	di->nb.notifier_call = charger_plug_notifier_call;
	ret = usb_charger_register_notifier(&di->nb);
	if(ret < 0) {
		pr_err("usb charger register notify failed\n");
	}
	mutex_init(&di->sysfs_data.dump_reg_lock);
	mutex_init(&di->sysfs_data.dump_reg_head_lock);
	ChargeSysfsCreateGroup(di->dev);
	charge_event_poll_register(charge_dev);
#ifdef CONFIG_HUAWEI_DSM
	dsm_register_client(&dsm_charge_monitor);
#endif
	g_charger_device_para = di;

	if (is_usb_psy_available(di)) {
		usb_pre = get_property_from_psy(di->usb_psy, POWER_SUPPLY_PROP_ONLINE);
		if (usb_pre) {
			ret = charger_plug_notifier_call(&di->nb, 0, NULL);
			if (ret != NOTIFY_OK)
				pr_err("charger plug notifier call failed\n");
		}
	}

	power_if_ops_register(&dcp_if_ops);
	power_if_ops_register(&sdp_if_ops);
	power_if_ops_register(&fcp_if_ops);
	honor_charger_log_ops.dev_data = g_charger_device_para;
	power_log_ops_register(&honor_charger_log_ops);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
	charge_glink_ops.dev_data = di;
	hihonor_oem_glink_ops_register(&charge_glink_ops);
#endif

	pr_info("honor charger probe ok!\n");
	return 0;
}

static void charge_event_poll_unregister(struct device *dev)
{
    if (!dev) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return;
    }

	sysfs_remove_file(&dev->kobj, &dev_attr_poll_charge_start_event.attr);
	g_sysfs_poll = NULL;
}

static int charge_remove(struct platform_device *pdev)
{
	struct charge_device_info *di = NULL;

    if (!pdev) {
        pr_err("%s: invalid param, fatal error\n", __func__);
        return -EINVAL;
    }

    di = dev_get_drvdata(&pdev->dev);
    if (!di) {
        pr_err("%s: Cannot get charge device info, fatal error\n", __func__);
        return -EINVAL;
    }
	usb_charger_notifier_unregister(&di->nb);
	cancel_delayed_work_sync(&di->smb_charger_work);
	charge_event_poll_unregister(charge_dev);
#ifdef CONFIG_HUAWEI_DSM
	dsm_unregister_client(dsm_chargemonitor_dclient, &dsm_charge_monitor);
#endif
	ChargeSysfsRemoveGroup(di->dev);
	charge_device_info_free(di);
	g_charger_device_para = NULL;
	di = NULL;
	return 0;
}

static void charge_shutdown(struct platform_device  *pdev)
{
	if (!pdev)
		pr_err("%s: invalid param\n", __func__);
}

#ifdef CONFIG_PM

static int charge_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int charge_resume(struct platform_device *pdev)
{
	return 0;
}
#endif

static const struct of_device_id charge_match_table[] = {
	{
		.compatible = "huawei,charger",
		.data = NULL,
	},
	{
	},
};

static struct platform_driver charge_driver =
{
	.probe = charge_probe,
	.remove = charge_remove,
#ifdef CONFIG_PM
	.suspend = charge_suspend,
	.resume = charge_resume,
#endif
	.shutdown = charge_shutdown,
	.driver =
	{
		.name = "huawei,charger",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(charge_match_table),
	},
};

static int __init charge_init(void)
{
	int ret;

	ret = platform_driver_register(&charge_driver);
	if (ret) {
		pr_info("charge_init register platform_driver_register failed!\n");
		return ret;
	}

	return 0;
}

static void __exit charge_exit(void)
{
	platform_driver_unregister(&charge_driver);
}

late_initcall(charge_init);
module_exit(charge_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei charger module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
