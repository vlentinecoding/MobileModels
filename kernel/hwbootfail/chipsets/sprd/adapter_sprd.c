/*
 * Copyright (c) Honor Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: implement the platform interface for boot fail
 * Author: yuanshuai
 * Create: 2021-02-03
 */
#include "adapter_sprd.h"

#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/reboot.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/of_fdt.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/thread_info.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/syscalls.h>

#include <securec.h>
#include <hwbootfail/chipsets/common/adapter_common.h>
#include <hwbootfail/chipsets/common/bootfail_chipsets.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>
#include <hwbootfail/core/boot_interface.h>

#define BL_LOG_NAME "fastboot_log"
#define KERNEL_LOG_NAME "last_kmsg"
#define APP_LOG_PATH "/data/log/android_logs/applogcat-log"
#define APP_LOG_PATH_MAX_LENGTH 128

#define BF_BL_LOG_NAME_SIZE 16
#define BF_KERNEL_LOG_NAME_SIZE 16

#define FASTBOOTLOG_TYPE 0
#define KMSGLOG_TYPE     1
#define APPLOG_TYPE      2

#define ROFA_MEM_SIZE   32

struct every_number_info {
	u64 rtc_time;
	u64 boot_time;
	u64 bootup_keypoint;
	u64 reboot_type;
	u64 exce_subtype;
	u64 fastbootlog_start_addr;
	u64 fastbootlog_size;
	u64 last_kmsg_start_addr;
	u64 last_kmsg_size;
	u64 last_applog_start_addr;
	u64 last_applog_size;
};

static struct bf_sprd_info g_bf_info;
// uboot have set KERNEL_STAGE to bootfail_info partition
static u32 g_current_bootstage = KERNEL_STAGE;

__weak long ksys_readlink(const char __user *path, char __user *buf, int bufsiz)
{
	print_err("please implement ksys_readlink\n");
	return 0;
}

long bf_readlink(const char __user *path, char __user *buf, int bufsiz)
{
#ifndef CONFIG_ARCH_HAS_SYSCALL_WRAPPER
	return sys_readlink(path, buf, bufsiz);
#else
	return ksys_readlink(path, buf, bufsiz);
#endif
}

static inline void bf_sprd_degrade(int excp_type)
{
	print_err("unsupported\n");
}

static inline void bf_sprd_bp(int excp_type)
{
	print_err("unsupported\n");
}

static inline void bf_sprd_load_backup(const char *part_name)
{
	print_err("unsupported\n");
}

static inline void bf_sprd_notify_storage_fault(unsigned long long bopd_mode)
{
	print_err("unsupported\n");
}

void *get_bf_mem_addr(void)
{
	if (g_bf_info.enable_flag != BF_SPRD_MEM_INFO_ENABLE) {
		print_err("g_bf_info have not init, please check\n");
		return NULL;
	}
	return g_bf_info.bf_smem_addr;
}

unsigned long get_bf_mem_size(void)
{
	if (g_bf_info.enable_flag != BF_SPRD_MEM_INFO_ENABLE) {
		print_err("g_bf_info have not init, please check\n");
		return 0;
	}
	return g_bf_info.bf_smem_size;
}

void *get_bf_extend_addr(void)
{
	if (g_bf_info.enable_flag != BF_SPRD_MEM_INFO_ENABLE) {
		print_err("g_bf_info have not init, please check\n");
		return NULL;
	}
	return g_bf_info.bf_extend_addr;
}

static char *get_bf_extend_bl_info(unsigned int *bl_log_size)
{
	struct bf_extend_header *beh = get_bf_extend_addr();
	char *bl_addr = NULL;

	if (beh == NULL) {
		print_invalid_params("bl log params get fail\n");
		*bl_log_size = 0;
		return NULL;
	}
	bl_addr = (char *)beh + beh->uboot_log_offset;
	*bl_log_size = beh->uboot_log_size;
	print_err("bl_log_addr:%x, bl_log_size:%x\n", bl_addr, *bl_log_size);
	return bl_addr;
}

/* Set rrecord info to sprd adapter */
static inline void get_rrecord_part_info(struct adapter *padp)
{
	padp->bfi_part.dev_path = BFI_DEV_PATH;
	padp->bfi_part.part_size = BFI_PART_SIZE;
}

