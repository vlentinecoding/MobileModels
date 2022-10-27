/*
 * adsp_battery.c
 *
 * adsp battery driver
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
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <log/hw_log.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/power_supply_interface.h>
#include <chipset_common/hwpower/coul_interface.h>
#include <chipset_common/hwpower/coul_calibration.h>
#include <huawei_platform/power/adsp/adsp_battery.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/battery_model_public.h>
#include <huawei_platform/power/adsp/fuelgauge_ic/rt9426a.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_event_ne.h>

#define HWLOG_TAG adsp_battery
HWLOG_REGIST();

#define ADSP_BATTERY_CAPACITY_FULL  10000
#define ADSP_BATTERY_NOT_SUSPEND_STATE  0
#define ADSP_BATTERY_SUSPEND_STATE  1
#define ADSP_BATTERY_CAPACITY_INVALID (-1)
#define ADSP_BATTERY_CAPACITY_JUMP_THRESHOLD 200
#define ADSP_BATTERY_CAPACITY_PERCENT 100
#define ADSP_BATTERY_FILTER_WINDOW_LEN 10
#define ADSP_SOC_DECIMAL_BASE_100 100
#define ADSP_BATTERY_CAPACITY_STEP 100
#define ADSP_BATTERY_CAPACITY_MIN_VOLT 3450000

static struct adsp_battery_device *g_di = NULL;

static enum power_supply_property battery_props[] = {
	POWER_SUPPLY_PROP_STATUS,
	POWER_SUPPLY_PROP_HEALTH,
	POWER_SUPPLY_PROP_PRESENT,
	POWER_SUPPLY_PROP_ONLINE,
	POWER_SUPPLY_PROP_TECHNOLOGY,
	POWER_SUPPLY_PROP_TEMP,
	POWER_SUPPLY_PROP_CYCLE_COUNT,
	POWER_SUPPLY_PROP_LIMIT_FCC,
	POWER_SUPPLY_PROP_VOLTAGE_NOW,
	POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN,
	POWER_SUPPLY_PROP_VOLTAGE_MAX,
	POWER_SUPPLY_PROP_CURRENT_NOW,
	POWER_SUPPLY_PROP_CURRENT_AVG,
	POWER_SUPPLY_PROP_CAPACITY,
	POWER_SUPPLY_PROP_CAPACITY_LEVEL,
	POWER_SUPPLY_PROP_CAPACITY_RM,
	POWER_SUPPLY_PROP_CAPACITY_FCC,
	POWER_SUPPLY_PROP_CHARGE_FULL,
	POWER_SUPPLY_PROP_CHARGE_COUNTER,
	POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN,
	POWER_SUPPLY_PROP_CHARGE_NOW,
	POWER_SUPPLY_PROP_ID_VOLTAGE,
	POWER_SUPPLY_PROP_BRAND,
};

static int battery_psy_get_prop(struct power_supply *psy,
		enum power_supply_property prop,
		union power_supply_propval *pval)
{
	struct adsp_battery_device *di = power_supply_get_drvdata(psy);
	int rc = 0;

	if (!di)
		return -EINVAL;

	pval->intval = -ENODATA;

	switch (prop) {
	case POWER_SUPPLY_PROP_STATUS:
		pval->intval = di->charge_status;
		break;
	case POWER_SUPPLY_PROP_PRESENT:
		pval->intval = di->psy_info.exist;
		break;
	case POWER_SUPPLY_PROP_TEMP:
		pval->intval = di->psy_info.temp_now;
		break;
	case POWER_SUPPLY_PROP_ONLINE:
		pval->intval = di->psy_info.exist;
		break;
	case POWER_SUPPLY_PROP_CYCLE_COUNT:
		pval->intval = di->psy_info.cycle_count;
		break;
	case POWER_SUPPLY_PROP_CAPACITY:
		pval->intval = (di->psy_info.ui_capacity + 50) / ADSP_BATTERY_CAPACITY_PERCENT; // 50: round base 10
		break;
	case POWER_SUPPLY_PROP_HEALTH:
		pval->intval = di->psy_info.health;
		break;
	case POWER_SUPPLY_PROP_TECHNOLOGY:
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
	case POWER_SUPPLY_PROP_CHARGE_COUNTER:
	case POWER_SUPPLY_PROP_CURRENT_NOW:
	case POWER_SUPPLY_PROP_VOLTAGE_NOW:
		rc = power_supply_get_property_value("bk_battery", prop, pval);
		break;
	case POWER_SUPPLY_PROP_LIMIT_FCC:
		pval->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CURRENT_AVG:
		pval->intval = 0;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_LEVEL:
		pval->intval = di->psy_info.capacity_level;
		break;
	case POWER_SUPPLY_PROP_CAPACITY_RM:
		pval->intval = di->psy_info.capacity_rm * RATIO_1K;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL:
	case POWER_SUPPLY_PROP_CAPACITY_FCC:
		pval->intval = di->psy_info.fcc * RATIO_1K;
		break;
	case POWER_SUPPLY_PROP_CHARGE_NOW:
		pval->intval = 0;
		break;
	case POWER_SUPPLY_PROP_ID_VOLTAGE:
		pval->intval = 0;
		break;
	case POWER_SUPPLY_PROP_VOLTAGE_MAX_DESIGN:
		pval->intval = di->batt_model_info.voltage_max_design * RATIO_1K;
		break;
	case POWER_SUPPLY_PROP_CHARGE_FULL_DESIGN:
		pval->intval = di->batt_model_info.charge_full_design * RATIO_1K;
		break;
	case POWER_SUPPLY_PROP_BRAND:
		pval->strval = di->batt_model_info.brand;
		break;
	default:
		rc = -EINVAL;
		break;
	}

	return rc;
}

static int battery_psy_set_prop(struct power_supply *psy,
		enum power_supply_property prop,
		const union power_supply_propval *pval)
{
	struct adsp_battery_device *di = power_supply_get_drvdata(psy);

	if (!di)
		return -EINVAL;

	switch (prop) {
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		//di->batt_info.voltage_max = pval->intval;
		// TODO: glink set voltage_max
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

static int battery_psy_prop_is_writeable(struct power_supply *psy,
		enum power_supply_property prop)
{
	switch (prop) {
	case POWER_SUPPLY_PROP_VOLTAGE_MAX:
		return 1;
	default:
		break;
	}

	return 0;
}

static int capital_battery_psy_get_prop(struct power_supply *psy,
		enum power_supply_property prop,
		union power_supply_propval *pval)
{
	return power_supply_get_property_value("battery", prop, pval);
}

static const struct power_supply_desc batt_psy_desc = {
	.name					= "battery",
	.type					= POWER_SUPPLY_TYPE_BATTERY,
	.properties				= battery_props,
	.num_properties			= ARRAY_SIZE(battery_props),
	.get_property			= battery_psy_get_prop,
	.set_property			= battery_psy_set_prop,
	.property_is_writeable	= battery_psy_prop_is_writeable,
};

static const struct power_supply_desc capital_batt_psy_desc = {
	.name					= "Battery",
	.type					= POWER_SUPPLY_TYPE_BATTERY,
	.properties				= battery_props,
	.num_properties			= ARRAY_SIZE(battery_props),
	.get_property			= capital_battery_psy_get_prop,
};

static void adsp_battery_parse_batt_model(struct adsp_battery_device *di)
{
	char *token = NULL;
	const char *batt_model_name = NULL;
	char model_name[MAX_BATT_NAME] = { 0 };
	char *name_ptr = model_name;

	if (!di)
		return;

	batt_model_name = bat_model_name();
	strlcpy(model_name, batt_model_name, sizeof(model_name));
	token = strsep(&name_ptr, "_");
	if (!token)
		return;

	strlcpy(di->batt_model_info.brand, token, sizeof(di->batt_model_info.brand));

	if (!name_ptr)
		goto ret;

	token = strsep(&name_ptr, "_");
	if (!token || kstrtoint(token, 0, &di->batt_model_info.charge_full_design))
		goto ret;

	token = strsep(&name_ptr, "_");
	if (token)
		kstrtoint(token, 0, &di->batt_model_info.voltage_max_design);

ret:
	hwlog_info("charge_full_design = %d, voltage_max_design = %d, brand = %s\n",
		di->batt_model_info.charge_full_design,
		di->batt_model_info.voltage_max_design,
		di->batt_model_info.brand);
}

static void bat_ui_vth_correct_dts(struct device_node *np,
	struct battery_ui_capacity_info *info)
{
	int data_len = sizeof(info->vth_soc_calibration_data);
	int nums_size = sizeof(int);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vth_correct_en", &info->vth_correct_en, 0);

	if (!info->vth_correct_en)
		return;
	if (power_dts_read_u32_array(power_dts_tag(HWLOG_TAG), np,
		"vth_correct_para",
		(u32 *)&info->vth_soc_calibration_data[0],
		data_len / nums_size))
		info->vth_correct_en = 0;
}

static void bat_ui_parse_work_interval(struct device_node *np, const char *tag,
	struct bat_ui_cap_interval_para *para)
{
	int row, col, len;
	int idata[BUC_WORK_INTERVAL_LEVEL * BUC_WORK_INTERVAL_TOTAL] = { 0 };

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		tag, idata, BUC_WORK_INTERVAL_LEVEL, BUC_WORK_INTERVAL_TOTAL);
	if (len < 0)
		return;

	for (row = 0; row < len / BUC_WORK_INTERVAL_TOTAL; row++) {
		col = row * BUC_WORK_INTERVAL_TOTAL + BUC_WORK_INTERVAL_MIN_SOC;
		para[row].min_soc = idata[col];
		col = row * BUC_WORK_INTERVAL_TOTAL + BUC_WORK_INTERVAL_MAX_SOC;
		para[row].max_soc = idata[col];
		col = row * BUC_WORK_INTERVAL_TOTAL + BUC_WORK_INTERVAL_VALUE;
		para[row].interval = idata[col];
	}
}

static void bat_ui_capacity_parse_dts(struct device_node *np,
	struct battery_ui_capacity_info *info)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"soc_at_term", &info->soc_at_term, BUC_CAPACITY_DIVISOR);
	bat_ui_vth_correct_dts(np, info);
	bat_ui_parse_work_interval(np, "charging_interval_para",
		info->charging_interval_para);
	bat_ui_parse_work_interval(np, "discharging_interval_para",
		info->discharging_interval_para);
}

static void bat_fault_parse_dts(struct device_node *np, struct battery_fault_info *info)
{
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_normal", &info->vol_cutoff_normal,
		BAT_FAULT_NORMAL_CUTOFF_VOL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_sleep", &info->vol_cutoff_sleep,
		BAT_FAULT_SLEEP_CUTOFF_VOL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_low_temp", &info->vol_cutoff_low_temp,
		BAT_FAULT_NORMAL_CUTOFF_VOL);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"vol_cutoff_filter_cnt", &info->vol_cutoff_filter_cnt,
		BAT_FAULT_CUTOFF_VOL_FILTERS);
}

static char *battery_dts_type_map[ADSP_BATTERY_DTS_END] = {
	[ADSP_BATTERY_DTS_FAULT] = "battery_fault",
	[ADSP_BATTERY_DTS_UI_CAPACITY] = "battery_ui_capacity",
};

static int adsp_battery_get_dts_type(const char *dts_name)
{
	int i;

	if (!dts_name)
		return -1;

	for (i = ADSP_BATTERY_DTS_BEGIN; i < ADSP_BATTERY_DTS_END; i++) {
		if (!strcmp(dts_name, battery_dts_type_map[i]))
			return i;
	}
	return -1;
}

static int adsp_battery_parse_dts(struct device_node *np,
	struct adsp_battery_device *di)
{
	struct device_node *child_node = NULL;
	int type;

	if (!di)
		return -1;

	hwlog_info("%s\n", __func__);
	for_each_child_of_node(np, child_node) {
		hwlog_info("find dts name: %s\n", child_node->name);
		type = adsp_battery_get_dts_type(child_node->name);
		switch (type) {
		case ADSP_BATTERY_DTS_FAULT:
			bat_fault_parse_dts(child_node, &di->batt_fault_info);
			break;
		case ADSP_BATTERY_DTS_UI_CAPACITY:
			bat_ui_capacity_parse_dts(child_node, &di->batt_ui_cap_info);
			break;
		default:
			break;
		}
	}
	return 0;
}

static void adsp_battery_init_psy(struct adsp_battery_device *di)
{
	struct power_supply_config psy_cfg = {};

	psy_cfg.drv_data = di;
	psy_cfg.of_node = di->dev->of_node;
	devm_power_supply_register(di->dev, &batt_psy_desc, &psy_cfg);
}

static void adsp_battery_dts_sync_callback(void *dev_data)
{
	struct adsp_battery_device *di = (struct adsp_battery_device *)dev_data;

	if (!di)
		return;

	schedule_delayed_work(&di->sync_work, 0);
}

static void adsp_battery_dts_glink_sync_work(struct work_struct *work)
{
	int ret;
	struct adsp_battery_device *di = container_of(work, struct adsp_battery_device, sync_work.work);

	if (!di)
		return;

	hwlog_info("sync battery_model dts to adsp\n");
	ret = hihonor_oem_glink_oem_set_prop(BATTERY_OEM_BATT_MODEL_CONF,
		&di->batt_model_info, sizeof(di->batt_model_info));
	if (ret) {
		hwlog_err("sync battery_model dts to adsp fail\n");
		goto next;
	}

	hwlog_info("sync battery_fault dts to adsp\n");
	ret = hihonor_oem_glink_oem_set_prop(BATTERY_OEM_BATT_FAULT_CONF,
		&di->batt_fault_info, sizeof(di->batt_fault_info));
	if (ret) {
		hwlog_err("sync battery_fault dts to adsp fail\n");
		goto next;
	}

	hwlog_info("sync battery_ui_capacity dts to adsp\n");
	ret = hihonor_oem_glink_oem_update_config(BATT_UI_CAP_CONFIG_ID,
		&di->batt_ui_cap_info, sizeof(di->batt_ui_cap_info));
	if (ret) {
		hwlog_err("sync battery_ui_capacity dts to adsp fail\n");
		goto next;
	}

	return;

next:
	schedule_delayed_work(&di->sync_work, msecs_to_jiffies(ADSP_BATTERY_SYNC_WORK_INTERVAL));
}

static unsigned int adsp_battery_get_update_work_interval(
    const struct adsp_battery_device *di)
{
	if (!di)
		return ADSP_BATTERY_CHARGE_UPDATE_INTERVAL;

	if (di->psy_info.voltage_now < ADSP_BATTERY_CAPACITY_MIN_VOLT)
		return ADSP_BATTERY_LOW_VOLT_UPDATE_INTERVAL;

	if (di->psy_info.temp_now <= ADSP_BATTERY_UPDATE_WORK_LOW_TEMP)
		return ADSP_BATTERY_LOW_TEMP_UPDATE_INTERVAL;

	if (di->charge_status == POWER_SUPPLY_STATUS_CHARGING)
		return ADSP_BATTERY_CHARGE_UPDATE_INTERVAL;
	else
		return ADSP_BATTERY_DISCHARGE_UPDATE_INTERVAL;
}

static void adsp_battery_capacity_check(struct adsp_battery_device *di)
{
	int capacity_diff;

	if (di->psy_info.ui_capacity > ADSP_BATTERY_CAPACITY_FULL) {
		di->psy_info.ui_capacity = ADSP_BATTERY_CAPACITY_FULL;
		return;
	}

	if (di->psy_info.ui_capacity < 0) {
		di->psy_info.ui_capacity = 0;
		return;
	}

	if (di->last_capacity == ADSP_BATTERY_CAPACITY_INVALID) {
		di->last_capacity = di->psy_info.ui_capacity;
		return;
	}

	capacity_diff = di->psy_info.ui_capacity - di->last_capacity;
	if (abs(capacity_diff) < ADSP_BATTERY_CAPACITY_JUMP_THRESHOLD) {
		di->last_capacity = di->psy_info.ui_capacity;
		return;
	}

	hwlog_info("bat_cap jump,e=%d cap=%d t=%d fc=%d v=%d c=%d rm=%d h=%d l=%d\n",
		di->psy_info.exist, di->psy_info.ui_capacity, di->psy_info.temp_now,
		di->psy_info.fcc, di->psy_info.voltage_now, di->psy_info.current_now,
		di->psy_info.capacity_rm, di->psy_info.health, di->last_capacity);

	if (capacity_diff >= ADSP_BATTERY_CAPACITY_JUMP_THRESHOLD)
		di->psy_info.ui_capacity = di->last_capacity + ADSP_BATTERY_CAPACITY_STEP;
	else if (capacity_diff <= -ADSP_BATTERY_CAPACITY_JUMP_THRESHOLD)
		di->psy_info.ui_capacity = di->last_capacity - ADSP_BATTERY_CAPACITY_STEP;

	di->last_capacity = di->psy_info.ui_capacity;
}

static void adsp_battery_update_work(struct work_struct *work)
{
	int ret;
	struct adsp_battery_device *di = container_of(work, struct adsp_battery_device, update_work.work);
	struct adsp_battery_psy_info psy_info = { 0 };
	unsigned int interval;

	if (di->soc_decimal_update) {
		hwlog_info("%s cap=%d rep_soc=%d round_soc=%d base=%d\n", __func__, di->psy_info.ui_capacity,
			di->soc_decimal.rep_soc, di->soc_decimal.round_soc, di->soc_decimal.base);
		ret = hihonor_oem_glink_oem_set_prop(BATTERY_OEM_SOC_DECIMAL,
			&di->soc_decimal, sizeof(di->soc_decimal));
		if (ret) {
			hwlog_err("sync soc_decimal to adsp fail\n");
		}
		di->soc_decimal_update = 0;
		goto restart_work;
	}

	if (hihonor_oem_glink_oem_get_prop(BATTERY_OEM_PSY_INFO, &psy_info, sizeof(psy_info)))
		goto restart_work;

	memcpy(&di->psy_info, &psy_info, sizeof(psy_info));
	adsp_battery_capacity_check(di);

	hwlog_info("bat_update,e=%d cap=%d t=%d fc=%d v=%d c=%d rm=%d h=%d l=%d\n",
		di->psy_info.exist, di->psy_info.ui_capacity, di->psy_info.temp_now,
		di->psy_info.fcc, di->psy_info.voltage_now, di->psy_info.current_now,
		di->psy_info.capacity_rm, di->psy_info.health, di->last_capacity);

	power_supply_sync_changed("battery");
	power_supply_sync_changed("Battery");

restart_work:
	interval = adsp_battery_get_update_work_interval(di);
	schedule_delayed_work(&di->update_work, msecs_to_jiffies(interval));
}

void adsp_battery_cancle_update_work(void)
{
	struct adsp_battery_device *di = g_di;

	if (di)
		cancel_delayed_work_sync(&di->update_work);
}

void adsp_battery_restart_update_work(void)
{
	struct adsp_battery_device *di = g_di;

	if (!di)
		return;
	// first: sync to adsp, update work send first
	hwlog_info("%s cap=%d rep_soc=%d\n", __func__, di->psy_info.ui_capacity,
		di->soc_decimal.rep_soc);
	di->soc_decimal_update = 1;

	// second: restart update work
	schedule_delayed_work(&di->update_work, msecs_to_jiffies(0));
}

/*
 * sync filter info
 *
 * rep_soc: 0.01%
 * round_soc: the soc without the base
 * base: 100=>0.01% 1=>1%
 */
