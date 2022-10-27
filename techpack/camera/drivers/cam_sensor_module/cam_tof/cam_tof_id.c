/*
 * cam_tof_id.c
 *
 * get tof TX id
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
#include "cam_tof_id.h"

#define MAX_TOF_IDPIN_NUM 1
#define TX_ID_NUM 3
#define TX_IDPIN_STATE_NUM (TX_ID_NUM * 2)
static unsigned int g_tof_idpin;
static unsigned int tx_idpin_state_info[TX_IDPIN_STATE_NUM];
static unsigned int tx_id[TX_ID_NUM];
static g_tof_idpin_state_count;

static void cam_tof_idpin_pull_up(struct msm_pinctrl_info *idpin)
{
	int ret;
	return_if_null(idpin);
	return_if_null(idpin->pinctrl);
	return_if_null(idpin->gpio_state_active);

	ret = pinctrl_select_state(idpin->pinctrl,
		idpin->gpio_state_active);
	if (ret)
		CAM_ERR(CAM_SENSOR, "cannot set pin to active state");
}

static void cam_tof_idpin_pull_down(struct msm_pinctrl_info* idpin)
{
	int ret;
	return_if_null(idpin);
	return_if_null(idpin->pinctrl);
	return_if_null(idpin->gpio_state_suspend);

	ret = pinctrl_select_state(idpin->pinctrl,
		idpin->gpio_state_suspend);
	if (ret)
		CAM_ERR(CAM_SENSOR, "cannot set pin to suspend state");
}

static int idpin_pinctrl_init(struct msm_pinctrl_info *idpin_pctrl,
	struct device *dev)
{
	return_error_if_null(dev);
	return_error_if_null(idpin_pctrl);

	idpin_pctrl->pinctrl = devm_pinctrl_get(dev);
	return_error_if_null(idpin_pctrl->pinctrl);

	idpin_pctrl->gpio_state_active = pinctrl_lookup_state(idpin_pctrl->pinctrl,
		"tof_idpin_default");
	return_error_if_null(idpin_pctrl->gpio_state_active);

	idpin_pctrl->gpio_state_suspend = pinctrl_lookup_state(idpin_pctrl->pinctrl,
		"tof_idpin_suspend");
	return_error_if_null(idpin_pctrl->gpio_state_suspend);

	return 0;
}

static int get_cam_tof_idpin_info(struct cam_sensor_ctrl_t *s_ctrl)
{
	int idpin;
	int i;
	int gpio_count;
	int id_count;
	int ret;

	return_error_if_null(s_ctrl);
	return_error_if_null(s_ctrl->of_node);

	gpio_count = of_gpio_named_count(s_ctrl->of_node, "cam-tof-idpin");
	if (gpio_count > MAX_TOF_IDPIN_NUM) {
		CAM_ERR(CAM_SENSOR, "tof %d get gpio_count max!", s_ctrl->id);
		gpio_count = MAX_TOF_IDPIN_NUM;
	}

	for (i = 0; i < gpio_count; i++) {
		idpin = of_get_named_gpio(s_ctrl->of_node, "cam-tof-idpin", i);
		if (!gpio_is_valid(idpin)) {
			CAM_ERR(CAM_SENSOR, "tof %d idpin is invalid!", s_ctrl->id);
			return -EINVAL;
		}

		g_tof_idpin = idpin;
		CAM_INFO(CAM_SENSOR,"tof %d idpin is %d", s_ctrl->id, g_tof_idpin);
	}

	id_count = of_property_count_elems_of_size(s_ctrl->of_node, "cam-tof-id", sizeof(u32));
	if (id_count <= 0 || id_count > TX_ID_NUM) {
		CAM_ERR(CAM_SENSOR, "id_count = %d out of range", id_count);
		return -1;
	}
	ret = of_property_read_u32_array(s_ctrl->of_node, "cam-tof-id", tx_id, id_count);
	if (ret < 0) {
		CAM_ERR(CAM_SENSOR, "get tof id array fail", s_ctrl->id);
		return ret;
	}

	g_tof_idpin_state_count = of_property_count_elems_of_size(s_ctrl->of_node,
		"cam-tof-idpin-state", sizeof(u32));
	if (g_tof_idpin_state_count <= 0 || g_tof_idpin_state_count > TX_IDPIN_STATE_NUM) {
		CAM_ERR(CAM_SENSOR, "g_tof_idpin_state_count = %d out of range", g_tof_idpin_state_count);
		return -1;
	}
	ret = of_property_read_u32_array(s_ctrl->of_node, "cam-tof-idpin-state",
		tx_idpin_state_info, g_tof_idpin_state_count);
	if (ret < 0) {
		CAM_ERR(CAM_SENSOR, "get tof id array fail", s_ctrl->id);
		return ret;
	}

	return 0;
}

static int get_tof_tx_idpin_state(unsigned int tof_idpin)
{
	int ret;

	ret = gpio_request(tof_idpin, NULL);
	if (ret)
		CAM_ERR(CAM_SENSOR, "tof idpin request fail");

	ret = gpio_get_value(tof_idpin);
	gpio_free(tof_idpin);
	CAM_INFO(CAM_SENSOR, "tof_idpin %u, state = %d", tof_idpin, ret);

	return ret;
}

int get_tof_tx_id(struct cam_sensor_ctrl_t *s_ctrl)
{
	struct msm_pinctrl_info idpin_pctrl = {0};
	unsigned int val_poll_up;
	unsigned int val_poll_down;
	int i;
	int j;

	return_error_if_null(s_ctrl);
	if (get_cam_tof_idpin_info(s_ctrl)) {
		CAM_INFO(CAM_SENSOR, "tof %d get idpin info failed", s_ctrl->id);
		return -1;
	}
	if (idpin_pinctrl_init(&idpin_pctrl, s_ctrl->soc_info.dev)) {
		CAM_ERR(CAM_SENSOR, "tof idpin pinctrl init failed");
		return -1;
	}

	cam_tof_idpin_pull_up(&idpin_pctrl);
	usleep_range(900, 1000); // delay 1ms
	val_poll_up = get_tof_tx_idpin_state(g_tof_idpin);
	cam_tof_idpin_pull_down(&idpin_pctrl);
	usleep_range(900, 1000); // delay 1ms
	val_poll_down = get_tof_tx_idpin_state(g_tof_idpin);
	CAM_INFO(CAM_SENSOR, "tof tx idpin val_poll_up = %u, val_poll_down = %u",
		val_poll_up, val_poll_down);

	for (i = 0, j = 0; (i < (g_tof_idpin_state_count - 1)) &&
		(j < TX_ID_NUM); i += 2, j++) {
		if ((val_poll_up == tx_idpin_state_info[i]) &&
			(val_poll_down == tx_idpin_state_info[i + 1])) {
			CAM_INFO(CAM_SENSOR, "tof tx id = %d", tx_id[j]);
			return tx_id[j];
		}
	}

	return -1;
}
