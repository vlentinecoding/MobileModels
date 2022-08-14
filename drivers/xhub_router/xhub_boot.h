/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Team:    Huawei DIVS
 * Date:    2020.07.20
 * Description: xhub boot module
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
 
#ifndef __LINUX_XHUB_CMU_H__
#define __LINUX_XHUB_CMU_H__
#include <linux/types.h>


#define IOMCU_CONFIG_SIZE   DDR_CONFIG_SIZE
#define IOMCU_CONFIG_START  DDR_CONFIG_ADDR_AP

#define SENSOR_MAX_RESET_TIME_MS 400
#define SENSOR_DETECT_AFTER_POWERON_TIME_MS 50
#define SENSOR_POWER_DO_RESET 0
#define SENSOR_POWER_NO_RESET 1
#define SENSOR_REBOOT_REASON_MAX_LEN 32

#define WARN_LEVEL 2
#define INFO_LEVEL 3

#define TPLCD_190 5
#define TPLCD_310 17
#define TPLCD2_190 18

#define DTS_COMP_190_B "190_207_6p72"
#define DTS_COMP_310_V "310_207_6p72"

#define DTS_COMP_190 "190"
#define DTS_COMP_310 "310"

#define SC_EXISTENCE   1
#define SC_INEXISTENCE 0

enum SENSOR_POWER_CHECK {
	SENSOR_POWER_STATE_OK = 0,
	SENSOR_POWER_STATE_INIT_NOT_READY,
	SENSOR_POWER_STATE_CHECK_ACTION_FAILED,
	SENSOR_POWER_STATE_CHECK_RESULT_FAILED,
	SENSOR_POWER_STATE_NOT_PMIC,
};

typedef struct {
	u32 mutex;
	u16 index;
	u16 pingpang;
	u32 buff;
	u32 ddr_log_buff_cnt;
	u32 ddr_log_buff_index;
	u32 ddr_log_buff_last_update_index;
} log_buff_t;

typedef enum DUMP_LOC {
	DL_NONE = 0,
	DL_TCM,
	DL_EXT,
	DL_BOTTOM = DL_EXT,
} dump_loc_t;

enum DUMP_REGION {
	DE_TCM_CODE,
	DE_DDR_CODE,
	DE_DDR_DATA,
	DE_BOTTOM = 16,
};

typedef struct dump_region_config {
	u8 loc;
	u8 reserved[3];
} dump_region_config_t;

typedef struct dump_config {
	u64 dump_addr;
	u32 dump_size;
	u32 reserved1;
	u64 ext_dump_addr;
	u32 ext_dump_size;
	u8 enable;
	u8 finish;
	u8 reason;
	u8 reserved2;
	dump_region_config_t elements[DE_BOTTOM];
} dump_config_t;

typedef struct {
	const char *dts_comp_mipi;
	uint8_t tplcd;
} lcd_module;

typedef struct {
	const char *dts_comp_lcd_model;
	uint8_t tplcd;
} lcd_model;

struct bright_data {
	uint32_t mipi_data;
	uint32_t bright_data;
	uint64_t time_stamp;
};

struct read_data_als_ud {
	float rdata;
	float gdata;
	float bdata;
	float irdata;
};

struct als_ud_config_t {
	u8 screen_status;
	u8 reserved[7]; // 7 is reserved nums
	u64 als_rgb_pa;
	struct bright_data bright_data_input;
	struct read_data_als_ud read_data_history;
};

struct config_on_ddr { // 200bytes, max size should less than 4KB
	u32 magic;
	dump_config_t dump_config;
	log_buff_t log_buff_cb_backup;
	u32 log_level;
	u64 reserved;
	struct als_ud_config_t als_ud_config;
	u8 phone_type_info[2]; // 2 is phone type info nums
	u8 rsv[6];
	u32 screen_status;
};

extern int (*api_xhub_mcu_recv) (const char *buf, unsigned int length);
extern int (*api_mculog_process) (const char *buf, unsigned int length);
int get_sensor_mcu_mode(void);
void sync_time_to_xhub(void);
extern int init_sensors_cfg_data_from_dts(void);
extern int send_status_req_to_mcu(void);
#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *xhub_get_shb_dclient(void);
#endif
void init_write_nv_work(void);
void close_nv_workqueue(void);
#endif /* __LINUX_XHUB_CMU_H__ */