void adsp_battery_ui_soc_sync_filter(int rep_soc, int round_soc, int base)
{
	int prev_soc;
	struct adsp_battery_device *di = g_di;

	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return;
	}

	/* step1: save soc info */
	di->soc_decimal.base = base;
	di->soc_decimal.rep_soc = rep_soc;
	di->soc_decimal.round_soc = round_soc;

	prev_soc = di->psy_info.ui_capacity / ADSP_BATTERY_CAPACITY_PERCENT;
	di->psy_info.ui_capacity = rep_soc;
	di->last_capacity = rep_soc;

	/* step2: capacity changed (example: 86% to 87%) */
	if (prev_soc != round_soc) {
		hwlog_info("sync filter ui_capacity=%d prev_soc=%d,rep_soc=%d,round_soc=%d\n",
			di->psy_info.ui_capacity, prev_soc, rep_soc, round_soc);
		power_supply_sync_changed("Battery");
		power_supply_sync_changed("battery");
	}
}

int adsp_battery_ui_soc_get_filter_sum(int base)
{
	struct adsp_battery_device *di = g_di;

	if (!di) {
		hwlog_err("%s di is null\n", __func__);
		return -1;
	}

	hwlog_info("%s get filter sum=%d\n", __func__, di->psy_info.ui_capacity);
	di->soc_decimal.rep_soc = di->psy_info.ui_capacity;
	di->soc_decimal.round_soc = di->psy_info.ui_capacity;
	di->soc_decimal.base = base;

	if (base == ADSP_SOC_DECIMAL_BASE_100)
		return di->psy_info.ui_capacity;
	return di->psy_info.ui_capacity / ADSP_BATTERY_CAPACITY_PERCENT;
}

