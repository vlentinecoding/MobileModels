/*
 * ti_smartamp.h
 *
 * ti smartpa related definition for adsp misc
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef _TFA_SMART_AMP_H
#define _TFA_SMART_AMP_H

#define TFA_ADSP_CMD_PARAM       0x00808b
#define TFA_ADSP_CMD_SIZE_LIMIT  512
#define TFA_ADSP_CMD_MASK        0xff
#define TFA_CMD_TO_ADSP_BUF(buf, cmd_id) \
do { \
	buf[0] = (cmd_id >> 16) & TFA_ADSP_CMD_MASK; \
	buf[1] = (cmd_id >> 8) & TFA_ADSP_CMD_MASK; \
	buf[2] = cmd_id & TFA_ADSP_CMD_MASK; \
} while (0)

#define TFA_CURRENT_R0_IDX_0     5
#define TFA_CURRENT_R0_IDX_1     6
#define TFA_CURRENT_TEMP_IDX_0   9
#define TFA_CURRENT_TEMP_IDX_1   10
#define TFA_CURRENT_F0_IDX_0     41
#define TFA_CURRENT_F0_IDX_1     42
#define TFA_CALC_PARAM(i, buf) \
	((buf[i * 3 + 0] << 16) + (buf[i * 3 + 1] << 8) + (buf[i * 3 + 2]))

#define TFA_TUNING_RW_MAX_SIZE  4096

#define AFE_TFA_SET_COMMEND     0x1000B921
#define AFE_TFA_SET_BYPASS      0x1000B923

#endif // _TFA_SMART_AMP_H

