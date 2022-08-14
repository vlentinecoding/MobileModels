/*
 * dp_cust_info.h
 *
 * dp custom info driver
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

#ifndef DP_CUST_INFO_H
#define DP_CUST_INFO_H

#include <linux/device.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/soc/qcom/dp_cust_interface.h>
#include "dp_factory.h"
#include "dp_dsm.h"

enum dp_link_rate {
	DP_LINK_RATE_RBR,
	DP_LINK_RATE_HBR,
	DP_LINK_RATE_HBR2,
	DP_LINK_RATE_HBR3,
};

enum dp_link_state {
	DP_LINK_CABLE_IN,        // typec cable in
	DP_LINK_CABLE_OUT,       // typec cable out
	DP_LINK_MULTI_HPD,       // report hpd repeatedly from PD module
	DP_LINK_AUX_FAILED,      // read/write regs failed by aux channel
	DP_LINK_SAFE_MODE,       // safe mode, 640*480
	DP_LINK_EDID_FAILED,     // read edid failed
	DP_LINK_LINK_FAILED,     // link training failed
	DP_LINK_LINK_RETRAINING, // link retraining by short irq
	DP_LINK_HDCP_FAILED,     // hdcp auth failed in DRM files

	DP_LINK_STATE_MAX,
};

struct dp_source_info {
	struct device_node *node;
	struct class *class;
	struct blocking_notifier_head notifier;
	enum dp_source_mode source_mode;
};

#ifdef CONFIG_HONOR_DP_CUST
struct dp_factory_info *dp_cust_get_factory_info(void);
struct dp_source_info *dp_cust_get_source_info(void);
struct dp_dsm_info *dp_cust_get_dsm_info(void);

int dp_cust_get_property_value(struct device_node *node,
	const char *prop_name, int default_value);
bool dp_cust_get_property_bool(struct device_node *node,
	const char *prop_name, bool default_setting);

void dp_link_state_event_report(const char *event);
void dp_source_send_event(enum dp_link_state state);
int dp_source_init(struct dp_source_info *info);
void dp_source_exit(struct dp_source_info *info);
#else
static inline struct dp_factory_info *dp_cust_get_factory_info(void)
{
	return NULL;
}

static inline struct dp_source_info *dp_cust_get_source_info(void)
{
	return NULL;
}

static inline struct dp_dsm_info *dp_cust_get_dsm_info(void)
{
	return NULL;
}

static inline int dp_cust_get_property_value(struct device_node *node,
	const char *prop_name, int default_value)
{
	return 0;
}

static inline bool dp_cust_get_property_bool(struct device_node *node,
	const char *prop_name, bool default_setting)
{
	return 0;
}

static inline void dp_link_state_event_report(const char *event)
{
}

static inline void dp_source_send_event(enum dp_link_state state)
{
}

static inline int dp_source_init(struct dp_source_info *info)
{
	return 0;
}

static inline void dp_source_exit(struct dp_source_info *info)
{
}
#endif // !CONFIG_HONOR_DP_CUST

#ifdef DP_DEBUG_ENABLE
int dp_cust_get_debug_revision(int *revision);
int dp_cust_set_debug_revision(int revision);
int dp_cust_get_debug_bpp(int *bpp);
int dp_cust_set_debug_bpp(int bpp);
int dp_cust_get_debug_vs_pe(int *v_level, int *p_level);
int dp_cust_set_debug_vs_pe(int v_level, int p_level);
int dp_cust_get_debug_edid(uint8_t **edid, int **size);
#endif

#endif // DP_CUST_INFO_H
