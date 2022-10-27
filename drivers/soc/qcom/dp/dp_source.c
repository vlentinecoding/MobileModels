/*
 * dp_source.c
 *
 * dp source driver
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
#include <linux/string.h>
#include <log/hw_log.h>
#include "dp_cust_info.h"

#define HWLOG_TAG dp_cust
HWLOG_REGIST();

struct dp_device_type {
	const char *name;
	struct device *dev;
	int index;
};

static struct dp_device_type hw_dp_device[] = {
	{
		.name  = "source",
		.dev   = NULL,
		.index = 0,
	},
};

#define DP_DEVICE_NUM ARRAY_SIZE(hw_dp_device)

struct dp_link_state_to_event {
    enum dp_link_state state;
    char *event;
};

static struct dp_link_state_to_event dp_link_event[] = {
	{ DP_LINK_CABLE_IN,        "DP_LINK_EVENT=CABLE_IN" },
	{ DP_LINK_CABLE_OUT,       "DP_LINK_EVENT=CABLE_OUT" },
	{ DP_LINK_MULTI_HPD,       "DP_LINK_EVENT=MULTI_HPD" },
	{ DP_LINK_AUX_FAILED,      "DP_LINK_EVENT=AUX_FAILED" },
	{ DP_LINK_SAFE_MODE,       "DP_LINK_EVENT=SAFE_MODE" },
	{ DP_LINK_EDID_FAILED,     "DP_LINK_EVENT=EDID_FAILED" },
	{ DP_LINK_LINK_FAILED,     "DP_LINK_EVENT=LINK_FAILED" },
	{ DP_LINK_LINK_RETRAINING, "DP_LINK_EVENT=LINK_RETRAINING" },
	{ DP_LINK_HDCP_FAILED,     "DP_LINK_EVENT=HDCP_FAILED" },
};

#define DP_LINK_EVENT_CNT ARRAY_SIZE(dp_link_event)

void dp_link_state_event_report(const char *event)
{
	char event_buf[DP_LINK_EVENT_BUF_MAX] = {0};
	char *envp[] = { event_buf, NULL };
	int ret;

	if (!event || !hw_dp_device[0].dev)
		return;

	ret = snprintf(event_buf, sizeof(event_buf), "%s", event);
	if (ret < 0)
		return;

	ret = kobject_uevent_env(&hw_dp_device[0].dev->kobj, KOBJ_CHANGE, envp);
	if (ret < 0)
		hwlog_info("send uevent failed %d\n", ret);
	else
		hwlog_info("send uevent %s success\n", envp[0]);
}

void dp_source_send_event(enum dp_link_state state)
{
	struct dp_source_info *info = dp_cust_get_source_info();
	int i;

#ifdef CONFIG_HONOR_DP_FACTORY
	return;
#endif
	if (!info)
		return;

	for (i = 0; i < DP_LINK_EVENT_CNT; i++) {
		if (state == dp_link_event[i].state) {
			dp_link_state_event_report(dp_link_event[i].event);
			return;
		}
	}
}

int dp_source_get_current_mode(void)
{
	struct dp_source_info *info = dp_cust_get_source_info();

	if (!info)
		return SAME_SOURCE;

	return info->source_mode;
}
EXPORT_SYMBOL_GPL(dp_source_get_current_mode);

int dp_source_reg_notifier(struct notifier_block *nb)
{
	struct dp_source_info *info = dp_cust_get_source_info();

	if (!nb || !info)
		return -EINVAL;

	return blocking_notifier_chain_register(&info->notifier, nb);
}
EXPORT_SYMBOL(dp_source_reg_notifier);

int dp_source_unreg_notifier(struct notifier_block *nb)
{
	struct dp_source_info *info = dp_cust_get_source_info();

	if (!nb || !info)
		return -EINVAL;

	return blocking_notifier_chain_unregister(&info->notifier, nb);
}
EXPORT_SYMBOL(dp_source_unreg_notifier);

static int dp_source_parse_dts(struct dp_source_info *info)
{
	info->source_mode = dp_cust_get_property_value(info->node,
		"default_source_mode", SAME_SOURCE);
#ifdef CONFIG_HONOR_DP_FACTORY
	info->source_mode = SAME_SOURCE;
	hwlog_info("%s: only support same source in factory version\n", __func__);
#endif

	hwlog_info("%s: get source mode %d success\n", __func__, info->source_mode);
	return 0;
}

static ssize_t dp_source_mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct dp_source_info *info = dp_cust_get_source_info();

	UNUSED(dev);
	UNUSED(attr);
	if (!buf || !info)
		return -EINVAL;

	return scnprintf(buf, PAGE_SIZE, "%d\n", info->source_mode);
}

static ssize_t dp_source_mode_store(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	struct dp_source_info *info = dp_cust_get_source_info();
	int last_mode;
	int source_mode;
	int ret;

	UNUSED(dev);
	UNUSED(attr);
	if (!buf || !info)
		return -EINVAL;

	last_mode = info->source_mode;
	ret = kstrtoint(buf, 0, &source_mode);
	if (ret != 0) {
		hwlog_err("%s: kstrtoint error\n", __func__);
		return -EINVAL;
	}

	if (source_mode != SAME_SOURCE && source_mode != DIFF_SOURCE) {
		hwlog_err("%s: invalid source_mode %d\n", __func__, source_mode);
		return -EINVAL;
	}

	info->source_mode = source_mode;
	if (last_mode == info->source_mode) {
		hwlog_info("%s: sync framework source mode state %d\n",
			__func__, info->source_mode);
	} else {
		hwlog_info("%s: source_mode store %d success\n", __func__, info->source_mode);
		blocking_notifier_call_chain(&info->notifier, source_mode, NULL);
	}

	return count;
}

static struct device_attribute hw_dp_device_attr[] = {
	__ATTR(source_mode, 0644, dp_source_mode_show, dp_source_mode_store),
};

static void dp_source_dev_distroy(struct class *class)
{
	int i;

	for (i = 0; i < DP_DEVICE_NUM; i++) {
		if (!IS_ERR_OR_NULL(hw_dp_device[i].dev)) {
			device_destroy(class, hw_dp_device[i].dev->devt);
			hw_dp_device[i].dev = NULL;
		}
	}
}

static int dp_source_dev_create(struct class *class)
{
	int i;
	int ret = 0;

	for (i = 0; i < DP_DEVICE_NUM; i++) {
		hw_dp_device[i].dev = device_create(class, NULL, 0, NULL, hw_dp_device[i].name);
		if (IS_ERR_OR_NULL(hw_dp_device[i].dev)) {
			ret = PTR_ERR(hw_dp_device[i].dev);
			goto err_out;
		}
	}

	return ret;

err_out:
	dp_source_dev_distroy(class);
	return ret;
}

static int dp_source_create_file(void)
{
	int ret = 0;
	int count = 0;
	int i;

	for (i = 0; i < DP_DEVICE_NUM; i++) {
		ret = device_create_file(hw_dp_device[i].dev, &hw_dp_device_attr[i]);
		if (ret != 0) {
			hwlog_err("%s: %s create failed\n", __func__, hw_dp_device[i].name);
			count = i;
			goto err_out;
		}
	}

	return 0;

err_out:
	for (i = 0; i < count; i++)
		device_remove_file(hw_dp_device[i].dev, &hw_dp_device_attr[i]);
	return ret;
}

int dp_source_init(struct dp_source_info *info)
{
	struct class *dp_class = NULL;
	int ret;

	if (!info || !info->node)
		return -EINVAL;

	ret = dp_source_parse_dts(info);
	if (ret < 0)
		return ret;

	dp_class = class_create(THIS_MODULE, "dp");
	if (IS_ERR(dp_class)) {
		hwlog_err("%s: create dp class failed\n", __func__);
		return PTR_ERR(dp_class);
	}

	ret = dp_source_dev_create(dp_class);
	if (ret < 0)
		goto exit_out_dev;

	ret = dp_source_create_file();
	if (ret < 0)
		goto exit_out_file;

	info->notifier.rwsem = (struct rw_semaphore)__RWSEM_INITIALIZER((info->notifier).rwsem);
	info->notifier.head = NULL;

	info->class = dp_class;
	hwlog_info("%s: init success\n", __func__);
	return 0;

exit_out_file:
	dp_source_dev_distroy(dp_class);
exit_out_dev:
	class_destroy(dp_class);
	return ret;
}

void dp_source_exit(struct dp_source_info *info)
{
	int i;

	if (!info || !info->class)
		return;

	for (i = 0; i < DP_DEVICE_NUM; i++) {
		if (!IS_ERR_OR_NULL(hw_dp_device[i].dev)) {
			device_remove_file(hw_dp_device[i].dev, &hw_dp_device_attr[i]);
			device_destroy(info->class, hw_dp_device[i].dev->devt);
			hw_dp_device[i].dev = NULL;
		}
	}

	class_destroy(info->class);
	info->class = NULL;
}

MODULE_DESCRIPTION("dp source driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
