/*
 * cam_buck.c
 *
 * buck driver
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

#include <linux/module.h>
#include <linux/param.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/of_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/regmap.h>
#include <linux/regulator/buck.h>

/* Voltage setting */
#define BUCK_VSEL0 0x00
#define BUCK_VSEL1 0x01
/* Control register */
#define BUCK_CONTROL 0x02
/* IC Type */
#define BUCK_ID1 0x03
/* IC mask version */
#define BUCK_ID2 0x04
/* Monitor register */
#define RT5748B_ENABLE 0x06           /* rt5748b */

/* VSEL bit definitions */
#define VSEL_BUCK_EN_FAN_KTB (1 << 7) /* fan53526 ktb8830 */
#define VSEL_BUCK_EN_RT (1 << 0)      /* rt5748b */
#define VSEL_MODE (1 << 6)
/* Chip ID and Verison */
#define DIE_ID 0x0F /* ID1 */
#define DIE_REV 0x0F /* ID2 */
#define FAN53526_ID 0x81
#define KTB8330_ID  0xA0
#define RT5748B_ID  0x00
/* Control bit definitions */
#define CTL_OUTPUT_DISCHG (1 << 7)
#define CTL_SLEW_MASK (0x7 << 4)
#define CTL_SLEW_SHIFT 4
#define CTL_RESET (1 << 2)
#define CTL_MODE_VSEL0_MODE BIT(0)
#define CTL_MODE_VSEL1_MODE BIT(1)

/* fan53526 ktb8830 */
#define BUCK_NVOLTAGES 128

/* rt5748b */
#define RT5748B_NVOLTAGES 200
#define RT5748B_SLAVE_ADDR 0x57

/* Low X0 Auto PFM/PWM
 * Low X1 Forced PWM
 * High 0X Auto PFM/PWM
 * High 1X Forced PWM
 */
