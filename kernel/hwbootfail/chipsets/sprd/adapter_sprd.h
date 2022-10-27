/*
 * Copyright (c) Honor Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: define adapter for sprd
 * Author: yuanshuai
 * Create: 2021-2-3
 */

#ifndef ADAPTER_SPRD_H
#define ADAPTER_SPRD_H

#include <hwbootfail/core/adapter.h>
#include <linux/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define BFI_DEV_PATH "/dev/block/by-name/rrecord"
#define LONG_PRESS_DEV_PATH "/dev/block/by-name/bootfail_info"
#define BOOT_PHASE_DEV_PATH        LONG_PRESS_DEV_PATH
#define BOOT_PHASE_OFFSET           0x4FFC00             /* 5MB - 1KB */

/* reserve memory map */
/* |-bootfail_mem_1KB-|-reserve_3KB-|-BF_EXTEND_HEAD_SIZE_1KB-|-BF_UBOOT_LOG_SIZE_512KB-| */

/* 8MB -128 KB */
#define BFI_PART_SIZE 0x7E0000

/* ----local macroes ---- */
#define BL_LOG_NAME "fastboot_log"
#define KERNEL_LOG_NAME "last_kmsg"
#define BL_LOG_MAX_LEN 0x80000

#define BF_BOPD_RESERVE 32

#define BF_UBOOT_LOG_SIZE 0x80000
#define BF_EXTEND_HEAD_SIZE 0x400
#define BF_EXTN_VALID 0x56414C49
#define BF_SPRD_MEM_INFO_ENABLE 0x454E4142

struct bf_sprd_info {
    unsigned int enable_flag;
    void *bf_smem_addr;
    unsigned long bf_smem_size;
    void *bf_extend_addr;
    unsigned long bf_extend_size;
};

struct bf_extend_header {
    u32 valid_flag;
    u32 boot_stage;
    u32 uboot_log_offset;
    u32 uboot_log_size;
    u32 klog_phy_addr;
    u32 klog_size;
};

int bf_rmem_init(void);
void *get_bf_mem_addr(void);
unsigned long get_bf_mem_size(void);
int sprd_adapter_init(struct adapter *padp);
void sprd_save_long_press_logs(void);

#ifdef __cplusplus
}
#endif
#endif