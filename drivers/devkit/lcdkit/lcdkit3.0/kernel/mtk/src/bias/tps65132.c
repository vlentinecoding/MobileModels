/*
 * tps65132.c
 *
 * tps65132 bias driver
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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
#include <linux/i2c.h>
#include <asm/unaligned.h>
#include <linux/gpio.h>
#include "tps65132.h"
#include "lcd_kit_common.h"
#include "lcd_kit_core.h"
#include "lcd_kit_bias.h"
#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
#include <huawei_platform/dev_detect/hw_dev_detect.h>
#endif
#ifdef CONFIG_DRM_MEDIATEK
#include "lcd_kit_drm_panel.h"
#endif

static int is_ocp2138_support = 0;
static struct tps65132_device_info *dev_info = NULL;
static int bias_hw_en = 0;
static int bias_hw_en_gpio = 0;

#define DTS_COMP_TPS65132 "ti,tps65132"
static struct tps65132_voltage voltage_table[] = {
	{ 4000000, TPS65132_VOL_40 },
	{ 4100000, TPS65132_VOL_41 },
	{ 4200000, TPS65132_VOL_42 },
	{ 4300000, TPS65132_VOL_43 },
	{ 4400000, TPS65132_VOL_44 },
	{ 4500000, TPS65132_VOL_45 },
	{ 4600000, TPS65132_VOL_46 },
	{ 4700000, TPS65132_VOL_47 },
	{ 4800000, TPS65132_VOL_48 },
	{ 4900000, TPS65132_VOL_49 },
	{ 5000000, TPS65132_VOL_50 },
	{ 5100000, TPS65132_VOL_51 },
	{ 5200000, TPS65132_VOL_52 },
	{ 5300000, TPS65132_VOL_53 },
	{ 5400000, TPS65132_VOL_54 },
	{ 5500000, TPS65132_VOL_55 },
	{ 5600000, TPS65132_VOL_56 },
	{ 5700000, TPS65132_VOL_57 },
	{ 5800000, TPS65132_VOL_58 },
	{ 5900000, TPS65132_VOL_59 },
	{ 6000000, TPS65132_VOL_60 },
};

static int tps65132_reg_init(struct i2c_client *client, unsigned char vpos_cmd, unsigned char vneg_cmd)
{
	unsigned char vpos = 0;
	unsigned char vneg = 0;
	unsigned char app_dis = 0;
	unsigned char ctl = 0;
	int ret;

	vpos = (unsigned char)i2c_smbus_read_byte_data(client, TPS65132_REG_VPOS);
	if (vpos < 0) {
		LCD_KIT_ERR("[%s]:read vpos voltage failed\n", __func__);
		goto exit;
	}

	vneg = (unsigned char)i2c_smbus_read_byte_data(client, TPS65132_REG_VNEG);
	if (vpos < 0) {
		LCD_KIT_ERR("[%s]:read vneg voltage failed\n", __func__);
		goto exit;
	}
	LCD_KIT_INFO("read default bias config:VSP:0x%x, VSN:0x%x\n", vpos, vneg);
	if (!is_ocp2138_support) {
		app_dis = i2c_smbus_read_byte_data(client, TPS65132_REG_APP_DIS);
		if (app_dis < 0) {
			LCD_KIT_ERR("[%s]:read app_dis failed\n", __func__);
			goto exit;
		}

		ctl = i2c_smbus_read_byte_data(client, TPS65132_REG_CTL);
		if (ctl < 0) {
			LCD_KIT_ERR("[%s]:read ctl failed\n", __func__);
			goto exit;
		}
		LCD_KIT_ERR("[%s]:read_byte app_dis:0x%x, ctl:0x%x\n", __func__, app_dis, ctl);
	}
	vpos = (vpos & (~TPS65132_REG_VOL_MASK)) | vpos_cmd;
	vneg = (vneg & (~TPS65132_REG_VOL_MASK)) | vneg_cmd;

	if (!is_ocp2138_support) {
		app_dis = app_dis | TPS65312_APPS_BIT | TPS65132_DISP_BIT |
			TPS65132_DISN_BIT;
		ctl = ctl | TPS65132_WED_BIT;
	}
	ret = i2c_smbus_write_byte_data(client, TPS65132_REG_VPOS, vpos);
	if (ret < 0) {
		LCD_KIT_ERR("[%s]:write vpos failed\n", __func__);
		goto exit;
	}

	ret = i2c_smbus_write_byte_data(client, TPS65132_REG_VNEG, vneg);
	if (ret < 0) {
		LCD_KIT_ERR("[%s]:write vneg failed\n", __func__);
		goto exit;
	}
	if (!is_ocp2138_support) {
		ret = i2c_smbus_write_byte_data(client, TPS65132_REG_APP_DIS, app_dis);
		if (ret < 0) {
			LCD_KIT_ERR("%s write app_dis failed\n", __func__);
			goto exit;
		}

		ret = i2c_smbus_write_byte_data(client, TPS65132_REG_CTL, ctl);
		if (ret < 0) {
			LCD_KIT_ERR("%s write ctl failed\n", __func__);
			goto exit;
		}
	}
exit:
	return ret;
}

void tps65132_set_voltage(int vpos, int vneg)
{
	int ret;

	if ((vpos < TPS65132_VOL_40) || (vpos > TPS65132_VOL_60)) {
		LCD_KIT_ERR("set vpos error, vpos = %d is out of range", vpos);
		return;
	}

	if ((vneg < TPS65132_VOL_40) || (vneg > TPS65132_VOL_60)) {
		LCD_KIT_ERR("set vneg error, vneg = %d is out of range", vneg);
		return;
	}
	if (dev_info == NULL)
		return;

	if (bias_hw_en) {
		ret = gpio_request(bias_hw_en_gpio, NULL);
		if (ret)
			LCD_KIT_ERR("Could not request hw_en_gpio\n");
		ret = gpio_direction_output(bias_hw_en_gpio, GPIO_DIR_OUT);
		if (ret)
			LCD_KIT_ERR("Set gpio output not success\n");
		gpio_set_value(bias_hw_en_gpio, GPIO_OUT_ONE);

		mdelay(HW_EN_DELAY);
	}
	ret = tps65132_reg_init(dev_info->client, vpos, vneg);
	if (ret)
		LCD_KIT_ERR("tps65132_reg_init not success\n");
	else
		LCD_KIT_INFO("tps65132 inited succeed\n");
}

static int tps65132_set_bias(int vpos, int vneg)
{
	int i;
	int vol_size = ARRAY_SIZE(voltage_table);

	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vpos) {
			vpos = voltage_table[i].value;
			break;
		}
	}
	if (i >= vol_size) {
		LCD_KIT_ERR("not found vsp voltage, use default voltage\n");
		vpos = TPS65132_VOL_55;
	}
	for (i = 0; i < vol_size; i++) {
		if (voltage_table[i].voltage == vneg) {
			vneg = voltage_table[i].value;
			break;
		}
	}

	if (i >= vol_size) {
		LCD_KIT_ERR("not found vsn voltage, use default voltage\n");
		vneg = TPS65132_VOL_55;
	}
	LCD_KIT_INFO("vpos = 0x%x, vneg = 0x%x\n", vpos, vneg);
	tps65132_set_voltage(vpos, vneg);
	return LCD_KIT_OK;
}

static struct lcd_kit_bias_ops bias_ops = {
	.set_bias_voltage = tps65132_set_bias,
	.dbg_set_bias_voltage = NULL,
};

static int tps65132_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int retval = 0;
	struct device_node *np = NULL;
#ifdef CONFIG_DRM_MEDIATEK
	struct mtk_panel_info *plcd_kit_info = NULL;
#endif

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_TPS65132);
	if (!np) {
		LCD_KIT_ERR("NOT FOUND device node %s!\n", DTS_COMP_TPS65132);
		retval = -ENODEV;
		goto failed_1;
	}
	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
		LCD_KIT_ERR("[%s,%d]: need I2C_FUNC_I2C\n", __func__, __LINE__);
		retval = -ENODEV;
		goto failed_1;
	}
	dev_info = kzalloc(sizeof(*dev_info), GFP_KERNEL);
	if (!dev_info) {
		dev_err(&client->dev, "failed to allocate device info data\n");
		retval = -ENOMEM;
		goto failed_1;
	}
	i2c_set_clientdata(client, dev_info);
	dev_info->dev = &client->dev;
	dev_info->client = client;

	retval = of_property_read_u32(np, "ocp2138_support", &is_ocp2138_support);
	if (retval < 0) {
		LCD_KIT_ERR("NOT FOUND ocp2138_support!\n");
	}
	retval = of_property_read_u32(np, "bias_hw_en", &bias_hw_en);
	if (retval < 0)
		LCD_KIT_ERR("get bias_hw_enable dts config failed\n");

	if (bias_hw_en != 0) {
		retval = of_property_read_u32(np, "bias_hw_en_gpio",
			&bias_hw_en_gpio);
		if (retval < 0)
			LCD_KIT_ERR("get bias_hw_en_gpio dts config failed\n");
	}
	/* gpio number offset */
