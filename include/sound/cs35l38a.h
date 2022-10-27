/*
 * linux/sound/cs35l38a.h -- Platform data for CS35L38A
 *
 * Copyright (c) 2020 Cirrus Logic Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __CS35L38A_H
#define __CS35L38A_H

/* INT/GPIO pin configuration */
struct irq_cfg {
	bool is_present;
	int irq_drv_sel;
	int irq_pol;
	int irq_gpio_sel;
	int irq_out_en;
	int irq_src_sel;
};

struct cs35l38a_platform_data {
	bool sclk_frc;
	bool lrclk_frc;
	bool multi_amp_mode;
	bool dcm_mode;
	int ldm_mode_sel;
	bool amp_gain_zc;
	bool amp_pcm_inv;
	bool pdm_ldm_exit;
	bool pdm_ldm_enter;
	bool imon_pol_inv;
	bool vmon_pol_inv;
	int boost_ind;
	int bst_vctl;
	int bst_vctl_sel;
	int bst_ipk;
	bool extern_boost;
	int temp_warn_thld;
	struct irq_cfg irq_config;
	//for spk safe
	int sub_spk_safe_mode;
	//for low  temp low voltage protect
	int final_digital_gain;
	int final_decay_gain;
	struct workqueue_struct *cs35l38a_monitor_work_queue;
	struct work_struct cs35l38a_monitor_work_sturct;
	atomic_t monitor_enable;
};

#endif /* __CS35L38A_H */
