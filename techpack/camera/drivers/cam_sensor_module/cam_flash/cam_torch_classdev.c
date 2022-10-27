/*
 * cam_torch_classdev.c
 *
 * Copyright (c) 2020-2020 Honor Technologies Co., Ltd.
 *
 * fled regulator interface
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

#include "cam_torch_classdev.h"
#include "cam_flash_dev.h"
#include "cam_res_mgr_api.h"

struct cam_torch_dt_data {
	uint32_t torch_index_array[CAM_FLASH_MAX_LED_TRIGGERS];
	const char *torch_name_array[CAM_FLASH_MAX_LED_TRIGGERS];
	uint32_t torch_num;
};

static int cam_torch_prepare(struct torch_classdev_data *flash_ctrl,
	bool regulator_enable)
{
	int rc = 0;

	if (!(flash_ctrl->switch_trigger)) {
		CAM_ERR(CAM_FLASH, "Invalid argument");
		return -EINVAL;
	}

	CAM_INFO(CAM_FLASH, "regulator_enable=%d", regulator_enable);

	if (regulator_enable &&
		(flash_ctrl->is_regulator_enabled == false)) {
#if IS_REACHABLE(CONFIG_LEDS_QPNP_FLASH_V2)
		rc = qpnp_flash_led_prepare(flash_ctrl->switch_trigger,
			ENABLE_REGULATOR, NULL);
#elif IS_REACHABLE(CONFIG_LEDS_QTI_FLASH)
		rc = qti_flash_led_prepare(flash_ctrl->switch_trigger,
			ENABLE_REGULATOR, NULL);
#endif
		if (rc) {
			CAM_ERR(CAM_FLASH,
				"Regulator enable failed rc = %d", rc);
			return rc;
		}

		flash_ctrl->is_regulator_enabled = true;
	} else if ((!regulator_enable) &&
		(flash_ctrl->is_regulator_enabled == true)) {
#if IS_REACHABLE(CONFIG_LEDS_QPNP_FLASH_V2)
		rc = qpnp_flash_led_prepare(flash_ctrl->switch_trigger,
			DISABLE_REGULATOR, NULL);
#elif IS_REACHABLE(CONFIG_LEDS_QTI_FLASH)
        rc = qti_flash_led_prepare(flash_ctrl->switch_trigger,
			DISABLE_REGULATOR, NULL);
#endif
		if (rc) {
			CAM_ERR(CAM_FLASH,
				"Regulator disable failed rc = %d", rc);
			return rc;
		}

		flash_ctrl->is_regulator_enabled = false;
	} else {
		CAM_ERR(CAM_FLASH, "Wrong Flash State : %d",
			flash_ctrl->is_regulator_enabled);
		rc = -EINVAL;
	}

	return rc;
}

static void cam_torch_set_brightness(struct led_classdev *cdev,
			enum led_brightness value)
{
	struct torch_classdev_data *data = container_of(cdev,
			struct torch_classdev_data, cdev_torch);

	if (!data->torch_trigger) {
		CAM_ERR(CAM_FLASH, "data->torch_trigger null\n");
		return;
	}

	CAM_INFO(CAM_FLASH, "value %d", value);

	mutex_lock(&data->lock);
	cam_res_mgr_led_trigger_event(data->torch_trigger, value);

	if (!data->switch_trigger) {
		CAM_ERR(CAM_FLASH, "No switch_trigger\n");
		mutex_unlock(&data->lock);
		return;
	}

	if (value > 0) {
		cam_torch_prepare(data, true);
		msleep(100);
		CAM_INFO(CAM_FLASH, "wake up");
		cam_res_mgr_led_trigger_event(data->switch_trigger,
			(enum led_brightness)LED_SWITCH_ON);
	}
	else {
		cam_res_mgr_led_trigger_event(data->switch_trigger,
			(enum led_brightness)LED_SWITCH_OFF);
		cam_torch_prepare(data, false);
	}
	mutex_unlock(&data->lock);
}

static int cam_torch_classdev_parse_dt(const struct device_node *of_node,
	struct cam_torch_dt_data *dt_data)
{
	int rc;
	uint32_t torch_num = 0;
	int count;

	CAM_INFO(CAM_FLASH, "Enter\n");
	if (!of_node || !dt_data) {
		CAM_ERR(CAM_FLASH, "Null ptr\n");
		return -EINVAL;
	}

	if (!of_get_property(of_node, "torch-light-num", &torch_num)) {
		CAM_INFO(CAM_FLASH, "No torch\n");
		return -ENODEV;
	}
	torch_num /= sizeof(uint32_t);
	if (torch_num > CAM_FLASH_MAX_LED_TRIGGERS) {
		CAM_ERR(CAM_FLASH, "torch_num is too large %d\n", torch_num);
		return -EINVAL;
	}

	count = of_property_count_strings(of_node, "torch-light-name");
	if (count != torch_num) {
		CAM_ERR(CAM_FLASH, "invalid count %d, torch_num %d\n", count, torch_num);
		return -EINVAL;
	}

	rc = of_property_read_string_array(of_node, "torch-light-name",
		dt_data->torch_name_array, count);
	if (rc < 0) {
		CAM_WARN(CAM_FLASH, "read string failed\n");
		return -ENODEV;
	}

	rc = of_property_read_u32_array(of_node, "torch-light-num",
		dt_data->torch_index_array, torch_num);
	if (rc < 0) {
		CAM_WARN(CAM_FLASH, "read index failed\n");
		return -ENODEV;
	}

	dt_data->torch_num = torch_num;

	CAM_INFO(CAM_FLASH, "torch_num:%d\n", dt_data->torch_num);
	CAM_INFO(CAM_FLASH, "Exit\n");
	return 0;
}

static int cam_torch_classdev_get_data(struct torch_classdev_data *torch_data,
	struct cam_flash_ctrl *fctrl,
	struct cam_torch_dt_data *dt_data,
	unsigned int index)
{
	uint32_t torch_id;

	if (index >= CAM_FLASH_MAX_LED_TRIGGERS) {
		CAM_ERR(CAM_FLASH, "invalid index: %d", index);
		return -EINVAL;
	}

	torch_data->cdev_torch.name = dt_data->torch_name_array[index];
	if (!torch_data->cdev_torch.name) {
		CAM_ERR(CAM_FLASH, "get name failed\n");
		return -ENODEV;
	}

	torch_id = dt_data->torch_index_array[index];
	if (torch_id >= CAM_FLASH_MAX_LED_TRIGGERS) {
		CAM_ERR(CAM_FLASH, "invalid torch_id: %d", torch_id);
		return -EINVAL;
	}

	if (!fctrl->torch_trigger[torch_id]) {
		CAM_ERR(CAM_FLASH, "no trigger\n");
		return -ENODEV;
	}
	torch_data->torch_trigger = fctrl->torch_trigger[torch_id];
	torch_data->cdev_torch.brightness = LED_OFF;
	torch_data->cdev_torch.brightness_set = cam_torch_set_brightness;
	torch_data->switch_trigger = fctrl->switch_trigger;
	torch_data->is_regulator_enabled = false;
	return 0;
}

int cam_torch_classdev_register(struct platform_device *pdev, void *data)
{
	int ret;
	int i;
	struct torch_classdev_data *torch_data = NULL;
	struct cam_flash_ctrl *fctrl = data;
	struct cam_torch_dt_data *dt_data = NULL;

	if (!pdev || !data || !pdev->dev.of_node) {
		CAM_ERR(CAM_FLASH, "Null ptr\n");
		return -EINVAL;
	}

	dt_data = devm_kzalloc(&pdev->dev, sizeof(*dt_data), GFP_KERNEL);
	if (!dt_data) {
		CAM_ERR(CAM_FLASH, "no mem\n");
		return -ENOMEM;
	}
	ret = cam_torch_classdev_parse_dt(pdev->dev.of_node, dt_data);
	if (ret < 0) {
		CAM_ERR(CAM_FLASH, "no dt\n");
		devm_kfree(&pdev->dev, dt_data);
		return -ENODEV;
	}

	for (i = 0; i < dt_data->torch_num; i++) {
		torch_data = devm_kzalloc(&pdev->dev,
			sizeof(*torch_data), GFP_KERNEL);
		if (!torch_data) {
			CAM_ERR(CAM_FLASH, "i = %d, no mem\n", i);
			devm_kfree(&pdev->dev, dt_data);
			return -ENOMEM;
		}

		ret = cam_torch_classdev_get_data(torch_data,
			fctrl, dt_data, i);
		if (ret < 0)
			goto exit;

		torch_data->dev = &pdev->dev;

		CAM_INFO(CAM_FLASH, "register %s\n", torch_data->cdev_torch.name);
		ret = devm_led_classdev_register(&pdev->dev,
			&torch_data->cdev_torch);
		if (ret < 0)
			goto exit;
	}

	devm_kfree(&pdev->dev, dt_data);
	return 0;

exit:
	CAM_ERR(CAM_FLASH, "%d cam torch register failed\n", i);
	devm_kfree(&pdev->dev, torch_data);
	devm_kfree(&pdev->dev, dt_data);
	return ret;
}
EXPORT_SYMBOL(cam_torch_classdev_register);
