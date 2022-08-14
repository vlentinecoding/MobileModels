/**
 * @brief LC898128 OIS PRODUCT
 *
 * @author Copyright (C) 2021 ON Semiconductor All Rights Reserved.
 *
 **/

#include <linux/module.h>
#include <linux/firmware.h>
#include <cam_sensor_cmn_header.h>
#include "cam_ois_core.h"
#include "cam_sensor_util.h"
#include "cam_debug_util.h"
#include "cam_common_util.h"
#include "cam_packet_util.h"
#include <linux/vmalloc.h>
#include "onsemi_ois_interface.h"

#include "phone_update.h"

unsigned int lc898128_read_ram_cmd(struct cam_ois_ctrl_t *o_ctrl, unsigned short reg)
{
	unsigned int reg_read = 0xFF;

	RamRead32A(o_ctrl, reg, &reg_read);
	CAM_DBG(CAM_OIS, "read 0x%x, 0x%x", reg, reg_read);

	return reg_read;
}

/* used for write ram and then readout */
unsigned int lc898128_write_ram_cmd(struct cam_ois_ctrl_t *o_ctrl,
	unsigned short const reg, uint32_t value)
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
	/* on Module vendor, Actuator Size, on vesion number */
	{ 0x03, 0x00, 0x000320001c }, /* LuxVisions */
	{ 0x01, 0x00, 0x000720001c }, /* sunny */
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
static unsigned int lc898128_get_oisfw_version(unsigned char on_module_vendor,
	unsigned char actuator_size)
{
	unsigned char i = 0;

	for (i = 0; i < 2; ++i) {
		if (on_module_vendor == version_info[i][0] &&
			actuator_size == version_info[i][1])
			return version_info[i][2];
	}

	return 0xff;
}

static int lc898128_get_mag_check_result(struct cam_ois_ctrl_t *o_ctrl)
{
	unsigned int otp_x = 0;
	unsigned int otp_y = 0;
	unsigned int srv_x = 0;
	unsigned int srv_y = 0;

	/* input check */
	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	/* impl */
	RamWrite32A(o_ctrl, CMD_RETURN_TO_CENTER, BOTH_SRV_OFF, 0);
	msleep(300);
	RamRead32A(o_ctrl, HALL_RAM_HXOFF, &otp_x);
	RamRead32A(o_ctrl, HALL_RAM_HYOFF, &otp_y);
	RamRead32A(o_ctrl, HALL_RAM_HXOUT0, &srv_x);
	RamRead32A(o_ctrl, HALL_RAM_HYOUT0, &srv_y);

	RamWrite32A(o_ctrl, CMD_RETURN_TO_CENTER, BOTH_SRV_ON, 0);

	/* result setting */
	o_ctrl->calib_data.mag_check_data[0] = otp_x;
	o_ctrl->calib_data.mag_check_data[1] = otp_y;
	o_ctrl->calib_data.mag_check_data[2] = srv_x;
	o_ctrl->calib_data.mag_check_data[3] = srv_y;

	CAM_INFO(CAM_OIS, "otpcen_x = %u otpcen_y = %u srvoff_x = %u srvoff_y = %u",
		o_ctrl->calib_data.mag_check_data[0], o_ctrl->calib_data.mag_check_data[1],
		o_ctrl->calib_data.mag_check_data[2], o_ctrl->calib_data.mag_check_data[3]);
	return 0;
}

