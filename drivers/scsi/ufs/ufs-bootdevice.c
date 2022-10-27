// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2015, 2017-2018, Linux Foundation. All rights reserved.
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

#include <linux/debugfs.h>
#include "ufs-qcom.h"
#include "ufs-bootdevice.h"
#include <linux/proc_fs.h>
#include "ufs_quirks.h"

#define TESTBUS_CFG_BUFF_LINE_SIZE	sizeof("0xXY, 0xXY")

struct __bootdevice {
	struct ufs_hba *hba;
	char product_name[MAX_NAME_LEN + 1];
	sector_t size;
	unsigned int manfid;
	char fw_version[MAX_PRL_LEN + 1];
	unsigned int specversion;
	u8 wb_enable;
	char prl[MAX_PRL_LEN+1];
};

static struct __bootdevice bootdevice;

#define UFS_PROC_SHOW(name, fmt, args...) \
static int ufs_##name##_show(struct seq_file *m, void *v) \
{ \
	if (bootdevice.hba) \
		seq_printf(m, fmt, args); \
	return 0; \
} \
static int ufs_##name##_open(struct inode *inode, struct file *file) \
{ \
	return single_open(file, ufs_##name##_show, inode->i_private); \
} \
static const struct file_operations name##_fops = { \
	.open = ufs_##name##_open, \
	.read = seq_read, \
	.llseek = seq_lseek, \
	.release = single_release, \
}

#define  EMMC_TYPE      0
#define  UFS_TYPE       1
#define BOOT_DEVICE_UFS UFS_TYPE

int get_bootdevice_type(void)
{
	return BOOT_DEVICE_UFS;
}

static int ufs_cid_show(struct seq_file *m, void *v)
{
	u32 cid[4];
	int i;

	if (bootdevice.hba) {
		memcpy(cid, (u32 *)&bootdevice.hba->unique_number, sizeof(cid));
		for (i = 0; i < 3; i++)
			cid[i] = be32_to_cpu(cid[i]);
		cid[3] = (((cid[3]) & 0xffff) << 16) | (((cid[3]) >> 16) & 0xffff);
		seq_printf(m, "%08x%08x%08x%08x\n", cid[0], cid[1], cid[2], cid[3]);
	}
	return 0;
}
static int ufs_cid_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_cid_show, inode->i_private);
}
static const struct file_operations cid_fops = {
	.open = ufs_cid_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void set_ufs_bootdevice_manfid(unsigned int manfid)
{
	bootdevice.manfid = manfid;
}

unsigned int get_ufs_bootdevice_manfid(void)
{
	return bootdevice.manfid;
}

static int ufs_manfid_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%06x\n", bootdevice.manfid);
	return 0;
}

static int ufs_manfid_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_manfid_show, inode->i_private);
}

static const struct file_operations manfid_fops = {
	.open		= ufs_manfid_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

void set_ufs_bootdevice_size(sector_t size)
{
	bootdevice.size = size;
}

sector_t get_ufs_bootdevice_size()
{
	return bootdevice.size;
}

static int ufs_size_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%llu\n", (unsigned long long)bootdevice.size);
	return 0;
}

static int ufs_size_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_size_show, inode->i_private);
}

static const struct file_operations size_fops = {
	.open		= ufs_size_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

static int ufs_name_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", "qcom-platform");
	return 0;
}

static int ufs_name_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_name_show, inode->i_private);
}

static const struct file_operations name_fops = {
	.open = ufs_name_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

void set_ufs_bootdevice_product_name(char *product_name)
{
	strlcpy(bootdevice.product_name,
		product_name,
		sizeof(bootdevice.product_name));
}

void get_ufs_bootdevice_product_name(char* product_name, u32 len)
{
	strlcpy(product_name, bootdevice.product_name, len); /* [false alarm] */
}

static int ufs_product_name_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s", bootdevice.product_name);
	return 0;
}

static int ufs_product_name_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_product_name_show, inode->i_private);
}

static const struct file_operations product_name_fops = {
	.open		= ufs_product_name_open,
	.read		= seq_read,
	.llseek		= seq_lseek,
	.release	= single_release,
};

void set_ufs_fw_version(char *fw_version)
{
	strlcpy(bootdevice.fw_version, fw_version, sizeof(bootdevice.fw_version));
}

