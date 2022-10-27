/*
 * dp_dsm.c
 *
 * dp dsm driver
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

#include <linux/device.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <log/hw_log.h>
#ifdef CONFIG_HUAWEI_DSM
#include <log/imonitor.h>
#endif
#include "dp_cust_info.h"
#include "dp_dsm.h"

#define HWLOG_TAG dp_cust
HWLOG_REGIST();

#define DP_DSM_TIME_US_OF_MS         1000 // 1ms = 1000us
#define DP_DSM_TIME_MS_OF_SEC        1000 // 1s = 1000ms
#define DP_DSM_TIME_SECOND_OF_MINUTE 60
#define DP_DSM_TIME_SECOND_OF_HOUR   (60 * DP_DSM_TIME_SECOND_OF_MINUTE)
#define DP_DSM_TIME_SECOND_OF_DAY    (24 * DP_DSM_TIME_SECOND_OF_HOUR)

#ifndef DP_DSM_DEBUG
#ifndef DP_DEBUG_ENABLE
#define DP_DSM_REPORT_NUM_IN_ONE_DAY 5    // for user version
#else
#define DP_DSM_REPORT_NUM_IN_ONE_DAY 1000 // for eng version
#endif // DP_DEBUG_ENABLE

#define DP_DSM_REPORT_TIME_INTERVAL  (DP_DSM_TIME_SECOND_OF_DAY * DP_DSM_TIME_MS_OF_SEC)
#else
#define DP_DSM_REPORT_NUM_IN_ONE_DAY 5
#define DP_DSM_REPORT_TIME_INTERVAL  (300 * 1000) // 5 minutes
#endif // DP_DSM_DEBUG

#ifdef CONFIG_HUAWEI_DSM
#define DP_SET_PARAM_I(o, s, v) imonitor_set_param_integer_v2(o, s, (long)v)
#else
#define DP_SET_PARAM_I(o, s, v)
#endif

#define DP_IMONITOR_EVENT_ID(t, f)   { t, t##_NO, f }
#define DP_IMONITOR_BASIC_INFO_NO    936000102
#define DP_IMONITOR_TIME_INFO_NO     936000103
#define DP_IMONITOR_EXTEND_INFO_NO   936000104
#define DP_IMONITOR_HPD_NO           936000105
#define DP_IMONITOR_LINK_TRAINING_NO 936000106
#define DP_IMONITOR_HOTPLUG_NO       936000107
#define DP_IMONITOR_HDCP_NO          936000108

typedef int (*imonitor_prepare_param_cb_t) (struct imonitor_eventobj *, struct dp_dsm_info *);

struct dp_imonitor_event_id {
    enum dp_imonitor_type type;
    unsigned int event_id;
    imonitor_prepare_param_cb_t prepare_cb;
};

static int dp_imonitor_basic_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info);
static int dp_imonitor_time_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info);
static int dp_imonitor_extend_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info);
static int dp_imonitor_hpd_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info);
static int dp_imonitor_link_train_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info);
static int dp_imonitor_hotplug_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info);
static int dp_imonitor_hdcp_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info);

static struct dp_imonitor_event_id imonitor_event_id[] = {
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_BASIC_INFO, dp_imonitor_basic_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_TIME_INFO, dp_imonitor_time_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_EXTEND_INFO, dp_imonitor_extend_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_HPD, dp_imonitor_hpd_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_LINK_TRAINING, dp_imonitor_link_train_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_HOTPLUG, dp_imonitor_hotplug_info),
	DP_IMONITOR_EVENT_ID(DP_IMONITOR_HDCP, dp_imonitor_hdcp_info),
};

static int dp_imonitor_basic_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info)
{
	UNUSED(obj);
	UNUSED(info);
	return 0;
}

static int dp_imonitor_time_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info)
{
	UNUSED(obj);
	UNUSED(info);
	return 0;
}

static int dp_imonitor_extend_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info)
{
	UNUSED(obj);
	UNUSED(info);
	return 0;
}

static int dp_imonitor_hpd_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info)
{
	UNUSED(obj);
	UNUSED(info);
	return 0;
}

static int dp_imonitor_link_train_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info)
{
	int ret = 0;

#ifdef DP_DSM_DEBUG
	hwlog_info("%s: cable_type %d\n", __func__, info->cable_type);
	hwlog_info("%s: safe_mode %d\n", __func__, info->safe_mode);
	hwlog_info("%s: hotplug_retval %d\n", __func__, info->hotplug_retval);
	hwlog_info("%s: link_train_retval %d\n", __func__, info->link_train_retval);
	hwlog_info("%s: connect_timeout %d\n", __func__, info->connect_timeout);
	hwlog_info("%s: aux_rw_count %d\n", __func__, info->aux_rw_count);
	hwlog_info("%s: aux_rw_retval %d\n", __func__, info->aux_rw_retval);
#endif

	ret += DP_SET_PARAM_I(obj, "CableType", info->cable_type);
	ret += DP_SET_PARAM_I(obj, "MaxWidth",  0);
	ret += DP_SET_PARAM_I(obj, "MaxHigh",   0);
	ret += DP_SET_PARAM_I(obj, "PixelClk",  0);
	ret += DP_SET_PARAM_I(obj, "MaxRate",   0);
	ret += DP_SET_PARAM_I(obj, "MaxLanes",  0);
	ret += DP_SET_PARAM_I(obj, "LinkRate",  0);
	ret += DP_SET_PARAM_I(obj, "LinkLanes", 0);
	ret += DP_SET_PARAM_I(obj, "VsPe",      0);
	ret += DP_SET_PARAM_I(obj, "SafeMode",  info->safe_mode);
	ret += DP_SET_PARAM_I(obj, "RedidRet",  info->hotplug_retval);
	ret += DP_SET_PARAM_I(obj, "LinkRet",   info->link_train_retval);
	ret += DP_SET_PARAM_I(obj, "AuxRw",     info->connect_timeout);
	ret += DP_SET_PARAM_I(obj, "AuxI2c",    0);
	ret += DP_SET_PARAM_I(obj, "AuxAddr",   0);
	ret += DP_SET_PARAM_I(obj, "AuxLen",    info->aux_rw_count);
	ret += DP_SET_PARAM_I(obj, "AuxRet",    info->aux_rw_retval);
	return ret;
}

static int dp_imonitor_hotplug_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info)
{
	UNUSED(obj);
	UNUSED(info);
	return 0;
}

static int dp_imonitor_hdcp_info(struct imonitor_eventobj *obj, struct dp_dsm_info *info)
{
	UNUSED(obj);
	UNUSED(info);
	return 0;
}

static uint32_t dp_imonitor_get_event_id(enum dp_imonitor_type type,
	imonitor_prepare_param_cb_t *prepare)
{
	int count = ARRAY_SIZE(imonitor_event_id);
	uint32_t event_id = 0;
	int i;

	if (type >= DP_IMONITOR_TYPE_NUM)
		goto err_out;

	for (i = 0; i < count; i++) {
		if (imonitor_event_id[i].type == type) {
			event_id = imonitor_event_id[i].event_id;
			*prepare = imonitor_event_id[i].prepare_cb;
			break;
		}
	}

err_out:
	return event_id;
}

#ifdef CONFIG_HUAWEI_DSM
static void dp_imonitor_report(enum dp_imonitor_type type, struct dp_dsm_info *info)
{
	struct imonitor_eventobj *obj = NULL;
	imonitor_prepare_param_cb_t prepare = NULL;
	uint32_t event_id;
	int ret;

	hwlog_info("%s enter\n", __func__);
	event_id = dp_imonitor_get_event_id(type, &prepare);
	if ((event_id == 0) || !prepare) {
		hwlog_err("%s: invalid type %d, event_id %u\n", __func__, type, event_id);
		return;
	}

	obj = imonitor_create_eventobj(event_id);
	if (!obj) {
		hwlog_err("%s: imonitor_create_eventobj %u failed\n", __func__, event_id);
		return;
	}

	ret = prepare(obj, info);
	if (ret < 0) {
		hwlog_info("%s: prepare param %u skip %d\n", __func__, event_id, ret);
		goto err_out;
	}

	ret = imonitor_send_event(obj);
	if (ret < 0) {
		hwlog_err("%s: imonitor_send_event %u failed %d\n", __func__, event_id, ret);
		goto err_out;
	}
	hwlog_info("%s event_id %u success\n", __func__, event_id);

err_out:
	if (obj)
		imonitor_destroy_eventobj(obj);
}
#else
static inline void dp_imonitor_report(enum dp_imonitor_type type, struct dp_dsm_info *info)
{
	UNUSED(type);
	UNUSED(info);
}
#endif // !CONFIG_HUAWEI_DSM

static void dp_imonitor_report_info_one(struct dp_dsm_info *info)
{
	int count = info->report_num[info->report_type];

	if (count < DP_DSM_REPORT_NUM_IN_ONE_DAY) {
		hwlog_info("%s: imonitor report %d %d/%d\n", __func__,
			info->report_type, count + 1, DP_DSM_REPORT_NUM_IN_ONE_DAY);
		dp_imonitor_report(info->report_type, info);
	} else {
		hwlog_info("%s: imonitor report %d %d/%d, skip\n", __func__,
			info->report_type, count + 1, DP_DSM_REPORT_NUM_IN_ONE_DAY);

		// record the key information of the last day
		info->report_skip_existed = true;
		info->last_report_num[info->report_type]++;
	}

	info->report_num[info->report_type]++;
}

static void dp_imonitor_report_info_print(struct dp_dsm_info *info)
{
	int i;

	if (!info->report_skip_existed)
		return;

	// the key information of the last day
	for (i = 0; i < DP_IMONITOR_TYPE_NUM; i++)
		hwlog_info("%s: last_report_num[%d]=%u\n", __func__, i, info->last_report_num[i]);

	info->report_skip_existed = false;
	memset(info->last_report_num, 0, sizeof(uint32_t) * DP_IMONITOR_TYPE_NUM);
}

static void dp_dsm_imonitor_report_work_fn(struct work_struct *work)
{
	struct dp_dsm_info *info = container_of(work, struct dp_dsm_info, report_work);

	if (!info)
		return;

	if (!time_is_after_jiffies(info->report_jiffies)) {
		memset(info->report_num, 0, sizeof(uint32_t) * DP_IMONITOR_TYPE_NUM);
		info->report_jiffies = jiffies + msecs_to_jiffies(DP_DSM_REPORT_TIME_INTERVAL);
		dp_imonitor_report_info_print(info);
	}

	dp_imonitor_report_info_one(info);
	hwlog_info("%s: imonitor report success\n", __func__);
}

void dp_dsm_imonitor_report(enum dp_imonitor_type type)
{
	struct dp_dsm_info *info = dp_cust_get_dsm_info();

#ifdef CONFIG_HONOR_DP_FACTORY
	return;
#endif
	if (!info)
		return;

	info->report_type = type;
	cancel_work_sync(&info->report_work);
	queue_work(system_freezable_wq, &info->report_work);
}

void dp_dsm_init_link_status(struct dp_dsm_info *info)
{
	if (!info)
		return;

	info->cable_type = 0;
	info->hotplug_retval = 0;
	info->link_train_retval = 0;
	info->aux_rw_retval = 0;
	info->aux_rw_count = 0;
	info->safe_mode = false;
	info->connect_timeout = false;
}

int dp_dsm_init(struct dp_dsm_info *info)
{
	if (!info)
		return 0;

	dp_dsm_init_link_status(info);
	info->report_jiffies = jiffies + msecs_to_jiffies(DP_DSM_REPORT_TIME_INTERVAL);
	info->report_skip_existed = false;
	INIT_WORK(&info->report_work, dp_dsm_imonitor_report_work_fn);
	return 0;
}

void dp_dsm_exit(struct dp_dsm_info *info)
{
	if (!info)
		return;

	cancel_work_sync(&info->report_work);
}

MODULE_DESCRIPTION("dp dsm driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
