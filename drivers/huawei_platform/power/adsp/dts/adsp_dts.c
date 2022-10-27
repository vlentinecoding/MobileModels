/*
 * adsp_dts.c
 *
 * adsp dts driver
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
#include <chipset_common/hwpower/power_debug.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <huawei_platform/power/adsp/adsp_dts_interface.h>
#include <huawei_platform/power/adsp/adsp_thermal.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/power_dts.h>

#define HWLOG_TAG adsp_dts
HWLOG_REGIST();

#define ADSP_DTS_SYNC_INTERVAL     1000
#define ADSP_DTS_CHECK_WORK_INTERVAL     10000
#define DEFAULT_TUSB_THRESHOLD     40
#define FG_JEITA_PARA_MAX_NUM      8
#define FG_JEITA_SC_VTERM_COMP_PARA_MAX_NUM 20
#define ADSP_DTS_MAX_RETRY_TIME    50

enum adsp_dts_sysfs_type {
	ADSP_DTS_SYSFS_BEGIN = 0,
	ADSP_DTS_SYSFS_THERMAL_CONFIG_DATA = ADSP_DTS_SYSFS_BEGIN,
	ADSP_DTS_SYSFS_THERMAL_BASIC_CONFIG,
	ADSP_DTS_SYSFS_END,
};

enum adsp_dts_type {
	ADSP_DTS_TYPE_BEGIN = 0,
	ADSP_DTS_TYPE_FSA9685 = ADSP_DTS_TYPE_BEGIN,
	ADSP_DTS_TYPE_BQ25970_MAIN,
	ADSP_DTS_TYPE_BQ25970_AUX,
	ADSP_DTS_TYPE_USCP,
	ADSP_DTS_TYPE_BTB_CHECK,
	ADSP_DTS_TYPE_FG_JEITA,
	ADSP_DTS_TYPE_END,
};

struct adsp_dts_info {
	char *name;
	u32 exist;
	u32 config_id;
	void *para;
	int size;
	int (*dts_parse)(struct device_node *np, void **para, int *size);
	int send_ok;
	int need_feedback;
	int probe_ok;
};

struct adsp_dts_device {
	struct device *dev;
	struct delayed_work sync_work;
	struct delayed_work check_work;
	struct thermal_config_data config_data[MAX_TEMP_NODE_COUNT];
	struct thermal_basic_config basic_config;
	struct delayed_work thermal_config_work;
	int retry_time;
};

static struct adsp_dts_device *g_adsp_dts_di = NULL;

struct bq25970_cfg_para  {
	unsigned int i2c_inst;
	int switching_frequency;
	unsigned int ic_role;
	int sense_r_actual;
	int sense_r_config;
};

struct uscp_cfg_para  {
	int gpio_pmic_chip;
	int gpio_uscp;
	int uscp_threshold_tusb;
	int open_mosfet_temp;
	int open_hiz_temp;
	int close_mosfet_temp;
	int interval_switch_temp;
	int dmd_hiz_enable;
};

struct btb_ck_para {
	int enable;
	int is_multi_btb;
	int min_th;
	int max_th;
	int diff_th;
	int times;
	int dmd_no;
};

struct btb_ck_config {
	struct btb_ck_para volt_ck_para;
	struct btb_ck_para temp_ck_para;
};

struct fsa9685_para {
	unsigned int i2c_instance;
	unsigned int i2c_slave_addr;
	unsigned int i2c_frequency;
};

enum fg_jeita_info {
	FG_JEITA_INFO_TEMP_LOW = 0,
	FG_JEITA_INFO_TEMP_HIGH,
	FG_JEITA_INFO_IBAT_MAX,
	FG_JEITA_INFO_VBAT_MAX,
	FG_JEITA_INFO_ITERM,
	FG_JEITA_INFO_MAX,
};

enum fg_jeita_sc_vterm_comp_info {
	FG_JEITA_SC_ID,
	FG_JEITA_SC_VTERM_COMP_UV,
	FG_JEITA_SC_VTERM_COMP_INFO_MAX,
};

struct fg_jeita_para
{
	int temp_low;
	int temp_high;
	unsigned int ibat_max;
	unsigned int vbat_max;
	unsigned int iterm;
};

struct fg_jeita_sc_vterm_comp_para {
	int sc_id;
	int vterm_comp;
};

struct fg_jeita_config
{
	int config_level;
	struct fg_jeita_para jeita_config[FG_JEITA_PARA_MAX_NUM];
	unsigned int support_multi_ic;
	int sc_vterm_comp_len;
	struct fg_jeita_sc_vterm_comp_para vterm_para[FG_JEITA_SC_VTERM_COMP_PARA_MAX_NUM];
};

static int bq25970_dts_parse(struct device_node *np, void **para, int *size)
{
	struct bq25970_cfg_para *cfg = NULL;

	if (!np || !para) {
		hwlog_err("np or para is null\n");
		return -EINVAL;
	}

	cfg = (struct bq25970_cfg_para *)kzalloc(sizeof(struct bq25970_cfg_para), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"i2c_inst", &cfg->i2c_inst, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"ic_role", &cfg->ic_role, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"switching_frequency", (int *)&cfg->switching_frequency, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_actual", (int *)&cfg->sense_r_actual, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"sense_r_config", (int *)&cfg->sense_r_config, 0);

	*para = cfg;
	*size = sizeof(*cfg);
	return 0;
}

static int fsa9685_dts_parse(struct device_node *np, void **para, int *size)
{
	struct fsa9685_para *dts = NULL;

	if (!np || !para) {
		hwlog_err("np or para is null\n");
		return -EINVAL;
	}

	dts = kzalloc(sizeof(struct fsa9685_para), GFP_KERNEL);
	if (!dts)
		return -ENOMEM;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"i2c_instance", &dts->i2c_instance, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"i2c_slave_addr", &dts->i2c_slave_addr, 0);
	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"i2c_frequency", &dts->i2c_frequency, 0);

	*para = dts;
	*size = sizeof(*dts);
	return 0;
}

static int uscp_dts_parse(struct device_node *np, void **para, int *size)
{
	struct uscp_cfg_para *cfg = NULL;

	cfg = (struct uscp_cfg_para *)kzalloc(sizeof(struct uscp_cfg_para), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"open_mosfet_temp", (u32 *)&cfg->open_mosfet_temp, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"close_mosfet_temp", (u32 *)&cfg->close_mosfet_temp, 0))
		return -EINVAL;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"interval_switch_temp", (u32 *)&cfg->interval_switch_temp, 0))
		return -EINVAL;

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"uscp_threshold_tusb", (u32 *)&cfg->uscp_threshold_tusb,
		DEFAULT_TUSB_THRESHOLD);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"open_hiz_temp", (u32 *)&cfg->open_hiz_temp,
		cfg->open_mosfet_temp);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"dmd_hiz_enable", (u32 *)&cfg->dmd_hiz_enable, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_pmic_chip", (u32 *)&cfg->gpio_pmic_chip, 0);

	(void)power_dts_read_u32(power_dts_tag(HWLOG_TAG), np,
		"gpio_uscp", (u32 *)&cfg->gpio_uscp, 0);

	*para = cfg;
	*size = sizeof(*cfg);

	return 0;
}

enum btb_ck_para_type {
	BTB_CK_ENABLE = 0,
	BTB_CK_IS_MULTI_BTB,
	BTB_CK_MIN_TH,
	BTB_CK_MAX_TH,
	BTB_CK_DIFF_TH,
	BTB_CK_TIMES,
	BTB_CK_DMD_NO,
	BTB_CK_PARA_TOTAL,
};

static void btb_ck_parse_check_para(struct device_node *np,
	struct btb_ck_para *data, const char *name)
{
	int len;
	int idata[BTB_CK_PARA_TOTAL] = { 0 };

	/* 1:only one line parameters */
	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		name, idata, 1, BTB_CK_PARA_TOTAL);
	if (len < 0)
		return;

	data->enable = idata[BTB_CK_ENABLE];
	data->is_multi_btb = idata[BTB_CK_IS_MULTI_BTB];
	data->min_th = idata[BTB_CK_MIN_TH];
	data->max_th = idata[BTB_CK_MAX_TH];
	data->diff_th = idata[BTB_CK_DIFF_TH];
	data->times = idata[BTB_CK_TIMES];
	data->dmd_no = idata[BTB_CK_DMD_NO];
	hwlog_info("[%s]enable=%d is_multi_btb=%d min_th=%d max_th=%d diff_th=%d times=%d dmd_no=%d\n",
		name, data->enable, data->is_multi_btb, data->min_th,
		data->max_th, data->diff_th, data->times, data->dmd_no);
}

