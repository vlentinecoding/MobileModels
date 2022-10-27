/*
 * Copyright (C) 2021 Honor Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is used for collect ddr info by dubai, and is distributed
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY.
 *
 * All rights reserved.
 */

#include <linux/slab.h>
#include <chipset_common/dubai/dubai_plat.h>
#include "dubai_qcom_plat.h"
#include "ddr_stats.h"

#define DDR_BUS_BIT 16
#define DDR_CNT 4
#define DOUBLE_SAMPLING 2
#define BITS_PER_BYTE 8

#define FLUX_COEFFICIENT (1000 * DDR_BUS_BIT * DDR_CNT * DOUBLE_SAMPLING / BITS_PER_BYTE)

static int32_t g_nr_freq = 0;
const static int32_t g_nr_ip = 1;
extern struct stats_entry *g_stats_entry;

extern int honor_ddr_freq_cnt_get(void);
extern int honor_ddr_time_in_freq_get(void);

static int32_t dubai_get_ddr_freq_cnt(void)
{
	return honor_ddr_freq_cnt_get();
}

static int32_t dubai_update_ddr_time_in_state(void)
{
	int32_t ret;

	ret = honor_ddr_time_in_freq_get();
	if (ret != g_nr_freq) {
		dubai_err("Failed to get ddr active times: %d", ret);
		return -1;
	}

	return 0;
}

static int32_t dubai_get_ddr_time_in_state(struct dubai_ddr_time_in_state *time_in_state)
{
	int32_t i, ret;
	int64_t total_time = 0;

	g_nr_freq = dubai_get_ddr_freq_cnt();
	if (g_nr_freq <= 0) {
		dubai_err("Failed to check ddr stats");
		return -1;
	}

	ret = dubai_update_ddr_time_in_state();
	if (ret < 0) {
		dubai_err("Faild to update ddr time_in_state stats");
		return -1;
	}

	for (i = 0; i < g_nr_freq; i++) {
		time_in_state->freq_time[i].freq = g_stats_entry[i].name;
		time_in_state->freq_time[i].time = g_stats_entry[i].duration;
		total_time += time_in_state->freq_time[i].time;
	}

	time_in_state->pd_time = total_time >> 1;
	time_in_state->sr_time = total_time - time_in_state->pd_time;

	return 0;
}

static int32_t dubai_get_ddr_ip_cnt(void)
{
	return g_nr_ip;
}

static int32_t dubai_get_ddr_ip_stats(int32_t ip_cnt, struct dubai_ddr_ip_stats *ip_stats)
{
	int32_t i, j;
	static int64_t cnt = 1;

	if ((ip_cnt < g_nr_ip) || (g_nr_freq > DDR_FREQ_CNT_MAX) || !ip_stats) {
		dubai_err("Invalid parameter");
		return -1;
	}

	for (i = 0; i < g_nr_ip; i++) {
		strcpy(ip_stats[i].ip, "TOTAL_10000");
		for (j = 0; j < g_nr_freq; j++) {
			ip_stats[i].freq_data[j].freq = g_stats_entry[j].name;
			ip_stats[i].freq_data[j].data =
				FLUX_COEFFICIENT * g_stats_entry[j].duration * g_stats_entry[j].name;
		}
		cnt += 1;
	}

	return 0;
}

static struct dubai_ddr_stats_ops ddr_ops = {
	.get_freq_cnt = dubai_get_ddr_freq_cnt,
	.get_time_in_state = dubai_get_ddr_time_in_state,
	.get_ip_cnt = dubai_get_ddr_ip_cnt,
	.get_ip_stats = dubai_get_ddr_ip_stats,
};

void dubai_qcom_ddr_stats_init(void)
{
	dubai_register_module_ops(DUBAI_MODULE_DDR, &ddr_ops);
}

void dubai_qcom_ddr_stats_exit(void)
{
	dubai_unregister_module_ops(DUBAI_MODULE_DDR);
}
