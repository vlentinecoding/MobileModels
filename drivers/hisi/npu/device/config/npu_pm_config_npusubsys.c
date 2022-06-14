/*
 * npu_pm_config_npusubsys.c
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

int npu_npusubsys_powerup(int work_mode, uint32_t subip_set, void **para)
{
	unused(para);
	unused(subip_set);
	return npu_plat_powerup_till_npucpu(work_mode);
}

int npu_npusubsys_powerdown(int work_mode, uint32_t subip_set, void *para)
{
	unused(para);
	unused(work_mode);
	unused(subip_set);
	return npu_plat_powerdown_npucpu(0, work_mode);
}
