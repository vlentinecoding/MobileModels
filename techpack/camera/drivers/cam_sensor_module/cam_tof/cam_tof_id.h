/*
 * cam_tof_id.h
 *
 * get tof TX id
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

#ifndef _CAM_TOF_ID_H_
#define _CAM_TOF_ID_H_
#include "cam_sensor_dev.h"

#define loge_if(x) \
	do { \
		int ret = (x); \
		if (ret) \
			CAM_ERR(CAM_SENSOR, "'%s' failed, ret = %d", #x, ret); \
	} while (0)

#define return_if_null(x) \
	do { \
		if (!x) { \
			CAM_ERR(CAM_SENSOR, "invalid params, %s", #x); \
			return; \
		} \
	} while (0)

#define return_error_if_null(x) \
	do { \
		if (!x) { \
			CAM_ERR(CAM_SENSOR, "invalid params, %s", #x); \
			return -EINVAL; \
		} \
	} while (0)

int get_tof_tx_id(struct cam_sensor_ctrl_t *s_ctrl);

#endif