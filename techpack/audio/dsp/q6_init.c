// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2017, 2019-2020 The Linux Foundation. All rights reserved.
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include "q6_init.h"
#ifndef CONFIG_HONOR_GKI_V1
#include "sound/hw_audio/hw_audio_interface.h"
#endif

static int __init audio_q6_init(void)
{
	adsp_err_init();
	audio_cal_init();
	rtac_init();
	adm_init();
	afe_init();
	spk_params_init();
	q6asm_init();
	q6lsm_init();
	voice_init();
	core_init();
	msm_audio_ion_init();
	audio_slimslave_init();
	avtimer_init();
#ifndef CONFIG_HONOR_GKI_V1
#ifdef CONFIG_SND_SOC_CS35LXX
	if (hw_get_smartpa_type() == CS35LXX || hw_get_smartpa_type() == CS35LXXA)
		crus_sp_init();
#endif  /* CONFIG_SND_SOC_CS35LXX */
#endif /* CONFIG_HONOR_GKI_V1 */
	msm_mdf_init();
	voice_mhi_init();
	digital_cdc_rsc_mgr_init();
#ifdef CONFIG_AUDIO_QGKI
#ifdef CONFIG_ELUS_ENABLE
    elliptic_driver_init();
#endif /* CONFIG_ELUS_ENABLE */
#endif /* CONFIG_AUDIO_QGKI */
	return 0;
}

static void __exit audio_q6_exit(void)
{
	digital_cdc_rsc_mgr_exit();
	msm_mdf_exit();
#ifndef CONFIG_HONOR_GKI_V1
#ifdef CONFIG_SND_SOC_CS35LXX
	if (hw_get_smartpa_type() == CS35LXX || hw_get_smartpa_type() == CS35LXXA)
		crus_sp_exit();
#endif  /* CONFIG_SND_SOC_CS35LXX */
#endif /* CONFIG_HONOR_GKI_V1 */
	avtimer_exit();
	audio_slimslave_exit();
	msm_audio_ion_exit();
	core_exit();
	voice_exit();
	q6lsm_exit();
	q6asm_exit();
	afe_exit();
	spk_params_exit();
	adm_exit();
	rtac_exit();
	audio_cal_exit();
	adsp_err_exit();
	voice_mhi_exit();
#ifdef CONFIG_AUDIO_QGKI
#ifdef CONFIG_ELUS_ENABLE
    elliptic_driver_exit();
#endif /* CONFIG_ELUS_ENABLE */
#endif /* CONFIG_AUDIO_QGKI */
}

module_init(audio_q6_init);
module_exit(audio_q6_exit);
MODULE_DESCRIPTION("Q6 module");
MODULE_LICENSE("GPL v2");
