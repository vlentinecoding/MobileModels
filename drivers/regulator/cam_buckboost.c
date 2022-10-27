/*
 * cam_buckboost.c
 *
 * buckboost driver
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

/* register control */
#define BUCKBOOST_CONTROL_ADDR        0x01
#define BUCKBOOST_CONTROL_RANGE_MASK  BIT(6)
#define BUCKBOOST_CONTROL_RANGE_SHIFT 6
#define BUCKBOOST_LOW_RANGE           0
#define BUCKBOOST_HIGH_RANGE          1

/* register status */
#define BUCKBOOST_STATUS_ADDR         0x02
#define BUCKBOOST_STATUS_HD_MASK      BIT(4)
#define BUCKBOOST_STATUS_HD_SHIFT     4
#define BUCKBOOST_STATUS_UV_MASK      BIT(3)
#define BUCKBOOST_STATUS_UV_SHIFT     3
#define BUCKBOOST_STATUS_OC_MASK      BIT(2)
#define BUCKBOOST_STATUS_OC_SHIFT     2
#define BUCKBOOST_STATUS_TSD_MASK     BIT(1)
#define BUCKBOOST_STATUS_TSD_SHIFT    1
#define BUCKBOOST_STATUS_PG_MASK      BIT(0)
#define BUCKBOOST_STATUS_PG_SHIFT     0

/* register devid */
#define BUCKBOOST_DEVID_ADDR          0x03
#define BUCKBOOST_DEVID_TI            0x04
#define BUCKBOOST_DEVID_RT            0xa8
#define BUCKBOOST_DEVID_AU            0xd0
#define TPS63810_REGULATOR_NAME       "tps63810-reg"
#define AU8310_REGULATOR_NAME         "au8310-reg"
#define RT6160_REGULATOR_NAME         "rt6160-reg"

/* register vout1 */
#define BUCKBOOST_VOUT1_ADDR          0x04
#define BUCKBOOST_VOUT1_HR_MIN        2025
#define BUCKBOOST_VOUT1_HR_MAX        5200
#define BUCKBOOST_VOUT1_STEP          25

#define RT6160_VOUT1_DEFAULT          3300
#define RT6160_VOUT1_MIN              2025
#define RT6160_VOUT1_MAX              5200
#define RT6160_VOUT1_STEP             25
#define BUCKBOOST_NVOLTAGES           128
#define VSEL_BUCK_EN                  (1 << 6 | 1 << 5 | 1 << 3) // High range & enable
#define BUCKBOOST_VOUT2_ADDR          0x05
unsigned int g_enable_gpio;

int buckboost_regulator_enable(struct regulator_dev *rdev)
{
	if (strcmp(rdev->desc->name, RT6160_REGULATOR_NAME) == 0) {
		dev_info(&rdev->dev, "rt6160 buckboost not need set enable reg");
		return 0;
	}

	dev_info(&rdev->dev, "buckboost need set enable reg");
	return regulator_enable_regmap(rdev);
}

int buckboost_regulator_disable(struct regulator_dev *rdev)
{
	int ret;

	if (!strcmp(rdev->desc->name, TPS63810_REGULATOR_NAME)) {
		dev_info(&rdev->dev, "TPS63810 buckboost need set disable reg");
		regulator_disable_regmap(rdev);
	}

	ret = gpio_direction_output(g_enable_gpio, 0);
	if (ret < 0) {
		dev_err(&rdev->dev, "fail to set gpio[%d] high, ret = %d\n",
			g_enable_gpio, ret);
		return ret;
	}

	return ret;
}

int buckboost_regulator_set_voltage_sel_regmap(struct regulator_dev *rdev,
	unsigned sel)
{
	int ret;

	dev_info(&rdev->dev, "%s, enter, sel = %d", __func__, sel);
	if (sel == 0)
		return 0;

	if (strcmp(rdev->desc->name, TPS63810_REGULATOR_NAME) == 0) { // TI ---> 4.1V
		sel += 4;
		dev_info(&rdev->dev, "%s, TI sel update to %d", __func__, sel);
	}

	ret = gpio_direction_output(g_enable_gpio, 1);
	if (ret < 0) {
		dev_err(&rdev->dev, "fail to set gpio[%d] high, ret = %d\n",
			g_enable_gpio, ret);
		return ret;
	}
	usleep_range(1000, 1000);
	ret = buckboost_regulator_enable(rdev); // driver will not auto use enable fun else RT.
	if (ret < 0) {
		dev_err(&rdev->dev, "%s, regulator_enable fail, ret = %d", __func__, ret);
	}

	return regulator_set_voltage_sel_regmap(rdev, sel);
}

static const struct regulator_ops buckboost_regulator_ops = {
	.set_voltage_sel = buckboost_regulator_set_voltage_sel_regmap,
	.get_voltage_sel = regulator_get_voltage_sel_regmap,
	.enable = buckboost_regulator_enable,
	.disable = buckboost_regulator_disable,
	.is_enabled = regulator_is_enabled_regmap,
	.map_voltage = regulator_map_voltage_linear,
	.list_voltage = regulator_list_voltage_linear,
};