static int btb_check_dts_parse(struct device_node *np, void **para, int *size)
{
	struct btb_ck_config *cfg = NULL;
	
	cfg = (struct btb_ck_config *)kzalloc(sizeof(struct btb_ck_config), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	btb_ck_parse_check_para(np, &cfg->volt_ck_para, "vol_check_para");
	btb_ck_parse_check_para(np, &cfg->temp_ck_para, "temp_check_para");

	*para = cfg;
	*size = sizeof(*cfg);

	return 0;
}

static int fg_jeita_dts_parse(struct device_node *np, void **para, int *size)
{
	struct fg_jeita_config *cfg = NULL;
	int row, col, len;
	int idata[FG_JEITA_INFO_MAX * FG_JEITA_PARA_MAX_NUM] = { 0 };

	cfg = (struct fg_jeita_config *)kzalloc(sizeof(*cfg), GFP_KERNEL);
	if (!cfg)
		return -ENOMEM;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
			"charge_para", idata, FG_JEITA_PARA_MAX_NUM, FG_JEITA_INFO_MAX);
	if (len < 0)
		return -1;

	for (row = 0; row < len / FG_JEITA_INFO_MAX; row++) {
		col = row * FG_JEITA_INFO_MAX + FG_JEITA_INFO_TEMP_LOW;
		cfg->jeita_config[row].temp_low = idata[col];
		col = row * FG_JEITA_INFO_MAX + FG_JEITA_INFO_TEMP_HIGH;
		cfg->jeita_config[row].temp_high = idata[col];
		col = row * FG_JEITA_INFO_MAX + FG_JEITA_INFO_IBAT_MAX;
		cfg->jeita_config[row].ibat_max = idata[col];
		col = row * FG_JEITA_INFO_MAX + FG_JEITA_INFO_VBAT_MAX;
		cfg->jeita_config[row].vbat_max = idata[col];
		col = row * FG_JEITA_INFO_MAX + FG_JEITA_INFO_ITERM;
		cfg->jeita_config[row].iterm = idata[col];
		cfg->config_level++;
	}

	cfg->sc_vterm_comp_len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"sc_vterm_comp", idata, FG_JEITA_SC_VTERM_COMP_PARA_MAX_NUM, FG_JEITA_SC_VTERM_COMP_INFO_MAX);
	if (cfg->sc_vterm_comp_len < 0)
		hwlog_info("get sc_vterm_comp para fail\n");

	for (row = 0; row < cfg->sc_vterm_comp_len / FG_JEITA_SC_VTERM_COMP_INFO_MAX; row++) {
		col = row * FG_JEITA_SC_VTERM_COMP_INFO_MAX + FG_JEITA_SC_ID;
		cfg->vterm_para[row].sc_id = idata[col];
		col = row *  FG_JEITA_SC_VTERM_COMP_INFO_MAX + FG_JEITA_SC_VTERM_COMP_UV;
		cfg->vterm_para[row].vterm_comp = idata[col];
		hwlog_info("sc_vterm_comp[%d]:%d %d\n", row, cfg->vterm_para[row].sc_id, cfg->vterm_para[row].vterm_comp);
	}

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "support_multi_ic", &cfg->support_multi_ic, 1))
		hwlog_info("get support_multi_ic para fail\n");

	*para = cfg;
	*size = sizeof(*cfg);

	return 0;
}

