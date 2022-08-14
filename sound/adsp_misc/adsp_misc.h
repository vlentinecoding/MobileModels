/*
 * adsp_misc.h
 *
 * adsp misc header
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __ADSP_MISC_DEFS_H__
#define __ADSP_MISC_DEFS_H__

#define SMARTPAKIT_NAME_MAX    64

enum smartpa_cmd {
	GET_CURRENT_R0 = 0,
	GET_CURRENT_TEMPRATURE,      /* 1 */
	GET_CURRENT_F0,              /* 2 */
	GET_CURRENT_Q,               /* 3 */
	GET_PARAMETERS,              /* 4 */
	GET_CURRENT_POWER,           /* 5 */
	GET_CMD_NUM,                 /* 6 */

	SET_ALGO_SECENE,             /* 7 */
	SET_CALIBRATION_VALUE,       /* 8 */
	CALIBRATE_MODE_START,        /* 9 */
	CALIBRATE_MODE_STOP,         /* 10 */
	SET_F0_VALUE,                /* 11 */

	SET_PARAMETERS,              /* 12 */
	SET_VOICE_VOLUME,            /* 13 */
	SET_LOW_POWER_MODE,          /* 14 */
	SET_SAFETY_STRATEGY,         /* 15 */
	SET_FADE_CONFIG,             /* 16 */
	SET_SCREEN_ANGLE,            /* 17 */
	SMARTPA_ALGO_ENABLE,         /* 18 */
	SMARTPA_ALGO_DISABLE,        /* 19 */
	CMD_NUM,

	SMARTPA_PRINT_MCPS,
	SMARTPA_DEBUG,
	SMARTPA_DSP_ENABLE,
	SMARTPA_DSP_DISABLE,
};

// chip provider, which must be the same definition with smartpa kernel
enum smartpakit_chip_vendor {
	CHIP_VENDOR_MAXIM = 0, // max98925
	CHIP_VENDOR_NXP,       // tfa9871, tfa9872, tfa9874, tfa9895
	CHIP_VENDOR_TI,        // tas2560, tas2562
	CHIP_VENDOR_OTHER,     // other vendor

	CHIP_VENDOR_MAX,
};

// smartpakit_info, which must be the same definition with smartpa hal
struct smartpakit_info {
	// common info
	unsigned int soc_platform;
	unsigned int algo_in;
	unsigned int out_device;
	unsigned int pa_num;
	char special_name_config[SMARTPAKIT_NAME_MAX];

	// smartpa chip info
	unsigned int algo_delay_time;
	unsigned int chip_vendor;
	char chip_model[SMARTPAKIT_NAME_MAX];
};

struct misc_io_async_param {
	unsigned int  para_length;
	unsigned char *param;
};

struct misc_io_sync_param {
	unsigned int  in_len;
	unsigned char *in_param;
	unsigned int  out_len;
	unsigned char *out_param;
};

struct adsp_misc_ctl_info {
	struct smartpakit_info pa_info;
	unsigned int      param_size;
	unsigned short    cmd;
	unsigned short    size;
	unsigned char     data[0];
};

struct misc_io_async_param_compat {
	unsigned int para_length;
	unsigned int param;
};

struct misc_io_sync_param_compat {
	unsigned int in_len;
	unsigned int in_param;
	unsigned int out_len;
	unsigned int out_param;
};

struct adsp_misc_data_pkg {
	unsigned short cmd;
	unsigned short size;
	unsigned char  data[0];
};

struct adsp_ctl_param {
	unsigned int in_len;
	void __user  *param_in;
	unsigned int out_len;
	void __user  *param_out;
};

#define MIN_PARAM_IN     sizeof(struct adsp_misc_ctl_info)
#define MIN_PARAM_OUT    sizeof(struct adsp_misc_data_pkg)

// The following is ioctrol command sent from AP to ADSP misc device,
// ADSP misc side need response these commands.
#define ADSP_MISC_IOCTL_ASYNCMSG \
	_IOWR('A', 0x0, struct misc_io_async_param) // AP send async msg to ADSP
#define ADSP_MISC_IOCTL_SYNCMSG \
	_IOW('A', 0x1, struct misc_io_sync_param) // AP send sync msg to ADSP

#define ADSP_MISC_IOCTL_ASYNCMSG_COMPAT \
	_IOWR('A', 0x0, struct misc_io_async_param_compat)
#define ADSP_MISC_IOCTL_SYNCMSG_COMPAT \
	_IOW('A', 0x1, struct misc_io_sync_param_compat)

#endif // __ADSP_MISC_DEFS_H__

