/*
 * smmu.c
 *
 * This is for vdec smmu.
 *
 * Copyright (c) 2010-2020 Huawei Technologies CO., Ltd.
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

#include "smmu.h"
#include <linux/delay.h>
#include <linux/regulator/consumer.h>
#include "smmu_regs.h"
#include "soc_venc_reg_interface.h"
#include "drv_mem.h"
#include "drv_common.h"
#include "venc_regulator.h"
#include "tvp_adapter.h"

#define SMR_ID_SIZE	31
#define HIVDEC_SMMU_TBU_OFFSET	0x20000
#define HIVDEC_SMMU_TCU_OFFSET	0x1A000
#define HIVDEC_SMMU_PRE_OFFSET	0x18000

struct smmu_virt_addr {
	int32_t *tbu_virt_addr;
	int32_t *pre_virt_addr;
};

struct smmu_entry {
	struct smmu_virt_addr reg_virt;
	bool smmu_init;
	struct regulator *tcu_regulator;
};

struct smmu_entry g_venc_smmu_entry;

struct smmu_entry *venc_smmu_get_entry()
{
	return &g_venc_smmu_entry;
}

#define tbu_reg_virt() (venc_smmu_get_entry()->reg_virt.tbu_virt_addr)
#define pre_reg_virt() (venc_smmu_get_entry()->reg_virt.pre_virt_addr)

#define rd_smmu_tbu_vreg(reg, dat)	\
do {	\
	(dat) = *((volatile uint32_t *)((int8_t *)tbu_reg_virt() + (reg))); \
} while (0)

#define wr_smmu_tbu_vreg(reg, dat)	\
do {	\
	*((volatile uint32_t *)((int8_t *)tbu_reg_virt() + (reg))) = (dat); \
} while (0)

static void set_tbu_reg(int32_t addr, uint32_t val, uint32_t bw, uint32_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp = 0;

	rd_smmu_tbu_vreg(addr, tmp);
	tmp &= ~(mask << bs);
	wr_smmu_tbu_vreg(addr, tmp | ((val & mask) << bs));
}

#define rd_smmu_pre_vreg(reg, dat)		\
do {	\
	(dat) = *((volatile uint32_t *)((int8_t *)pre_reg_virt() + (reg))); \
} while (0)

#define wr_smmu_pre_vreg(reg, dat)		\
do {	\
	*((volatile uint32_t *)((int8_t *)pre_reg_virt() + (reg))) = (dat); \
} while (0)


static void set_pre_reg(int32_t addr, uint32_t val, uint32_t bw, uint32_t bs)
{
	uint32_t mask = (1UL << bw) - 1UL;
	uint32_t tmp;

	rd_smmu_pre_vreg(addr, tmp);
	tmp &= ~(mask << bs);
	wr_smmu_pre_vreg(addr, tmp | ((val & mask) << bs));
}


int32_t venc_smmu_cfg(struct encode_info *channelcfg, uint32_t *reg_base)
{
	int i;
	struct smmu_entry *entry = venc_smmu_get_entry();

	if (entry->smmu_init != true) {
		HI_FATAL_VENC(" tbu is not init");
		return SMMU_ERR;
	}

	/* tbu req init */
	set_tbu_reg(SMMU_TBU_IRPT_CLR_NS, 0xFFFFFFFF, 32, 0); // 32: bit width
	set_tbu_reg(SMMU_TBU_IRPT_MASK_NS, 0x0, 32, 0); // 32: bit width
	set_tbu_reg(SMMU_TBU_IRPT_CLR_S, 0xFFFFFFFF, 32, 0); // 32: bit width
	set_tbu_reg(SMMU_TBU_IRPT_MASK_S, 0X0, 32, 0); // 32: bit width

	/* tbu pmu */
	for (i = 0; i < 16; i++)
		set_tbu_reg(SMMU_TBU_PMCG_EVCNTR + i * 0x4, 0x0, 32, 0); // 32: bit width
	set_tbu_reg(SMMU_TBU_PMCG_CNTENSET0_0, 0xFFFF, 20, 0); // 20: bit width
	for (i = 0; i < 16; i++)
		set_tbu_reg(SMMU_TBU_PMCG_EVTYPER_N + i * 0x4, 0x80 + i, 8, 0); // 8: bit width
	set_tbu_reg(SMMU_TBU_PMCG_SMR, 0xFFFFFFFF, 32, 0); // 32: bit width
	set_tbu_reg(SMMU_TBU_PMCG_CR, 0x1, 1, 0);
	set_tbu_reg(SMMU_TBU_PMCG_CAPR, 0x1, 1, 0);

	return SMMU_OK;
}

