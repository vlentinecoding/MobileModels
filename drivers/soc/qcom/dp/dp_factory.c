/*
 * dp_factory.c
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

#include <linux/delay.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/of_gpio.h>
#include <linux/string.h>
#include <log/hw_log.h>
#include "dp_cust_info.h"
#include "dp_factory.h"

#define HWLOG_TAG dp_cust
HWLOG_REGIST();

#define INFO_BUF_MAX            512

#define DP_FACTORY_LANES        4
#define DP_FACTORY_RATE_HBR2    2
#define DP_FACTORY_RESOLUTION_H 3840
#define DP_FACTORY_RESOLUTION_V 2160
// Some TVs can only support 59 fps(max: 60fps), so the maximum fps here is set to 55.
#define DP_FACTORY_FPS          55

#define DP_FACTORY_CHECK_RET(a, b) (((a) >= (b)) ? "ok" : "error")

struct dp_factory_link_state_to_event {
	enum dp_manufacture_link_state state;
	char *event;
};

// for MMIE test in factory version
static struct dp_factory_link_state_to_event dp_factory_link_event[] = {
	{ DP_MANUFACTURE_LINK_CABLE_IN,        "MANUFACTURE_DP_LINK_EVENT=CABLE_IN" },
	{ DP_MANUFACTURE_LINK_CABLE_OUT,       "MANUFACTURE_DP_LINK_EVENT=CABLE_OUT" },
	{ DP_MANUFACTURE_LINK_AUX_FAILED,      "MANUFACTURE_DP_LINK_EVENT=AUX_FAILED" },
	{ DP_MANUFACTURE_LINK_SAFE_MODE,       "MANUFACTURE_DP_LINK_EVENT=SAFE_MODE" },
	{ DP_MANUFACTURE_LINK_EDID_FAILED,     "MANUFACTURE_DP_LINK_EVENT=EDID_FAILED" },
	{ DP_MANUFACTURE_LINK_LINK_FAILED,     "MANUFACTURE_DP_LINK_EVENT=LINK_FAILED" },
	{ DP_MANUFACTURE_LINK_HPD_NOT_EXISTED, "MANUFACTURE_DP_LINK_EVENT=HPD_NOT_EXISTED" },
	{ DP_MANUFACTURE_LINK_REDUCE_RATE,     "MANUFACTURE_DP_LINK_EVENT=LINK_REDUCE_RATE" },
	{ DP_MANUFACTURE_LINK_INVALID_COMBINATIONS,
		"MANUFACTURE_DP_LINK_EVENT=INVALID_COMBINATIONS" },
};

#define DP_MANUFACTURE_LINK_EVENT_CNT ARRAY_SIZE(dp_factory_link_event)

static void dp_factory_save_link_event(struct dp_factory_info *info,
	enum dp_manufacture_link_state state, int index)
{
	int ret;

	if (state == DP_MANUFACTURE_LINK_CABLE_IN || state == DP_MANUFACTURE_LINK_CABLE_OUT)
		return;

	info->link_event_no = state;
	ret = snprintf(info->link_event, sizeof(info->link_event),
		"%s", dp_factory_link_event[index].event);
	if (ret < 0)
		hwlog_info("%s: link_event copy failed\n", __func__);
}

void dp_factory_send_event(enum dp_manufacture_link_state state)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();
	int i;

	if (!info || !info->need_report_event)
		return;

	for (i = 0; i < DP_MANUFACTURE_LINK_EVENT_CNT; i++) {
		if (state == dp_factory_link_event[i].state) {
			dp_link_state_event_report(dp_factory_link_event[i].event);
			dp_factory_save_link_event(info, state, i);
			return;
		}
	}
}

static bool dp_factory_is_link_success(struct dp_factory_info *info)
{
	if (info->aux_rw_state == DP_AUX_RW_FAILED) {
		hwlog_err("%s: aux rw failed\n", __func__);
		dp_factory_send_event(DP_MANUFACTURE_LINK_AUX_FAILED);
		return false;
	} else if (info->safe_mode) {
		hwlog_err("%s: enter safe mode\n", __func__);
		dp_factory_send_event(DP_MANUFACTURE_LINK_SAFE_MODE);
		return false;
	} else if (info->hotplug_retval != 0 || info->link_train_retval != 0) {
		hwlog_err("%s: link failed\n", __func__);
		dp_factory_send_event(DP_MANUFACTURE_LINK_LINK_FAILED);
		return false;
	}

	return true;
}

bool dp_factory_is_link_downgrade(void)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();

	if (!info)
		return false;

	if (!dp_factory_is_link_success(info)) {
		hwlog_err("%s: link failed in factory version\n", __func__);
		info->factory_state = DP_FACTORY_FAILED;
		info->link_status = DP_FACTORY_LINK_FAILED;
		return true;
	}

	hwlog_info("%s: link lanes %d(%d) rate %d(%d)\n", __func__,
		info->link_lanes, info->init_lanes, info->link_rate, info->init_rate);

	if (info->link_lanes < info->init_lanes || info->link_rate < info->init_rate) {
		hwlog_err("%s: link downgrade is not allowed in factory version\n", __func__);
		info->factory_state = DP_FACTORY_FAILED;
		info->link_status = DP_FACTORY_LINK_DOWNGRADE_FAILED;
		dp_factory_send_event(DP_MANUFACTURE_LINK_REDUCE_RATE);
		return true;
	}

	return false;
}
EXPORT_SYMBOL(dp_factory_is_link_downgrade);

bool dp_factory_is_4k_60fps(void)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();

	if (!info)
		return true;

	hwlog_info("%s: init lanes %d(%d) rate %d(%d)\n", __func__,
		info->init_lanes, info->lanes_threshold,
		info->init_rate, info->rate_threshold);
	hwlog_info("%s: resolution h %d(%d) v %d(%d) fps %d(%d)\n", __func__,
		info->resolution_h, info->resolution_h_threshold,
		info->resolution_v, info->resolution_v_threshold,
		info->fps, info->fps_threshold);

	if (!info->audio_supported)
		hwlog_info("%s: audio not supported\n", __func__);
	else
		hwlog_info("%s: audio supported\n", __func__);

	if (!info->test_enable) {
		hwlog_info("%s: dp factory test %s, skip\n", __func__,
			info->test_enable ? "enable" : "disable");
		return true;
	}

	if (info->init_lanes == 0 || info->init_rate == 0 ||
		info->resolution_h == 0 || info->resolution_v == 0 || info->fps == 0) {
		hwlog_info("%s: invalid lanes rate or resolution, skip\n", __func__);
		return true;
	}

	// check whether the cable is dongle cable
	if (info->check_lanes_rate && (info->init_lanes < info->lanes_threshold ||
		info->init_rate < info->rate_threshold)) {
		hwlog_err("%s: invalid lanes is not allowed in factory version\n", __func__);
		info->factory_state = DP_FACTORY_FAILED;
		info->link_status = DP_FACTORY_LINK_FAILED;
		dp_factory_send_event(DP_MANUFACTURE_LINK_INVALID_COMBINATIONS);
		return false;
	}

	// check whether the external combination supports 4K
	if (info->check_display_4k && (info->resolution_h < info->resolution_h_threshold ||
		info->resolution_v < info->resolution_v_threshold)) {
		hwlog_err("%s: invalid resolution is not allowed in factory version\n", __func__);
		info->factory_state = DP_FACTORY_FAILED;
		info->link_status = DP_FACTORY_LINK_FAILED;
		dp_factory_send_event(DP_MANUFACTURE_LINK_INVALID_COMBINATIONS);
		return false;
	}

	// check whether the external combination supports 60fps
	if (info->check_display_60fps && info->fps < info->fps_threshold) {
		hwlog_err("%s: invalid fps is not allowed in factory version\n", __func__);
		info->factory_state = DP_FACTORY_FAILED;
		info->link_status = DP_FACTORY_LINK_FAILED;
		dp_factory_send_event(DP_MANUFACTURE_LINK_INVALID_COMBINATIONS);
		return false;
	}

	return true;
}
EXPORT_SYMBOL(dp_factory_is_4k_60fps);

enum dp_factory_state dp_factory_get_state(void)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();

	if (!info)
		return DP_FACTORY_CONNECTED;

	return info->factory_state;
}
EXPORT_SYMBOL(dp_factory_get_state);

void dp_factory_init_link_status(struct dp_factory_info *info)
{
	info->link_status = DP_FACTORY_LINK_SUCC;
	info->link_event_no = 0;
	memset(info->link_event, 0, sizeof(info->link_event));

	info->hotplug_retval = 0;
	info->link_train_retval = 0;
	info->aux_rw_state = DP_AUX_RW_SUCC;
	info->safe_mode = false;
	info->connect_timeout = false;
	info->audio_supported = false;

	info->sink_lanes = 0;
	info->sink_rate = 0;
	info->init_lanes = 0;
	info->init_rate = 0;
	info->link_lanes = 0;
	info->link_rate = 0;
	info->resolution_h = 0;
	info->resolution_v = 0;
	info->fps = 0;
}

int dp_factory_init(struct dp_factory_info *info)
{
	if (!info || !info->node)
		return -EINVAL;

	info->lanes_threshold = dp_cust_get_property_value(info->node,
		"ext_env_std_lanes", DP_FACTORY_LANES);
	info->rate_threshold = dp_cust_get_property_value(info->node,
		"ext_env_std_rate", DP_FACTORY_RATE_HBR2);
	info->resolution_h_threshold = dp_cust_get_property_value(info->node,
		"ext_env_std_resolution_h", DP_FACTORY_RESOLUTION_H);
	info->resolution_v_threshold = dp_cust_get_property_value(info->node,
		"ext_env_std_resolution_v", DP_FACTORY_RESOLUTION_V);
	info->fps_threshold = dp_cust_get_property_value(info->node,
		"ext_env_std_fps", DP_FACTORY_FPS);

	// default: need to check lanes/rate/4k/60fps in factory version
	info->test_enable = true;

	info->check_lanes_rate = dp_cust_get_property_bool(info->node,
		"check_lanes_rate", false);
	info->check_display_4k = dp_cust_get_property_bool(info->node,
		"check_display_4k", false);
	info->check_display_60fps = dp_cust_get_property_bool(info->node,
		"check_display_60fps", false);
	info->need_report_event = dp_cust_get_property_bool(info->node,
		"need_report_event", false);

	info->test_running = false;
	dp_factory_init_link_status(info);
	return 0;
}

void dp_factory_exit(struct dp_factory_info *info)
{
	UNUSED(info);
}

static int dp_factory_set_test_event(const char *val, const struct kernel_param *kp)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();
	int state = 0;

	UNUSED(kp);
	if (!val || !info)
		return -EINVAL;

	if (kstrtoint(val, 0, &state) < 0) {
		hwlog_err("%s: invalid params %s\n", __func__, val);
		return -EINVAL;
	}

	hwlog_info("%s: link state is %d\n", __func__, state);
	dp_factory_send_event(state);
	return 0;
}

static int dp_factory_get_test_result(char *buffer, const struct kernel_param *kp)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();

	UNUSED(kp);
	if (!buffer || !info)
		return -EINVAL;

	if (!info->test_running)
		return snprintf(buffer, (unsigned long)INFO_BUF_MAX, "Not yet tested.\n");

	if (info->link_status == DP_FACTORY_LINK_CR_FAILED)
		return snprintf(buffer, (unsigned long)INFO_BUF_MAX,
			"test failed: 0x%x %s\nlink cr failed!\n",
			info->link_event_no, info->link_event);

	if (info->link_status == DP_FACTORY_LINK_CH_EQ_FAILED)
		return snprintf(buffer, (unsigned long)INFO_BUF_MAX,
			"test failed: 0x%x %s\nlink ch_eq failed!\n",
			info->link_event_no, info->link_event);

	if (info->link_status == DP_FACTORY_LINK_DOWNGRADE_FAILED)
		return snprintf(buffer, (unsigned long)INFO_BUF_MAX,
			"test failed: 0x%x %s\nlink downgrade failed!\n",
			info->link_event_no, info->link_event);

	return snprintf(buffer, (unsigned long)INFO_BUF_MAX,
		"check external combinations: %s\ntest %s: 0x%x %s\n"
		"sink_lanes=%u[%s]\nsink_rate=%u[%s]\n"
		"init_lanes=%u[%s]\ninit_rate=%u[%s]\n"
		"*link_lanes=%u[%s]\n*link_rate=%u[%s]\n"
		"resolution_h=%u[%s]\nresolution_v=%u[%s]\nfps=%u[%s]\n",
		info->test_enable ? "enable" : "disable",
		(info->link_status == DP_FACTORY_LINK_SUCC) ? "success" : "failed",
		info->link_event_no, info->link_event,
		info->sink_lanes, DP_FACTORY_CHECK_RET(info->sink_lanes, info->lanes_threshold),
		info->sink_rate, DP_FACTORY_CHECK_RET(info->sink_rate, info->rate_threshold),
		info->init_lanes, DP_FACTORY_CHECK_RET(info->init_lanes, info->lanes_threshold),
		info->init_rate, DP_FACTORY_CHECK_RET(info->init_rate, info->rate_threshold),
		info->link_lanes, DP_FACTORY_CHECK_RET(info->link_lanes, info->lanes_threshold),
		info->link_rate, DP_FACTORY_CHECK_RET(info->link_rate, info->rate_threshold),
		info->resolution_h, DP_FACTORY_CHECK_RET(info->resolution_h,
			info->resolution_h_threshold),
		info->resolution_v, DP_FACTORY_CHECK_RET(info->resolution_v,
			info->resolution_v_threshold),
		info->fps, DP_FACTORY_CHECK_RET(info->fps, info->fps_threshold));
}

static int dp_factory_get_test_enable(char *buffer, const struct kernel_param *kp)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();

	UNUSED(kp);
	if (!buffer || !info)
		return -EINVAL;

	return snprintf(buffer, (unsigned long)INFO_BUF_MAX, "%d\n", info->test_enable);
}

static int dp_factory_set_test_enable(const char *val, const struct kernel_param *kp)
{
	struct dp_factory_info *info = dp_cust_get_factory_info();
	int enable = 0;

	UNUSED(kp);
	if (!val || !info)
		return -EINVAL;

	if (kstrtoint(val, 0, &enable) < 0) {
		hwlog_err("%s: invalid params %s\n", __func__, val);
		return -EINVAL;
	}

	info->test_enable = (bool)enable;
	hwlog_info("%s: test_enable is %d\n", __func__, info->test_enable);
	return 0;
}

static struct kernel_param_ops param_ops_test_event = {
	.get = NULL,
	.set = dp_factory_set_test_event,
};

static struct kernel_param_ops param_ops_test_result = {
	.get = dp_factory_get_test_result,
};

static struct kernel_param_ops param_ops_test_enable = {
	.get = dp_factory_get_test_enable,
	.set = dp_factory_set_test_enable,
};

module_param_cb(test_event, &param_ops_test_event, NULL, 0644);
module_param_cb(test_result, &param_ops_test_result, NULL, 0444);
module_param_cb(test_enable, &param_ops_test_enable, NULL, 0644);

MODULE_DESCRIPTION("dp factory driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
