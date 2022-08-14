/*
 * adsp_misc_interface.h
 *
 * adsp misc interface header
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef __ADSP_MISC_INTERFACE_DEFS_H__
#define __ADSP_MISC_INTERFACE_DEFS_H__

struct smartpa_afe_interface {
	int (*send_tfa_cal_apr)(void *buf, int cmd_size, bool read);
	int (*afe_tisa_get_set)(u8 *user_data, uint32_t param_id,
		uint8_t get_set, uint32_t length, uint32_t module_id);
};

void register_adsp_intf(struct smartpa_afe_interface *intf);
#endif // __ADSP_MISC_INTERFACE_DEFS_H__

