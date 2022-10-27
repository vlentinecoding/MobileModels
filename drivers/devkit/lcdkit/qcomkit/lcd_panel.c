/*
 * lcd_panel.c
 *
 * lcd panel function for lcd driver
 *
 * Copyright (c) 2021-2022 Honor Technologies Co., Ltd.
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
#include <securec.h>
#include <linux/input/qpnp-power-on.h>
#include "lcd_defs.h"
#include "lcd_sysfs.h"
#include "lcd_kit_core.h"
#ifdef CONFIG_APP_INFO
#include <misc/app_info.h>
#endif
#include <huawei_platform/log/log_jank.h>
#include <linux/fb.h>
#include "lcd_panel.h"
#include "lcd_bl.h"
#include "lcd_bias.h"
#include "lcd_notifier.h"

#if defined CONFIG_HUAWEI_DSM
static struct dsm_dev dsm_lcd = {
	.name = "dsm_lcd",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = NULL,
	.buff_size = 1024,
};

struct dsm_client *lcd_dclient = NULL;
#endif

#define DEFAULT_PANEL_NAME  "Default dsi panel"
static int lcd_get_project_id (char *buff);
static int panel_proximity_power_off(void);

static struct lcd_kit_ops lcd_ops = {
	.get_status_by_type = panel_get_status_by_type,
	.get_project_id = lcd_get_project_id,
	.proximity_power_off = panel_proximity_power_off,
};

/* panel information */
static struct panel_info *g_panel_info[FB_MAX];
/* current registered fb num */
static int registered_fb_num;
/* notify fp hbm completed */
#ifdef CONFIG_FINGERPRINT
extern void ud_fp_on_hbm_completed(void);
#endif
struct panel_info *get_panel_info(int idx)
{
	if (idx >= FB_MAX) {
		LCD_ERR("idx exceed max fb number\n");
		return NULL;
	}
	return g_panel_info[idx];
}

#ifdef CONFIG_HUAWEI_DSM
int lcd_dsm_client_record(struct dsm_client *lcd_dclient, char *record_buf,
	int lcd_dsm_error_no)
{
	if (!lcd_dclient || !record_buf) {
		LCD_ERR("null pointer!\n");
		return LCD_FAIL;
	}


	if (!dsm_client_ocuppy(lcd_dclient)) {
		dsm_client_record(lcd_dclient, record_buf);
		dsm_client_notify(lcd_dclient, lcd_dsm_error_no);
		return LCD_OK;
	}
	LCD_ERR("dsm_client_ocuppy failed!\n");
	return LCD_FAIL;
}
#endif

void panel_reset_tp_depend_lcd(void)
{
	struct panel_info *pinfo = g_panel_data.pinfo;
	struct ts_kit_ops *ts_ops =  ts_kit_get_ops();

	if (!ts_ops || !pinfo) {
		LCD_ERR("ts_ops or pinfo is null\n");
		return;
	}

	LCD_INFO("tp reset is exec.\n");
	if (pinfo->reset_tp_lcd_gap_sleep)
		msleep(pinfo->reset_tp_lcd_gap_sleep);
	if (ts_ops->ts_reset)
		ts_ops->ts_reset(pinfo->reset_tp_pull_low_sleep);
}

static void panel_set_thp_proximity_state(int power_state)
{
	struct panel_info *pinfo = g_panel_data.pinfo;

	if (!pinfo || !pinfo->proximity.support)
		return;

	pinfo->proximity.panel_power_state = power_state;
}

static void panel_set_proximity_sem(bool sem_lock)
{
	struct panel_info *pinfo = g_panel_data.pinfo;

	if (!pinfo->proximity.support || !pinfo)
		return;

	if (sem_lock == true)
		down(&pinfo->proximity_poweroff_sem);
	else
		up(&pinfo->proximity_poweroff_sem);
}

int panel_get_proxmity_status(int state)
{
	struct ts_kit_ops *ts_ops = NULL;
	struct panel_info *pinfo = g_panel_data.pinfo;
	static bool ts_get_proxmity_flag = true;

	if (!pinfo->proximity.support)
		return TP_PROXMITY_DISABLE;

	if (state) {
		ts_get_proxmity_flag = true;
		LCD_INFO("[Proximity_feature] get status %d\n",
			pinfo->proximity.work_status);
		return pinfo->proximity.work_status ;
	}
	if (ts_get_proxmity_flag == false) {
		LCD_INFO("[Proximity_feature] get status %d\n",
			pinfo->proximity.work_status);
		return pinfo->proximity.work_status;
	}
	ts_ops = ts_kit_get_ops();
	if (!ts_ops) {
		LCD_ERR("ts_ops is null\n");
		return TP_PROXMITY_DISABLE;
	}
	if (ts_ops->get_tp_proxmity) {
		pinfo->proximity.work_status = (int)ts_ops->get_tp_proxmity();
		LCD_INFO("[Proximity_feature] get status %d\n",
				pinfo->proximity.work_status );
	}
	ts_get_proxmity_flag = false;
	return pinfo->proximity.work_status;
}