static int buckboost_device_setup(struct buck_device_info *di,
	struct buck_platform_data *pdata)
{
	int ret = 0;

	/* Setup voltage control register */
	switch (pdata->sleep_vsel_id) {
	case VSEL_ID_0:
		di->sleep_reg = BUCKBOOST_VOUT1_ADDR;
		di->vol_reg = BUCKBOOST_VOUT2_ADDR;
		di->enable_reg = BUCKBOOST_CONTROL_ADDR;
		break;
	case VSEL_ID_1:
		di->sleep_reg = BUCKBOOST_VOUT2_ADDR;
		di->vol_reg = BUCKBOOST_VOUT1_ADDR;
		di->enable_reg = BUCKBOOST_CONTROL_ADDR;
		break;
	default:
		dev_err(di->dev, "Invalid VSEL ID!\n");
		return -EINVAL;
	}

	/* Init voltage range and step */
	di->vsel_min = 2025000;
	di->vsel_step = 25000;
	di->vsel_count = BUCKBOOST_NVOLTAGES;

	return ret;
}

static int buckboost_regulator_register(struct buck_device_info *di,
	struct regulator_config *config)
{
	struct regulator_desc *rdesc = &di->desc;
	struct regulator_dev *rdev;

	if (di->chip_id == BUCKBOOST_DEVID_TI) {
		rdesc->name = TPS63810_REGULATOR_NAME;
	} else if (di->chip_id == BUCKBOOST_DEVID_AU) {
		rdesc->name = AU8310_REGULATOR_NAME;
	} else {
		rdesc->name = RT6160_REGULATOR_NAME;
	}

	rdesc->supply_name = "vin";
	rdesc->ops = &buckboost_regulator_ops;
	rdesc->type = REGULATOR_VOLTAGE;
	rdesc->n_voltages = di->vsel_count;
	rdesc->enable_reg = di->enable_reg;
	rdesc->enable_mask = VSEL_BUCK_EN;
	rdesc->min_uV = di->vsel_min;
	rdesc->uV_step = di->vsel_step;
	rdesc->vsel_reg = di->vol_reg;
	rdesc->vsel_mask = di->vsel_count - 1;
	rdesc->owner = THIS_MODULE;

	rdev = devm_regulator_register(di->dev, &di->desc, config);

	return PTR_ERR_OR_ZERO(rdev);
}

static const struct regmap_config buckboost_regmap_config = {
	.reg_bits = 8,
	.val_bits = 8,
};

static struct buck_platform_data *buckboost_parse_dt(struct device *dev,
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
	g_enable_gpio = enable_gpio;

	return pdata;
}

static const struct of_device_id buckboost_dt_ids[] = {
	{
		.compatible = "honor,cam_buckboost",
	},
	{ }
};
MODULE_DEVICE_TABLE(of, buckboost_dt_ids);

static int buckboost_regulator_probe(struct i2c_client *client,
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
		pdata = buckboost_parse_dt(&client->dev, np, &di->desc);

	if (!pdata || !pdata->regulator) {
		dev_err(&client->dev, "Platform data not found!\n");
		return -ENODEV;
	}
	di->regulator = pdata->regulator;

	regmap = devm_regmap_init_i2c(client, &buckboost_regmap_config);
	if (IS_ERR(regmap)) {
		dev_err(&client->dev, "Failed to allocate regmap!\n");
		return PTR_ERR(regmap);
	}

	di->dev = &client->dev;
	i2c_set_clientdata(client, di);

	ret = gpio_request(pdata->enable_gpio, "buckboost");
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
	usleep_range(1000, 1000);
	ret = regmap_read(regmap, BUCKBOOST_DEVID_ADDR, &val);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to get chip ID!\n");
		goto EXIT;
	}
	di->chip_id = val;
	dev_info(&client->dev, "buckboost chip_id = 0x%x\n", di->chip_id);

	/* Device init */
	ret = buckboost_device_setup(di, pdata);
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

	ret = buckboost_regulator_register(di, &config);
	if (ret < 0) {
		dev_err(&client->dev, "Failed to register regulator!\n");
		goto EXIT;
	}
	dev_info(&client->dev, "probe succ\n");

EXIT:
	if (gpio_direction_output(pdata->enable_gpio, 0))
		dev_err(&client->dev, "Failed to set enable gpio to low!\n");

	return 0;
}

static const struct i2c_device_id buckboost_id[] = {
	{
		.name = "buckboost-regulator",
	},
	{ },
};
MODULE_DEVICE_TABLE(i2c, buckboost_id);

static struct i2c_driver buckboost_regulator_driver = {
	.driver = {
		.name = "buckboost-regulator",
		.of_match_table = of_match_ptr(buckboost_dt_ids),
	},
	.probe = buckboost_regulator_probe,
	.id_table = buckboost_id,
};

module_i2c_driver(buckboost_regulator_driver);

MODULE_DESCRIPTION("BUCKBOOST driver");
MODULE_LICENSE("GPL v2");
