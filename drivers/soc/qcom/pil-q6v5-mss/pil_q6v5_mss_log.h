/*
 * Copyright (C) 2013 Huawei Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#ifndef PIL_Q6V5_MSS_LOG
#define PIL_Q6V5_MSS_LOG
#include <linux/types.h>
#include <linux/soc/qcom/smem.h>

#define SMEM_NUM_SMD_STREAM_CHANNELS 64

enum {
    SMEM_ID_VENDOR2 = SMEM_ID_VENDOR1 + 1,
    SMEM_HW_SW_BUILD_ID,
    SMEM_SMD_BASE_ID_2,
    SMEM_SMD_FIFO_BASE_ID_2 = SMEM_SMD_BASE_ID_2 + SMEM_NUM_SMD_STREAM_CHANNELS,
    SMEM_CHANNEL_ALLOC_TBL_2 = SMEM_SMD_FIFO_BASE_ID_2 + SMEM_NUM_SMD_STREAM_CHANNELS,
    SMEM_I2C_MUTEX = SMEM_CHANNEL_ALLOC_TBL_2 + SMEM_NUM_SMD_STREAM_CHANNELS,
    SMEM_SCLK_CONVERSION,
    SMEM_SMD_SMSM_INTR_MUX,
    SMEM_SMSM_CPU_INTR_MASK,
    SMEM_APPS_DEM_SLAVE_DATA,
    SMEM_QDSP6_DEM_SLAVE_DATA,
    SMEM_VSENSE_DATA,
    SMEM_CLKREGIM_SOURCES,
    SMEM_SMD_FIFO_BASE_ID,
    SMEM_USABLE_RAM_PARTITION_TABLE = SMEM_SMD_FIFO_BASE_ID + SMEM_NUM_SMD_STREAM_CHANNELS,
    SMEM_POWER_ON_STATUS_INFO,
    SMEM_DAL_AREA,
    SMEM_SMEM_LOG_POWER_IDX,
    SMEM_SMEM_LOG_POWER_WRAP,
    SMEM_SMEM_LOG_POWER_EVENTS,
    SMEM_ERR_CRASH_LOG,
    SMEM_ERR_F3_TRACE_LOG,
    SMEM_NUM_ITEMS, /* 498 */
};

void save_modem_reset_log(char reason[], int reasonLength);
void wpss_reset_save_log(char reason[], int reasonLength);

int create_modem_log_queue(void);
void destroy_modem_log_queue(void);

#endif