static int panel_proximity_power_off(void)
{
	struct panel_info *pinfo = g_panel_data.pinfo;
	struct ts_kit_ops *ts_ops = ts_kit_get_ops();

	LCD_INFO("[Proximity_feature] panel_proximity_power_off enter!\n");
	if (!pinfo->proximity.support) {
		LCD_INFO("[Proximity_feature] thp_proximity not support exit!\n");
		return LCD_FAIL;
	}

	if (pinfo->proximity.panel_power_state == LCD_POWER_ON) {
		LCD_INFO("[Proximity_feature] power state is on exit!\n");
		return LCD_FAIL;
	}
	if (pinfo->proximity.panel_power_state == LCD_POWER_OFF) {
		LCD_INFO("[Proximity_feature] power state is off exit!\n");
		return LCD_FAIL;
	}
	if (pinfo->proximity.work_status == TP_PROXMITY_DISABLE) {
		LCD_INFO("[Proximity_feature] thp_proximity has been disabled exit!\n");
		return LCD_FAIL;
	}

	panel_set_proximity_sem(true);
	pinfo->proximity.work_status = TP_PROXMITY_DISABLE;
	if (ts_ops && ts_ops->ts_power_notify)
		ts_ops->ts_power_notify(TS_2ND_POWER_OFF, SHORT_SYNC);

	if (gpio_is_valid(pinfo->panel->reset_config.reset_gpio) &&
		!pinfo->panel->reset_gpio_always_on) {
		gpio_set_value(pinfo->panel->reset_config.reset_gpio, 0);
		msleep(pinfo->proximity.reset_to_bias_delay);
		LCD_INFO("[Proximity_feature] reset--> low!\n");
	}

	if (pinfo->bias.support && pinfo->bias.enabled) {
		panel_bias_off(pinfo->panel);
		panel_bias_enable(pinfo, false);
	}
	panel_set_thp_proximity_state(LCD_POWER_OFF);
	panel_set_proximity_sem(false);
	LCD_INFO("[Proximity_feature] lcd_kit_proximity_power_off exit!\n");
	return LCD_OK;
}

static int lcd_bl_ic_set_backlight(unsigned int bl_lvl)
{
	struct lcd_kit_bl_ops *bl_ops = NULL;
	struct panel_info *pinfo = g_panel_data.pinfo;

	if (!pinfo->power_on) {
		LCD_INFO("No need to set backlight when power off\n");
		return LCD_OK;
	}

	bl_ops = lcd_kit_get_bl_ops();
	if (!bl_ops) {
		LCD_INFO("bl_ops is null!\n");
		return LCD_FAIL;
	}

	if (bl_ops->set_backlight)
		bl_ops->set_backlight(bl_lvl);

	return LCD_OK;
}

static int lcd_bl_ic_en_backlight(bool enable)
{
	struct lcd_kit_bl_ops *bl_ops = NULL;
	struct panel_info *pinfo = g_panel_data.pinfo;

	if (!pinfo->power_on) {
		LCD_INFO("No need to set backlight when power off\n");
		return LCD_OK;
	}

	bl_ops = lcd_kit_get_bl_ops();
	if (!bl_ops) {
		LCD_INFO("bl_ops is null!\n");
		return LCD_FAIL;
	}

	if (bl_ops->en_backlight)
		bl_ops->en_backlight(enable);

	return LCD_OK;
}

static void panel_parse_oem_info(struct dsi_panel *panel, struct panel_info *pinfo)
{
	struct dsi_parser_utils *utils = &panel->utils;
	const char *string;
	int rc = 0;
	/* parse oeminfo */
	pinfo->oeminfo.support = utils->read_bool(utils->data,
			"qcom,mdss-dsi-panel-oem-info-enabled");
	if (!pinfo->oeminfo.support) {
		LCD_INFO("not support oem info\n");
		return;
	}
	pinfo->oeminfo.barcode_2d.support = utils->read_bool(utils->data,
			"qcom,mdss-dsi-panel-oem-2d-barcode-enabled");
	if (pinfo->oeminfo.barcode_2d.support) {
		if (utils->read_u32(utils->data, "qcom,mdss-dsi-panel-oem-2d-barcode-offset",
				&pinfo->oeminfo.barcode_2d.offset))
			LCD_ERR("failed to read: qcom,mdss-dsi-panel-oem-2d-barcode-offset\n");
	}
	pinfo->oeminfo.project_id.support = utils->read_bool(utils->data,
			"qcom,mdss-dsi-panel-oem-project-id-enabled");
	if (pinfo->oeminfo.project_id.support) {
		if (utils->read_u32(utils->data, "qcom,mdss-dsi-panel-oem-project-id-offset",
				&pinfo->oeminfo.project_id.offset))
			LCD_ERR("failed to read: qcom,mdss-dsi-panel-oem-project-id-offset\n");
		rc = utils->read_string(utils->data,
			"qcom,mdss-dsi-panel-oem-project-id-default", &string);
		if (!rc) {
			memcpy(pinfo->oeminfo.project_id.default_project_id, string, strlen(string)+1);
			LCD_INFO("lcd default project_id:%s", pinfo->oeminfo.project_id.default_project_id);
		}
	}
	pinfo->oeminfo.sn_data.support = utils->read_bool(utils->data,
			"qcom,mdss-dsi-panel-oem-sn-enabled");
}

static void panel_parse_fps(struct panel_info *pinfo,
	struct dsi_parser_utils *utils)
{
	int rc;

