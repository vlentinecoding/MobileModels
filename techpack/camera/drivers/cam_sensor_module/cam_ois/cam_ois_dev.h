/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 */
#ifndef _CAM_OIS_DEV_H_
#define _CAM_OIS_DEV_H_

#include <linux/i2c.h>
#include <linux/gpio.h>
#include <media/v4l2-event.h>
#include <media/v4l2-subdev.h>
#include <media/v4l2-ioctl.h>
#include <media/cam_sensor.h>
#include <cam_sensor_i2c.h>
#include <cam_sensor_spi.h>
#include <cam_sensor_io.h>
#include <cam_cci_dev.h>
#include <cam_req_mgr_util.h>
#include <cam_req_mgr_interface.h>
#include <cam_mem_mgr.h>
#include <cam_subdev.h>
#include "cam_soc_util.h"
#include "cam_context.h"

#define DEFINE_MSM_MUTEX(mutexname) \
	static struct mutex mutexname = __MUTEX_INITIALIZER(mutexname)

enum cam_ois_state {
	CAM_OIS_INIT,
	CAM_OIS_ACQUIRE,
	CAM_OIS_CONFIG,
	CAM_OIS_START,
};

/**
 * struct cam_ois_registered_driver_t - registered driver info
 * @platform_driver      :   flag indicates if platform driver is registered
 * @i2c_driver           :   flag indicates if i2c driver is registered
 *
 */
struct cam_ois_registered_driver_t {
	bool platform_driver;
	bool i2c_driver;
};

/**
 * struct cam_ois_i2c_info_t - I2C info
 * @slave_addr      :   slave address
 * @i2c_freq_mode   :   i2c frequency mode
 *
 */
struct cam_ois_i2c_info_t {
	uint16_t slave_addr;
	uint8_t i2c_freq_mode;
};

/**
 * struct cam_ois_soc_private - ois soc private data structure
 * @ois_name        :   ois name
 * @i2c_info        :   i2c info structure
 * @power_info      :   ois power info
 *
 */
struct cam_ois_soc_private {
	const char *ois_name;
	struct cam_ois_i2c_info_t i2c_info;
	struct cam_sensor_power_ctrl_t power_info;
};

/**
 * struct cam_ois_intf_params - bridge interface params
 * @device_hdl   : Device Handle
 * @session_hdl  : Session Handle
 * @ops          : KMD operations
 * @crm_cb       : Callback API pointers
 */
struct cam_ois_intf_params {
	int32_t device_hdl;
	int32_t session_hdl;
	int32_t link_hdl;
	struct cam_req_mgr_kmd_ops ops;
	struct cam_req_mgr_crm_cb *crm_cb;
};

/**
 * struct ois_calib_data - calib data
 * @gyro_gain         : gyro gain x/y
 * @mag_check_data    : otp_x/y, srv_x/y
 * @gyro_offset_data  : gyro offset x/y/z
 * @calib_ctrl_flag   : calib ctrl flag
 */
struct ois_calib_data {
	uint32_t gyro_gain[2];
	uint32_t mag_check_data[4];
	unsigned short gyro_offset_data[3];
	int32_t gyro_offset_result;
	uint8_t calib_ctrl_flag;
};

enum ois_mode {
	OIS_CMD_DISABLE = 0,
	OIS_CMD_ENABLE,
	OIS_CMD_MAG_CHECK,
	OIS_CMD_GRYO_OFFSET_CALIB,
	OIS_CMD_HALL_CHECK,
	OIS_CMD_SET_GRYO_GAIN,
	OIS_CMD_MAX,
};

/**
 * struct ois_ops_table - extend function
 * @set_gyro_gain     : set gyro gain
 * @get_gyro_gain     : get gyro gain
 * @set_ois_fac_mode  : set ois factory mode
 * @set_ctrl_flag     : set calib ctrl flag
 */
struct ois_ops_table {
	int (*set_gyro_gain) (void *o_ctrl, unsigned int gain_x, unsigned int gain_y);
	int (*get_gyro_gain) (void *o_ctrl);
	int (*set_ois_fac_mode) (void *o_ctrl, int32_t mode);
	int (*set_ctrl_flag) (void *o_ctrl, int32_t flag);
	int (*set_calib_data) (void *o_ctrl);
};

/**
 * struct cam_ois_ctrl_t - OIS ctrl private data
 * @device_name     :   ois device_name
 * @pdev            :   platform device
 * @ois_mutex       :   ois mutex
 * @soc_info        :   ois soc related info
 * @io_master_info  :   Information about the communication master
 * @cci_i2c_master  :   I2C structure
 * @v4l2_dev_str    :   V4L2 device structure
 * @bridge_intf     :   bridge interface params
 * @i2c_init_data   :   ois i2c init settings
 * @i2c_mode_data   :   ois i2c mode settings
 * @i2c_time_data   :   ois i2c time write settings
 * @i2c_calib_data  :   ois i2c calib settings
 * @ois_device_type :   ois device type
 * @cam_ois_state   :   ois_device_state
 * @ois_fw_flag     :   flag for firmware download
 * @is_ois_calib    :   flag for Calibration data
 * @module_code_data:   module_code_data
 * @opcode          :   ois opcode
 * @device_name     :   Device name
 *
 */
struct cam_ois_ctrl_t {
	char device_name[CAM_CTX_DEV_NAME_MAX_LENGTH];
	struct platform_device *pdev;
	struct mutex ois_mutex;
	struct cam_hw_soc_info soc_info;
	struct camera_io_master io_master_info;
	enum cci_i2c_master_t cci_i2c_master;
	enum cci_device_num cci_num;
	struct cam_subdev v4l2_dev_str;
	struct cam_ois_intf_params bridge_intf;
	struct i2c_settings_array i2c_init_data;
	struct i2c_settings_array i2c_calib_data;
	struct i2c_settings_array i2c_mode_data;
	struct i2c_settings_array i2c_time_data;
	enum msm_camera_device_type_t ois_device_type;
	enum cam_ois_state cam_ois_state;
	char ois_name[32];
	uint8_t ois_fw_flag;
	uint8_t is_ois_calib;
	uint8_t module_code_data;
	struct cam_ois_opcode opcode;
	struct ois_calib_data calib_data;
	struct ois_ops_table const *ops;
};

/**
 * @brief : API to register OIS hw to platform framework.
 * @return struct platform_device pointer on on success, or ERR_PTR() on error.
 */
int cam_ois_driver_init(void);

/**
 * @brief : API to remove OIS Hw from platform framework.
 */
void cam_ois_driver_exit(void);
#endif /*_CAM_OIS_DEV_H_ */
