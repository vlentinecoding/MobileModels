/**
 * @brief		LC898129 OIS PRODUCT
 *
 * @author		Copyright (C) 2021 ON Semiconductor All Rights Reserved.
 *
 **/

#include <linux/module.h>
#include <linux/firmware.h>
#include <cam_sensor_cmn_header.h>
#include "cam_ois_core.h"
#include "cam_ois_soc.h"
#include "cam_sensor_util.h"
#include "cam_debug_util.h"
#include "cam_res_mgr_api.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include <linux/vmalloc.h>

#include "onsemi_ois_interface.h"

#include "lc898129.h"

unsigned int lc898129_read_ram_cmd(struct cam_ois_ctrl_t *o_ctrl, unsigned short reg)
{
	unsigned int reg_read = 0xFF;

	RamRead32A(o_ctrl, reg, &reg_read);
	CAM_DBG(CAM_OIS, "read 0x%x, 0x%x", reg, reg_read);

	return reg_read;
}

/* used for write ram and then readout */
unsigned int lc898129_write_ram_cmd(struct cam_ois_ctrl_t *o_ctrl, unsigned short const reg,
	uint32_t value)
{
	unsigned int reg_read = 0xFF;

	RamWrite32A(o_ctrl, reg, value, 0);
	CAM_DBG(CAM_OIS, "write 0x%x, 0x%x", reg, value);
	RamRead32A(o_ctrl, reg, &reg_read);
	CAM_DBG(CAM_OIS, "read 0x%x, 0x%x", reg, reg_read);

	return reg_read;
}

/* ois fw version info */
static unsigned int version_info [][3] = {
    /* on Module vendor, Actuator Size,   on vesion number */
	{ 0x07,             0x03,         0x0007200102 }, /* sunny */
};

enum shared_gyro_vendor_t {
	GYRO_VENDOR_INVEN_ICM20690_V,
	GYRO_VENDOR_ST_LSM6DSM_V,
};

struct gyro_map_tab {
	enum shared_gyro_vendor_t vendor;
	unsigned int gyroselect;
};

static struct gyro_map_tab gyrotable[] = {
	{ GYRO_VENDOR_INVEN_ICM20690_V, 0x00 }, /* select 0: icm20690 */
	{ GYRO_VENDOR_ST_LSM6DSM_V, 0x02 }, /* select 2: lsm6dsm */
};

/* get ois fw version */
static unsigned int lc898129_get_oisfw_version(unsigned char on_module_vendor,
	unsigned char actuator_size)
{
	unsigned char i = 0;

	for (i = 0; i < 1; ++i) {
		if (on_module_vendor == version_info[i][0] &&
			actuator_size == version_info[i][1])
			return version_info[i][2];
	}

	return 0xff;
}

static int lc898129_gyro_offset_calib(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t ret = EXE_END;

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	/* impl */
	CAM_INFO(CAM_OIS, "start MeasGyAcOffset");
	ret = MeasGyAcOffset(o_ctrl);

	if (ret == EXE_END) {
		CAM_INFO(CAM_OIS, "MeasGyAcOffset fin ok, ret = 0x%x", ret);
		o_ctrl->calib_data.gyro_offset_result = 0;
		GetGyroOffset(o_ctrl, &o_ctrl->calib_data.gyro_offset_data[0],
			&o_ctrl->calib_data.gyro_offset_data[1], &o_ctrl->calib_data.gyro_offset_data[2]);
	} else {
		CAM_INFO(CAM_OIS, "MeasGyAcOffset fin err, ret = 0x%x", ret);
		o_ctrl->calib_data.gyro_offset_result = -1;
	}

	CAM_INFO(CAM_OIS, "gyro offset X = %u, Y = %u, Z = %u, ret = %d",
		o_ctrl->calib_data.gyro_offset_data[0],
		o_ctrl->calib_data.gyro_offset_data[1],
		o_ctrl->calib_data.gyro_offset_data[2],
		o_ctrl->calib_data.gyro_offset_result);

	return 0;
}

static int lc898129_get_gyro_gain(void *ctrl)
{
	int ret;
	uint32_t UlBuffer[64] = {0};
	struct cam_ois_ctrl_t *o_ctrl = (struct cam_ois_ctrl_t *)ctrl;

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	/* get gyro gain*/
	ret = InfoMatRead129(o_ctrl, 0, 0, 0x3F, (uint32_t *)&UlBuffer);
	if (ret || UlBuffer[0] == 0 || UlBuffer[1] == 0) {
		CAM_ERR(CAM_OIS, "ret = %d, gain x: 0x%x, gain y: 0x%x", ret, UlBuffer[0], UlBuffer[1]);
		return -1;
	}

	o_ctrl->calib_data.gyro_gain[0] = UlBuffer[0]; /* gyro gain x */
	o_ctrl->calib_data.gyro_gain[1] = UlBuffer[1]; /* gyro gain y */

	CAM_INFO(CAM_OIS, "gyro gain x: 0x%x, gyro gain y: 0x%x",
		o_ctrl->calib_data.gyro_gain[0], o_ctrl->calib_data.gyro_gain[1]);

	return 0;
}

