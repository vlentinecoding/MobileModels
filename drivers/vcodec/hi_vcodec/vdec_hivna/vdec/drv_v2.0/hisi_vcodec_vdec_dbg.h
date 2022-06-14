/*
 * hisi_vcodec_vdec_dbg.h
 *
 * This is for vdec debug
 *
 * Copyright (c) 2020-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef HISI_VCODEC_VDEC_DBG
#define HISI_VCODEC_VDEC_DBG

#include <linux/device.h>

#define MAX_SETTING_STRING_LEN 64

enum vdec_setting_option_type {
	SETTING_OPT_TYPE_BOOL,
	SETTING_OPT_TYPE_INT,
	SETTING_OPT_TYPE_UINT,
	SETTING_OPT_TYPE_INT64,
	SETTING_OPT_TYPE_UINT64,
	SETTING_OPT_TYPE_STRING,
};

struct vdec_setting_option {
	const char *name;
	const char *help;
	const size_t offset;
	const enum vdec_setting_option_type type;
	union {
		bool enable;
		int     i32;
		long long i64;
		char    *str;
	} default_value;
	const long long min_value;
	const long long max_value;
};

/* IMPORTANT: the length of array in vdec_setting_info should be equal MAX_SETTING_STRING_LEN */
struct vdec_setting_info {
	bool enable_power_off_when_vdec_idle;
};

void vdec_init_setting_info(const struct vdec_setting_info *info);

#ifdef ENABLE_VDEC_PROC

ssize_t vdec_show_setting_info(char *buf, const size_t count, const struct vdec_setting_info *setting_info);
ssize_t vdec_store_setting_info(struct vdec_setting_info *setting_info, const char *buf, const size_t count);

#endif

#endif