void *bf_rmem_init_by_dts(char *node_name, unsigned int *size)
{
    struct device_node *node = NULL;
//    const u32 *dts_addr = NULL;
    unsigned long target_addr= 0;
	struct resource r_node;
	int node_info;

	node = of_find_node_by_name(NULL, node_name);
	if (node == NULL) {
		print_err("bootfail mem not find in dts\n", node_name);
		goto smem_init_err;
	}
	node_info = of_address_to_resource(node, 0, &r_node);
	if (node_info) {
		goto smem_init_err;
	}
	of_node_put(node);
	target_addr = r_node.start;
	*size = (unsigned int)(r_node.end - r_node.start + 1);

    return (void *)target_addr;
smem_init_err:
    *size = 0;
    return NULL;
}

static bool bf_extend_header_init(void)
{
    struct bf_extend_header *header = NULL;
	char *kbuf_addr = NULL;

	if (g_bf_info.enable_flag != BF_SPRD_MEM_INFO_ENABLE) {
		print_err("g_bf_info have not init, please check\n");
		return false;
	}
	header = (struct bf_extend_header *)g_bf_info.bf_extend_addr;
    if (header == NULL) {
        print_err("extend header error\n");
        return false;
    }

    header->uboot_log_offset = BF_EXTEND_HEAD_SIZE;
	if (BF_EXTEND_HEAD_SIZE <= sizeof(*header)) {
		print_err("bootfail extend head size too long\n");
		return false;
	}
    header->uboot_log_size = BF_UBOOT_LOG_SIZE;
	if ((BF_EXTEND_HEAD_SIZE + BF_UBOOT_LOG_SIZE) > g_bf_info.bf_extend_size) {
		print_err("bootfail extend size too long\n");
		return false;
	}
	header->klog_size = log_buf_len_get();
	kbuf_addr = log_buf_addr_get();
	header->klog_phy_addr = virt_to_phys(kbuf_addr);

	header->valid_flag = BF_EXTN_VALID;
    print_err("extend header init success, extend header size %x, uboot log size %x\n",
        header->uboot_log_offset, header->uboot_log_size);
    print_err("klog phy buff %x, size %x\n",
        header->klog_phy_addr, header->klog_size);
	return true;
}

/**
 * @brief Init reserve ddr region, get ddr info from dtsi.
 * @param NONE.
 * @return BF_OK on success.
 * @since 1.0
 * @version 1.0
 */
int bf_rmem_init(void)
{
    void *bf_smem_addr = NULL;
    unsigned int smem_size = 0;

    bf_smem_addr = bf_rmem_init_by_dts("bootfail_mem", &smem_size);
    if (bf_smem_addr == NULL || smem_size == 0) {
        print_err("bf_rmem_init_by_dts fail\n");
        return BF_NOT_INIT_SUCC;
    }
    if (smem_size != BF_SIZE_1M) {
        print_err("bootfail init fail, please reserve 1MB ddr region\n");
        return BF_NOT_INIT_SUCC;
    }
#ifdef CONFIG_ARM
	g_bf_info.bf_smem_addr = ioremap_nocache(bf_smem_addr, smem_size);
#else
	g_bf_info.bf_smem_addr = ioremap_wc((phys_addr_t)bf_smem_addr, smem_size);
#endif
	if (g_bf_info.bf_smem_addr == NULL) {
		print_err("bootfail share mem ioremap fail\n");
		goto smem_init_err;
	}
    g_bf_info.bf_smem_size = BF_SIZE_1K - ROFA_MEM_SIZE;
    g_bf_info.bf_extend_addr = (void *)((unsigned long)g_bf_info.bf_smem_addr + BF_SIZE_4K);
	g_bf_info.bf_extend_size = smem_size - BF_SIZE_4K;
	g_bf_info.enable_flag = BF_SPRD_MEM_INFO_ENABLE;
	if (bf_extend_header_init() == false)
		goto smem_init_err;
    print_err("reserve ddr region addr:%p, size:%p\n", bf_smem_addr, smem_size);
    print_err("ioremap addr:%x, extend header:%x, extend size:%x\n",
		g_bf_info.bf_smem_addr, g_bf_info.bf_extend_addr, g_bf_info.bf_extend_size);
	return BF_OK;

smem_init_err:
	return BF_NOT_INIT_SUCC;
}

