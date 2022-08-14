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
#include "lcd_defs.h"
#include "lcd_sysfs.h"
#include "lcd_kit_core.h"
#ifdef CONFIG_APP_INFO
#include <misc/app_info.h>
#endif
#include <huawei_platform/log/log_jank.h>
#include "lcd_panel.h"

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

static struct lcd_kit_ops lcd_ops = {
	.get_status_by_type = panel_get_status_by_type,
};

/* panel information */
static struct panel_info *g_panel_info[FB_MAX];
/* current registered fb num */
static int registered_fb_num;
/* notify fp hbm completed */
extern void ud_fp_on_hbm_completed(void);

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

static void panel_parse_oem_info(struct dsi_panel *panel, struct panel_info *pinfo)
{
	struct dsi_parser_utils *utils = &panel->utils;

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
			DSI_ERR("failed to read: qcom,mdss-dsi-panel-oem-2d-barcode-offset\n");
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

	pinfo->lcd_panel_batch_info.support = utils->read_bool(utils->data,
					"qcom,mdss-dsi-panel-read-batch-info-enabled");
	if (!pinfo->lcd_panel_batch_info.support ) {
		LCD_ERR("not support read panel batch info!\n");
		return;
	}
	pinfo->lcd_panel_batch_info.batch_match_hbm = utils->read_bool(utils->data,
					"qcom,mdss-dsi-panel-batch-match-hbm-enabled");
	if (!pinfo->lcd_panel_batch_info.support ) {
		LCD_ERR("not support panel batch match hbm!\n");
		return;
	}
	rc = utils->read_u32(utils->data,
			"qcom,mdss-dsi-panel-batch-value",
			&pinfo->lcd_panel_batch_info.expect_val);
	if (rc)
		LCD_ERR("failed to read: qcom,mdss-dsi-panel-batch-value, rc=%d\n", rc);
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
	rc = utils->read_u32(utils->data, "qcom,mdss-dsi-bl-max-nit",
		&panel->bl_config.bl_max_nit);
	if (rc)
		DSI_ERR("failed to read: qcom,mdss-dsi-bl-max-nit, rc=%d\n", rc);
	pinfo->four_byte_bl = utils->read_bool(utils->data, "qcom,mdss-dsi-bl-four-byte-bl-enabled");
	panel_parse_oem_info(panel, pinfo);
	panel_parse_fps(pinfo, utils);
	panel_parse_batch_info(pinfo, utils);
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

	if (panel->power_mode == SDE_MODE_DPMS_LP1 ||
		panel->power_mode == SDE_MODE_DPMS_LP2) {
		LCD_INFO("current mode is aod and lhbm return\n");
		return -1;
     }
	dsi_panel_set_cmd(panel, DSI_CMD_SET_LHBM_ENABLE);
	ud_fp_on_hbm_completed();
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

static int panel_on(struct dsi_panel *panel, int step)
{
	int rc = 0;
	struct ts_kit_ops *ts_ops = ts_kit_get_ops();

	if (!panel || !panel->pdata || !panel->pdata->pinfo) {
		LCD_ERR("pointer is null\n");
		return -1;
	}
	switch (step) {
	case PANEL_INIT_NONE:
		LCD_INFO("panel init none step\n");
		LCD_INFO("dsi panel: %s\n", panel->name);
		LOG_JANK_D(JLID_KERNEL_LCD_POWER_ON, "%s", "LCD_POWER_ON");
		break;
	case PANEL_INIT_POWER_ON:
		LCD_INFO("panel init power on step\n");
		panel->pdata->pinfo->panel_state = LCD_POWER_ON;
		if (ts_ops && ts_ops->ts_power_notify)
			ts_ops->ts_power_notify(TS_RESUME_DEVICE, NO_SYNC);
		break;
	case PANEL_INIT_MIPI_LP_SEND_SEQUENCE:
		LCD_INFO("panel init mipi lp step\n");
		panel->pdata->pinfo->power_on = true;
		panel->pdata->pinfo->panel_state = LCD_LP_ON;
		if (ts_ops && ts_ops->ts_power_notify)
			ts_ops->ts_power_notify(TS_AFTER_RESUME, NO_SYNC);
		break;
	case PANEL_INIT_MIPI_HS_SEND_SEQUENCE:
		LCD_INFO("panel init mipi hs step\n");
		panel->pdata->pinfo->panel_state = LCD_HS_ON;
		break;
	default:
		LCD_ERR("not support step:%d\n", step);
		rc = -1;
		break;
	}
	return rc;
}

static int panel_off(struct dsi_panel *panel, int step)
{
	int rc = 0;
	struct ts_kit_ops *ts_ops = ts_kit_get_ops();

	if (!panel || !panel->pdata || !panel->pdata->pinfo) {
		LCD_ERR("pointer is null\n");
		return -1;
	}
	switch (step) {
	case PANEL_UNINIT_NONE:
		LCD_INFO("panel uninit none step\n");
		break;
	case PANEL_UNINIT_MIPI_HS_SEND_SEQUENCE:
		LCD_INFO("panel uninit mipi hs step\n");
		panel->pdata->pinfo->power_on = false;
		if (ts_ops && ts_ops->ts_power_notify)
			ts_ops->ts_power_notify(TS_EARLY_SUSPEND, NO_SYNC);
		break;
	case PANEL_UNINIT_MIPI_LP_SEND_SEQUENCE:
		LCD_INFO("panel uninit mipi lp step\n");
		break;
	case PANEL_UNINIT_POWER_OFF:
		panel->pdata->pinfo->panel_state = LCD_POWER_OFF;
		panel->pdata->pinfo->hbm.mode = HBM_EXIT;
		LCD_INFO("panel uninit power off step\n");
		if (ts_ops && ts_ops->ts_power_notify) {
			ts_ops->ts_power_notify(TS_BEFORE_SUSPEND, NO_SYNC);
			ts_ops->ts_power_notify(TS_SUSPEND_DEVICE, NO_SYNC);
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
		ud_fp_on_hbm_completed();
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
	int rc = 0;

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
	LCD_INFO("batch value: %d\n", read_value);
	if (pinfo->lcd_panel_batch_info.expect_val != read_value)
		pinfo->local_hbm_enabled = false;
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

struct panel_data g_panel_data = {
	.panel_init = panel_init,
	.panel_hbm_set = panel_hbm_set,
	.panel_hbm_fp_set = panel_hbm_fp_set,
	.create_sysfs = lcd_create_sysfs,
	.on = panel_on,
	.off = panel_off,
	.print_bkl = print_backlight,
	.boot_set = panel_set_boot,
};