	pinfo->fps_list_len = utils->count_u32_elems(utils->data,
				  "qcom,dsi-fps-list");
	if (pinfo->fps_list_len < 1) {
		LCD_ERR("fps list not present\n");
		return;
	}

	pinfo->fps_list = kcalloc(pinfo->fps_list_len, sizeof(u32),
			GFP_KERNEL);
	if (!pinfo->fps_list)
		return;

	rc = utils->read_u32_array(utils->data,
			"qcom,dsi-fps-list",
			pinfo->fps_list,
			pinfo->fps_list_len);
	if (rc) {
		LCD_ERR("fps rate list parse failed\n");
		kfree(pinfo->fps_list);
		pinfo->fps_list = NULL;
	}
}

static void panel_parse_batch_info(struct panel_info *pinfo,
			struct dsi_parser_utils *utils)
{
	int rc;
	int i;

	pinfo->lcd_panel_batch_info.support = utils->read_bool(utils->data,
					"qcom,mdss-dsi-panel-read-batch-info-enabled");
	if (!pinfo->lcd_panel_batch_info.support ) {
		LCD_ERR("not support read panel batch info!\n");
		return;
	}
	pinfo->lcd_panel_batch_info.batch_match_hbm = utils->read_bool(utils->data,
					"qcom,mdss-dsi-panel-batch-match-hbm-enabled");
	if (!pinfo->lcd_panel_batch_info.support ) {
		LCD_INFO("not support panel batch match hbm!\n");
		return;
	}
	pinfo->lcd_panel_batch_info.cnt = of_property_count_u32_elems(utils->data,
			"qcom,mdss-dsi-panel-batch-value");

	pinfo->lcd_panel_batch_info.expect_val = kcalloc(pinfo->lcd_panel_batch_info.cnt,
			sizeof(int), GFP_KERNEL);

	if (!pinfo->lcd_panel_batch_info.expect_val) {
		LCD_ERR("kzalloc expect info fail\n");
		return;
	}

	for (i = 0; i < pinfo->lcd_panel_batch_info.cnt; i++) {
		of_property_read_u32_index(utils->data,
		"qcom,mdss-dsi-panel-batch-value", i, &rc);
		pinfo->lcd_panel_batch_info.expect_val[i] = rc;
 		LCD_INFO("%d dts otp value is %d\n",i,rc);
	}
}

static void panel_parse_proximity_info(struct panel_info *pinfo,
			struct dsi_parser_utils *utils)
{
	pinfo->proximity.support = utils->read_bool(utils->data,
			"qcom,mdss-dsi-panel-proximity-enabled");
	if (!pinfo->proximity.support) {
		LCD_INFO("not support thp proximity\n");
		return;
	}
	pinfo->proximity.work_status = TP_PROXMITY_DISABLE;
	pinfo->proximity.panel_power_state = LCD_POWER_ON;

	if (utils->read_u32(utils->data,
		"qcom,mdss-dsi-panel-proximity-reset-to-dispon-delay",
		&pinfo->proximity.reset_to_dispon_delay))
		LCD_ERR("failed to read: qcom,mdss-dsi-panel-proximity-reset-to-dispon-delay\n");
	if (utils->read_u32(utils->data,
		"qcom,mdss-dsi-panel-proximity-reset-to-bias-delay",
		&pinfo->proximity.reset_to_bias_delay))
		LCD_ERR("failed to read: qcom,mdss-dsi-panel-proximity-reset-to-bias-delay\n");
}

static void panel_parse_quickly_sleepout(struct panel_info *pinfo,
			struct dsi_parser_utils *utils)
{
	int rc = 0;
	pinfo->quickly_sleep_out.support = utils->read_bool(utils->data,
		"qcom,panel-quickly-sleepout-support");
	if(pinfo->quickly_sleep_out.support) {
		rc = utils->read_u32(utils->data, "qcom,panel-quickly-sleepout-interval",
			&pinfo->quickly_sleep_out.interval);
		if (rc)
			LCD_ERR("failed to read: panel-quickly-sleepout-interval, rc=%d\n", rc);
	}
}

static void panel_parse_force_poweroff_info(struct panel_info *pinfo,
			struct dsi_parser_utils *utils)
{
	pinfo->pwrkey_press.support = utils->read_bool(utils->data,
		"qcom,mdss-dsi-panel-force-power-off");
	if (utils->read_u32(utils->data,"qcom,mdss-dsi-panel-force-power-time",
		&pinfo->pwrkey_press.timer_val))
		LCD_ERR("failed to read: com,mdss-dsi-panel-force-power-time\n");
}

