/*
 * lcd_panel.h
 *
 * lcd panel function for lcd driver
 *
 * Copyright (c) 2021-2022 Honor Technologies Co., Ltd.
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
#ifndef LCD_PANEL_H
#define LCD_PANEL_H
#if defined CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#ifdef CONFIG_LCD_FACTORY
#include "lcd_factory.h"
#endif

#include "lcd_sysfs.h"
#include <linux/semaphore.h>

#define HBM_EXIT	0
#define HBM_ENTER	1
#define AOD_LK_BL	100
#define AOD_ULK_BL	101

#define LCD_ESD_OK 1
#define LCD_ESD_ERROR (-1)

#if defined CONFIG_HUAWEI_DSM

#define DMD_RECORD_BUF_LEN 100
#endif

#define MAX_REG_READ_COUNT 4
#define MAX_REG_READ_ESD_COUNT 7

#define WORK_DELAY_TIME_READ_BATCH 20000
#define READ_PANEL_BATCH_NUM 1
#define READ_PANEL_BATCH_MAX_COUNT 3

#define PROJECTID_LEN 10

#define TP_PROXMITY_DISABLE 0
#define TP_PROXMITY_ENABLE  1

#define MAX_DELAY_TIME 200
/* LCD init step */
enum panel_init_step {
    PANEL_INIT_NONE = 0,
    PANEL_INIT_POWER_ON,
    PANEL_INIT_SEND_SEQUENCE,
    PANEL_INIT_MIPI_LP_SEND_SEQUENCE,
    PANEL_INIT_MIPI_HS_SEND_SEQUENCE,
};

/* LCD uninit step */
enum panel_uninit_step {
    PANEL_UNINIT_NONE = 0,
    PANEL_UNINIT_POWER_OFF,
    PANEL_UNINIT_SEND_SEQUENCE,
    PANEL_UNINIT_MIPI_LP_SEND_SEQUENCE,
    PANEL_UNINIT_MIPI_HS_SEND_SEQUENCE,
};

enum esd_judge_type {
	ESD_UNEQUAL,
	ESD_EQUAL,
	ESD_BIT_VALID,
};

enum panel_bias_type {
	PANEL_SET_BISA_VOL,
	PANEL_SET_BISA_POWER_DOWN,
	PANEL_SET_BISA_DISABLE,
};

struct hbm_desc {
	bool enabled;
	int mode;
};

struct lcd_kit_2d_barcode {
	u32 support;
	u32 offset;
	u8 barcode_data[OEM_INFO_SIZE_MAX];
};

struct lcd_kit_project_id {
	u32 support;
	u32 offset;
	char default_project_id[PROJECTID_LEN + 1];
	char id[PROJECTID_LEN + 1];
};

struct lcd_kit_sn_data {
	u32 support;
	unsigned char sn_code[LCD_SN_CODE_LENGTH];
};

struct lcd_kit_oem_info {
	u32 support;
	struct lcd_kit_2d_barcode barcode_2d;
	struct lcd_kit_sn_data sn_data;
	struct lcd_kit_project_id project_id;
};

struct panel_batch_info {
	bool support;
	bool batch_match_hbm;
	u32 cnt;
	u32 *expect_val;
	struct dsi_panel_cmd_set *cmd_set;
};

struct lcd_bias_info {
	bool support;
	bool enabled;
	int enable_gpio;
	int vsp_gpio;
	int vsn_gpio;
	u32 poweron_vsp_delay;
	u32 poweron_vsn_delay;
	u32 poweroff_vsp_delay;
	u32 poweroff_vsn_delay;
};

struct lcd_thp_proximity {
	bool support;
	int work_status;
	int panel_power_state;
	u32 reset_to_bias_delay;
	u32 reset_to_dispon_delay;
};

struct lcd_kit_quickly_sleep_out {
	bool support;
	u32 interval;
	u32 panel_on_tag;
	struct timeval panel_on_record_tv;
};

struct lcd_kit_power_key_info {
	unsigned int support;
	unsigned int long_press_flag;
	unsigned int timer_val;
	struct notifier_block nb;
	struct delayed_work pf_work;
};

struct panel_info {
	const char *lcd_model;
	int panel_state;
#ifdef CONFIG_LCD_FACTORY
	struct lcd_fact_info *fact_info;
	void *sde_connector;
	atomic_t lcd_esd_pending;
	atomic_t lcd_noti_comp;
	struct completion lcd_test_comp;
#endif
	struct hbm_desc hbm;
	struct lcd_kit_oem_info oeminfo;
	struct dsi_panel *panel;
	struct lcd_bias_info bias;
	bool local_hbm_enabled;
	u32 *fps_list;
	u32 fps_list_len;
	bool power_on;
	bool four_byte_bl;
	bool ts_poweroff_in_lp_step;
	bool ts_poweroff_before_disp_off;
	bool reset_tp_depend_lcd;
	u32 reset_tp_lcd_gap_sleep;
	u32 reset_tp_pull_low_sleep;
	struct panel_batch_info lcd_panel_batch_info;
	struct lcd_thp_proximity proximity;
	struct lcd_kit_quickly_sleep_out quickly_sleep_out;
	struct semaphore proximity_poweroff_sem;
	struct lcd_kit_power_key_info pwrkey_press;
};

struct panel_data {
	struct panel_info *pinfo;
	struct platform_ops *plat_ops;
	int (*create_sysfs)(struct kobject *obj);
	int (*panel_init)(struct dsi_panel *panel);
	int (*panel_hbm_set)(struct dsi_panel *panel,
		struct display_engine_ddic_hbm_param *hbm_cfg);
	int (*panel_hbm_fp_set)(struct dsi_panel *panel, int mode);
	int (*on)(struct dsi_panel *panel, int step);
	int (*off)(struct dsi_panel *panel, int step);
	int (*print_bkl)(struct dsi_panel *panel, u32 bl_lvl);
	int (*boot_set)(struct dsi_panel *panel);
	int (*set_backlight)(u32 bl_lvl);
	int (*en_backlight)(bool enable);
	void (*disp_on_check_delay)(struct dsi_panel *panel);
	void (*reset_tp_depend_lcd)(void);
	void (*set_proximity_sem)(bool lock);
	int (*get_proxmity_status)(int sem_lock);
};

struct platform_ops {
	int (*read_project_id)(struct dsi_panel *panel, u8 *recv_buf, u32 recv_buf_len);
	int (*send_column_inversion_cmd)(struct dsi_panel *panel);
	int (*send_dot_inversion_cmd)(struct dsi_panel *panel);
	int (*force_power_off)(struct dsi_panel *panel);
};

extern struct panel_data g_panel_data;

/* oem info cmd */
struct oem_info_cmd {
	unsigned char type;
	int (*func)(struct panel_info *pinfo, char *oem_data, int len);
};

#if defined CONFIG_HUAWEI_DSM
int lcd_dsm_client_record(struct dsm_client *lcd_dclient, char *record_buf,
	int lcd_dsm_error_no);
#endif
#endif
