/*
 * Copyright (c) Honor Device Co., Ltd. 2021-2021. All rights reserved.
 * Description: save bootloader log function realization.
 * Author:  guanhang
 * Create:  2021-08-30
 */

#include "rawdata_oper_log_collector/rawdata_oper_log_collector.h"
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/reboot.h>
#include <securec.h>
#include <linux/of_address.h>
#include <linux/platform_device.h>
#include <linux/of_fdt.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/thread_info.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>
#include <linux/soc/qcom/smem.h>
#include <linux/syscalls.h>
#include <hwbootfail/chipsets/common/bootfail_common.h>


#define BLI_DEV_PATH                 "/dev/block/by-name/bootloader_info"
#define LOG_HEAD_MAGIC               0x0000abcd
#define HEADER_SIZE                  4096      // one block size
#define MAX_DEBUG_MESSAGE_LENGTH     0x80

#define CONTROL_INFO_LENGTH          4096
#define LOG_BODY_SIZE                0x040000  // 256KB
#define FASTBOOT_LOG_DATA_LENGTH     0x200000  // 2MB
#define RAWDATA_OPERATE_DATA_LENGTH  0x020000  // 128KB
#define RAWDATA_OPERATE_PER_LENGTH   0x010000  // 64KB

#define NORMAL_LOG_BODY_NUM          8
#define ABNORMAL_LOG_BODY_NUM        16
#define RAWDATA_OPERATE_LOG_BODY_NUM 2

#define BOOTLOADER_INFO_HEADER_OFFSET         0
#define CONTROL_DATA_PARTITION_OFFSET         (BOOTLOADER_INFO_HEADER_OFFSET + HEADER_SIZE)
#define NORMAL_LOG_PARTITON_OFFSET            (CONTROL_DATA_PARTITION_OFFSET + CONTROL_INFO_LENGTH)
#define ABNORMAL_LOG_PARTITON_OFFSET          (NORMAL_LOG_PARTITON_OFFSET + NORMAL_LOG_BODY_NUM * LOG_BODY_SIZE)
#define FASTBOOT_LOG_PARTITION_OFFSET         (ABNORMAL_LOG_PARTITON_OFFSET + ABNORMAL_LOG_BODY_NUM * LOG_BODY_SIZE)
#define RAWDATA_OPERATE_LOG_PARTITION_OFFSET  (FASTBOOT_LOG_PARTITION_OFFSET + FASTBOOT_LOG_DATA_LENGTH)

enum rawdata_log_type_enum {
    BL_RAWDATA_LOG_TYPE = 0,
    KERNEL_RAWDATA_LOG_TYPE,
    MAX_RAWDATA_LOG_TYPE
};

struct body_info {
    u32 partition_offset; // offset in bootloader log partiton
    u32 body_offset;      // offset in cur log Body
    u32 size;             // log Size
    u32 state;            // body state
} __packed;

struct rawdata_oper_log_area_header {
    u32 magic_num;
    u32 type;
    u32 size;
    struct body_info body_info[RAWDATA_OPERATE_LOG_BODY_NUM];
} __packed;

static char *rawdata_oper_log = NULL;

static int rawdata_oper_log_init()
{
    int ret;
    rawdata_oper_log = kzalloc(RAWDATA_OPERATE_PER_LENGTH, GFP_KERNEL);
    if (rawdata_oper_log == NULL) {
        print_err("%s: alloc mem for rawdata_oper_log fail.\n", __func__);
        return -1;
    }

    ret = read_part(BLI_DEV_PATH, RAWDATA_OPERATE_LOG_PARTITION_OFFSET + RAWDATA_OPERATE_PER_LENGTH,
        (char *)rawdata_oper_log, RAWDATA_OPERATE_PER_LENGTH);
    if (ret != 0) {
        print_err("%s: read bootloader_info fail.\n", __func__);
        kfree(rawdata_oper_log);
        rawdata_oper_log = NULL;
        return -1;
    }

    return 0;
}

static void flush_rawdata_oper_log_to_storage(
    struct rawdata_oper_log_area_header *rawdata_oper_log_hdr,
    const char *buf)
{
    int ret;
    char log_buf[MAX_DEBUG_MESSAGE_LENGTH + 1] = {0};
    u32 rtc_time = get_sys_rtc_time();

    if (sprintf_s(log_buf, (sizeof(log_buf) - 1), "%d-%s", rtc_time, buf) < 0) {
        print_err("%s: sprintf_s rtc time fail!\n", __func__);
        return;
    }

    if ((rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].size -
        rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].body_offset) >= strlen(log_buf)) {
        ret = memcpy_s((rawdata_oper_log + rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].body_offset),
            strlen(log_buf), log_buf, strlen(log_buf));
        rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].body_offset += strlen(log_buf);
    } else {
        ret = memcpy_s(rawdata_oper_log, strlen(log_buf), log_buf, strlen(log_buf));
        rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].body_offset = 0;
    }
    if (ret != EOK) {
        print_err("memcpy_s failed, ret: %d\n", ret);
        return;
    }

    ret = write_part(BLI_DEV_PATH, RAWDATA_OPERATE_LOG_PARTITION_OFFSET + RAWDATA_OPERATE_PER_LENGTH,
        (const char *)rawdata_oper_log, RAWDATA_OPERATE_PER_LENGTH);
    if (ret != 0) {
        print_err("%s: write body offset fail.\n", __func__);
        return;
    }

    // update body offset.
    if (rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].body_offset >=
        rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].size) {
        rawdata_oper_log_hdr->body_info[KERNEL_RAWDATA_LOG_TYPE].body_offset = 0;
    }

    ret = write_part(BLI_DEV_PATH, RAWDATA_OPERATE_LOG_PARTITION_OFFSET,
        (const char *)rawdata_oper_log_hdr, HEADER_SIZE);
    if (ret != 0) {
        print_err("%s: update body offset fail.\n", __func__);
    }
}

void write_rawdata_oper_log_to_storage(const char *buf, unsigned int len)
{
    int ret;
    static is_first = true;
    struct rawdata_oper_log_area_header rawdata_oper_log_hdr;

    if ((buf == NULL) || (len > MAX_DEBUG_MESSAGE_LENGTH)) {
        print_err("%s: para is invalid, len:%d.\n", __func__, len);
        return;
    }

    ret = read_part(BLI_DEV_PATH, RAWDATA_OPERATE_LOG_PARTITION_OFFSET, (char *)&rawdata_oper_log_hdr, HEADER_SIZE);
    if (ret != 0) {
        print_err("%s: read bootloader_info fail.\n", __func__);
        return;
    }

    if (is_first) {
        ret = rawdata_oper_log_init();
        if (ret != 0) {
            print_err("%s: rawdata_oper_log_init fail.\n", __func__);
            return;
        }
        is_first = false;
    }

    if (rawdata_oper_log == NULL) {
        print_err("%s: rawdata_oper_log is null.\n", __func__);
        return;
    }

    flush_rawdata_oper_log_to_storage(&rawdata_oper_log_hdr, buf);
}