static int ufs_fw_version_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s",bootdevice.fw_version);
	return 0;
}
static int ufs_fw_version_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_fw_version_show, inode->i_private);
}
static const struct file_operations fw_version_fops = {
	.open = ufs_fw_version_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int ufs_pre_eol_info_show(struct seq_file *m, void *v)
{
	int err;
	int buff_len = QUERY_DESC_HEALTH_DEF_SIZE;
	u8 desc_buf[QUERY_DESC_HEALTH_DEF_SIZE];

	if (bootdevice.hba) {
		pm_runtime_get_sync(bootdevice.hba->dev);
		err = ufshcd_read_health_desc(bootdevice.hba, desc_buf, buff_len);
		pm_runtime_put_sync(bootdevice.hba->dev);
		if (err) {
			seq_printf(m, "Reading Health Descriptor failed. err = %d\n",
				err);
			return err;
		}
		seq_printf(m, "0x%02x\n", (u8)desc_buf[2]);
	}
	return 0;
}
static int ufs_pre_eol_info_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_pre_eol_info_show, inode->i_private);
}
static const struct file_operations pre_eol_info_fops = {
	.open = ufs_pre_eol_info_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int ufs_life_time_est_typ_a_show(struct seq_file *m, void *v)
{
	int err;
	int buff_len = QUERY_DESC_HEALTH_DEF_SIZE;
	u8 desc_buf[QUERY_DESC_HEALTH_DEF_SIZE];

	if (bootdevice.hba) {
		pm_runtime_get_sync(bootdevice.hba->dev);
		err = ufshcd_read_health_desc(bootdevice.hba, desc_buf, buff_len);
		pm_runtime_put_sync(bootdevice.hba->dev);
		if (err) {
			seq_printf(m, "Reading Health Descriptor failed. err = %d\n",
				err);
			return err;
		}
		seq_printf(m, "0x%02x\n", (u8)desc_buf[3]);
	}
	return 0;
}
static int ufs_life_time_est_typ_a_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_life_time_est_typ_a_show, inode->i_private);
}
static const struct file_operations life_time_est_typ_a_fops = {
	.open = ufs_life_time_est_typ_a_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static int ufs_life_time_est_typ_b_show(struct seq_file *m, void *v)
{
	int err;
	int buff_len = QUERY_DESC_HEALTH_DEF_SIZE;
	u8 desc_buf[QUERY_DESC_HEALTH_DEF_SIZE];

	if (bootdevice.hba) {
		pm_runtime_get_sync(bootdevice.hba->dev);
		err = ufshcd_read_health_desc(bootdevice.hba, desc_buf, buff_len);
		pm_runtime_put_sync(bootdevice.hba->dev);
		if (err) {
			seq_printf(m, "Reading Health Descriptor failed. err = %d\n",
				err);
			return err;
		}
		seq_printf(m, "0x%02x\n", (u8)desc_buf[4]);
	}
	return 0;
}
static int ufs_life_time_est_typ_b_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_life_time_est_typ_b_show, inode->i_private);
}
static const struct file_operations life_time_est_typ_b_fops = {
	.open = ufs_life_time_est_typ_b_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

UFS_PROC_SHOW(type, "%d\n", BOOT_DEVICE_UFS);

void set_ufs_bootdevice_specversion(unsigned int specversion)
{
	bootdevice.specversion = specversion;
}

unsigned int get_ufs_bootdevice_specversion(void)
{
	return bootdevice.specversion;
}

static int ufs_specversion_show(struct seq_file *m, void *v)
{
	seq_printf(m, "0x%x\n", bootdevice.specversion);
	return 0;
}

static int ufs_specversion_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_specversion_show, inode->i_private);
}

static const struct file_operations specversion_fops = {
	.open = ufs_specversion_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};


void set_bootdevice_wb_enable(u8 wb_enable)
{
	bootdevice.wb_enable = wb_enable;
}

static int ufs_wb_enable_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%d\n", bootdevice.wb_enable);
	return 0;
}

static int ufs_wb_enable_open(struct inode *inode, struct file *file)
{
	return single_open(file, ufs_wb_enable_show, inode->i_private);
}