static int lc898128_gyro_offset_calib(struct cam_ois_ctrl_t *o_ctrl)
{
	uint32_t ret = EXE_END;

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	/* impl */
	CAM_INFO(CAM_OIS, "start XuChangMeasGyAcOffset");
	ret = XuChangMeasGyAcOffset(o_ctrl);

	if (ret == EXE_END) {
		CAM_INFO(CAM_OIS, "XuChangMeasGyAcOffset fin ok, ret = 0x%x", ret);
		o_ctrl->calib_data.gyro_offset_result = 0;
	} else {
		CAM_INFO(CAM_OIS, "XuChangMeasGyAcOffset fin err, ret = 0x%x", ret);
		o_ctrl->calib_data.gyro_offset_result = -1;
	}

	XuChangGetGyroOffset(o_ctrl, &o_ctrl->calib_data.gyro_offset_data[0],
		&o_ctrl->calib_data.gyro_offset_data[1], &o_ctrl->calib_data.gyro_offset_data[2]);

	CAM_INFO(CAM_OIS, "gyro offset X = %u, Y = %u, Z = %u, ret = %d",
		o_ctrl->calib_data.gyro_offset_data[0],
		o_ctrl->calib_data.gyro_offset_data[1],
		o_ctrl->calib_data.gyro_offset_data[2],
		o_ctrl->calib_data.gyro_offset_result);

	return 0;
}

static int lc898128_get_gyro_gain(void *ctrl)
{
	struct cam_ois_ctrl_t *o_ctrl = (struct cam_ois_ctrl_t *)ctrl;

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	RamRead32A(o_ctrl, GyroFilterTableX_gxzoom,
		&o_ctrl->calib_data.gyro_gain[0]); /* gyro gain x */
	RamRead32A(o_ctrl, GyroFilterTableY_gyzoom,
		&o_ctrl->calib_data.gyro_gain[1]); /* gyro gain y */

	CAM_INFO(CAM_OIS, "gyro gain x: 0x%x, gyro gain y: 0x%x",
		o_ctrl->calib_data.gyro_gain[0], o_ctrl->calib_data.gyro_gain[1]);

	return 0;
}

static int lc898128_set_gyro_gain(void *ctrl,
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

	lc898128_write_ram_cmd(o_ctrl, GyroFilterTableX_gxzoom, gain_x);
	lc898128_write_ram_cmd(o_ctrl, GyroFilterTableY_gyzoom, gain_y);

	o_ctrl->calib_data.gyro_gain[0] = gain_x;
	o_ctrl->calib_data.gyro_gain[1] = gain_y;

	return 0;
}

static int lc898128_set_ois_fac_mode(void *ctrl, int32_t mode)
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
		OisEna(o_ctrl);
		break;
	case OIS_CMD_DISABLE:
		OisDis(o_ctrl);
		break;
	case OIS_CMD_MAG_CHECK:
		ret = lc898128_get_mag_check_result(o_ctrl);
		break;
	case OIS_CMD_GRYO_OFFSET_CALIB:
		ret = lc898128_gyro_offset_calib(o_ctrl);
		break;
	default:
		CAM_ERR(CAM_OIS, "invalid fac mode %d", mode);
		break;
	}
	return ret;
}

static int lc898128_set_ctrl_flag(void *ctrl, int32_t flag)
{
	struct cam_ois_ctrl_t *o_ctrl = (struct cam_ois_ctrl_t *)ctrl;

	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	/* 3：complete calib */
	if (flag == 3)
		XuChangWrGyroGainData(o_ctrl, 1);

	return 0;
}

static int lc898128_set_calib_data(void *ctrl)
{
	struct cam_ois_ctrl_t *o_ctrl = (struct cam_ois_ctrl_t *)ctrl;
	if (o_ctrl->cam_ois_state < CAM_OIS_CONFIG) {
		CAM_WARN(CAM_OIS,
			"Not in right state to control OIS: %d",
			o_ctrl->cam_ois_state);
		return -1;
	}

	CAM_INFO(CAM_OIS, "set calib data");
	return RecoveryCorrectCoeffDataSave(o_ctrl);
}

/*
 * configure OIS, including 1.angleCorrection\ 2.3.g+a offset\ 4. gyro gain\ 5.gyro select\ 6.start SMA
 */