static int panel_parse(struct dsi_panel *panel, struct panel_info *pinfo)
{
	int rc = 0;
	struct dsi_parser_utils *utils = &panel->utils;

	if (!utils) {
		LCD_ERR("utils is null\n");
		return -1;
	}
	/* parse dt */
	pinfo->lcd_model = utils->get_property(utils->data,
		"qcom,mdss-dsi-lcd-model", NULL);
	if (!pinfo->lcd_model)
		pinfo->lcd_model = DEFAULT_PANEL_NAME;

	pinfo->hbm.enabled = utils->read_bool(utils->data,
		"qcom,mdss-dsi-panel-hbm-enabled");
	pinfo->local_hbm_enabled = utils->read_bool(utils->data,
		"qcom,mdss-dsi-panel-local-hbm-enabled");
	pinfo->ts_poweroff_in_lp_step = utils->read_bool(utils->data,
		"qcom,mdss-dsi-panel-poweroff-tp-in-lp");
	pinfo->ts_poweroff_before_disp_off = utils->read_bool(utils->data,
		"qcom,mdss-dsi-panel-poweroff-tp-before-dispoff");
	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-bl-max-nit",
		&panel->bl_config.bl_max_nit);
	if (rc)
		LCD_ERR("failed to read: qcom,mdss-dsi-bl-max-nit, rc=%d\n", rc);
	pinfo->four_byte_bl = utils->read_bool(utils->data, "qcom,mdss-dsi-bl-four-byte-bl-enabled");
	pinfo->reset_tp_depend_lcd = utils->read_bool(utils->data,
		"qcom,mdss-dsi-reset-tp-depend-lcd");
	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-reset-tp-lcd-gap-sleep",
		&pinfo->reset_tp_lcd_gap_sleep);
	if (rc)
		LCD_ERR("failed to read: qcom,mdss-dsi-reset-tp-lcd-gap-sleep, rc=%d\n", rc);
	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-reset-tp-pull-low-sleep",
		&pinfo->reset_tp_pull_low_sleep);
	if (rc)
		LCD_ERR("failed to read: qcom,mdss-dsi-reset-tp-pull-low-sleep, rc=%d\n", rc);
	panel_parse_quickly_sleepout(pinfo, utils);
	panel_parse_oem_info(panel, pinfo);
	panel_parse_fps(pinfo, utils);
	panel_parse_batch_info(pinfo, utils);
	panel_parse_bias_info(pinfo, utils);
	panel_parse_proximity_info(pinfo, utils);
	panel_parse_force_poweroff_info(pinfo, utils);
	return 0;
}

static int panel_init(struct dsi_panel *panel)
{
	int ret;
	struct panel_info *pinfo = NULL;

	if (!panel || !panel->name || !panel->pdata) {
		LCD_ERR("null pointer\n");
		return -1;
	}
	panel->enable_esd_check = true;
	pinfo = kzalloc(sizeof(struct panel_info), GFP_KERNEL);
	if (!pinfo) {
		LCD_ERR("kzalloc pinfo fail\n");
		return -1;
	}
	pinfo->power_on = true;
	pinfo->panel_state = LCD_POWER_ON;
	pinfo->bias.enabled = false;
	sema_init(&pinfo->proximity_poweroff_sem, 1);
	/* panel dtsi parse */
	ret = panel_parse(panel, pinfo);
	if (ret)
		LCD_ERR("panel parse failed\n");
#ifdef CONFIG_LCD_FACTORY
	ret = factory_init(panel, pinfo);
	if (ret)
		LCD_ERR("factory init failed\n");
#endif
#ifdef CONFIG_APP_INFO
	/* set app_info */
	ret = app_info_set("lcd type", panel->name);
	if (ret)
		LCD_ERR("set app info failed\n");
#endif
#if defined CONFIG_HUAWEI_DSM
	lcd_dclient = dsm_register_client(&dsm_lcd);
#endif
	/* register extern callback */
	lcd_kit_ops_register(&lcd_ops);

	panel->pdata->pinfo = pinfo;
	if (registered_fb_num >= FB_MAX) {
		kfree(pinfo);
		LCD_ERR("exceed max fb number\n");
		return -1;
	}
	pinfo->panel = panel;
	g_panel_info[registered_fb_num++] = pinfo;
	g_panel_data.pinfo = pinfo;
	if (pinfo->bias.support)
		panel_bias_gpio_config(pinfo);
	if (pinfo->pwrkey_press.support)
		lcd_register_power_key_notify();
#if defined(LCD_DEBUG_ENABLE)
	debugfs_init();
#endif
	return 0;
}

static int panel_hbm_mmi_set(struct dsi_panel *panel, int level)
{
	struct hbm_desc *hbm = NULL;

	hbm = &(panel->pdata->pinfo->hbm);
	if (!hbm->enabled)
		return 0;
	if (level == 0) {
		dsi_panel_set_hbm_level(panel, panel->bl_config.bl_level);
		hbm->mode = HBM_EXIT;
		return 0;
	}
	dsi_panel_set_hbm_level(panel, level);
	hbm->mode = HBM_ENTER;
	return 0;
}

