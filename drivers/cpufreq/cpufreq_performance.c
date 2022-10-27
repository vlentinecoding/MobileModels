// SPDX-License-Identifier: GPL-2.0-only
/*
 *  linux/drivers/cpufreq/cpufreq_performance.c
 *
 *  Copyright (C) 2002 - 2003 Dominik Brodowski <linux@brodo.de>
 */

#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/cpufreq.h>
#include <linux/init.h>
#include <linux/module.h>

#define BOOT_MAX_FREQ_NAME "bootmaxfreq="
static int boot_maxfreq;

static void init_boot_maxfreq(void)
{
	char *ptr;

	ptr = strstr(saved_command_line, BOOT_MAX_FREQ_NAME);
	if (!ptr) {
		boot_maxfreq = -1;
	} else {
		ptr += strlen(BOOT_MAX_FREQ_NAME);
		boot_maxfreq = simple_strtol(ptr, NULL, 10);
	}
}

static unsigned int find_next_max_freq(struct cpufreq_policy *policy) {
	struct cpufreq_frequency_table *freq_table = policy->freq_table;
	struct cpufreq_frequency_table *pos;
	unsigned int next_max = freq_table->frequency;
	unsigned int freq;

	cpufreq_for_each_valid_entry(pos, freq_table) {
		freq = pos->frequency;
		if (freq >= boot_maxfreq)
			continue;

		if (freq > next_max) {
			next_max = freq;
		}
	}

	return next_max;
}

static void cpufreq_gov_performance_limits(struct cpufreq_policy *policy)
{
	int freq;

	if (boot_maxfreq < 0) {
		pr_err("setting to %u kHz\n", policy->max);
		__cpufreq_driver_target(policy, policy->max, CPUFREQ_RELATION_H);
		return;
	}

	freq = find_next_max_freq(policy);
	pr_err("setting to %u kHz\n", freq);
	__cpufreq_driver_target(policy, freq, CPUFREQ_RELATION_H);
}

static struct cpufreq_governor cpufreq_gov_performance = {
	.name		= "performance",
	.owner		= THIS_MODULE,
	.limits		= cpufreq_gov_performance_limits,
};

static int __init cpufreq_gov_performance_init(void)
{
	init_boot_maxfreq();
	pr_err("boot_maxfreq %d kHz\n", boot_maxfreq);
	return cpufreq_register_governor(&cpufreq_gov_performance);
}

static void __exit cpufreq_gov_performance_exit(void)
{
	cpufreq_unregister_governor(&cpufreq_gov_performance);
}

#ifdef CONFIG_CPU_FREQ_DEFAULT_GOV_PERFORMANCE
struct cpufreq_governor *cpufreq_default_governor(void)
{
	return &cpufreq_gov_performance;
}
#endif
#ifndef CONFIG_CPU_FREQ_GOV_PERFORMANCE_MODULE
struct cpufreq_governor *cpufreq_fallback_governor(void)
{
	return &cpufreq_gov_performance;
}
#endif

MODULE_AUTHOR("Dominik Brodowski <linux@brodo.de>");
MODULE_DESCRIPTION("CPUfreq policy governor 'performance'");
MODULE_LICENSE("GPL");

core_initcall(cpufreq_gov_performance_init);
module_exit(cpufreq_gov_performance_exit);
