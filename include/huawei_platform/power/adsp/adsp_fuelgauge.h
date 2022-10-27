
/*
 * adsp_fuelgauge_dts.h
 *
 * driver for adsp fuelgauge dts
 *
 * Copyright (c) 2021-2021 Honor Technologies Co., Ltd.
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

#ifndef _ADSP_FUELGAUGE_H_
#define _ADSP_FUELGAUGE_H_

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/fs.h>
#include <linux/power_supply.h>
#include <linux/delay.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/uaccess.h>
#include <linux/debugfs.h>
#include <linux/seq_file.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>

#define MAX_BATT_NAME			32
#define MAX_FUELGAUGE_NAME		32
#define MAX_SE_INSTANCE_NUM		16

#define ADSP_BATTERY_WORK_INTERVAL		1000

enum se_ins_map_info {
	SE_INS_MAP_QUP_INDEX,
	SE_INS_MAP_SE_INDEX,
	SE_INS_MAP_INS_INDEX,
	SE_INS_MAP_TOTAL,
};

struct se_instance_map {
	int qup_index;
	int se_index;
	int ins_index;
};

struct batt_model_info {
	u32 voltage_max_design;
	u32 charge_full_design;
	char brand[MAX_BATT_NAME]; 
};

struct adsp_fg_device {
	struct device *dev;
	struct delayed_work sync_work;
	struct delayed_work event_work;
	struct batt_model_info batt_model;
	char batt_model_name[MAX_BATT_NAME];
	int qup_index;
	int se_index;
	int ins_index;
	struct se_instance_map se_ins_map[MAX_SE_INSTANCE_NUM];
	u32 fuelgauge_type;
	void *fuel_para;
	int para_size;
	void *fuel_aging_para;
};

struct fuelgauge_info {
	u32 fuelgauge_type;
	char fuelgauge_name[MAX_FUELGAUGE_NAME];
	struct coul_cali_ops *cali_ops;
	int (*parse_para)(struct device_node *np, 
		const char *batt_model_name, void **fuel_para, int *para_size);
	int (*parse_aging_para)(struct device_node *np, 
		const char *batt_model_name, void **fuel_para, int *para_size);
};

enum fuel_type_info {
	FUELGAUGE_TYPE_BEGIN = 0,
	FUELGAUGE_TYPE_RT9426,
	FUELGAUGE_TYPE_RT9426A,
	FUELGAUGE_TYPE_BQ27Z561,
	FUELGAUGE_TYPE_CW2217,
	FUELGAUGE_TYPE_END
};

#endif /* _ADSP_FUELGAUGE_H_ */


