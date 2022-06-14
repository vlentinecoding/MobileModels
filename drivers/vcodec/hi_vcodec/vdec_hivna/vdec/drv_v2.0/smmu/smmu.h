/*
 * smmu.h
 *
 * This is for smmu driver.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
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

#ifndef HIVDEC_SMMU_H
#define HIVDEC_SMMU_H

#define SMMU_OK     0
#define SMMU_ERR   (-1)

struct smmu_reg_info {
	hi_u8  *smmu_tbu_reg_vir;
	hi_u8  *smmu_sid_reg_vir;
};

struct smmu_tbu_info {
	hi_u32 mmu_tbu_num;
	hi_u32 mmu_tbu_offset;
	hi_u32 mmu_sid_offset;
};

struct smmu_entry {
	struct smmu_reg_info reg_info;
	hi_u8 smmu_init;
	struct smmu_tbu_info tbu_info;
};

hi_s32 smmu_map_reg(void);
void smmu_unmap_reg(void);

hi_s32 smmu_init(void);
void smmu_deinit(void);

#endif