static struct adsp_dts_info adsp_dts_info_tbl[ADSP_DTS_TYPE_END] = {
	[ADSP_DTS_TYPE_FSA9685] = {
		.name = "fsa9685",
		.exist = 0,
		.config_id = SWITCH_CONFIG_ID,
		.para = NULL,
		.size = 0,
		.need_feedback = 1,
		.dts_parse = fsa9685_dts_parse,
	},
	[ADSP_DTS_TYPE_BQ25970_MAIN] = {
		.name = "bq25970_main",
		.exist = 0,
		.config_id = SC_MAIN_CONFIG_ID,
		.para = NULL,
		.size = 0,
		.need_feedback = 1,
		.dts_parse = bq25970_dts_parse,
	},
	[ADSP_DTS_TYPE_BQ25970_AUX] = {
		.name = "bq25970_aux",
		.exist = 0,
		.config_id = SC_AUX_CONFIG_ID,
		.para = NULL,
		.size = 0,
		.need_feedback = 1,
		.dts_parse = bq25970_dts_parse,
	},
	[ADSP_DTS_TYPE_USCP] = {
		.name = "usb_short_circuit_protect",
		.exist = 0,
		.config_id = USCP_CONFIG_ID,
		.para = NULL,
		.size = 0,
		.dts_parse = uscp_dts_parse,
	},
	[ADSP_DTS_TYPE_BTB_CHECK] = {
		.name = "btb_check",
		.exist = 0,
		.config_id = DC_BTB_CK_CONFIG_ID,
		.para = NULL,
		.size = 0,
		.dts_parse = btb_check_dts_parse,
	},
	[ADSP_DTS_TYPE_FG_JEITA] = {
		.name = "fg_jeita",
		.exist = 0,
		.config_id = FG_JEITA_CONFIG_ID,
		.para = NULL,
		.size = 0,
		.dts_parse = fg_jeita_dts_parse,
	},
};

