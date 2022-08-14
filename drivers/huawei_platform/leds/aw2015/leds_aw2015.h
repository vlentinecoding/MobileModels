/*
 * Copyright (c) 2015, The Linux Foundation. All rights reserved.
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

#ifndef __LINUX_AW2015_LED_H__
#define __LINUX_AW2015_LED_H__

#define AW2015_TIME_PT_REG_VAL_0       0x0
#define AW2015_TIME_PT_REG_VAL_1       0x1
#define AW2015_TIME_PT_REG_VAL_2       0x2
#define AW2015_TIME_PT_REG_VAL_3       0x3
#define AW2015_TIME_PT_REG_VAL_4       0x4
#define AW2015_TIME_PT_REG_VAL_5       0x5
#define AW2015_TIME_PT_REG_VAL_6       0x6
#define AW2015_TIME_PT_REG_VAL_7       0x7
#define AW2015_TIME_PT_REG_VAL_8       0x8
#define AW2015_TIME_PT_REG_VAL_9       0x9
#define AW2015_TIME_PT_REG_VAL_A       0xA
#define AW2015_TIME_PT_REG_VAL_B       0xB
#define AW2015_TIME_PT_REG_VAL_C       0xC
#define AW2015_TIME_PT_REG_VAL_D       0xD
#define AW2015_TIME_PT_REG_VAL_E       0xE
#define AW2015_TIME_PT_REG_VAL_F       0xF

#define AW2015_TIME_0MS         0
#define AW2015_TIME_130MS       130
#define AW2015_TIME_260MS       260
#define AW2015_TIME_380MS       380
#define AW2015_TIME_510MS       510
#define AW2015_TIME_770MS       770
#define AW2015_TIME_1040MS      1040
#define AW2015_TIME_1600MS      1600
#define AW2015_TIME_2100MS      2100
#define AW2015_TIME_2600MS      2600
#define AW2015_TIME_3100MS      3100
#define AW2015_TIME_4200MS      4200
#define AW2015_TIME_5200MS      5200
#define AW2015_TIME_6200MS      6200
#define AW2015_TIME_7300MS      7300
#define AW2015_TIME_8300MS      8300

/* The definition of each time described as shown in figure.
 *        /-----------\
 *       /      |      \
 *      /|      |      |\
 *     / |      |      | \-----------
 *       |hold_time_ms |      |
 *       |             |      |
 * rise_time_ms  fall_time_ms |
 *                       off_time_ms
 */

struct aw2015_platform_data {
	int imax;
	int led_current;
	int rise_time_ms;
	int hold_time_ms;
	int fall_time_ms;
	int off_time_ms;
	struct aw2015_led *led;
};

struct aw2015_pattern_time {
	uint8_t value; // reg value
	uint16_t period_ms; // time of ms
} aw2015_pattern_time_table[] = {
	{ AW2015_TIME_PT_REG_VAL_0, AW2015_TIME_0MS },
	{ AW2015_TIME_PT_REG_VAL_1, AW2015_TIME_130MS },
	{ AW2015_TIME_PT_REG_VAL_2, AW2015_TIME_260MS },
	{ AW2015_TIME_PT_REG_VAL_3, AW2015_TIME_380MS },
	{ AW2015_TIME_PT_REG_VAL_4, AW2015_TIME_510MS },
	{ AW2015_TIME_PT_REG_VAL_5, AW2015_TIME_770MS },
	{ AW2015_TIME_PT_REG_VAL_6, AW2015_TIME_1040MS },
	{ AW2015_TIME_PT_REG_VAL_7, AW2015_TIME_1600MS },
	{ AW2015_TIME_PT_REG_VAL_8, AW2015_TIME_2100MS },
	{ AW2015_TIME_PT_REG_VAL_9, AW2015_TIME_2600MS },
	{ AW2015_TIME_PT_REG_VAL_A, AW2015_TIME_3100MS },
	{ AW2015_TIME_PT_REG_VAL_B, AW2015_TIME_4200MS },
	{ AW2015_TIME_PT_REG_VAL_C, AW2015_TIME_5200MS },
	{ AW2015_TIME_PT_REG_VAL_D, AW2015_TIME_6200MS },
	{ AW2015_TIME_PT_REG_VAL_E, AW2015_TIME_7300MS },
	{ AW2015_TIME_PT_REG_VAL_F, AW2015_TIME_8300MS },
};

#endif
