/*
 * camkit_check_cable.c
 *
 * Check the camera btb cable status.
 *
 * Copyright (c) 2021-2021 Honor Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 1, as published by the Free Software Foundation, and
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
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/pinctrl/consumer.h>
#include <linux/device.h>
#include <cam_sensor_cmn_header.h>

#include "securec.h"
#include "cam_check_cable.h"
#include "../cam_sensor_dmd/cam_dmd_util.h"

#define MAX_BTB_CHECK_GPIO_PER_CAM 4

static unsigned int \
	gpio_list[MAX_CAMERAS][MAX_BTB_CHECK_GPIO_PER_CAM];

static void cam_btb_gpio_pullup(struct msm_pinctrl_info* g_btb_pctrl)
{
	int ret = 0;

	if (!g_btb_pctrl) {
		CAM_ERR(CAM_SENSOR, "msm_pinctrl_info is NULL");
		return;
	}

	if (!g_btb_pctrl->pinctrl || !g_btb_pctrl->gpio_state_active) {
		CAM_ERR(CAM_SENSOR, "cannot set pin to active state");
		return;
	}

	ret = pinctrl_select_state(
		g_btb_pctrl->pinctrl,
		g_btb_pctrl->gpio_state_active);
	if (ret)
		CAM_ERR(CAM_SENSOR, "cannot set pin to active state");
}

static void cam_btb_gpio_pulldown(struct msm_pinctrl_info* g_btb_pctrl)
{
	int ret = 0;

	if (!g_btb_pctrl) {
		CAM_ERR(CAM_SENSOR, "msm_pinctrl_info is NULL");
		return;
	}

	if (!g_btb_pctrl->pinctrl || !g_btb_pctrl->gpio_state_suspend) {
		CAM_ERR(CAM_SENSOR, "cannot set pin to suspend state");
		return;
	}

	ret = pinctrl_select_state(
		g_btb_pctrl->pinctrl,
		g_btb_pctrl->gpio_state_suspend);
	if (ret)
		CAM_ERR(CAM_SENSOR, "cannot set pin to suspend state");
}

static int btb_pinctrl_init(
	struct msm_pinctrl_info *btb_pctrl, struct device *dev)
{

	if (IS_ERR_OR_NULL(dev)) {
		CAM_ERR(CAM_SENSOR, "device NULL");
		return -EINVAL;
	}

	if (!btb_pctrl) {
		CAM_ERR(CAM_SENSOR, "msm_pinctrl_info is NULL");
		return -EINVAL;
	}

	btb_pctrl->pinctrl = devm_pinctrl_get(dev);
	if (IS_ERR_OR_NULL(btb_pctrl->pinctrl)) {
		CAM_ERR(CAM_SENSOR, "Getting pinctrl handle failed");
		return -EINVAL;
	}
	btb_pctrl->gpio_state_active =
		pinctrl_lookup_state(btb_pctrl->pinctrl,
				"cam_btb_default");
	if (IS_ERR_OR_NULL(btb_pctrl->gpio_state_active)) {
		CAM_ERR(CAM_SENSOR,
			"Failed to get the active state pinctrl handle");
		return -EINVAL;
	}
	btb_pctrl->gpio_state_suspend
		= pinctrl_lookup_state(btb_pctrl->pinctrl,
				"cam_btb_suspend");
	if (IS_ERR_OR_NULL(btb_pctrl->gpio_state_suspend)) {
		CAM_ERR(CAM_SENSOR,
			"Failed to get the suspend state pinctrl handle");
		return -EINVAL;
	}

	return 0;
}


static int get_btb_check_gpio_info_from_dts(struct cam_sensor_ctrl_t *s_ctrl)
{
	int ret = 0;
	int i = 0;
	int gpio_count = 0;
	int gpio_list_lens = 0;

	if (!s_ctrl) {
		CAM_ERR(CAM_SENSOR, "sensor_ctrl is NULL");
		return -EINVAL;
	}

	if (!s_ctrl->of_node) {
		CAM_ERR(CAM_SENSOR, "device node NULL");
		return -EINVAL;
	}

	/* if gpio_list is not 0, no need get, skip */
	if (gpio_list[s_ctrl->id][0]) {
		return 0;
	}

	gpio_count = of_gpio_named_count(s_ctrl->of_node, "cam-btb-gpios");
	if (gpio_count > MAX_BTB_CHECK_GPIO_PER_CAM) {
		CAM_ERR(CAM_SENSOR, "cam %d get gpio_count max!", s_ctrl->id);
		gpio_count = MAX_BTB_CHECK_GPIO_PER_CAM;
	}

	for (i = 0; i < gpio_count; i++) {
		ret = of_get_named_gpio(s_ctrl->of_node, "cam-btb-gpios", i);

		if (ret > 0) {
			gpio_list[s_ctrl->id][i] = ret;
			CAM_INFO(CAM_SENSOR,"cam %d gpio[%d] is %d", s_ctrl->id,
				i, gpio_list[s_ctrl->id][i]);
			gpio_list_lens++;
		}
	}

	if (!gpio_list_lens) {
		return -EINVAL;
	}

	return 0;
}

static int check_cable(unsigned int cable_gpio)
{
	int ret;

	ret = gpio_request(cable_gpio, NULL);
	if (ret) {
		CAM_ERR(CAM_SENSOR, "cable gpio gpio_request fail");
	}
	ret = gpio_get_value(cable_gpio);

	gpio_free(cable_gpio);

	CAM_INFO(CAM_SENSOR, "cable gpio %u ret=%d", cable_gpio, ret);

	return ret;
}

void check_camera_btb_gpio_info(struct cam_sensor_ctrl_t *s_ctrl)
{

	int gpio_value[MAX_BTB_CHECK_GPIO_PER_CAM] = {0};
	struct msm_pinctrl_info g_btb_pctrl = {0};
	int i = 0;

	if (!s_ctrl) {
		CAM_ERR(CAM_SENSOR, "sensor_ctrl NULL");
		return;
	}

	if (get_btb_check_gpio_info_from_dts(s_ctrl)) {
		CAM_INFO(CAM_SENSOR, "cam %d get btb_info failed", s_ctrl->id);
		return;
	}

	if (btb_pinctrl_init(&g_btb_pctrl, s_ctrl->soc_info.dev)) {
		CAM_ERR(CAM_SENSOR, "Initialization of btb_pinctrl failed");
		return;
	}

	cam_btb_gpio_pullup(&g_btb_pctrl);
	for (i = 0; i < MAX_BTB_CHECK_GPIO_PER_CAM; i++) {
		if (gpio_list[s_ctrl->id][i]) {
			gpio_value[i] = check_cable(gpio_list[s_ctrl->id][i]);
		}
	}

	cam_btb_gpio_pulldown(&g_btb_pctrl);
	for (i = 0; i < MAX_BTB_CHECK_GPIO_PER_CAM; i++) {
		if (gpio_list[s_ctrl->id][i]) {
			if (gpio_value[i] != check_cable(gpio_list[s_ctrl->id][i])) {
				camkit_hiview_report_id(DSM_CAMERA_I2C_ERR,
					s_ctrl->sensordata->slave_info.sensor_id,
					BTB_GPIO_CHECK_FAILED);
				pr_err("cam%d(sensor:0x%x,gpio:%d) check failed",
					s_ctrl->id, s_ctrl->sensordata->slave_info.sensor_id,
					gpio_list[s_ctrl->id][i]);
			}
		}
	}
}