#ifdef CONFIG_DRM_MEDIATEK
	plcd_kit_info = lcm_get_panel_info();
	if (plcd_kit_info != NULL && bias_hw_en_gpio != 0)
		bias_hw_en_gpio += plcd_kit_info->gpio_offset;
#endif

#ifdef CONFIG_HUAWEI_DEV_SELFCHECK
	/* detect current device successful, set the flag as present */
	set_hw_dev_detect_result(DEV_DETECT_DC_DC);
#endif
	lcd_kit_bias_register(&bias_ops);
	return retval;

failed_1:
	return retval;
}

static const struct of_device_id tps65132_match_table[] = {
	{
		.compatible = DTS_COMP_TPS65132,
		.data = NULL,
	},
	{},
};


static const struct i2c_device_id tps65132_i2c_id[] = {
	{ "tps65132", 0 },
	{}
};

MODULE_DEVICE_TABLE(of, tps65132_match_table);


static struct i2c_driver tps65132_driver = {
	.id_table = tps65132_i2c_id,
	.probe = tps65132_probe,
	.driver = {
		.name = "tps65132",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(tps65132_match_table),
	},
};

static int __init tps65132_module_init(void)
{
	int ret;

	ret = i2c_add_driver(&tps65132_driver);
	if (ret)
		LCD_KIT_ERR("Unable to register tps65132 driver\n");
	return ret;
}
static void __exit tps65132_exit(void)
{
	i2c_del_driver(&tps65132_driver);
}

module_init(tps65132_module_init);
module_exit(tps65132_exit);

MODULE_DESCRIPTION("TPS65132 driver");
MODULE_LICENSE("GPL");