static int panel_hbm_set_handle(struct dsi_panel *panel, int dimming,
	int level)
{
	int rc;
	static int last_level = 0;
	struct hbm_desc *hbm = NULL;

	hbm = &(panel->pdata->pinfo->hbm);
	if (!hbm->enabled)
		return 0;
	if ((level < 0) || (level > HBM_SET_MAX_LEVEL)) {
		LCD_ERR("input param invalid, hbm_level %d!\n", level);
		return -1;
	}
	if (level > 0) {
		if (last_level == 0) {
			/* enable hbm */
			rc = dsi_panel_set_cmd(panel, DSI_CMD_SET_HBM_ENABLE);
			if (rc)
				LCD_ERR("set hbm enable failed\n");
			if (!dimming) {
				rc = dsi_panel_set_cmd(panel, DSI_CMD_SET_HBM_DIMM_OFF);
				if (rc)
					LCD_ERR("set hbm dimming off failed\n");
			}
			hbm->mode = HBM_ENTER;
		}
	} else {
		if (last_level == 0) {
			/* disable dimming */
			rc = dsi_panel_set_cmd(panel, DSI_CMD_SET_HBM_DIMM_OFF);
			if (rc)
				LCD_ERR("set hbm dimming off failed\n");
		} else {
			/* exit hbm */
			if (dimming) {
				rc = dsi_panel_set_cmd(panel, DSI_CMD_SET_HBM_DIMM_ON);
				if (rc)
					LCD_ERR("set hbm dimming on failed\n");
			} else {
				rc = dsi_panel_set_cmd(panel, DSI_CMD_SET_HBM_DIMM_OFF);
				if (rc)
					LCD_ERR("set hbm dimming off failed\n");
			}
			rc = dsi_panel_set_cmd(panel, DSI_CMD_SET_HBM_DISABLE);
			if (rc)
				LCD_ERR("set hbm disable failed\n");
			hbm->mode = HBM_EXIT;
		}
	}
	/* set hbm level */
	rc = dsi_panel_set_hbm_level(panel, level);
	if (rc)
		LCD_ERR("set backlight failed\n");
	last_level = level;
	return 0;
}

static int panel_local_hbm_mmi_set(struct dsi_panel *panel, int level)
{
	LCD_INFO("local_hbm_enabled:%d\n",panel->pdata->pinfo->local_hbm_enabled);
	if (!panel->pdata->pinfo->local_hbm_enabled)
		return -1;

	if (level == 0) {
		dsi_panel_set_cmd(panel, DSI_CMD_SET_LHBM_DISABLE);
		return 0;
	}

	if (panel->power_mode == SDE_MODE_DPMS_LP1 ||
		panel->power_mode == SDE_MODE_DPMS_LP2) {
		LCD_INFO("current mode is aod and lhbm return\n");
		return -1;
        }

	dsi_panel_set_cmd(panel, DSI_CMD_SET_LHBM_ENABLE);
	return 0;
}

static int panel_local_hbm_identify_set(struct dsi_panel *panel, int level)
{
	LCD_INFO("local_hbm_enabled:%d\n",panel->pdata->pinfo->local_hbm_enabled);
	if (!panel->pdata->pinfo->local_hbm_enabled)
		return -1;

	if (level == 0) {
		dsi_panel_set_cmd(panel, DSI_CMD_SET_LHBM_DISABLE);
		if (panel->power_mode != SDE_MODE_DPMS_LP1 &&
			panel->power_mode != SDE_MODE_DPMS_LP2) {
			LCD_INFO("current mode is not aod and LhbmExit\n");
			dsi_panel_set_cmd(panel, DSI_CMD_SET_TIMING_SWITCH);
		}
		return 0;
	}

	dsi_panel_set_cmd(panel, DSI_CMD_SET_LHBM_ENABLE);
#ifdef CONFIG_FINGERPRINT
	ud_fp_on_hbm_completed();
#endif
	return 0;
}

static int panel_hbm_set(struct dsi_panel *panel,
	struct display_engine_ddic_hbm_param *hbm_cfg)
{
	int rc;

	if (!panel || !panel->pdata || !panel->pdata->pinfo) {
		LCD_ERR("panel have null pointer\n");
		return -EINVAL;
	}
	if (!hbm_cfg) {
		LCD_ERR("hbm_cfg is null\n");
		return -EINVAL;
	}
	LCD_INFO("hbm tpye:%d, level:%d, dimming:%d\n", hbm_cfg->type,
		hbm_cfg->level, hbm_cfg->dimming);
	mutex_lock(&panel->panel_lock);
	switch (hbm_cfg->type) {
	case HBM_FOR_MMI:
		rc = panel_hbm_mmi_set(panel, hbm_cfg->level);
		break;
	case HBM_FOR_LIGHT:
		rc = panel_hbm_set_handle(panel, hbm_cfg->level, hbm_cfg->dimming);
		break;
	case LOCAL_HBM_FOR_MMI:
		rc = panel_local_hbm_mmi_set(panel, hbm_cfg->level);
		break;
	case LOCAL_HBM_FOR_IDENTIFY:
		rc = panel_local_hbm_identify_set(panel, hbm_cfg->level);
		break;
	default:
		LCD_ERR("not support type:%d\n", hbm_cfg->type);
		rc = -1;
	}
	mutex_unlock(&panel->panel_lock);
	return rc;
}

