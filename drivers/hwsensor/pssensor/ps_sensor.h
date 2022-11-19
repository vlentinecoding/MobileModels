/*
 * ps_sensor.h
 *
 * ps sensor driver API
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#ifndef PS_SENSOR_H
#define PS_SENSOR_H

#include <../apsensor_channel/ap_sensor.h>
#include <../apsensor_channel/ap_sensor_route.h>

int thp_prox_event_report(int report_value[], unsigned int len);

#endif /* PS_SENSOR_H */