static int lc898129_set_gyro_gain(void *ctrl,
	unsigned int gain_x, unsigned int gain_y)
{
	struct cam_ois_ctrl_t *o_ctrl = (struct cam_ois_ctrl_t *)ctrl;
	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	/* check input */
	if (gain_x == 0 || gain_y == 0) {
		CAM_ERR(CAM_OIS, "gain is 0");
		return -1;
	}

	lc898129_write_ram_cmd(o_ctrl, 0x84B8, gain_x);
	lc898129_write_ram_cmd(o_ctrl, 0x84EC, gain_y);

	o_ctrl->calib_data.gyro_gain[0] = gain_x;
	o_ctrl->calib_data.gyro_gain[1] = gain_y;

	return 0;
}

static int lc898129_set_ois_fac_mode(void *ctrl, int32_t mode)
{
	int ret = 0;
	struct cam_ois_ctrl_t *o_ctrl = (struct cam_ois_ctrl_t *)ctrl;

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	CAM_INFO(CAM_OIS, "set_ois_fac_mode %d", mode);
	switch (mode) {
	case OIS_CMD_ENABLE:
		lc898129_write_ram_cmd(o_ctrl, 0xF012, 0x00010000);
		break;
	case OIS_CMD_DISABLE:
		lc898129_write_ram_cmd(o_ctrl, 0xF012, 0x00000000);
		break;
	case OIS_CMD_GRYO_OFFSET_CALIB:
		ret = lc898129_gyro_offset_calib(o_ctrl);
		break;
	default:
		CAM_ERR(CAM_OIS, "invalid fac mode %d", mode);
		break;
	}
	return ret;
}

static int lc898129_set_ctrl_flag(void *ctrl, int32_t flag)
{
	struct cam_ois_ctrl_t *o_ctrl = (struct cam_ois_ctrl_t *)ctrl;

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	/* flag 3: complete calib */
	if (flag == 3) {
		CalibrationDataSave0(o_ctrl); /* write calib gyro gain */
	}

	return 0;
}

/*
 * configure OIS, including 1.angleCorrection\ 2.3.g+a offset\ 4. gyro gain\ 5.gyro select\ 6.start SMA
 */
static int lc898129_configure_ois(struct cam_ois_ctrl_t *o_ctrl)
{
    unsigned char ret = 0;
	enum shared_gyro_vendor_t gyro_vendor;
	unsigned int gyro_select = 0;
	int i;

	/* step 1, SetAngleCorrection */
	CAM_INFO(CAM_OIS, "degree -90, arragement 0");
	ret = SetAngleCorrection(o_ctrl, -90, 0, 0);
	if (ret != 0) {
		CAM_ERR(CAM_OIS, "SetAngleCorrection failed");
		return -1;
	}

	/* step 2, gyro_select */
	gyro_vendor = GYRO_VENDOR_ST_LSM6DSM_V;

	CAM_INFO(CAM_OIS, "download fw gyro vendor %d", gyro_vendor);
	for (i = 0; i < sizeof(gyrotable) / sizeof(gyrotable[0]); i++){
		if(gyro_vendor == gyrotable[i].vendor){
			gyro_select = gyrotable[i].gyroselect;
			break;
		}
	}
	CAM_INFO(CAM_OIS, "ois download gyro %u", gyro_select);

	/* step 2.2 call select gyro */
	lc898129_write_ram_cmd(o_ctrl, 0xF015, gyro_select);
	lc898129_write_ram_cmd(o_ctrl, 0xF017, 0x00000001);

	lc898129_read_ram_cmd(o_ctrl, 0xF01F);
	lc898129_read_ram_cmd(o_ctrl, 0xF012);

	return 0;
}

static const struct ois_ops_table ois_fops = {
	.set_gyro_gain          = lc898129_set_gyro_gain,
	.get_gyro_gain          = lc898129_get_gyro_gain,
	.set_ois_fac_mode       = lc898129_set_ois_fac_mode,
	.set_ctrl_flag          = lc898129_set_ctrl_flag,
};

int lc898129_download_ois_fw(struct cam_ois_ctrl_t *o_ctrl)
{
	unsigned char ret;
	unsigned int fw_version_latest;
	uint32_t fw_version_current = 0;
	uint8_t actuator_size = 3;
	uint8_t module_vendor = 7;

	CAM_INFO(CAM_OIS, "module_vendor %d, actuator_size %d", module_vendor, actuator_size);
	RamRead32A(o_ctrl, 0x8000, &fw_version_current);
	fw_version_latest = lc898129_get_oisfw_version(module_vendor, actuator_size);
	CAM_INFO(CAM_OIS, "fw_version_current 0x%x, fw_version_latest 0x%x", fw_version_current, fw_version_latest);

	/* download fw process was here, changing to flash download */
	if ((fw_version_current & 0xffff) < (fw_version_latest & 0xffff)) {
		CAM_INFO(CAM_OIS, "starting flash download:: vendor_onsemi: %u, actuator_size: %u", module_vendor, actuator_size);
		ret = FlashProgram129(o_ctrl, 0, module_vendor, actuator_size);
		CAM_INFO(CAM_OIS, "finished flash download:: vendor_onsemi: %u, actuator_size: %u", module_vendor, actuator_size);
		if (0 != ret) {
			CAM_ERR(CAM_OIS, "select download error, ret = 0x%x", ret);
			return -1;
		}
	} else {
		CAM_INFO(CAM_OIS, "ois fw version is updated, skip download");
	}

	CAM_INFO(CAM_OIS, "set activemode");
	SetActiveMode(o_ctrl);

	lc898129_configure_ois(o_ctrl);
	o_ctrl->ops = &ois_fops;
	return 0;
}
