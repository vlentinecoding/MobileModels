/*
 * dp_dsm.h
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

#ifndef DP_DSM_H
#define DP_DSM_H

#include <linux/workqueue.h>
#include <linux/soc/qcom/dp_cust_interface.h>

enum dp_imonitor_type {
	DP_IMONITOR_TYPE,
	DP_IMONITOR_BASIC_INFO = DP_IMONITOR_TYPE,
	DP_IMONITOR_TIME_INFO,
	DP_IMONITOR_EXTEND_INFO,
	DP_IMONITOR_HPD,
	DP_IMONITOR_LINK_TRAINING,
	DP_IMONITOR_HOTPLUG,
	DP_IMONITOR_HDCP,

	DP_IMONITOR_TYPE_NUM,
};

struct dp_dsm_info {
	struct work_struct report_work;
	enum dp_imonitor_type report_type;
	uint32_t report_num[DP_IMONITOR_TYPE_NUM];
	uint32_t last_report_num[DP_IMONITOR_TYPE_NUM];
	unsigned long report_jiffies;
	bool report_skip_existed; // reported events skipped last day

	int cable_type;        // "CableType"
	int hotplug_retval;    // "RedidRet"
	int link_train_retval; // "LinkRet"
	int aux_rw_retval;     // "AuxRet"
	int aux_rw_count;      // "AuxLen"
	bool safe_mode;        // "SafeMode"
	bool connect_timeout;  // "AuxRw"
};

#ifdef CONFIG_HONOR_DP_CUST
void dp_dsm_imonitor_report(enum dp_imonitor_type type);
void dp_dsm_init_link_status(struct dp_dsm_info *info);
int dp_dsm_init(struct dp_dsm_info *info);
void dp_dsm_exit(struct dp_dsm_info *info);
#else
static inline void dp_dsm_imonitor_report(enum dp_imonitor_type type)
{
}

static inline void dp_dsm_init_link_status(struct dp_dsm_info *info)
{
}

static inline int dp_dsm_init(struct dp_dsm_info *info)
{
	return 0;
}

static inline void dp_dsm_exit(struct dp_dsm_info *info)
{
}
#endif // !CONFIG_HONOR_DP_CUST

#endif // DP_DSM_H