static int sprd_read_from_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *phys_mem_addr,
	unsigned long data_len)
{
	unsigned long i;
	unsigned long bytes_to_read;
	char *pdst = NULL;

	if (phys_mem_addr == NULL || dst == 0 ||
		dst_max == 0 || data_len == 0) {
		print_invalid_params("bootfail: dst: %u, dst_max: %u, data_len: %u\n",
			dst, dst_max, data_len);
		return -1;
	}

	bytes_to_read = min(dst_max, data_len);
	pdst = (char *)(uintptr_t)dst;
	for (i = 0; i < bytes_to_read; i++) {
		*pdst = readb(phys_mem_addr);
		pdst++;
		phys_mem_addr++;
	}

	return 0;
}

static int sprd_write_to_phys_mem(unsigned long dst,
	unsigned long dst_max,
	void *src,
	unsigned long src_len)
{
	unsigned long i;
	unsigned long bytes_to_write;
	char *psrc = NULL;
	char *pdst = NULL;

	if (src == NULL || dst == 0 || dst_max == 0 || src_len == 0) {
		print_invalid_params("bootfail: dst: %u, dst_max: %u, src_len: %u\n",
			dst, dst_max, src_len);
		return -1;
	}

	bytes_to_write = min(dst_max, src_len);
	pdst = (char *)(uintptr_t)dst;
	psrc = (char *)src;
	for (i = 0; i < bytes_to_write; i++) {
		writeb(*psrc, pdst);
		psrc++;
		pdst++;
	}

	return 0;
}

static void sprd_shutdown(void)
{
	print_err("unsupported\n");
}

static void sprd_reboot(void)
{
	kernel_restart("bootfail");
}

/* Set reboot and shutdown method to adapter */
static void get_sysctl_ops(struct adapter *padp)
{
	padp->sys_ctl.reboot = sprd_reboot;
	padp->sys_ctl.shutdown = sprd_shutdown;
}

static void set_boot_stage_to_disk(struct work_struct *work)
{
	write_part(BOOT_PHASE_DEV_PATH, BOOT_PHASE_OFFSET,
		(const char*)&g_current_bootstage, sizeof(u32));
	print_err("set boot stage:%x done\n", g_current_bootstage);
}

static DECLARE_WORK(set_boot_phase_work, &set_boot_stage_to_disk);

static int sprd_set_boot_stage(int stage)
{
	g_current_bootstage = stage;
	print_err("start set boot stage:%x\n", stage);
	schedule_work(&set_boot_phase_work);
	return BF_OK;
}

static int sprd_get_boot_stage(int *stage)
{
	if (stage == NULL) {
		print_invalid_params("stage is NULL\n");
		return BF_PLATFORM_ERR;
	}
	*stage = g_current_bootstage;
	return BF_OK;
}

/**
 * @brief Set bootstage method to adapter.
 *        record bootstage by sprd share memory function
 * @param padp - bootfail sprd adapter pointer
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void get_boot_stage_ops(struct adapter *padp)
{
	padp->stage_ops.set_stage = sprd_set_boot_stage;
	padp->stage_ops.get_stage = sprd_get_boot_stage;
}

/* Set reserve ddr info to sprd adapter */
static void get_phys_mem_info(struct adapter *padp)
{
	padp->pyhs_mem_info.base = (uintptr_t)get_bf_mem_addr();
	if (padp->pyhs_mem_info.base == 0) {
		print_err("rmem addr init fail\n");
		goto rmem_err;
	}
	padp->pyhs_mem_info.size = get_bf_mem_size();
	if (padp->pyhs_mem_info.size != BF_SIZE_1K - BF_BOPD_RESERVE) {
		print_err("get reserve mem size err, please check!!!\n");
		goto rmem_err;
	}
	padp->pyhs_mem_info.ops.read = sprd_read_from_phys_mem;
	padp->pyhs_mem_info.ops.write = sprd_write_to_phys_mem;
	print_err("pyhs_mem_info:%x, %x\n", padp->pyhs_mem_info.base,
		padp->pyhs_mem_info.size);
	return;
rmem_err:
	padp->pyhs_mem_info.base = 0;
	padp->pyhs_mem_info.size = 0;
	padp->pyhs_mem_info.ops.read = NULL;
	padp->pyhs_mem_info.ops.write = NULL;
	return;
}

