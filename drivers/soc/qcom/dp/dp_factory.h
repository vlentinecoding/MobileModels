/*
 * dp_factory.h
 *
 * dp factory driver
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
 */

#ifndef DP_FACTORY_H
#define DP_FACTORY_H

#include <linux/of.h>
#include <linux/soc/qcom/dp_cust_interface.h>

#define DP_AUX_RW_COUNT_FIRST     1
#define DP_AUX_RW_COUNT_THRESHOLD 200

enum dp_aux_rw_state {
    DP_AUX_RW_SUCC,
    DP_AUX_RW_FAILED,
};

enum dp_factory_link_status {
	DP_FACTORY_LINK_SUCC,
	DP_FACTORY_LINK_FAILED,
	DP_FACTORY_LINK_DOWNGRADE_FAILED,
	DP_FACTORY_LINK_CR_FAILED,
	DP_FACTORY_LINK_CH_EQ_FAILED,
};

enum dp_manufacture_link_state {
	DP_MANUFACTURE_LINK_CABLE_IN,
	DP_MANUFACTURE_LINK_CABLE_OUT,
	DP_MANUFACTURE_LINK_AUX_FAILED,
	DP_MANUFACTURE_LINK_SAFE_MODE,
	DP_MANUFACTURE_LINK_EDID_FAILED,
	DP_MANUFACTURE_LINK_LINK_FAILED,
	DP_MANUFACTURE_LINK_HPD_NOT_EXISTED,
	DP_MANUFACTURE_LINK_REDUCE_RATE,
	DP_MANUFACTURE_LINK_INVALID_COMBINATIONS, // combinations not support 4k@60fps

	DP_MANUFACTURE_LINK_STATE_MAX,
};

struct dp_factory_info {
	struct device_node *node;

	uint32_t lanes_threshold;
	uint32_t rate_threshold;
	uint32_t resolution_h_threshold; // horizontal
	uint32_t resolution_v_threshold; // vertical
	uint32_t fps_threshold;

	bool test_enable;
	bool check_lanes_rate;
	bool check_display_4k;
	bool check_display_60fps;
	bool need_report_event;

	bool test_running;
	enum dp_factory_state factory_state;
	enum dp_factory_link_status link_status;
	uint32_t link_event_no;
	char link_event[DP_LINK_EVENT_BUF_MAX];

	int hotplug_retval;
	int link_train_retval;
	enum dp_aux_rw_state aux_rw_state;
	bool safe_mode;
	bool connect_timeout;
	bool audio_supported;

	// from sink devices
	uint32_t sink_lanes;
	uint32_t sink_rate;
	// initial lanes and rate
	uint32_t init_lanes;
	uint32_t init_rate;
	// actual link lanes and rate
	uint32_t link_lanes;
	uint32_t link_rate;
	uint32_t resolution_h;
	uint32_t resolution_v;
	uint32_t fps;
};

#ifdef CONFIG_HONOR_DP_FACTORY
void dp_factory_send_event(enum dp_manufacture_link_state state);
void dp_factory_init_link_status(struct dp_factory_info *info);
int dp_factory_init(struct dp_factory_info *info);
void dp_factory_exit(struct dp_factory_info *info);
#else
static inline void dp_factory_send_event(enum dp_manufacture_link_state event)
{
}

static inline void dp_factory_init_link_status(struct dp_factory_info *info)
{
}

static inline int dp_factory_init(struct dp_factory_info *info)
{
	return 0;
}

static inline void dp_factory_exit(struct dp_factory_info *info)
{
}
#endif // !CONFIG_HONOR_DP_FACTORY

#endif // DP_FACTORY_H