static void lcd_kit_disp_on_check_delay(struct dsi_panel *panel)
{
	struct panel_info *pinfo = NULL;
	long delta_time_bl_to_panel_on;
	unsigned int delay_margin;
	struct timeval tv = {0};
	int max_delay_margin = MAX_DELAY_TIME;

	if (!panel || !panel->pdata || !panel->pdata->pinfo) {
		LCD_ERR("pointer is null\n");
		return;
	}
	pinfo = panel->pdata->pinfo;

	memset(&tv, 0, sizeof(struct timeval));
	do_gettimeofday(&tv);
	/* change s to us */
	delta_time_bl_to_panel_on = (tv.tv_sec - pinfo->quickly_sleep_out.panel_on_record_tv.tv_sec) *
		1000000 + tv.tv_usec - pinfo->quickly_sleep_out.panel_on_record_tv.tv_usec;
	/* change us to ms */
	delta_time_bl_to_panel_on /= 1000;
	if (delta_time_bl_to_panel_on >= pinfo->quickly_sleep_out.interval) {
		LCD_INFO("%lu > %d, no need delay\n",
			delta_time_bl_to_panel_on,
			pinfo->quickly_sleep_out.interval);
		pinfo->quickly_sleep_out.panel_on_tag = false;
		return;
	}
	delay_margin = pinfo->quickly_sleep_out.interval -
		delta_time_bl_to_panel_on;
	if (delay_margin > max_delay_margin) {
		LCD_INFO("something maybe error");
		pinfo->quickly_sleep_out.panel_on_tag = false;
		return;
	}
	usleep_range(delay_margin*1000, delay_margin*1000+100);
	LCD_INFO("backlight on delay %dms\n", delay_margin);
	pinfo->quickly_sleep_out.panel_on_tag = false;
}

void lcd_kit_disp_on_record_time(struct dsi_panel *panel)
{
	struct panel_info *pinfo = NULL;

	if (!panel || !panel->pdata || !panel->pdata->pinfo) {
		LCD_ERR("pointer is null\n");
		return;
	}
	pinfo = panel->pdata->pinfo;
	do_gettimeofday(&pinfo->quickly_sleep_out.panel_on_record_tv);
	LCD_INFO("display on at %lu seconds %lu mil seconds\n",
		pinfo->quickly_sleep_out.panel_on_record_tv.tv_sec,
		pinfo->quickly_sleep_out.panel_on_record_tv.tv_usec);
	pinfo->quickly_sleep_out.panel_on_tag = true;
}

void panel_status_notify(int fb_blank)
{
	int blank;
	struct fb_event event;

	blank = fb_blank;
	event.info = NULL;
	event.data = &blank;

	fb_notifier_call_chain(FB_EVENT_PANEL, &event);
}

static int panel_on(struct dsi_panel *panel, int step)
{
	int rc = 0;
	struct ts_kit_ops *ts_ops = ts_kit_get_ops();
	struct panel_info *pinfo = NULL;

	if (!panel || !panel->pdata || !panel->pdata->pinfo) {
		LCD_ERR("pointer is null\n");
		return -1;
	}

	pinfo = panel->pdata->pinfo;
	switch (step) {
	case PANEL_INIT_NONE:
		LCD_INFO("panel init none step\n");
		LCD_INFO("dsi panel: %s\n", panel->name);
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "LCD_POWER_ON");
		if (panel_get_proxmity_status(LCD_POWER_ON)) {
			LCD_INFO("[Proximity_feature] proximity enable, no continue\n");
		} else {
			if (pinfo->bias.support && pinfo->bias.enabled) {
				LCD_INFO("panel_bias_on step\n");
				panel_bias_enable(pinfo, true);
				panel_bias_on(panel);
			}
		}
		break;
	case PANEL_INIT_POWER_ON:
		LCD_INFO("panel init power on step\n");
		panel->pdata->pinfo->panel_state = LCD_POWER_ON;
		if (ts_ops && ts_ops->ts_power_notify)
			ts_ops->ts_power_notify(TS_RESUME_DEVICE, NO_SYNC);
		break;
	case PANEL_INIT_MIPI_LP_SEND_SEQUENCE:
		LCD_INFO("panel init mipi lp step\n");
		if (pinfo->quickly_sleep_out.support)
			lcd_kit_disp_on_record_time(panel);
		panel->pdata->pinfo->power_on = true;
		panel->pdata->pinfo->panel_state = LCD_LP_ON;
		if (ts_ops && ts_ops->ts_power_notify)
			ts_ops->ts_power_notify(TS_AFTER_RESUME, NO_SYNC);
#ifdef CONFIG_LCD_FACTORY
		if (atomic_read(&panel->pdata->pinfo->lcd_noti_comp))
			complete(&panel->pdata->pinfo->lcd_test_comp);
#endif
		break;
	case PANEL_INIT_MIPI_HS_SEND_SEQUENCE:
		LCD_INFO("panel init mipi hs step\n");
		panel->pdata->pinfo->panel_state = LCD_HS_ON;
		panel_status_notify(FB_BLANK_UNBLANK);
		break;
	default:
		LCD_ERR("not support step:%d\n", step);
		rc = -1;
		break;
	}
	if (!rc)
		panel_set_thp_proximity_state(LCD_POWER_ON);
	return rc;
}

