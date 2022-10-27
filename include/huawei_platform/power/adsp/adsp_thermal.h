/*
 * adsp_thermal.h
 *
 * adsp thermal define
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

#ifndef _ADSP_THERMAL_H_
#define _ADSP_THERMAL_H_

#define MAX_THERMAL_LEVEL          16
#define DC_IIN_LIMIT_COUNT         2
#define MAX_TEMP_NODE_COUNT        8

struct thermal_basic_config
{
    short index_mapping;
    short node_num;
};

enum THERMAL_STATE_TYPE
{
    THERMAL_STATE_SCREEN_OFF,
    THERMAL_STATE_VIDEO,
    THERMAL_STATE_VIDEO_CHAT,
    THERMAL_STATE_LIVE_VIDEO,
    THERMAL_STATE_NAVIGATION,
    THERMAL_STATE_RETAILDEMO,
    THERMAL_STATE_HIGHSPEED,
    THERMAL_STATE_CAMERA,
    THERMAL_STATE_CLONE,
    THERMAL_STATE_GAME,
    THERMAL_STATE_VR,
    THERMAL_STATE_MAX,
    THERMAL_STATE_INVALID = 14,
};

struct thermal_limit_info
{
	unsigned short buck_5v_iin_limit;
	unsigned short buck_5v_chg_limit;
	unsigned short buck_9v_iin_limit;
	unsigned short buck_9v_chg_limit;
	unsigned short w_5v_current_limit;
	unsigned short w_9v_current_limit;
	unsigned short dc_current_limit[DC_IIN_LIMIT_COUNT];
	unsigned short hdc_current_limit[DC_IIN_LIMIT_COUNT];
	unsigned short w_dc_current_limit[DC_IIN_LIMIT_COUNT];
	unsigned short w_hdc_current_limit[DC_IIN_LIMIT_COUNT];
	unsigned short wlcurrent_ctrl;
	unsigned short voltage;
};

struct thermal_temp_config
{
	short temp_high;
	short temp_low;
	struct thermal_limit_info limit_info;
};

struct thermal_channel_config
{
	short thermal_channel;
	char level_cnt[THERMAL_STATE_INVALID];
	struct thermal_temp_config temp_config[THERMAL_STATE_MAX][MAX_THERMAL_LEVEL];
};

struct thermal_channel_state_config
{
	short thermal_channel;
	short thermal_state;
	short level_cnt;
	struct thermal_temp_config temp_config[MAX_THERMAL_LEVEL];
};

struct thermal_config_data
{
	int node_index;
	struct thermal_channel_config chan_config;
};

#endif /* _ADSP_THERMAL_H_ */