static struct adsp_dts_info *adsp_dts_get_info(const char *name)
{
	unsigned int i;

	for (i = ADSP_DTS_TYPE_BEGIN; i < ADSP_DTS_TYPE_END; i++) {
		if (!strcmp(name, adsp_dts_info_tbl[i].name))
			return &adsp_dts_info_tbl[i];
	}

	hwlog_err("invalid dts name=%s\n", name);
	return NULL;
}

static int adsp_parse_dts(struct device_node *np, struct adsp_dts_device *di)
{
#ifdef CONFIG_OF
	struct device_node *child_node = NULL;
	const char *status = NULL;
	struct adsp_dts_info *info = NULL;
	int ret;

	hwlog_info("%s\n", __func__);
	for_each_child_of_node(np, child_node) {
		if (power_dts_read_string(power_dts_tag(HWLOG_TAG), 
			child_node, "status", &status)) {
			hwlog_err("childnode without status property\n");
			continue;
		}
		if (!status || strcmp(status, "ok"))
			continue;

		hwlog_info("find dts name: %s\n", child_node->name);
		info = adsp_dts_get_info(child_node->name);
		if (!info)
			continue;

		if (!info->dts_parse)
			continue;
		info->exist = 1;
		ret = info->dts_parse(child_node, &info->para, &info->size);
		if (ret) {
			hwlog_err("parse %s dts fail\n", info->name);
			return -1;
		}
	}
#endif /* CONFIG_OF */

	return 0;
}

static void adsp_dts_sync_callback(void *dev_data)
{
	struct adsp_dts_device *di = (struct adsp_dts_device *)dev_data;
	int i;
	struct adsp_dts_info *info = NULL;

	if (!di)
		return;

	for (i = ADSP_DTS_TYPE_BEGIN; i < ADSP_DTS_TYPE_END; i++) {
		info = &adsp_dts_info_tbl[i];
		info->send_ok = 0;
	}

	schedule_delayed_work(&di->sync_work, 0);
	schedule_delayed_work(&di->thermal_config_work, 0);
}

static void adsp_dts_glink_sync_work(struct work_struct *work)
{
	int ret;
	int i;
	struct adsp_dts_info *info = NULL;
	struct adsp_dts_device *di = container_of(work, struct adsp_dts_device, sync_work.work);
	int restart_flag = 0;

	if (!di)
		return;

	hwlog_info("sync dts to adsp\n");
	for (i = ADSP_DTS_TYPE_BEGIN; i < ADSP_DTS_TYPE_END; i++) {
		info = &adsp_dts_info_tbl[i];
		if (!info->exist || info->send_ok)
			continue;
		ret = hihonor_oem_glink_oem_update_config(info->config_id, info->para, info->size);
		if (ret) {
			hwlog_err("sync dts to adsp fail, name: %s\n", info->name);
			restart_flag = 1;
			continue;
		}
		info->send_ok = 1;
	}

	if (restart_flag)
		schedule_delayed_work(&di->sync_work, msecs_to_jiffies(ADSP_DTS_SYNC_INTERVAL));
	else
		schedule_delayed_work(&di->check_work, msecs_to_jiffies(ADSP_DTS_CHECK_WORK_INTERVAL));
}

static void adsp_dts_check_work(struct work_struct *work)
{
	struct adsp_dts_info *info = NULL;
	struct adsp_dts_device *di = container_of(work, struct adsp_dts_device, check_work.work);
	int ret;
	int i;
	int restart_flag = 0;
	int recheck_flag = 0;

	if (di->retry_time >= ADSP_DTS_MAX_RETRY_TIME)
		return;

	for (i = ADSP_DTS_TYPE_BEGIN; i < ADSP_DTS_TYPE_END; i++) {
		info = &adsp_dts_info_tbl[i];
		if (!info->exist || !info->need_feedback || info->probe_ok)
			continue;

		info->send_ok = 0;
		recheck_flag = 1;
		hwlog_info("%d probe fail, send again\n", i);
		ret = hihonor_oem_glink_oem_update_config(info->config_id, info->para, info->size);
		if (ret) {
			hwlog_err("sync dts to adsp fail, name: %s\n", info->name);
			restart_flag = 1;
			continue;
		}
		info->send_ok = 1;
	}

	di->retry_time++;
	if (restart_flag)
		schedule_delayed_work(&di->sync_work, msecs_to_jiffies(ADSP_DTS_SYNC_INTERVAL));
	else if (recheck_flag)
		schedule_delayed_work(&di->check_work, msecs_to_jiffies(ADSP_DTS_CHECK_WORK_INTERVAL));
}

