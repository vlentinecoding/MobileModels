/*
 * lcd_bias.c
 *
 * lcd bias function for lcd driver
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

#include "lcd_bias.h"
#include "lcd_defs.h"

static struct lcd_kit_bias_ops *g_bias_ops;

void panel_parse_bias_info(struct panel_info *pinfo,
			struct dsi_parser_utils *utils)
{
	if (!pinfo || !utils || !utils->data) {
		LCD_ERR("Invalid  parameters!\n");
		return;
	}
	pinfo->bias.support = utils->read_bool(utils->data,
		"qcom,mdss-dsi-panel-bias-support");
	LCD_INFO("panel-bias-support =%d\n", pinfo->bias.support);
	if (!pinfo->bias.support) {
		LCD_INFO("not support panel bias\n");
		return;
	}
	if (pinfo->bias.support) {
		pinfo->bias.enable_gpio = utils->get_named_gpio(utils->data,
					"qcom,mdss-panel-bias-enable-gpio", 0);
		if (!gpio_is_valid(pinfo->bias.enable_gpio))
			LCD_ERR("read panel-bias-enable-gpio fail\n");
		pinfo->bias.vsp_gpio = utils->get_named_gpio(utils->data,
					"qcom,mdss-panel-vsp-gpio", 0);
		if (!gpio_is_valid(pinfo->bias.vsp_gpio))
			LCD_ERR("read panel-bias-vsp-gpio fail\n");
		pinfo->bias.vsn_gpio = utils->get_named_gpio(utils->data,
					"qcom,mdss-panel-vsn-gpio", 0);
		if (!gpio_is_valid(pinfo->bias.vsn_gpio))
			LCD_ERR("read panel-bias-vsn-gpio fail\n");

		pinfo->bias.poweron_vsp_delay = of_property_count_u32_elems(
			utils->data, "qcom,mdss-panel-poweron-vsp-sleep");
		pinfo->bias.poweron_vsn_delay = of_property_count_u32_elems(
			utils->data, "qcom,mdss-panel-poweron-vsn-sleep");
		pinfo->bias.poweroff_vsp_delay = of_property_count_u32_elems(
			utils->data, "qcom,mdss-panel-poweroff-vsp-sleep");
		pinfo->bias.poweroff_vsn_delay = of_property_count_u32_elems(
			utils->data, "qcom,mdss-panel-poweroff-vsn-sleep");
	}
}

void panel_bias_gpio_config(struct panel_info *pinfo)
{
	int rc = 0;
	struct lcd_bias_info *bias = NULL;

	if (!pinfo) {
		LCD_ERR("pinfo is null!\n");
		return;
	}
	bias = &pinfo->bias;
	if (gpio_is_valid(bias->enable_gpio)) {
		rc = gpio_request(bias->enable_gpio, "enable_gpio");
		if (rc) {
			LCD_ERR("request for bias enable_gpio failed, rc=%d\n", rc);
			return;
		}
	}

	if (gpio_is_valid(bias->vsp_gpio)) {
		rc = gpio_request(bias->vsp_gpio, "vsp_gpio");
		if (rc) {
			LCD_ERR("request for vsp_gpio failed, rc=%d\n", rc);
			return;
		}
	}
	if (gpio_is_valid(bias->vsn_gpio)) {
		rc = gpio_request(bias->vsn_gpio, "vsn_gpio");
		if (rc) {
			LCD_ERR("request for vsn_gpio failed, rc=%d\n", rc);
			return;
		}
	}

	bias->enabled = true;
	LCD_INFO("panel bias config succ\n");
	return;
}

void panel_bias_enable(struct panel_info *pinfo, bool enable)
{
	if (!pinfo) {
		LCD_ERR("pinfo is null!\n");
		return;
	}

	if (enable) {
		gpio_direction_output(pinfo->bias.enable_gpio, 1);
		gpio_set_value(pinfo->bias.enable_gpio, 1);
	} else {
		gpio_set_value(pinfo->bias.enable_gpio, 0);
	}
}

static void panel_vsp_enable(struct panel_info *pinfo, bool enable)
{
	if (!pinfo) {
		LCD_ERR("pinfo is null!\n");
		return;
	}

	if (enable) {
		gpio_direction_output(pinfo->bias.vsp_gpio, 1);
		gpio_set_value(pinfo->bias.vsp_gpio, 1);
		msleep(pinfo->bias.poweron_vsp_delay);
	} else {
		gpio_set_value(pinfo->bias.vsp_gpio, 0);
		msleep(pinfo->bias.poweroff_vsp_delay);
	}
}

static void panel_vsn_enable(struct panel_info *pinfo, bool enable)
{
	if (!pinfo) {
		LCD_ERR("pinfo is null!\n");
		return;
	}

	if (enable) {
		gpio_direction_output(pinfo->bias.vsn_gpio, 1);
		gpio_set_value(pinfo->bias.vsn_gpio, 1);
		msleep(pinfo->bias.poweron_vsn_delay);
	} else {
		gpio_set_value(pinfo->bias.vsn_gpio, 0);
		msleep(pinfo->bias.poweroff_vsn_delay);
	}
}

static int panel_bias_ctrl(struct dsi_panel *panel, int type)
{
	int ret = LCD_OK;
	struct lcd_kit_bias_ops *bias_ops = lcd_kit_get_bias_ops();
	struct dsi_regulator_info *power_info = &panel->power_info;
	int vsn;
	int vsp;
	int i;

	if (!power_info || !bias_ops) {
		LCD_ERR("panel_info is null\n");
		return -EINVAL;
	}

	switch (type) {
	case PANEL_SET_BISA_VOL:
		for (i = 0; i < power_info->count; i++) {
			if (!strncmp(power_info->vregs[i].vreg_name, "vsp",
				strlen("vsp")))
				vsp = power_info->vregs[i].max_voltage;
			else if (!strncmp(power_info->vregs[i].vreg_name, "vsn",
				strlen("vsn")))
				vsn = power_info->vregs[i].max_voltage;
		}

		if (bias_ops->set_bias_voltage)
			ret = bias_ops->set_bias_voltage(vsp, vsn);
		break;
	case PANEL_SET_BISA_POWER_DOWN:
		for (i = 0; i < power_info->count; i++) {
			if (!strncmp(power_info->vregs[i].vreg_name, "vsp",
				strlen("vsp")))
				vsp = power_info->vregs[i].off_min_voltage;
			else if (!strncmp(power_info->vregs[i].vreg_name, "vsn",
				strlen("vsn")))
				vsn = power_info->vregs[i].off_min_voltage;
		}

		if (bias_ops->set_bias_voltage)
			ret = bias_ops->set_bias_power_down(vsp, vsn);
		break;
	default:
		LCD_ERR("not support type\n");
		ret = -EINVAL;
		break;
	}

	return ret;
}

void panel_bias_on(struct dsi_panel *panel)
{
	int ret;

	if (!panel || !panel->pdata) {
		LCD_ERR("Invalid parameters!\n");
		return;
	}
	LCD_INFO("panel_bias_on\n");
	ret = panel_bias_ctrl(panel, PANEL_SET_BISA_VOL);
	if (ret)
		LCD_ERR("Warning: bias voltage set fail, user default!!\n");
	panel_vsp_enable(panel->pdata->pinfo, 1);
	panel_vsn_enable(panel->pdata->pinfo, 1);
}

void panel_bias_off(struct dsi_panel *panel)
{
	int ret;

	if (!panel || !panel->pdata) {
		LCD_ERR("Invalid parameters!\n");
		return;
	}
	LCD_INFO("panel_bias_off\n");
	ret = panel_bias_ctrl(panel, PANEL_SET_BISA_POWER_DOWN);
	if (ret)
		LCD_ERR("Warning: bias voltage set fail, user default!!\n");
	panel_vsn_enable(panel->pdata->pinfo, 0);
	panel_vsp_enable(panel->pdata->pinfo, 0);
}

int lcd_kit_bias_register(struct lcd_kit_bias_ops *ops)
{
	if (g_bias_ops) {
		LCD_ERR("g_bias_ops has already been registered!\n");
		return LCD_FAIL;
	}
	g_bias_ops = ops;
	LCD_INFO("g_bias_ops register success!\n");
	return LCD_OK;
}

int lcd_kit_bias_unregister(struct lcd_kit_bias_ops *ops)
{
	if (g_bias_ops == ops) {
		g_bias_ops = NULL;
		LCD_INFO("g_bias_ops unregister success!\n");
		return LCD_OK;
	}
	LCD_ERR("g_bias_ops unregister fail!\n");
	return LCD_FAIL;
}

struct lcd_kit_bias_ops *lcd_kit_get_bias_ops(void)
{
	return g_bias_ops;
}