static int panel_off(struct dsi_panel *panel, int step)
{
	int rc = 0;
	struct ts_kit_ops *ts_ops = ts_kit_get_ops();
	struct panel_info *pinfo = NULL;

	if (!panel || !panel->pdata || !panel->pdata->pinfo) {
		LCD_ERR("pointer is null\n");
		return -1;
	}

	pinfo = panel->pdata->pinfo;
	switch (step) {
	case PANEL_UNINIT_NONE:
		LCD_INFO("panel uninit none step\n");
		if (pinfo->ts_poweroff_before_disp_off &&
			ts_ops && ts_ops->ts_power_notify)
			ts_ops->ts_power_notify(TS_EARLY_SUSPEND, NO_SYNC);
		/* proximity_feature after ts_early_suspend is completed */
		if (panel_get_proxmity_status(LCD_POWER_OFF))
			panel_set_thp_proximity_state(LCD_POWER_SUSPEND);
		break;
	case PANEL_UNINIT_MIPI_HS_SEND_SEQUENCE:
		LCD_INFO("panel uninit mipi hs step\n");
		panel->pdata->pinfo->power_on = false;
		if (!pinfo->ts_poweroff_before_disp_off &&
			ts_ops && ts_ops->ts_power_notify)
			ts_ops->ts_power_notify(TS_EARLY_SUSPEND, NO_SYNC);
		break;
	case PANEL_UNINIT_MIPI_LP_SEND_SEQUENCE:
		LCD_INFO("panel uninit mipi lp step\n");
		if (pinfo->ts_poweroff_in_lp_step &&
				ts_ops && ts_ops->ts_power_notify) {
			ts_ops->ts_power_notify(TS_BEFORE_SUSPEND, NO_SYNC);
			ts_ops->ts_power_notify(TS_SUSPEND_DEVICE, NO_SYNC);
		}
		if (panel_get_proxmity_status(LCD_POWER_OFF)) {
			LCD_INFO("[Proximity_feature] proximity enable, no continue\n");
		} else {
			if (pinfo->bias.support && pinfo->bias.enabled)
				panel_bias_off(panel);
		}
		panel_status_notify(FB_BLANK_POWERDOWN);
		break;
	case PANEL_UNINIT_POWER_OFF:
		panel->pdata->pinfo->panel_state = LCD_POWER_OFF;
		panel->pdata->pinfo->hbm.mode = HBM_EXIT;
		LCD_INFO("panel uninit power off step\n");
		if (!pinfo->ts_poweroff_in_lp_step &&
				ts_ops && ts_ops->ts_power_notify) {
			ts_ops->ts_power_notify(TS_BEFORE_SUSPEND, NO_SYNC);
			ts_ops->ts_power_notify(TS_SUSPEND_DEVICE, NO_SYNC);
		}
		if (panel_get_proxmity_status(LCD_POWER_OFF)) {
			LCD_INFO("[Proximity_feature] proximity enable, no continue\n");
		} else {
			if (pinfo->bias.support && pinfo->bias.enabled)
				panel_bias_enable(pinfo, false);
			panel_set_thp_proximity_state(LCD_POWER_OFF);
		}
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_OFF, "%s", "LCD_POWER_OFF");
		break;
	default:
		LCD_ERR("not support step:%d\n", step);
		rc = -1;
		break;
	}
	return rc;
}

static int panel_hbm_fp_set(struct dsi_panel *panel, int mode)
{
	int bl_level;

	if (!panel->pdata->pinfo) {
		LCD_ERR("pinfo is null\n");
		return -1;
	}
	if (mode == HBM_MODE_ON) {
		dsi_panel_set_cmd(panel, DSI_CMD_SET_FP_HBM_ENABLE);
		panel->pdata->pinfo->hbm.mode = HBM_ENTER;
#ifdef CONFIG_FINGERPRINT
		ud_fp_on_hbm_completed();
#endif
	} else {
		dsi_panel_set_cmd(panel, DSI_CMD_SET_FP_HBM_DISABLE);
		panel->pdata->pinfo->hbm.mode = HBM_EXIT;
		bl_level = panel->bl_config.bl_level;
		LCD_INFO("restore bl_level = %d\n", bl_level);
		dsi_panel_set_hbm_level(panel, bl_level);
	}
	return 0;
}

static int print_backlight(struct dsi_panel *panel, u32 bl_lvl)
{
	static int last_level = 0;

	if (last_level == 0 && bl_lvl != 0) {
		LCD_INFO("screen on, backlight level = %d\n", bl_lvl);
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_ON, "LCD_BACKLIGHT_ON,%u", bl_lvl);
	} else if (last_level !=0 && bl_lvl == 0) {
		LCD_INFO("screen off, backlight level = %d\n", bl_lvl);
		LOG_JANK_D(JLID_KERNEL_LCD_BACKLIGHT_OFF, "LCD_BACKLIGHT_OFF");
	}
	LCD_INFO("backlight level = %d\n", bl_lvl);
	last_level = bl_lvl;
	return 0;
}

int panel_get_status_by_type(int type, int *status)
{
	int ret;
	struct panel_info *pinfo = NULL;

	if (!status) {
		LCD_ERR("status is null\n");
		return LCD_FAIL;
	}
	pinfo = get_panel_info(0);
	if (!pinfo) {
		LCD_ERR("pinfo is null\n");
		return LCD_FAIL;
	}
	switch (type) {
	case PT_STATION_TYPE:
#ifdef CONFIG_LCD_FACTORY
		if (!pinfo->fact_info) {
			LCD_ERR("fact_info is null\n");
			return LCD_FAIL;
		}
		LCD_INFO("pt_flag = %d\n", pinfo->fact_info->pt_flag);
		*status = pinfo->fact_info->pt_flag;
#endif
		ret = LCD_OK;
		break;
	default:
		LCD_ERR("not support type\n");
		ret = LCD_FAIL;
		break;
	}
	return ret;
}

