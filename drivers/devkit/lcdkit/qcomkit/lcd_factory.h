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

struct lcd_fact_info {
	/* test config */
	char lcd_cmd_now[LCD_CMD_NAME_MAX];
	int pt_flag;
	int pt_reset_enable;
	struct lcd_checkreg checkreg;
	struct lcd_pcd_errflag pcd_errflag_check;
};

#endif
