/*
 * dp_cust_interface.h
 *
 * dp custom interface driver
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

#ifndef DP_CUST_INTERFACE_H
#define DP_CUST_INTERFACE_H

#include <linux/types.h>
#include <linux/notifier.h>
#include <drm/drm_dp_helper.h>

#ifndef MAX
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#endif

#ifndef MIN
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x)  (sizeof(x) / sizeof((x)[0]))
#endif

#define DP_LINK_EVENT_BUF_MAX    256

#define DP_DIFF_SOURCE_HDISPLAY  1920
#define DP_DIFF_SOURCE_VDISPLAY  1080
#define DP_DIFF_SOURCE_VREFRESH  60

#ifndef CONFIG_HONOR_DP_FACTORY
#define DP_CONNECT_COMP_DELAY_MS 8000
#else
#define DP_CONNECT_COMP_DELAY_MS 5000
#endif

#define DP_EDID_BLOCK_NUM_MAX     4
#define DP_EDID_BLOCK_SIZE        128
#define DP_EDID_BLOCK_LEN(e)      (((e)->extensions + 1) * DP_EDID_BLOCK_SIZE)

#define DP_BPP_SUPPORTED_MIN      18
#define DP_BPP_SUPPORTED_MAX      24
#define DP_BPP_SUPPORTED_HDR_MAX  30

enum dp_source_mode {
	DIFF_SOURCE = 0,
	SAME_SOURCE,
};

enum dp_factory_state {
    DP_FACTORY_PENDING,
    DP_FACTORY_CONNECTED,
    DP_FACTORY_FAILED,
};

enum dp_cust_param {
	DP_PARAM_BASE,
	DP_PARAM_CABLE_TYPE = DP_PARAM_BASE,
	DP_PARAM_HOTPLUG_RETVAL, // hotplug success or failed
	DP_PARAM_LINK_TRAIN_RETVAL,
	DP_PARAM_AUX_ERR_NO,
	DP_PARAM_SAFE_MODE, // whether or not to be safe_mode(force display)
	DP_PARAM_CONNECT_TIMEOUT,
	DP_PARAM_AUDIO_SUPPORTED,
	DP_PARAM_EDID,
	DP_PARAM_DPCD,
	DP_PARAM_DS_PORT,
	DP_PARAM_NUM,
};

#ifdef CONFIG_HONOR_DP_CUST
void dp_cust_set_connect_state(bool connected);
void dp_cust_set_sink_lanes_rate(uint32_t lanes, uint32_t rate);
void dp_cust_set_init_lanes_rate(uint32_t lanes, uint32_t rate);
void dp_cust_set_link_lanes_rate(uint32_t lanes, uint32_t rate);
void dp_cust_set_resolution_fps(uint32_t h, uint32_t v, uint32_t fps);
void dp_cust_set_param(enum dp_cust_param param, void *data, int size);

void dp_cust_hex_dump(const char *prefix, void *data, int size);
void dp_cust_get_dpcd_revision(uint8_t *revision);
void dp_cust_get_vs_pe(uint8_t *v_level, uint8_t *p_level);
void dp_cust_get_edid(bool native, bool read, struct drm_dp_aux_msg *msg);
uint32_t dp_cust_get_supported_bpp(void);
int dp_source_get_current_mode(void);
int dp_source_reg_notifier(struct notifier_block *nb);
int dp_source_unreg_notifier(struct notifier_block *nb);
#else
static inline void dp_cust_set_connect_state(bool connected)
{
}

static inline void dp_cust_set_sink_lanes_rate(uint32_t lanes, uint32_t rate)
{
}

static inline void dp_cust_set_init_lanes_rate(uint32_t lanes, uint32_t rate)
{
}

static inline void dp_cust_set_link_lanes_rate(uint32_t lanes, uint32_t rate)
{
}

static inline void dp_cust_set_resolution_fps(uint32_t h, uint32_t v, uint32_t fps)
{
}

static inline void dp_cust_set_param(enum dp_cust_param param, void *data, int size)
{
}

static inline void dp_cust_hex_dump(const char *prefix, void *data, int size)
{
}

static inline void dp_cust_get_dpcd_revision(uint8_t *revision)
{
}

static inline void dp_cust_get_vs_pe(uint8_t *v_level, uint8_t *p_level)
{
}

static inline void dp_cust_get_edid(bool native, bool read, struct drm_dp_aux_msg *msg)
{
}

static inline uint32_t dp_cust_get_supported_bpp(void)
{
	return DP_BPP_SUPPORTED_HDR_MAX;
}

static inline int dp_source_get_current_mode(void)
{
	return SAME_SOURCE;
}

static inline int dp_source_reg_notifier(struct notifier_block *nb)
{
	return 0;
}

static inline int dp_source_unreg_notifier(struct notifier_block *nb)
{
	return 0;
}
#endif // !CONFIG_HONOR_DP_CUST

#ifdef CONFIG_HONOR_DP_FACTORY
bool dp_factory_is_link_downgrade(void);
bool dp_factory_is_4k_60fps(void);
enum dp_factory_state dp_factory_get_state(void);
#else
static inline bool dp_factory_is_link_downgrade(void)
{
	return false;
}

static inline bool dp_factory_is_4k_60fps(void)
{
	return true;
}

static inline enum dp_factory_state dp_factory_get_state(void)
{
	return DP_FACTORY_CONNECTED;
}
#endif // !CONFIG_HONOR_DP_FACTORY

#endif // DP_CUST_INTERFACE_H