/**
 * @brief Get xbl UEFI and abl log.
 * @param pbuf - desc buffer.
 * @param buf_size - copy size, actually limited by rainbow bl log size.
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void capture_bl_log(char *pbuf, unsigned int buf_size)
{
	unsigned int uboot_log_size = 0;
	char *bl_addr = NULL;
	char *dst_buf = NULL;
	size_t bytes_to_read;
	int i;

	if (pbuf == NULL) {
		print_invalid_params("pbuf is null\n");
		return;
	}
	bl_addr = get_bf_extend_bl_info(&uboot_log_size);
	if (bl_addr == NULL) {
		print_invalid_params("bl log params get fail\n");
		return;
	}

	if (bl_addr == NULL ||
		uboot_log_size > BF_UBOOT_LOG_SIZE) {
		print_invalid_params("uboot log params err, force to 512KB\n");
		uboot_log_size = BF_UBOOT_LOG_SIZE;
	}
	dst_buf = pbuf;
	bytes_to_read = min(buf_size, uboot_log_size);
	for (i = 0; i < bytes_to_read; i++) {
		*dst_buf = readb(bl_addr);
		dst_buf++;
		bl_addr++;
	}
}

/**
 * @brief Get kernel log.
 * @param pbuf - desc buffer.
 * @param buf_size - copy size, actually limited by kernel log buff size
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void capture_kernel_log(char *pbuf, unsigned int buf_size)
{
	char *kbuf_addr = NULL;
	unsigned int kbuf_size = log_buf_len_get();
	errno_t ret;

	if (kbuf_size > BF_SIZE_1M) {
		print_invalid_params("klog buffer size too large, force to 1MB\n");
		kbuf_size = BF_SIZE_1M;
	}
	if (pbuf == NULL) {
		print_invalid_params("pbuf is null\n");
		return;
	}
	kbuf_addr = log_buf_addr_get();
	if (kbuf_addr == NULL) {
		print_invalid_params("kbuf_addr or buf_size err\n");
		return;
	}
	ret = memcpy_s(pbuf, buf_size,
		kbuf_addr, min(kbuf_size, buf_size));
	if (ret != EOK)
		print_err("memcpy_s failed, ret: %d\n", ret);
}

/**
 * @brief Set get kernel log and fastboot log method to adapter.
 * @param padp - bootfail sprd adapter pointer
 * @return NONE.
 * @since 1.0
 * @version 1.0
 */
static void get_log_ops_info(struct adapter *padp)
{
	errno_t ret;
	/* set bl log info */
	ret = strncpy_s(padp->bl_log_ops.log_name,
		BF_BL_LOG_NAME_SIZE,
		BL_LOG_NAME, min(strlen(BL_LOG_NAME),
		sizeof(padp->bl_log_ops.log_name) - 1));
	if (ret != EOK)
		print_err("bl log name strncpy_s failed, ret: %d\n", ret);

	padp->bl_log_ops.log_size = (unsigned int)BF_UBOOT_LOG_SIZE;
	padp->bl_log_ops.capture_bl_log = capture_bl_log;

	/* set kernel log info */
	ret = strncpy_s(padp->kernel_log_ops.log_name,
		BF_KERNEL_LOG_NAME_SIZE,
		KERNEL_LOG_NAME, min(strlen(KERNEL_LOG_NAME),
		sizeof(padp->kernel_log_ops.log_name) - 1));
        if (ret != EOK)
                print_err("kernel log name strncpy_s failed, ret: %d\n", ret);
	padp->kernel_log_ops.log_size = BF_SIZE_1M;
	padp->kernel_log_ops.capture_kernel_log = capture_kernel_log;
}

