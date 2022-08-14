/*
 * huawei_charger_common.h
 *
 * common interface for huawei charger driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HUAWEI_CHARGER_COMMON_H_
#define _HUAWEI_CHARGER_COMMON_H_

#include <linux/types.h>

int charge_enable_powerpath(bool path_enable);
int charger_set_input_current(u32 uA);

#endif /* _HUAWEI_CHARGER_COMMON_H_ */