static int adsp_battery_event_notifier_call(struct notifier_block *nb,
	unsigned long event, void *data)
{
	struct adsp_battery_device *di = g_di;

	if (!di)
		return NOTIFY_OK;

	switch (event) {
	case POWER_NE_START_CHARGING:
		di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
		break;
	case POWER_NE_STOP_CHARGING:
		di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;
		break;
	case POWER_NE_SUSPEND_CHARGING:
		di->charge_status = POWER_SUPPLY_STATUS_NOT_CHARGING;
		break;
	default:
		return NOTIFY_OK;
	}

	if (di->psy_info.ui_capacity == ADSP_BATTERY_CAPACITY_FULL &&
		di->charge_status == POWER_SUPPLY_STATUS_CHARGING)
		di->charge_status = POWER_SUPPLY_STATUS_FULL;

	hwlog_info("receive event=%lu,status=%d\n", event, di->charge_status);
	return NOTIFY_OK;
}

static void adsp_battery_handle_notify_event(void *dev_data, u32 notification, void *data)
{
	struct adsp_battery_device *di = (struct adsp_battery_device *)dev_data;

	if (!di)
		return;

	if (notification == OEM_NOTIFY_FG_READY && data)
	{
		memcpy(&di->psy_info, data, sizeof(di->psy_info));
		adsp_battery_capacity_check(di);
		schedule_delayed_work(&di->update_work, msecs_to_jiffies(ADSP_BATTERY_SYNC_WORK_INTERVAL));
	}
	else if (notification == OEM_NOTIFY_BATT_LOW_VOLT) {
		hwlog_info("receive low volt event, update data\n");
		schedule_delayed_work(&di->update_work, msecs_to_jiffies(ADSP_BATTERY_SYNC_WORK_INTERVAL));
	}
	else if (notification == OEM_NOTIFY_USB_ONLINE && data) {
		if (*(int *)data)
			di->charge_status = POWER_SUPPLY_STATUS_CHARGING;
	}
	else if (notification == OEM_NOTIFY_BAT_CAPACITY_CHANGED) {
		schedule_delayed_work(&di->update_work, 0);
	}
}