int32_t venc_open_tbu()
{
	uint32_t tbu_crack_val = 0;
	uint32_t tbu_cr_val = 0;
	uint32_t query_interval_ms = 1;
	uint32_t max_query_cnt = 1000;
	uint32_t try_cnt;

	/* smmu pre cfg */
	set_pre_reg(SMMU_PRE_SID, 0x9, 8, 0); // 8: bit width
	set_pre_reg(SMMU_PRE_SSID, 0x1, 8, 0); // 8: bit width
	set_pre_reg(SMMU_PRE_SSIDV, 0x1, 4, 0); // 4: bit width

	/* open tbu */
	set_tbu_reg(SMMU_TBU_SCR, 0x1, 1, 0);
	set_tbu_reg(SMMU_TBU_CR, 0x17, 8, 8); // 8: bit width, 8: bit start
	set_tbu_reg(SMMU_TBU_CR, 0x1, 1, 0);

	for (try_cnt = 0; try_cnt < max_query_cnt; try_cnt++) {
		rd_smmu_tbu_vreg(SMMU_TBU_CRACK, tbu_crack_val);
		if ((tbu_crack_val & 0x3) == 0x3)
			break;
		udelay(query_interval_ms);
	}
	if (try_cnt == max_query_cnt) {
		HI_FATAL_VENC("tbu connect with tcu is failed");
		return SMMU_ERR;
	}

	rd_smmu_tbu_vreg(SMMU_TBU_CR, tbu_cr_val);
	if ((tbu_crack_val & 0xFF00) < (tbu_cr_val & 0xFF00)) {
		HI_FATAL_VENC("check tok_trans_gnt failed, tok_trans_gnt is %d", tbu_crack_val);
		return SMMU_ERR;
	}

	return SMMU_OK;
}

void venc_close_tbu()
{
	uint32_t tbu_crack_val = 0;
	uint32_t query_interval_ms = 1;
	uint32_t max_query_cnt = 1000;
	uint32_t try_cnt;

	set_tbu_reg(SMMU_TBU_CR, 0x0, 1, 0);
	for (try_cnt = 0; try_cnt < max_query_cnt; try_cnt++) {
		rd_smmu_tbu_vreg(SMMU_TBU_CRACK, tbu_crack_val);
		if ((tbu_crack_val & 0x3) == 0x1)
			break;

		udelay(query_interval_ms);
	}
	if (try_cnt == max_query_cnt)
		HI_FATAL_VENC("tbu disconnect with tcu is failed");
}

int32_t venc_tcu_regulator_enable()
{
	const char* tcu_regulator_name = "ldo_smmu_tcu";
	struct smmu_entry *entry = venc_smmu_get_entry();

	entry->tcu_regulator = devm_regulator_get(&(venc_get_device()->dev), tcu_regulator_name);
	if (IS_ERR_OR_NULL(entry->tcu_regulator)) {
		HI_FATAL_VENC("get tcu regulator failed, error no is %ld", PTR_ERR(entry->tcu_regulator));
		entry->tcu_regulator = HI_NULL;
		return HI_FAILURE;
	}

	if (regulator_enable(entry->tcu_regulator)) {
		HI_FATAL_VENC("enable smmu tcu regulator failed");
		return SMMU_ERR;
	}

	return SMMU_OK;
}

void venc_tcu_regulator_disable()
{
	struct smmu_entry *entry = venc_smmu_get_entry();

	if (regulator_disable(entry->tcu_regulator))
		HI_FATAL_VENC("disable smmu tcu regulator failed");
}

int32_t venc_smmu_init(bool is_protected, int32_t core_id)
{
#ifdef SUPPORT_SECURE_VENC
	if (is_protected == true) {
		HI_DBG_VENC("config_master smmu reg to secure");
		config_master(SEC_VENC_ON, core_id);
	} else {
		HI_DBG_VENC("reset_master smmu reg to non-secure");
		config_master(SEC_VENC_OFF, core_id);
	}
#endif

	return SMMU_OK;
}

int32_t venc_smmu_init_tbu(int32_t core_id)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	struct smmu_entry *entry = venc_smmu_get_entry();
	uint32_t *reg_base_addr = venc->ctx[core_id].reg_base;

	if (entry->smmu_init != false) {
		return SMMU_OK;
	}

	if (reg_base_addr == NULL) {
		HI_FATAL_VENC("reg base addr is NULL");
		return SMMU_ERR;
	}

	(void)memset_s(entry, sizeof(*entry), 0, sizeof(*entry));
	entry->reg_virt.tbu_virt_addr = (uint32_t *)((uint64_t)(uintptr_t)reg_base_addr + HIVDEC_SMMU_TBU_OFFSET);
	entry->reg_virt.pre_virt_addr = (int32_t *)((uint64_t)(uintptr_t)reg_base_addr + HIVDEC_SMMU_PRE_OFFSET);

	if (venc_tcu_regulator_enable() != SMMU_OK) {
		HI_FATAL_VENC("venc enable tcu regulator failed");
		return SMMU_ERR;
	}

	if (venc_open_tbu() != SMMU_OK) {
		HI_FATAL_VENC("venc open tbu failed");
		venc_tcu_regulator_disable();
		return SMMU_ERR;
	}
	entry->smmu_init = true;

	return SMMU_OK;
}

void venc_smmu_deinit()
{
	struct smmu_entry *entry = venc_smmu_get_entry();

	if (entry->smmu_init != true)
		return;

	venc_close_tbu();
	venc_tcu_regulator_disable();
	entry->smmu_init = false;
}

void venc_smmu_debug(const uint32_t *reg_base, bool first_cfg_flag)
{
	return;
}