static int buck_set_mode(struct regulator_dev *rdev, unsigned int mode)
{
	struct buck_device_info *di = rdev_get_drvdata(rdev);

	switch (mode) {
	case REGULATOR_MODE_FAST:
		regmap_update_bits(rdev->regmap, di->mode_reg,
			di->mode_mask, di->mode_mask);
		break;
	case REGULATOR_MODE_NORMAL:
		regmap_update_bits(rdev->regmap, di->vol_reg, di->mode_mask, 0);
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static unsigned int buck_get_mode(struct regulator_dev *rdev)
{
	struct buck_device_info *di = rdev_get_drvdata(rdev);
	unsigned int val;
	int ret = 0;

	ret = regmap_read(rdev->regmap, di->mode_reg, &val);
	if (ret < 0)
		return ret;
	if (val & di->mode_mask)
		return REGULATOR_MODE_FAST;
	else
		return REGULATOR_MODE_NORMAL;
}

static const struct regulator_ops buck_regulator_ops = {
	.set_voltage_sel = regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.set_voltage_time_sel = regulator_set_voltage_time_sel,
	.enable = regulator_enable_regmap,
	.disable = regulator_disable_regmap,
	.is_enabled = regulator_is_enabled_regmap,
	.map_voltage = regulator_map_voltage_linear,
	.list_voltage = regulator_list_voltage_linear,
	.set_mode = buck_set_mode,
	.get_mode = buck_get_mode,
};

/* For 00,01,03,05 options:
 * VOUT = 0.60V + NSELx * 10mV, from 0.60 to 1.23V.
 * For 04 option:
 * VOUT = 0.603V + NSELx * 12.826mV, from 0.603 to 1.411V.
 * */
static int buck_device_setup(struct buck_device_info *di,
	struct buck_platform_data *pdata)
{
	int ret = 0;

	/* Setup voltage control register */
	switch (pdata->sleep_vsel_id) {
	case VSEL_ID_0:
		di->sleep_reg = BUCK_VSEL0;
		di->vol_reg = BUCK_VSEL1;
		di->enable_reg = BUCK_VSEL1;
		break;
	case VSEL_ID_1:
		di->sleep_reg = BUCK_VSEL1;
		di->vol_reg = BUCK_VSEL0;
		di->enable_reg = BUCK_VSEL0;
		break;
	default:
		dev_err(di->dev, "Invalid VSEL ID!\n");
		return -EINVAL;
	}

	/* Setup mode control register */
	di->mode_reg = BUCK_CONTROL;
	switch (pdata->sleep_vsel_id) {
	case VSEL_ID_0:
		di->mode_mask = CTL_MODE_VSEL1_MODE;
		break;
	case VSEL_ID_1:
		di->mode_mask = CTL_MODE_VSEL0_MODE;
		break;
	}

	/* Init voltage range and step */
	if (di->chip_id == RT5748B_ID) {
		di->enable_reg = RT5748B_ENABLE;
		di->vsel_min = 300000;
		di->vsel_step = 5000;
		di->vsel_count = RT5748B_NVOLTAGES;
	} else {
		di->vsel_min = 600000;
		di->vsel_step = 6250;
		di->vsel_count = BUCK_NVOLTAGES;
	}

	return ret;
}

static int buck_regulator_register(struct buck_device_info *di,
	struct regulator_config *config)
{
	struct regulator_desc *rdesc = &di->desc;
	struct regulator_dev *rdev;

	if (di->chip_id == FAN53526_ID) {
		rdesc->name = "fan53526";
		rdesc->enable_mask = VSEL_BUCK_EN_FAN_KTB;
	} else if (di->chip_id == KTB8330_ID) {
		rdesc->name = "ktb8330";
		rdesc->enable_mask = VSEL_BUCK_EN_FAN_KTB;
	} else {
		rdesc->name = "rt5748b";
		rdesc->enable_mask = VSEL_BUCK_EN_RT;
	}

	rdesc->supply_name = "vin";
	rdesc->ops = &buck_regulator_ops;
	rdesc->type = REGULATOR_VOLTAGE;
	rdesc->n_voltages = di->vsel_count;
	rdesc->enable_reg = di->enable_reg;
	rdesc->min_uV = di->vsel_min;
	rdesc->uV_step = di->vsel_step;
	rdesc->vsel_reg = di->vol_reg;
	rdesc->vsel_mask = di->vsel_count - 1;
	rdesc->owner = THIS_MODULE;

	rdev = devm_regulator_register(di->dev, &di->desc, config);
	return PTR_ERR_OR_ZERO(rdev);
}

static const struct regmap_config buck_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static struct buck_platform_data *buck_parse_dt(struct device *dev,
	struct device_node *np, const struct regulator_desc *desc)
{
	struct buck_platform_data *pdata;
	int ret;
	u32 sleep_vsel_id;
	u32 enable_gpio;

	pdata = devm_kzalloc(dev, sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return NULL;

	pdata->regulator = of_get_regulator_init_data(dev, np, desc);

	ret = of_property_read_u32(np, "suspend-voltage-selector", &sleep_vsel_id);
	if (!ret)
		pdata->sleep_vsel_id = sleep_vsel_id;

	enable_gpio = of_get_named_gpio(np, "enable-gpio", 0);
	if (!gpio_is_valid(enable_gpio)) {
		dev_err(dev, "invalid gpio[%d]\n", enable_gpio);
	}
	pdata->enable_gpio = enable_gpio;

	return pdata;
}

static const struct of_device_id buck_dt_ids[] = {
	{
		.compatible = "honor,cam_buck",
	},
	{ }
};
MODULE_DEVICE_TABLE(of, buck_dt_ids);

static int buck_regulator_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	struct device_node *np = client->dev.of_node;
	struct buck_device_info *di;
	struct buck_platform_data *pdata;
	struct regulator_config config = { };
	struct regmap *regmap;
	unsigned int val;
	int ret;

	di = devm_kzalloc(&client->dev, sizeof(struct buck_device_info),
		GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	pdata = dev_get_platdata(&client->dev);
	if (!pdata)
		pdata = buck_parse_dt(&client->dev, np, &di->desc);

	if (!pdata || !pdata->regulator) {
		dev_err(&client->dev, "Platform data not found!\n");
		return -ENODEV;
	}
	di->regulator = pdata->regulator;

	regmap = devm_regmap_init_i2c(client, &buck_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&client->dev, "Failed to allocate regmap!\n");
		return PTR_ERR(regmap);
	}

	di->dev = &client->dev;
	i2c_set_clientdata(client, di);

	ret = gpio_request(pdata->enable_gpio, "buck");
	if (ret < 0) {
		dev_err(&client->dev, "fail to request gpio[%d], ret = %d\n",
			pdata->enable_gpio, ret);
		return ret;
	}

	ret = gpio_direction_output(pdata->enable_gpio, 1);
	if (ret < 0) {
		dev_err(&client->dev, "fail to set gpio[%d] high, ret = %d\n",
			pdata->enable_gpio, ret);
		gpio_free(pdata->enable_gpio);
		return ret;
	}
	/* Get chip ID */
	ret = regmap_read(regmap, BUCK_ID1, &val);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to get chip ID, update addr!\n");
		client->addr = RT5748B_SLAVE_ADDR; /* modify actual adress */
		ret = regmap_read(regmap, BUCK_ID1, &val);
		if (ret < 0) {
			dev_err(&client->dev, "Failed to get chip ID again!\n");
			goto EXIT;
		}
	}

	if (val == FAN53526_ID || val == KTB8330_ID || val == RT5748B_ID) {
		dev_info(&client->dev, "get buck chip id succ. id = 0x%x\n", val);
	} else {
		dev_err(&client->dev, "get buck chip id fail, 0x%x\n", val);
		goto EXIT;
	}
	di->chip_id = val;

	/* Device init */
	ret = buck_device_setup(di, pdata);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to setup device!\n");
		goto EXIT;
	}
	/* Register regulator */
	config.dev = di->dev;
	config.init_data = di->regulator;
	config.regmap = regmap;
	config.driver_data = di;
	config.of_node = np;

	ret = buck_regulator_register(di, &config);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to register regulator!\n");
		goto EXIT;
	}
	dev_info(&client->dev, "probe succ\n");
	return ret;

EXIT:
	if (gpio_direction_output(pdata->enable_gpio, 0))
		dev_err(&client->dev, "Failed to set enable gpio to low!\n");
	gpio_free(pdata->enable_gpio);
	return 0;
}

static const struct i2c_device_id buck_id[] = {
	{
		.name = "buck-regulator",
	},
	{ },
};
MODULE_DEVICE_TABLE(i2c, buck_id);

static struct i2c_driver buck_regulator_driver = {
	.driver = {
		.name = "buck-regulator",
		.of_match_table = of_match_ptr(buck_dt_ids),
	},
	.probe = buck_regulator_probe,
	.id_table = buck_id,
};

module_i2c_driver(buck_regulator_driver);

MODULE_DESCRIPTION("buck driver");
MODULE_LICENSE("GPL v2");