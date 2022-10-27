/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015, Linux Foundation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef UFS_BOOT_DEVICE_H_
#define UFS_BOOT_DEVICE_H_

#include "ufshcd.h"

#define MAX_NAME_LEN 32
#define MAX_PRL_LEN 5

#define UFS_VENDOR_HI1861      0x8B6
#define UFS_VENDOR_SANDISK     0x145

void set_bootdevice_wb_enable(u8 wb_enable);
void set_ufs_bootdevice_specversion(unsigned int specversion);
unsigned int get_ufs_bootdevice_specversion(void);
void set_ufs_bootdevice_manfid(unsigned int manfid);
unsigned int get_ufs_bootdevice_manfid(void);
void set_ufs_bootdevice_product_name(char *product_name);
void set_ufs_bootdevice_size(sector_t size);
void set_ufs_fw_version(char *fw_version);
int ufs_debug_proc_init_bootdevice(struct ufs_hba *hba);

#define QUERY_DESC_GEOMETRY_MAX_SIZE    0x44
#define SERIAL_NUM_SIZE 12

void ufs_get_geometry_info(struct ufs_hba *hba);
void ufs_set_sec_unique_number(struct ufs_hba *hba,
		uint8_t *str_desc_buf, char *product_name);
int get_bootdevice_type(void);

#endif /* End of Header */