static struct hihonor_glink_ops adsp_battery_dts_glink_ops = {
	.sync_data = adsp_battery_dts_sync_callback,
	.notify_event =adsp_battery_handle_notify_event,
};

static int adsp_battery_probe(struct platform_device *pdev)
{
	int ret;
	struct adsp_battery_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	hwlog_info("%s %d\n", __func__, __LINE__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	g_di = di;

	ret = adsp_battery_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	di->last_capacity = ADSP_BATTERY_CAPACITY_INVALID;
	di->charge_status = POWER_SUPPLY_STATUS_DISCHARGING;

	adsp_battery_parse_batt_model(di);
	adsp_battery_init_psy(di);
	di->event_nb.notifier_call = adsp_battery_event_notifier_call;
	ret = power_event_nc_register(POWER_NT_CHARGING, &di->event_nb);
	if (ret)
		goto fail_free_mem;

	adsp_battery_dts_glink_ops.dev_data = di;
	ret = hihonor_oem_glink_ops_register(&adsp_battery_dts_glink_ops);
	if (ret) {
		hwlog_err("%s fail to register glink ops\n", __func__);
		goto fail_free_mem;
	}

	platform_set_drvdata(pdev, di);
	INIT_DELAYED_WORK(&di->sync_work, adsp_battery_dts_glink_sync_work);
	INIT_DELAYED_WORK(&di->update_work, adsp_battery_update_work);
	di->soc_decimal_update = 0;
	return 0;

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_di = NULL;
	return ret;
}

static int adsp_battery_remove(struct platform_device *pdev)
{
	struct adsp_battery_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	devm_kfree(&pdev->dev, di);
	return 0;
}

#ifdef CONFIG_PM
static int adsp_battery_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct adsp_battery_device *di = platform_get_drvdata(pdev);
	int suspend = ADSP_BATTERY_SUSPEND_STATE;

	hihonor_oem_glink_notify_state(OEM_NOTIFY_AP_SUSPEND_STATE, &suspend, sizeof(suspend));
	cancel_delayed_work_sync(&di->update_work);
	return 0;
}

