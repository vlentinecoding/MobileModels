/*
 * npu_pm_config_smmutbu.c
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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

#include "npu_adapter.h"
#include "npu_platform.h"
#include "npu_proc_ctx.h"

int npu_smmu_tbu_powerup(int work_mode, uint32_t subip_set, void **para)
{
	int ret = 0;
	struct npu_platform_info *plat_info = NULL;

	unused(subip_set);
	unused(para);

	plat_info = npu_plat_get_info();
	cond_return_error(plat_info == NULL, -1, "npu_plat_get_info failed\n");

	if (work_mode != NPU_SEC)
		ret = npu_plat_powerup_smmu(plat_info->pdev);
	return ret;
}

int npu_smmu_tbu_powerdown(int work_mode, uint32_t subip_set, void *para)
{
	int ret = 0;
	struct npu_proc_ctx *proc_ctx = NULL;

	unused(work_mode);
	unused(subip_set);

	proc_ctx = (struct npu_proc_ctx *)para;

	if (work_mode != NPU_SEC)
		ret = npu_plat_poweroff_smmu(proc_ctx->devid);
	return ret;
}