static const struct file_operations wb_enable_fops = {
	.open = ufs_wb_enable_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

static const char * const ufs_proc_list[] = {
	"cid",
	"type",
	"manfid",
	"size",
	"product_name",
	"rev",
	"pre_eol_info",
	"life_time_est_typ_a",
	"life_time_est_typ_b",
	"name",
	"specversion",
	"wb_enable",
};
static const struct file_operations *proc_fops_list[] = {
	&cid_fops,
	&type_fops,
	&manfid_fops,
	&size_fops,
	&product_name_fops,
	&fw_version_fops,
	&pre_eol_info_fops,
	&life_time_est_typ_a_fops,
	&life_time_est_typ_b_fops,
	&name_fops,
	&specversion_fops,
	&wb_enable_fops,
};

int ufs_debug_proc_init_bootdevice(struct ufs_hba *hba)
{
	struct proc_dir_entry *prEntry;
	struct proc_dir_entry *bootdevice_dir;
	int i, num;

	bootdevice_dir = proc_mkdir("bootdevice", NULL);

	if (!bootdevice_dir) {
		pr_notice("[%s]: failed to create /proc/bootdevice\n",
			__func__);
		return -1;
	}

	num = ARRAY_SIZE(ufs_proc_list);
	for (i = 0; i < num; i++) {
		prEntry = proc_create(ufs_proc_list[i], 0,
			bootdevice_dir, proc_fops_list[i]);
		if (prEntry)
			continue;
		pr_notice(
			"[%s]: failed to create /proc/bootdevice/%s\n",
			__func__, ufs_proc_list[i]);
	}

	bootdevice.hba = hba;
	return 0;
}
EXPORT_SYMBOL(ufs_debug_proc_init_bootdevice);

void ufs_get_geometry_info(struct ufs_hba *hba)
{
	int err;
	uint8_t desc_buf[QUERY_DESC_GEOMETRY_MAX_SIZE];
	u64 total_raw_device_capacity;

	err =
	    ufshcd_read_geometry_desc(hba, desc_buf, QUERY_DESC_GEOMETRY_MAX_SIZE);
	if (err) {
		dev_err(hba->dev, "%s: Failed getting geometry info\n", __func__);
		goto out;
	}
	total_raw_device_capacity =
		(u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 0] << 56 |
		(u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 1] << 48 |
		(u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 2] << 40 |
		(u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 3] << 32 |
		(u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 4] << 24 |
		(u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 5] << 16 |
		(u64)desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 6] << 8 |
		desc_buf[GEOMETRY_DESC_PARAM_DEV_CAP + 7] << 0;
	set_ufs_bootdevice_size(total_raw_device_capacity);

out:
	return;
}

void ufs_set_sec_unique_number(struct ufs_hba *hba,
					uint8_t *str_desc_buf,
					char *product_name)
{
	int i, idx;
	uint8_t snum_buf[SERIAL_NUM_SIZE + 1];

	memset(&hba->unique_number, 0, sizeof(hba->unique_number));
	memset(snum_buf, 0, sizeof(snum_buf));

	switch (hba->manufacturer_id) {
	case UFS_VENDOR_SAMSUNG:
		/* Samsung V4 UFS need 24 Bytes for serial number, transfer unicode to 12 bytes
		 * the magic number 12 here was following original below HYNIX/TOSHIBA decoding method
		*/
		for (i = 0; i < 12; i++) {
			idx = QUERY_DESC_HDR_SIZE + i * 2 + 1;
			snum_buf[i] = str_desc_buf[idx];
		}
		break;
	case UFS_VENDOR_SKHYNIX:
		/* hynix only have 6Byte, add a 0x00 before every byte */
		for (i = 0; i < 6; i++) {
			/*lint -save  -e679 */
			snum_buf[i * 2] = 0x0;
			snum_buf[i * 2 + 1] =
				str_desc_buf[QUERY_DESC_HDR_SIZE + i];
			/*lint -restore*/
		}
		break;
	case UFS_VENDOR_TOSHIBA:
		/*
		 * toshiba: 20Byte, every two byte has a prefix of 0x00, skip
		 * and add two 0x00 to the end
		 */
		for (i = 0; i < 10; i++) {
			snum_buf[i] =
				str_desc_buf[QUERY_DESC_HDR_SIZE + i * 2 + 1];/*lint !e679*/
		}
		snum_buf[10] = 0;
		snum_buf[11] = 0;
		break;
	case UFS_VENDOR_HI1861:
		memcpy(snum_buf, str_desc_buf + QUERY_DESC_HDR_SIZE, 12);
		break;
	case UFS_VENDOR_MICRON:
		memcpy(snum_buf, str_desc_buf + QUERY_DESC_HDR_SIZE, 4);
		for(i = 4; i < 12; i++) {
			snum_buf[i] = 0;
		}
		break;
	case UFS_VENDOR_SANDISK:
		for (i = 0; i < 12; i++) {
			idx = QUERY_DESC_HDR_SIZE + i * 2 + 1;
			snum_buf[i] = str_desc_buf[idx];
		}

		break;
	default:
		dev_err(hba->dev, "unknown ufs manufacturer id\n");
		break;
	}

	hba->unique_number.manufacturer_id = hba->manufacturer_id;
	hba->unique_number.manufacturer_date = hba->manufacturer_date;
	memcpy(hba->unique_number.serial_number, snum_buf, SERIAL_NUM_SIZE);
#ifdef CONFIG_HONOR_KERNEL_DEBUG
	pr_notice("ufs_debug manufacturer_id:%d\n", hba->unique_number.manufacturer_id);
	pr_notice("ufs_debug manufacturer_date:%d\n", hba->unique_number.manufacturer_date);
#endif
}
