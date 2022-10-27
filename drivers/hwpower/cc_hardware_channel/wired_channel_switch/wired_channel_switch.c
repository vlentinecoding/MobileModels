/*
 * wired_channel_switch.c
 *
 * wired channel switch
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwpower/wired_channel_switch.h>
#include <huawei_platform/power/common_module/power_platform.h>
#include <chipset_common/hwpower/power_printk.h>
#include <chipset_common/hwpower/power_vote.h>

#define HWLOG_TAG wired_chsw
HWLOG_REGIST();

static struct wired_chsw_device_ops *g_chsw_ops;


static const char * const g_chsw_channel_type_table[] = {
	[WIRED_CHANNEL_MAIN] = "wired_channel_main",
	[WIRED_CHANNEL_AUX] = "wired_channel_aux",
	[WIRED_CHANNEL_ALL] = "wired_channel_all",
};

struct wird_chsw_dev {
	struct device *dev;
};

static struct wird_chsw_dev *g_wired_chsw_dev;

static int wird_chsw_vote_callback(struct power_vote_object *obj,
	void *data, int result, const char *client_str)
{
	struct wird_chsw_dev *l_dev = (struct wird_chsw_dev *)data;

	if (!l_dev || !client_str) {
		hwlog_err("l_dev or client_str is null\n");
		return -EINVAL;
	}

	hwlog_info("result=%d client_str=%s\n", result, client_str);
	//reslut: 1: close 0: open
	g_chsw_ops->set_wired_channel(WIRED_CHANNEL_ALL, result);

	return 0;
}

static const char *wired_chsw_get_channel_type_string(int type)
{
	if ((type >= WIRED_CHANNEL_BEGIN) && (type < WIRED_CHANNEL_END))
		return g_chsw_channel_type_table[type];

	return "illegal channel type";
}

int wired_chsw_ops_register(struct wired_chsw_device_ops *ops)
{
	if (ops && !g_chsw_ops) {
		g_chsw_ops = ops;
		hwlog_info("wired_chsw ops register ok\n");
		return 0;
	}

	hwlog_err("wired_chsw ops register fail\n");
	return -EPERM;
}

int wired_chsw_get_wired_channel(int channel_type)
{
	if (!g_chsw_ops || !g_chsw_ops->get_wired_channel) {
		hwlog_err("g_chsw_ops or get_wired_channel is null\n");
		return WIRED_CHANNEL_RESTORE;
	}

	if ((channel_type < WIRED_CHANNEL_BEGIN) || (channel_type >= WIRED_CHANNEL_END))
		return WIRED_CHANNEL_RESTORE;

	return g_chsw_ops->get_wired_channel(channel_type);
}


int wired_chsw_set_wired_channel(int channel_type, const char *client_name, int state)
{
	if (!g_chsw_ops || !g_chsw_ops->set_wired_channel) {
		hwlog_err("g_chsw_ops or set_wired_channel is null\n");
		return 0;
	}

	if ((channel_type < WIRED_CHANNEL_BEGIN) || (channel_type >= WIRED_CHANNEL_END) || (channel_type == WIRED_CHANNEL_AUX))
		return 0;

	power_vote_set(WIRED_CHSW_VOTE_OBJECT, client_name, state, state);

	return 0;
}

int wired_chsw_set_wired_channel_wireless(int channel_type, int state)
{
	int ret = 0;
	int new_state;

	if (!g_chsw_ops || !g_chsw_ops->set_wired_channel) {
		hwlog_err("g_chsw_ops or set_wired_channel is null\n");
		return 0;
	}

	if ((channel_type < WIRED_CHANNEL_BEGIN) || (channel_type >= WIRED_CHANNEL_END))
		return 0;

	new_state = wired_chsw_get_wired_channel(channel_type);
	if (state == new_state) {
		hwlog_info("%s is already %s\n", wired_chsw_get_channel_type_string(channel_type),
			((new_state == WIRED_CHANNEL_RESTORE) ? "on" : "off"));
		return 0;
	}
	power_vote_set(WIRED_CHSW_VOTE_OBJECT, WIRED_CHSW_CLIENT_WLC_TX, state, state);
	if (!ret) {
		new_state = wired_chsw_get_wired_channel(channel_type);
		hwlog_info("%s is set to %s\n", wired_chsw_get_channel_type_string(channel_type),
			((new_state == WIRED_CHANNEL_RESTORE) ? "on" : "off"));
	}

	return 0;
}

int wired_chsw_set_wired_reverse_channel(int state)
{
	if (!g_chsw_ops || !g_chsw_ops->set_wired_reverse_channel) {
		hwlog_err("g_chsw_ops or set_wired_reverse_channel is null\n");
		return -1;
	}

	return g_chsw_ops->set_wired_reverse_channel(state);
}

static int wired_chsw_check_ops(void)
{
	if (!g_chsw_ops || !g_chsw_ops->set_wired_channel) {
		hwlog_err("g_chsw_ops is null\n");
		return -EINVAL;
	}

	return 0;
}

static int wired_chsw_probe(struct platform_device *pdev)
{
	int ret;
	struct wird_chsw_dev *di;

	ret = wired_chsw_check_ops();
	if (ret)
		return -1;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_wired_chsw_dev = di;

	power_vote_create_object(WIRED_CHSW_VOTE_OBJECT, POWER_VOTE_SET_ANY,
		wird_chsw_vote_callback, g_wired_chsw_dev);

	return 0;
}

static const struct of_device_id wired_chsw_match_table[] = {
	{
		.compatible = "huawei,wired_channel_switch",
		.data = NULL,
	},
	{},
};

static struct platform_driver wired_chsw_driver = {
	.probe = wired_chsw_probe,
	.driver = {
		.name = "huawei,wired_channel_switch",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(wired_chsw_match_table),
	},
};

static int __init wired_chsw_init(void)
{
	return platform_driver_register(&wired_chsw_driver);
}

static void __exit wired_chsw_exit(void)
{
	struct wird_chsw_dev *di = g_wired_chsw_dev;
	if (!di)
		return;

	kfree(di);
	g_wired_chsw_dev = NULL;
	platform_driver_unregister(&wired_chsw_driver);
}

module_init(wired_chsw_init);
module_exit(wired_chsw_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("wired channel switch module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
