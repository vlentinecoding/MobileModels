/*
 * buck.h
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

#ifndef __BUCK_H__
#define __BUCK_H__

/* VSEL ID */
enum {
	VSEL_ID_0 = 0,
	VSEL_ID_1,
};

struct buck_device_info {
	struct device *dev;
	struct regulator_desc desc;
	struct regulator_init_data *regulator;
	/* IC Type and Rev */
	int chip_id;
	int chip_rev;
	/* Voltage setting register */
	unsigned int vol_reg;
	unsigned int sleep_reg;
	unsigned int enable_reg;
	/* Voltage range and step(linear) */
	unsigned int vsel_min;
	unsigned int vsel_step;
	unsigned int vsel_count;
	/* Mode */
	unsigned int mode_reg;
	unsigned int mode_mask;
};

struct buck_platform_data {
	struct regulator_init_data *regulator;
	/* Sleep VSEL ID */
	unsigned int sleep_vsel_id;
	unsigned int enable_gpio;
};

#endif /* __BUCK_H__ */