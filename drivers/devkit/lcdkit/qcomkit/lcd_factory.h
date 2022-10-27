/*
 * lcd_factory.c
 *
 * lcd factory test function for lcd driver
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
#ifndef LCD_FACTORY_H
#define LCD_FACTORY_H

#define LCD_CMD_NAME_MAX 100
#define MAX_REG_READ_COUNT 4

/* pcd errflag detect */
#define PCD_ERRFLAG_SUCCESS	0
#define PCD_FAIL		1
#define LCD_KIT_PCD_SIZE	3
#define DMD_ERR_INFO_LEN	50

#define HOR_LINE_TEST_TIMES	3
#define MILLISEC_TIME	1000
#define LCD_ESD_ENABLE	0
#define LCD_ESD_DISABLE	1
#define LCD_RST_DOWN_TIME	300

/* enum */
enum inversion_mode {
	COLUMN_MODE = 0,
	DOT_MODE,
};

struct lcd_inversion {
	u32 support;
	u32 mode;
};

/* vertical line test picture index */
enum {
	PIC1_INDEX,
	PIC2_INDEX,
	PIC3_INDEX,
	PIC4_INDEX,
	PIC5_INDEX,
};

/* pcd errflag detect */
enum {
	PCD_COMPARE_MODE_EQUAL = 0,
	PCD_COMPARE_MODE_BIGGER = 1,
	PCD_COMPARE_MODE_MASK = 2,
};

struct lcd_checkreg {
	bool enabled;
	int expect_count;
	uint8_t *expect_val;
};

struct lcd_pcd_errflag {
	int pcd_support;
	int pcd_value_compare_mode;
	int exp_pcd_mask;
	int pcd_det_num;
	int pcd_value;
};

struct horizontal_line_desc {
	int hor_support;
	int hor_duration;
	int hor_no_reset;
};

struct vertical_line_desc {
	int vtc_support;
	int vtc_no_reset;
};

struct lcd_fact_info {
	/* test config */
	char lcd_cmd_now[LCD_CMD_NAME_MAX];
	int pt_flag;
	int pt_reset_enable;
	struct lcd_checkreg checkreg;
	struct lcd_pcd_errflag pcd_errflag_check;
	struct vertical_line_desc vtc_line_test;
	struct horizontal_line_desc hor_line_test;
	struct lcd_inversion inversion;
};

int lcd_inversion_get_mode(char *buf);
#endif
