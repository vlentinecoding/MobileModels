// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/of.h>
#include <linux/of_gpio.h>
#include <cam_sensor_cmn_header.h>
#include <cam_sensor_util.h>
#include <cam_sensor_io.h>
#include <cam_req_mgr_util.h>

#include "cam_ois_soc.h"
#include "cam_debug_util.h"

#define CMD_IO_ADR_ACCESS 0xC000 // IO Write Access
#define CMD_IO_DAT_ACCESS 0xD000 // IO Read Access

void RamWrite32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t data, uint32_t delay)
{
	struct cam_sensor_i2c_reg_setting i2c_reg_settings = {0};
	struct cam_sensor_i2c_reg_array i2c_reg_array = {0};
	int32_t rc = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return;
	}

	i2c_reg_settings.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	i2c_reg_settings.data_type = CAMERA_SENSOR_I2C_TYPE_DWORD;
	i2c_reg_settings.size = 1;
	i2c_reg_array.reg_addr = addr;
	i2c_reg_array.reg_data = data;
	i2c_reg_array.delay = delay;
	i2c_reg_settings.reg_setting = &i2c_reg_array;

	rc = camera_io_dev_write(&o_ctrl->io_master_info,
		&i2c_reg_settings);
	if (rc < 0) {
		CAM_ERR(CAM_OIS, "%s : write addr 0x%04x data 0x%x failed rc %d",
			o_ctrl->ois_name, addr, data, rc);
	}
	CAM_DBG(CAM_OIS, "[OISDEBUG] write addr 0x%08x = 0x%08x", addr, data);
}

void RamRead32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t* data)
{
	uint32_t read_val = 0;
	int32_t rc = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return;
	}

	rc = camera_io_dev_read(&(o_ctrl->io_master_info),
		addr, &read_val,
		CAMERA_SENSOR_I2C_TYPE_WORD, CAMERA_SENSOR_I2C_TYPE_DWORD);
	if (rc > 0 && data) {
		*data = read_val;
	} else {
		CAM_ERR(CAM_OIS, "%s : read fail at addr 0x%x, data = 0x%x, rc = %d",
			o_ctrl->ois_name, addr, *data, rc);
	}
	CAM_DBG(CAM_OIS, "[OISDEBUG] read addr 0x%04x = 0x%08x", addr, *data);
}

void CntWrt(struct cam_ois_ctrl_t *o_ctrl, uint8_t *data, uint32_t length, uint32_t delay)
{
	static struct cam_sensor_i2c_reg_array w_data[256] = { {0} };
	struct cam_sensor_i2c_reg_setting write_setting;
	uint32_t i = 0;
	uint32_t addr = 0;
	int32_t rc = 0;
	if (!data || !o_ctrl || (length < 3)) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return;
	}

	addr = (((uint16_t)(data[0]) << 8) | data[1]) & 0xFFFF;
	for (i = 0;i < (length -2) && i < 256; i++) {
		w_data[i].reg_addr = addr;
		w_data[i].reg_data = data[i+2];
		w_data[i].delay = 0;
		w_data[i].data_mask = 0;
	}
	write_setting.size = length - 2;
	write_setting.addr_type = CAMERA_SENSOR_I2C_TYPE_WORD;
	write_setting.data_type = CAMERA_SENSOR_I2C_TYPE_BYTE;
	write_setting.delay = delay;
	write_setting.reg_setting = w_data;

	rc = camera_io_dev_write_continuous(&(o_ctrl->io_master_info),
		&write_setting, 1);
	if (rc < 0) {
		CAM_ERR(CAM_OIS, "OIS CntWrt write failed %d", rc);
	}
	for (i = 0; i < (length -2) && i < 256; i+=4) {
		CAM_DBG(CAM_OIS, "[OISDEBUG] write addr 0x%04x = 0x%02x 0x%02x 0x%02x 0x%02x",
			addr, data[i+2], data[i+3], data[i+4], data[i+5]);
	}
}

void CntRd(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint8_t *data, uint32_t length)
{
	int32_t rc = 0;
	int32_t i = 0;
	rc = camera_io_dev_read_seq(&o_ctrl->io_master_info,
		addr, data,
		CAMERA_SENSOR_I2C_TYPE_WORD,
		CAMERA_SENSOR_I2C_TYPE_BYTE,
		length);
	for (i = 0; i < length; i++) {
		CAM_DBG(CAM_OIS, "[OISDEBUG] read addr 0x%04x[%d] = 0x%02x", addr, i, data[i]);
	}
	if (rc) {
		CAM_ERR(CAM_EEPROM, "read failed rc %d",
			rc);
	}
}

void IOWrite32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t data, uint32_t delay)
{
	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return;
	}

	RamWrite32A(o_ctrl, CMD_IO_ADR_ACCESS, addr, 0);
	RamWrite32A(o_ctrl, CMD_IO_DAT_ACCESS, data, delay);
}

void IORead32A(struct cam_ois_ctrl_t *o_ctrl, uint32_t addr, uint32_t* data)
{
	uint32_t read_val = 0;
	int32_t rc = 0;

	if (!o_ctrl) {
		CAM_ERR(CAM_OIS, "Invalid Args");
		return;
	}

	RamWrite32A(o_ctrl, CMD_IO_ADR_ACCESS, addr, 0);
	rc = camera_io_dev_read(&(o_ctrl->io_master_info),
		CMD_IO_DAT_ACCESS, &read_val,
		CAMERA_SENSOR_I2C_TYPE_WORD, CAMERA_SENSOR_I2C_TYPE_DWORD);
	if (rc > 0 && data) {
		*data = read_val;
	} else {
		CAM_ERR(CAM_OIS, "%s : read fail at addr 0x%x, data = 0x%x, rc = %d",
			o_ctrl->ois_name, addr, *data, rc);
	}
	CAM_DBG(CAM_OIS, "[OISDEBUG] read addr 0x%04x = 0x%08x", addr, *data);
}