/* SPRD platform adapter init */
static void platform_adapter_init(struct adapter *padp)
{
	if (padp == NULL) {
		print_err("padp is NULL\n");
		return;
	}

	get_rrecord_part_info(padp);
	get_phys_mem_info(padp);
	get_log_ops_info(padp);
	get_sysctl_ops(padp);
	get_boot_stage_ops(padp);
	padp->prevent.degrade = bf_sprd_degrade;
	padp->prevent.bypass = bf_sprd_bp;
	padp->prevent.load_backup = bf_sprd_load_backup;
	padp->notify_storage_fault = bf_sprd_notify_storage_fault;
}

/**
 * @brief Init bootfail sprd adapter, include common init and platform init.
 * @param padp - bootfail sprd adapter pointer
 * @return 0 on success.
 * @since 1.0
 * @version 1.0
 */
int sprd_adapter_init(struct adapter *padp)
{
	if (common_adapter_init(padp) != 0) {
		print_err("init adapter common failed\n");
		return -1;
	}
	platform_adapter_init(padp);
	return 0;
}

static void capture_app_log(char *pbuf, unsigned int buf_size)
{
	int ret;
	mm_segment_t fs;

	fs = get_fs();
	set_fs(KERNEL_DS);

	ret = bf_readlink(APP_LOG_PATH, pbuf, buf_size);

	if (ret < 0)
		print_err("read %s failed ,ret = %d\n", APP_LOG_PATH, ret);

	set_fs(fs);
}

static void save_long_press_meta_log(struct every_number_info *pinfo)
{
	int bootstage;
	unsigned int bl_log_size;
	char *bl_addr = NULL;
	int ret;

	ret = sprd_get_boot_stage(&bootstage);
	if (ret != BF_OK) {
		print_err("get boot stage fail\n");
		bootstage = INVALID_STAGE;
	}
	pinfo->rtc_time = get_sys_rtc_time();
	pinfo->boot_time = get_bootup_time();
	pinfo->bootup_keypoint = bootstage;
	pinfo->reboot_type = KERNEL_SYSTEM_FREEZE;
	pinfo->exce_subtype = KERNEL_SYSTEM_FREEZE;

	pinfo->fastbootlog_start_addr = sizeof(struct every_number_info);
	bl_addr = get_bf_extend_bl_info(&bl_log_size);
	if (bl_addr == NULL) {
		print_invalid_params("bl log params get fail\n");
		return;
	}

	pinfo->fastbootlog_size = bl_log_size;
	pinfo->last_kmsg_start_addr = pinfo->fastbootlog_start_addr +
		pinfo->fastbootlog_size;
	pinfo->last_kmsg_size = BF_SIZE_1M;
	pinfo->last_applog_start_addr = pinfo->last_kmsg_start_addr +
		pinfo->last_kmsg_size;
	pinfo->last_applog_size = APP_LOG_PATH_MAX_LENGTH;

	write_part(LONG_PRESS_DEV_PATH, 0, (const char *)pinfo,
		sizeof(struct every_number_info));
}

static void save_long_press_log(u32 type, u64 offset, u64 size)
{
	char *buf = NULL;

	if (size == 0) {
		print_err("log type %d, invalid size\n", type);
		return;
	}

	buf = vzalloc(size);
	if (buf == NULL)
		return;

	switch (type) {
	case FASTBOOTLOG_TYPE:
		capture_bl_log(buf, size);
		break;
	case KMSGLOG_TYPE:
		capture_kernel_log(buf, size);
		break;
	case APPLOG_TYPE:
		capture_app_log(buf, size);
		break;
	default:
		print_err("log type %d invliad type\n", type);
		goto out;
	}

	write_part(LONG_PRESS_DEV_PATH, offset, buf, size);

out:
	vfree(buf);
}

void sprd_save_long_press_logs(void)
{
	struct every_number_info info;

	save_long_press_meta_log(&info);

	save_long_press_log(FASTBOOTLOG_TYPE, info.fastbootlog_start_addr,
		info.fastbootlog_size);
	save_long_press_log(KMSGLOG_TYPE, info.last_kmsg_start_addr,
		info.last_kmsg_size);
	save_long_press_log(APPLOG_TYPE, info.last_applog_start_addr,
		info.last_applog_size);
}