static int read_sn_code(struct dsi_panel *panel)
{
	struct panel_info *pinfo = NULL;
	int rc = 0;

	pinfo = panel->pdata->pinfo;
	if (!pinfo) {
		LCD_ERR("pinfo is null\n");
		return LCD_FAIL;
	}
	if (!pinfo->oeminfo.sn_data.support)
		return rc;
	rc = dsi_panel_receive_data(panel, DSI_CMD_READ_OEMINFO,
		pinfo->oeminfo.sn_data.sn_code, LCD_SN_CODE_LENGTH);
	if (rc) {
		LCD_ERR("read sn reg failed\n");
		return rc;
	}
	LCD_INFO("sn: %s\n", panel->pdata->pinfo->oeminfo.sn_data.sn_code);
	return rc;
}

static int read_batch_match(struct dsi_panel *panel)
{
	struct panel_info *pinfo = NULL;
	uint8_t read_value = 0;
	int rc = LCD_OK;
	int i;

	pinfo = panel->pdata->pinfo;
	if (!pinfo) {
		LCD_ERR("pinfo is null\n");
		return LCD_FAIL;
	}
	if (!pinfo->lcd_panel_batch_info.support ||
		!pinfo->lcd_panel_batch_info.batch_match_hbm)
		return rc;
	rc = dsi_panel_receive_data(panel, DSI_CMD_READ_BATCHINFO,
		&read_value, MAX_REG_READ_COUNT);
	if (rc) {
		LCD_ERR("read batch reg failed\n");
		return rc;
	}
	LCD_INFO("LHBM OTP batch value: %d\n", read_value);
	for (i = 0; i < pinfo->lcd_panel_batch_info.cnt; i++) {
		if (pinfo->lcd_panel_batch_info.expect_val[i] == read_value) {
			break;
		}
	}

	if (i >= pinfo->lcd_panel_batch_info.cnt) {
		LCD_INFO("LHBM is not supported\n");
		pinfo->local_hbm_enabled = false;
	}
	return rc;
}

static int read_2d_barcode(struct dsi_panel *panel)
{
	struct panel_info *pinfo = NULL;
	int rc = 0;

	pinfo = panel->pdata->pinfo;
	if (!pinfo) {
		LCD_ERR("pinfo is null\n");
		return LCD_FAIL;
	}
	if (!pinfo->oeminfo.barcode_2d.support)
		return rc;
	rc = dsi_panel_receive_data(panel, DSI_CMD_READ_OEMINFO,
		pinfo->oeminfo.barcode_2d.barcode_data, OEM_INFO_SIZE_MAX - 1);
	if (rc) {
		LCD_ERR("read sn reg failed\n");
		return rc;
	}
	LCD_INFO("barcode_data: %s\n", pinfo->oeminfo.barcode_2d.barcode_data);
	return rc;
}

static int panel_set_boot(struct dsi_panel *panel)
{
	static bool inited = false;

	if (!inited) {
		/* read panel sn code */
		read_sn_code(panel);
		/* read panel batch value */
		read_batch_match(panel);
		/* read 2d barcode */
		read_2d_barcode(panel);
		inited = true;
	}
	return 0;
}

static int lcd_get_project_id (char *buff)
{
	struct panel_info *pinfo = NULL;

	if (buff == NULL) {
		LCD_ERR("buff is null\n");
		return LCD_FAIL;
	}
	pinfo = get_panel_info(0);

	/* use read project id */
	if (pinfo->oeminfo.project_id.support &&
		(strlen(pinfo->oeminfo.project_id.id) > 0)) {
		strncpy_s(buff, 10, pinfo->oeminfo.project_id.id,
			strlen(pinfo->oeminfo.project_id.id)+1);
		LCD_INFO("use read project id is %s\n", buff);
		return LCD_OK;
	}

	/* use default project id */
	if (pinfo->oeminfo.project_id.support &&
		(strlen(pinfo->oeminfo.project_id.default_project_id) > 0)) {
		strncpy_s(buff, 10, pinfo->oeminfo.project_id.default_project_id,
			strlen(pinfo->oeminfo.project_id.default_project_id)+1);
		LCD_INFO("use default project id:%s\n", buff);
		return LCD_OK;
	}

	LCD_ERR("not support get lcd project_id\n");
	return LCD_FAIL;
}

struct panel_data g_panel_data = {
	.panel_init = panel_init,
	.panel_hbm_set = panel_hbm_set,
	.panel_hbm_fp_set = panel_hbm_fp_set,
	.create_sysfs = lcd_create_sysfs,
	.on = panel_on,
	.off = panel_off,
	.print_bkl = print_backlight,
	.boot_set = panel_set_boot,
	.set_backlight = lcd_bl_ic_set_backlight,
	.en_backlight = lcd_bl_ic_en_backlight,
	.disp_on_check_delay = lcd_kit_disp_on_check_delay,
	.reset_tp_depend_lcd = panel_reset_tp_depend_lcd,
	.set_proximity_sem =  panel_set_proximity_sem,
	.get_proxmity_status = panel_get_proxmity_status,
};