static int adsp_battery_resume(struct platform_device *pdev)
{
	struct adsp_battery_device *di = platform_get_drvdata(pdev);
	int suspend = ADSP_BATTERY_NOT_SUSPEND_STATE;
	struct adsp_battery_psy_info psy_info = { 0 };
	int ret;

	hihonor_oem_glink_notify_state(OEM_NOTIFY_AP_SUSPEND_STATE, &suspend, sizeof(int));
	ret = hihonor_oem_glink_oem_get_prop(BATTERY_OEM_PSY_INFO, &psy_info, sizeof(psy_info));
	if (!ret) {
		memcpy(&di->psy_info, &psy_info, sizeof(psy_info));
		di->last_capacity = di->psy_info.ui_capacity;
	}
	schedule_delayed_work(&di->update_work, 0);
	return 0;
}
#endif

static const struct of_device_id adsp_battery_match_table[] = {
	{
		.compatible = "honor,adsp_battery",
		.data = NULL,
	},
	{},
};

static struct platform_driver adsp_battery_driver = {
	.probe = adsp_battery_probe,
	.remove = adsp_battery_remove,
#ifdef CONFIG_PM
	.suspend = adsp_battery_suspend,
	.resume = adsp_battery_resume,
#endif
	.driver = {
		.name = "honor,adsp_battery",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_battery_match_table),
	},
};

static int __init adsp_battery_init(void)
{
	return platform_driver_register(&adsp_battery_driver);
}

static void __exit adsp_battery_exit(void)
{
	platform_driver_unregister(&adsp_battery_driver);
}

device_initcall_sync(adsp_battery_init);
module_exit(adsp_battery_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adsp battery module driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
