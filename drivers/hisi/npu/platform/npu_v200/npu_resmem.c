/*
 * npu_resmem.c
 *
 * about npu resmem
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
#include "npu_resmem.h"

#include <linux/of.h>

#include "npu_log.h"

void npu_plat_set_resmem_desc(
	struct npu_mem_desc *resmem_desc, u32 base, u32 len)
{
	if (resmem_desc != NULL) {
		resmem_desc->base = base;
		resmem_desc->len = len;
	}
}

void npu_plat_set_resmem_info(struct npu_resmem_info *resmem_info)
{
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_TSCPU_FW_IDX]),
		NPU_NS_TSCPU_FW_BASE_ADDR, NPU_NS_TSCPU_FW_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_CTRL_INFO_IDX]),
		NPU_NS_CTRL_INFO_BASE_ADDR, NPU_NS_CTRL_INFO_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_LOG_MEM_IDX]),
		NPU_NS_LOG_BASE_ADDR, NPU_NS_LOG_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_BBOX_MEM_IDX]),
		NPU_NS_BBOX_BASE_ADDR, NPU_NS_BBOX_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_COMM_SQCQ_RESMEM_IDX]),
		NPU_NS_COMM_SQCQ_BASE_ADDR, NPU_NS_COMM_SQCQ_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_DFX_SQCQ_RESMEM_IDX]),
		NPU_NS_DFX_SQCQ_BASE_ADDR, NPU_NS_DFX_SQCQ_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_HWTS_SDMA_SQCQ_RESMEM_IDX]),
		NPU_NS_HWTS_SDMA_SQ_BASE_ADDR, NPU_NS_HWTS_SDMA_SQ_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_TS_SDMA_SQCQ_RESMEM_IDX]),
		NPU_NS_TS_SDMA_SQCQ_BASE_ADDR, NPU_NS_TS_SDMA_SQCQ_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_TS_CONFIG_RESMEM_IDX]),
		NPU_NS_TS_CONFIG_BASE_ADDR, NPU_NS_TS_CONFIG_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_desc[NPU_CHIP_CFG_IDX]),
		NPU_NS_CHIP_CFG_BASE_ADDR, NPU_NS_CHIP_CFG_SIZE);

	npu_plat_set_resmem_desc(&(resmem_info->resmem_sec_desc[TSCPU_EL3_S]),
		NPU_S_TSCPU_FW_EL3_ADDR, NPU_S_TSCPU_EL3_SIZE);
	npu_plat_set_resmem_desc(&(resmem_info->resmem_sec_desc[TSCPU_EL1_S]),
		NPU_S_TSCPU_FW_EL1_ADDR, NPU_S_TSCPU_EL1_SIZE);
}

int npu_plat_set_resmem(struct npu_platform_info *plat_info)
{
	struct npu_resmem_info *resmem_info = &(plat_info->resmem_info);

	npu_plat_set_resmem_info(resmem_info);
	if (plat_info->dts_info.feature_switch[NPU_FEATURE_KERNEL_LOAD_IMG])
		resmem_info->tsfw_buf = &(resmem_info->resmem_desc[NPU_TSCPU_FW_IDX]);
	else
		resmem_info->tsfw_buf = &(resmem_info->resmem_sec_desc[TSCPU_EL3_S]);
	resmem_info->info_buf = &(resmem_info->resmem_desc[NPU_CTRL_INFO_IDX]);
	resmem_info->sqcq_buf = &(resmem_info->resmem_desc[NPU_COMM_SQCQ_RESMEM_IDX]);
	resmem_info->sdma_sq_buf = &(resmem_info->resmem_desc[NPU_HWTS_SDMA_SQCQ_RESMEM_IDX]);
	resmem_info->chip_cfg_buf = &(resmem_info->resmem_desc[NPU_CHIP_CFG_IDX]);

	return 0;
}
