/*
 * Copyright (C) 2016 MediaTek Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See http://www.gnu.org/licenses/gpl-2.0.html for more details.
 */

#ifndef _HF_SENSOR_TYPE_H_
#define _HF_SENSOR_TYPE_H_

enum {
	/* follow google default sensor type */
	SENSOR_TYPE_ACCELEROMETER = 1,
	SENSOR_TYPE_MAGNETIC_FIELD,
	SENSOR_TYPE_ORIENTATION,
	SENSOR_TYPE_GYROSCOPE,
	SENSOR_TYPE_LIGHT,
	SENSOR_TYPE_PRESSURE,
	SENSOR_TYPE_TEMPERATURE,
	SENSOR_TYPE_PROXIMITY,
	SENSOR_TYPE_GRAVITY,
	SENSOR_TYPE_LINEAR_ACCELERATION,
	SENSOR_TYPE_ROTATION_VECTOR,
	SENSOR_TYPE_RELATIVE_HUMIDITY,
	SENSOR_TYPE_AMBIENT_TEMPERATURE,
	SENSOR_TYPE_MAGNETIC_FIELD_UNCALIBRATED,
	SENSOR_TYPE_GAME_ROTATION_VECTOR,
	SENSOR_TYPE_GYROSCOPE_UNCALIBRATED,
	SENSOR_TYPE_SIGNIFICANT_MOTION,
	SENSOR_TYPE_STEP_DETECTOR,
	SENSOR_TYPE_STEP_COUNTER,
	SENSOR_TYPE_GEOMAGNETIC_ROTATION_VECTOR,
	SENSOR_TYPE_HEART_RATE,
	SENSOR_TYPE_TILT_DETECTOR,
	SENSOR_TYPE_WAKE_GESTURE,
	SENSOR_TYPE_GLANCE_GESTURE,
	SENSOR_TYPE_PICK_UP_GESTURE,
	SENSOR_TYPE_WRIST_TILT_GESTURE,
	SENSOR_TYPE_DEVICE_ORIENTATION,
	SENSOR_TYPE_POSE_6DOF,
	SENSOR_TYPE_STATIONARY_DETECT,
	SENSOR_TYPE_MOTION_DETECT,
	SENSOR_TYPE_HEART_BEAT,
	SENSOR_TYPE_DYNAMIC_SENSOR_META,
	SENSOR_TYPE_ADDITIONAL_INFO,
	SENSOR_TYPE_LOW_LATENCY_OFFBODY_DETECT,
	SENSOR_TYPE_ACCELEROMETER_UNCALIBRATED,

	/* follow mtk add sensor type */
	SENSOR_TYPE_PEDOMETER = 55,
	SENSOR_TYPE_IN_POCKET,
	SENSOR_TYPE_ACTIVITY,
	SENSOR_TYPE_PDR,
	SENSOR_TYPE_FREEFALL,
	SENSOR_TYPE_FLAT,
	SENSOR_TYPE_FACE_DOWN,
	SENSOR_TYPE_SHAKE,
	SENSOR_TYPE_BRINGTOSEE,
	SENSOR_TYPE_ANSWER_CALL,
	SENSOR_TYPE_GEOFENCE,
	SENSOR_TYPE_FLOOR_COUNTER,
	SENSOR_TYPE_EKG,
	SENSOR_TYPE_PPG1,
	SENSOR_TYPE_PPG2,
	SENSOR_TYPE_RGBW,
	SENSOR_TYPE_GYRO_TEMPERATURE,
	SENSOR_TYPE_SAR,
	SENSOR_TYPE_OIS,
	SENSOR_TYPE_GYRO_SECONDARY,
	SENSOR_TYPE_HW_MOTION,
	SENSOR_TYPE_FINGER_SENSE,
	SENSOR_TYPE_AOD,
	SENSOR_TYPE_RPC_MOTION,
	SENSOR_TYPE_SENSOR_MAX,
};

enum {
	ID_OFFSET = 1,

	/* follow google default sensor type */
	ID_ACCELEROMETER = 0,
	ID_MAGNETIC_FIELD,
	ID_ORIENTATION,
	ID_GYROSCOPE,
	ID_LIGHT,
	ID_PRESSURE,
	ID_TEMPERATURE,
	ID_PROXIMITY,
	ID_GRAVITY,
	ID_LINEAR_ACCELERATION,
	ID_ROTATION_VECTOR,
	ID_RELATIVE_HUMIDITY,
	ID_AMBIENT_TEMPERATURE,
	ID_MAGNETIC_FIELD_UNCALIBRATED,
	ID_GAME_ROTATION_VECTOR,
	ID_GYROSCOPE_UNCALIBRATED,
	ID_SIGNIFICANT_MOTION,
	ID_STEP_DETECTOR,
	ID_STEP_COUNTER,
	ID_GEOMAGNETIC_ROTATION_VECTOR,
	ID_HEART_RATE,
	ID_TILT_DETECTOR,
	ID_WAKE_GESTURE,
	ID_GLANCE_GESTURE,
	ID_PICK_UP_GESTURE,
	ID_WRIST_TILT_GESTURE,
	ID_DEVICE_ORIENTATION,
	ID_POSE_6DOF,
	ID_STATIONARY_DETECT,
	ID_MOTION_DETECT,
	ID_HEART_BEAT,
	ID_DYNAMIC_SENSOR_META,
	ID_ADDITIONAL_INFO,
	ID_LOW_LATENCY_OFFBODY_DETECT,
	ID_ACCELEROMETER_UNCALIBRATED,

	/* follow mtk add sensor type */
	ID_PEDOMETER = SENSOR_TYPE_PEDOMETER - ID_OFFSET,
	ID_IN_POCKET,
	ID_ACTIVITY,
	ID_PDR,
	ID_FREEFALL,
	ID_FLAT,
	ID_FACE_DOWN,
	ID_SHAKE,
	ID_BRINGTOSEE,
	ID_ANSWER_CALL,
	ID_GEOFENCE,
	ID_FLOOR_COUNTER,
	ID_EKG,
	ID_PPG1,
	ID_PPG2,
	ID_RGBW,
	ID_GYRO_TEMPERATURE,
	ID_SAR,
	ID_OIS,
	ID_GYRO_SECONDARY,
	ID_HW_MOTION,
	ID_FINGER_SENSE,
	ID_AOD,
	ID_RPC_MOTION,
	ID_SENSOR_MAX,
};

enum {
	SENSOR_ACCURANCY_UNRELIALE,
	SENSOR_ACCURANCY_LOW,
	SENSOR_ACCURANCY_MEDIUM,
	SENSOR_ACCURANCY_HIGH,
};

#endif