static void adsp_dts_handle_notify(void *dev_data, u32 notification, void *data)
{
	u32 dts_type;

	if (notification != OEM_NOTIFY_IC_READY || !data)
		return;

	dts_type = *(u32 *)data;
	if (dts_type >= ADSP_DTS_TYPE_END)
		return;

	adsp_dts_info_tbl[dts_type].probe_ok = 1; /* adsp prode ok */
}

static struct hihonor_glink_ops adsp_dts_glink_ops = {
	.sync_data = adsp_dts_sync_callback,
	.notify_event = adsp_dts_handle_notify,
};

static ssize_t adsp_dts_send_conf_store(void *dev_data, const char *buf, size_t size)
{
	struct adsp_dts_device *di = (struct adsp_dts_device *)dev_data;

	if (!di)
		return -1;

	schedule_delayed_work(&di->sync_work, 0);
	return size;
}

static void adsp_dts_thermal_config_work(struct work_struct *work)
{
	int ret;
	int i;
	struct adsp_dts_device *di = container_of(work, struct adsp_dts_device, thermal_config_work.work);

	if (!di)
		return;

	hwlog_info("sync thermal basic config to adsp\n");
	ret = hihonor_oem_glink_oem_update_config(THERMAL_BASIC_CONFIG_ID,
		&di->basic_config, sizeof(di->basic_config));
	if (ret) {
		hwlog_err("sync thermal basic config to adsp fail\n");
		goto next;
	}

	hwlog_info("sync thermal config data to adsp\n");
	for (i = 0; i < di->basic_config.node_num; i++) {
		ret = hihonor_oem_glink_oem_update_config(THERMAL_CONFIG_ID,
			&di->config_data[i], sizeof(di->config_data[i]));
		if (ret) {
			hwlog_err("sync thermal config data to adsp fail, node index: %d\n", i);
			goto next;
		}
	}

	return;

next:
	schedule_delayed_work(&di->thermal_config_work, msecs_to_jiffies(ADSP_DTS_SYNC_INTERVAL));
}


static ssize_t adsp_dts_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t adsp_dts_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info adsp_dts_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(adsp_dts, 0660, ADSP_DTS_SYSFS_THERMAL_CONFIG_DATA, thermal_config),
	power_sysfs_attr_rw(adsp_dts, 0660, ADSP_DTS_SYSFS_THERMAL_BASIC_CONFIG, thermal_basic),
};

#define ADSP_DTS_SYSFS_ATTRS_SIZE  ARRAY_SIZE(adsp_dts_sysfs_field_tbl)