static int lc898128_configure_ois(struct cam_ois_ctrl_t *o_ctrl)
{
	unsigned char ret = 0;
	enum shared_gyro_vendor_t gyro_vendor;
	unsigned int gyro_select = 0;
	int i;

	/* step 1, SetAngleCorrection */
	CAM_INFO(CAM_OIS, "degree -90, arragement 0");
	ret = XuChangSetAngleCorrection(o_ctrl, -90, 0, 0);
	if (ret != 0) {
		CAM_ERR(CAM_OIS, "XuChangSetAngleCorrection failed");
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
	if (!strcmp(o_ctrl->ois_name, "ois_lc898128_slave")) {
		lc898128_write_ram_cmd(o_ctrl, CMD_GYROINITIALCOMMAND, 0x00000102);
		CAM_INFO(CAM_OIS, "set gyro monitor");
	} else {
		lc898128_write_ram_cmd(o_ctrl, CMD_GYROINITIALCOMMAND, 0x02);
		CAM_INFO(CAM_OIS, "set gyro master");
	}
	lc898128_write_ram_cmd(o_ctrl, 0xF017, 0x00000001);
	/* close AGG */
	lc898128_write_ram_cmd(o_ctrl, 0x87AC, 0x0);
	lc898128_write_ram_cmd(o_ctrl, 0x87E4, 0x0);

	lc898128_read_ram_cmd(o_ctrl, CMD_OIS_ENABLE);

	return 0;
}

static const struct ois_ops_table ois_fops = {
	.set_gyro_gain          = lc898128_set_gyro_gain,
	.get_gyro_gain          = lc898128_get_gyro_gain,
	.set_ois_fac_mode       = lc898128_set_ois_fac_mode,
	.set_ctrl_flag          = lc898128_set_ctrl_flag,
	.set_calib_data         = lc898128_set_calib_data,
};

int lc898128_download_ois_fw(struct cam_ois_ctrl_t *o_ctrl)
{
	unsigned char ret;
	unsigned int fw_version_latest;
	uint32_t fw_version_current = 0;
	uint8_t module_vendor = o_ctrl->module_code_data;
	uint8_t actuator_size = 0;
	unsigned int line_calib_status = 0;
	unsigned int cross_calib_status = 0;

	CAM_INFO(CAM_OIS, "module_vendor %u, actuator_size %u", module_vendor, actuator_size);
	RamRead32A(o_ctrl, 0x8000, &fw_version_current);
	fw_version_latest = lc898128_get_oisfw_version(module_vendor, actuator_size);
	CAM_INFO(CAM_OIS, "fw_version_current 0x%x, fw_version_latest 0x%x", fw_version_current, fw_version_latest);

	/* download fw process was here, changing to flash download */
	if ((fw_version_current & 0xffff) != (fw_version_latest & 0xffff)) {
		CAM_INFO(CAM_OIS, "starting flash download:: vendor_onsemi: %u, actuator_size: %u", module_vendor, actuator_size);
		ret = FlashDownload128(o_ctrl, 1, module_vendor, actuator_size);
		CAM_INFO(CAM_OIS, "finished flash download:: vendor_onsemi: %u, actuator_size: %u", module_vendor, actuator_size);
		if (0 != ret) {
			CAM_ERR(CAM_OIS, "select download error, ret = 0x%x", ret);
			return -1;
		}
	} else {
		CAM_INFO(CAM_OIS, "ois fw version is updated, skip download");
	}

	RamRead32A(o_ctrl, 0x668, &line_calib_status);
	RamRead32A(o_ctrl, 0x66c, &cross_calib_status);
	/* 1 means calib data exist */
	if (line_calib_status == 1 && cross_calib_status == 1) {
		CAM_INFO(CAM_OIS, "no need write linearity& cross talk");
		o_ctrl->is_ois_calib = 0;
	}

	CAM_INFO(CAM_OIS, "set activemode");
	XuChangSetActiveMode(o_ctrl);

	lc898128_configure_ois(o_ctrl);
	o_ctrl->ops = &ois_fops;
	return 0;
}
