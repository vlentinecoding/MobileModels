/*
 * Copyright (c) Honor power team. All rights reserved.
 */

#include <linux/slab.h>
#include <chipset_common/dubai/dubai_plat.h>
#include <linux/devfreq.h>

extern struct devfreq *g_devfreq_dubai;
extern int honor_dubai_get_gpu_info(struct devfreq *g_devfreq_dubai);
extern unsigned int honor_dubai_get_gpu_freq_cnt(struct devfreq *g_devfreq_dubai);

static atomic_t stats_enable = ATOMIC_INIT(0);

static int dubai_set_gpu_enable(bool enable)
{
	atomic_set(&stats_enable, enable ? 1 : 0);
	dubai_info("Gpu stats enable: %d", enable ? 1 : 0);

	return 0;
}

static int dubai_get_gpu_freq_num(void)
{
	return honor_dubai_get_gpu_freq_cnt(g_devfreq_dubai);
}

static int dubai_get_gpu_info(struct dubai_gpu_freq_info *data, int num)
{
	int ret, i, freq_num;

	if (!atomic_read(&stats_enable))
		return -EPERM;

	freq_num = dubai_get_gpu_freq_num();
	if (!data || (num != freq_num)) {
		dubai_err("Invalid param: %d, %d", num, freq_num);
		ret = -EINVAL;
		goto end;
	}

	ret = honor_dubai_get_gpu_info(g_devfreq_dubai);
	if (ret) {
		dubai_err("Failed to get gpu stats");
		goto end;
	}

	for (i = 0; i < freq_num; i++) {
		data[i].freq = g_devfreq_dubai->profile->freq_table[i];
		data[i].run_time = jiffies_to_usecs(g_devfreq_dubai->time_in_state[i]);
		data[i].idle_time = 0;
	}

end:
	return ret;
}

static struct dubai_gpu_stats_ops gpu_ops = {
	.enable = dubai_set_gpu_enable,
	.get_num = dubai_get_gpu_freq_num,
	.get_stats = dubai_get_gpu_info,
};

void dubai_gpu_freq_stats_init(void)
{
	dubai_register_module_ops(DUBAI_MODULE_GPU, &gpu_ops);
}

void dubai_gpu_freq_stats_exit(void)
{
	dubai_unregister_module_ops(DUBAI_MODULE_GPU);
	atomic_set(&stats_enable, 0);
}