static struct attribute *adsp_dts_sysfs_attrs[ADSP_DTS_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group adsp_dts_sysfs_attr_group = {
	.attrs = adsp_dts_sysfs_attrs,
};

static ssize_t adsp_dts_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct power_sysfs_attr_info *info = NULL;

	info = power_sysfs_lookup_attr(attr->attr.name,
		adsp_dts_sysfs_field_tbl, ADSP_DTS_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case ADSP_DTS_SYSFS_THERMAL_CONFIG_DATA:
		break;
	case ADSP_DTS_SYSFS_THERMAL_BASIC_CONFIG:
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static ssize_t adsp_dts_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct adsp_dts_device *di = g_adsp_dts_di;
	struct power_sysfs_attr_info *info = NULL;
	struct thermal_basic_config *basic_config = NULL;
	struct thermal_channel_state_config *chan_state_data = NULL;
	struct thermal_channel_config *channel_data = NULL;
	int thermal_channel = 0;
	int thermal_state = 0;
	int level_cnt = 0;

	if (!di)
		return -1;

	info = power_sysfs_lookup_attr(attr->attr.name,
		adsp_dts_sysfs_field_tbl, ADSP_DTS_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case ADSP_DTS_SYSFS_THERMAL_CONFIG_DATA:
		hwlog_info("store thermal config data\n");
		if (count != sizeof(struct thermal_channel_state_config)) {
			hwlog_err("thermal_channel_state_config is invalid\n");
			return -EINVAL;
		}
		chan_state_data = (struct thermal_channel_state_config *)buf;
		thermal_channel = chan_state_data->thermal_channel;
		thermal_state = chan_state_data->thermal_state;
		level_cnt = chan_state_data->level_cnt;
		hwlog_info("thermal_channel = %d\n", thermal_channel);
		hwlog_info("thermal_state = %d\n", thermal_state);
		hwlog_info("%d, %d, %d, %d\n",
			chan_state_data->temp_config[0].temp_low,
			chan_state_data->temp_config[0].temp_high,
			chan_state_data->temp_config[0].limit_info.dc_current_limit[0],
			chan_state_data->temp_config[0].limit_info.dc_current_limit[1]);
		di->config_data[thermal_channel].node_index = thermal_channel;
		channel_data = &di->config_data[thermal_channel].chan_config;
		channel_data->thermal_channel = thermal_channel;
		channel_data->level_cnt[thermal_state] = level_cnt;
		memcpy(channel_data->temp_config[thermal_state], chan_state_data->temp_config,
			sizeof(channel_data->temp_config[thermal_state]));
		break;
	case ADSP_DTS_SYSFS_THERMAL_BASIC_CONFIG:
		hwlog_info("store thermal basic config\n");
		if (count != sizeof(struct thermal_basic_config)) {
			hwlog_err("thermal_basic_config is invalid\n");
			return -EINVAL;
		}
		basic_config = (struct thermal_basic_config *)buf;
		hwlog_info("index_mapping = %d\n", basic_config->index_mapping);
		hwlog_info("node_num = %d\n", basic_config->node_num);
		if (basic_config->node_num > MAX_TEMP_NODE_COUNT)
			return -EINVAL;
		memcpy(&di->basic_config, basic_config, sizeof(*basic_config));
		schedule_delayed_work(&di->thermal_config_work, 0);
		break;
	default:
		return -EINVAL;
	}
	return count;
}

static void adsp_dts_sysfs_create_group(struct device *dev)
{
	power_sysfs_init_attrs(adsp_dts_sysfs_attrs,
		adsp_dts_sysfs_field_tbl, ADSP_DTS_SYSFS_ATTRS_SIZE);
	power_sysfs_create_group("hw_power", "adsp_dts",
		&adsp_dts_sysfs_attr_group);
}

static void adsp_dts_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &adsp_dts_sysfs_attr_group);
}

static int adsp_dts_probe(struct platform_device *pdev)
{
	int ret;
	struct adsp_dts_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	hwlog_info("%s %d\n", __func__, __LINE__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	g_adsp_dts_di = di;

	ret = adsp_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	adsp_dts_glink_ops.dev_data = di;
	ret = hihonor_oem_glink_ops_register(&adsp_dts_glink_ops);
	if (ret) {
		hwlog_err("%s fail to register glink ops\n", __func__);
		goto fail_free_mem;
	}

	INIT_DELAYED_WORK(&di->sync_work, adsp_dts_glink_sync_work);
	INIT_DELAYED_WORK(&di->check_work, adsp_dts_check_work);
	INIT_DELAYED_WORK(&di->thermal_config_work, adsp_dts_thermal_config_work);

	power_dbg_ops_register("adsp_dts_send_conf", di,
		NULL,
		(power_dbg_store)adsp_dts_send_conf_store);

	adsp_dts_sysfs_create_group(di->dev);
	return 0;

fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_adsp_dts_di = NULL;

	return ret;
}

static int adsp_dts_remove(struct platform_device *pdev)
{
	struct adsp_dts_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	adsp_dts_sysfs_remove_group(di->dev);
	devm_kfree(&pdev->dev, di);
	return 0;
}

static const struct of_device_id adsp_dts_match_table[] = {
	{
		.compatible = "honor,adsp_dts",
		.data = NULL,
	},
	{},
};


static struct platform_driver adsp_dts_driver = {
	.probe = adsp_dts_probe,
	.remove = adsp_dts_remove,
	.driver = {
		.name = "honor,adsp_dts",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_dts_match_table),
	},
};

static int __init adsp_dts_init(void)
{
	return platform_driver_register(&adsp_dts_driver);
}

static void __exit adsp_dts_exit(void)
{
	platform_driver_unregister(&adsp_dts_driver);
}

device_initcall_sync(adsp_dts_init);
module_exit(adsp_dts_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adsp dts module driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
