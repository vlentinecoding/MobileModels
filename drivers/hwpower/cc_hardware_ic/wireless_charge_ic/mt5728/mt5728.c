/*
 * mt5728.c
 *
 * mt5728 driver
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

#include <securec.h>
#include "mt5728.h"

#define HWLOG_TAG wireless_mt5728
HWLOG_REGIST();
#define MT_PRINTF_BUF_SIZE 32

static char g_mt_printf_buf[MT_PRINTF_BUF_SIZE];
static struct mt5728_dev_info *g_mt5728_di;
static struct wakeup_source g_mt5728_wakelock;

void mt5728_get_dev_info(struct mt5728_dev_info **di)
{
	if (!g_mt5728_di || !di)
		return;

	*di = g_mt5728_di;
}

bool mt5728_is_pwr_good(void)
{
	int gpio_val;
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di)
		return false;

	if (!di->g_val.mtp_chk_complete)
		return true;

	return true;
}

static int mt5728_i2c_read(struct i2c_client *client,
	u8 *cmd, int cmd_len, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!mt5728_is_pwr_good())
			return -WLC_ERR_I2C_R;
		if (!power_i2c_read_block(client, cmd, cmd_len, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_R;
}

static int mt5728_i2c_write(struct i2c_client *client, u8 *buf, int buf_len)
{
	int i;

	for (i = 0; i < I2C_RETRY_CNT; i++) {
		if (!mt5728_is_pwr_good())
			return -WLC_ERR_I2C_W;
		if (!power_i2c_write_block(client, buf, buf_len))
			return 0;
		power_usleep(DT_USLEEP_10MS);
	}

	return -WLC_ERR_I2C_W;
}

int mt5728_read_block(u16 reg, u8 *data, u8 len)
{
	u8 cmd[MT5728_ADDR_LEN];
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di || !data)
		return -WLC_ERR_PARA_NULL;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;

	return mt5728_i2c_read(di->client, cmd, MT5728_ADDR_LEN, data, len);
}

int mt5728_write_block(u16 reg, u8 *data, u8 data_len)
{
	u8 cmd[MT5728_ADDR_LEN + data_len];
	struct mt5728_dev_info *di = NULL;
	errno_t rc = EOK;

	mt5728_get_dev_info(&di);
	if (!di || !data)
		return -WLC_ERR_PARA_NULL;

	cmd[0] = reg >> BITS_PER_BYTE;
	cmd[1] = reg & BYTE_MASK;
	rc = memcpy_s(&cmd[MT5728_ADDR_LEN], data_len, data, data_len);
	if (rc != EOK) {
		hwlog_info("%s : memcpy_s is failed, rc = %d\n", __FUNCTION__, rc);
	}

	return mt5728_i2c_write(di->client, cmd, MT5728_ADDR_LEN + data_len);
}

int mt5728_read_byte(u16 reg, u8 *data)
{
	return mt5728_read_block(reg, data, BYTE_LEN);
}

int mt5728_read_word(u16 reg, u16 *data)
{
	int ret;
	u8 buff[WORD_LEN] = {0};

	ret = mt5728_read_block(reg, buff, WORD_LEN);
	if (ret)
		return -WLC_ERR_I2C_R;

	*data = buff[0] | buff[1] << BITS_PER_BYTE;
	return 0;
}

int mt5728_write_byte(u16 reg, u8 data)
{
	return mt5728_write_block(reg, &data, BYTE_LEN);
}

int mt5728_write_word(u16 reg, u16 data)
{
	u8 buff[WORD_LEN] = {0};

	buff[0] = data & BYTE_MASK;
	buff[1] = data >> BITS_PER_BYTE;

	return mt5728_write_block(reg, buff, WORD_LEN);
}

int mt5728_read_byte_mask(u16 reg, u8 mask, u8 shift, u8 *data)
{
	int ret;
	u8 val = 0;

	ret = mt5728_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*data = val;

	return 0;
}

int mt5728_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data)
{
	int ret;
	u8 val = 0;

	ret = mt5728_read_byte(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return mt5728_write_byte(reg, val);
}

int mt5728_read_word_mask(u16 reg, u16 mask, u16 shift, u16 *data)
{
	int ret;
	u16 val = 0;

	ret = mt5728_read_word(reg, &val);
	if (ret)
		return ret;

	val &= mask;
	val >>= shift;
	*data = val;

	return 0;
}

int mt5728_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data)
{
	int ret;
	u16 val = 0;

	ret = mt5728_read_word(reg, &val);
	if (ret)
		return ret;

	val &= ~mask;
	val |= ((data << shift) & mask);

	return mt5728_write_word(reg, val);
}

int mt5728_get_hw_chip_id(u16 *chip_id)
{
	return mt5728_read_word(MT5728_HW_CHIP_ID_ADDR, chip_id);
}

int mt5728_get_chip_id(u16 *chip_id)
{
	return mt5728_read_word(MT5728_CHIP_ID_ADDR, chip_id);
}

static int mt5728_get_chip_info(struct mt5728_chip_info *info)
{
	int ret;
	u8 chip_info[MT5728_CHIP_INFO_LEN] = {0};

	ret = mt5728_read_block(MT5728_CHIP_INFO_ADDR,
		chip_info, MT5728_CHIP_INFO_LEN);
	if (ret)
		return ret;

	info->chip_id = (u16)(chip_info[0] | (chip_info[1] << 8));
	info->cust_id = chip_info[2];
	info->hw_id = chip_info[3];
	info->minor_ver = (u16)(chip_info[4] | (chip_info[5] << 8));
	info->major_ver = (u16)(chip_info[6] | (chip_info[7] << 8));

	return 0;
}

int mt5728_get_chip_info_str(char *info_str, int len)
{
	int ret;
	struct mt5728_chip_info chip_info = {0};
	info_str = &g_mt_printf_buf[0];

	if (!info_str || (len != WL_CHIP_INFO_STR_LEN))
		return -WLC_ERR_PARA_WRONG;

	ret = mt5728_get_chip_info(&chip_info);
	if (ret)
		return ret;

	memset_s(info_str, WL_CHIP_INFO_STR_LEN, 0, WL_CHIP_INFO_STR_LEN);
	snprintf_s(info_str, sizeof(g_mt_printf_buf), WL_CHIP_INFO_STR_LEN,
		"chip_id:mt0x%x minor_ver:0x%x major_ver:0x%x",
		chip_info.chip_id, chip_info.minor_ver, chip_info.major_ver);

	return 0;
}

int mt5728_get_chip_fw_version(u8 *data, int len, void *dev_data)
{
	struct mt5728_chip_info chip_info = {0};

	/* fw version length must be 4 */
	if (!data || (len != 4)) {
		hwlog_err("get_chip_fw_version: para err\n");
		return -WLC_ERR_PARA_WRONG;
	}

	if (mt5728_get_chip_info(&chip_info)) {
		hwlog_err("get_chip_fw_version: get chip info failed\n");
		return -WLC_ERR_I2C_R;
	}

	/* byte[0:1]=major_fw_ver, byte[2:3]=minor_fw_ver */
	data[0] = (u8)((chip_info.major_ver) & BYTE_MASK);
	data[1] = (u8)((chip_info.major_ver >> BITS_PER_BYTE) & BYTE_MASK);
	data[2] = (u8)((chip_info.minor_ver) & BYTE_MASK);
	data[3] = (u8)((chip_info.minor_ver >> BITS_PER_BYTE) & BYTE_MASK);

	return 0;
}

int mt5728_get_mode(u16 *mode)
{
	int ret;

	if (!mode)
		return -WLC_ERR_PARA_NULL;

	ret = mt5728_read_word(MT5728_OP_MODE_ADDR, mode);
	if (ret) {
		hwlog_err("get_mode: failed\n");
		return -WLC_ERR_I2C_R;
	}

	*mode &= MT5728_SYS_MODE_MASK;
	hwlog_err("get_mode: 0x%x\n", *mode);
	return 0;
}

static void mt5728_wake_lock(void)
{
	if (g_mt5728_wakelock.active) {
		hwlog_info("[wake_lock] already locked\n");
		return;
	}

	__pm_stay_awake(&g_mt5728_wakelock);
	hwlog_info("wake_lock\n");
}

static void mt5728_wake_unlock(void)
{
	if (!g_mt5728_wakelock.active) {
		hwlog_info("[wake_unlock] already unlocked\n");
		return;
	}

	__pm_relax(&g_mt5728_wakelock);
	hwlog_info("wake_unlock\n");
}

void mt5728_enable_irq(void)
{
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (!di->irq_active) {
		hwlog_info("[enable_irq] ++\n");
		enable_irq(di->irq_int);
		di->irq_active = true;
	}
	hwlog_info("[enable_irq] --\n");
	mutex_unlock(&di->mutex_irq);
}

void mt5728_disable_irq_nosync(void)
{
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di)
		return;

	mutex_lock(&di->mutex_irq);
	if (di->irq_active) {
		hwlog_info("[disable_irq_nosync] ++\n");
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
	}
	hwlog_info("[disable_irq_nosync] --\n");
	mutex_unlock(&di->mutex_irq);
}

void mt5728_chip_enable(int enable)
{
	int gpio_val;
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di)
		return;

	if (enable == RX_EN_ENABLE)
		gpio_set_value(di->gpio_en, di->gpio_en_valid_val);
	else
		gpio_set_value(di->gpio_en, !di->gpio_en_valid_val);

	gpio_val = gpio_get_value(di->gpio_en);
	hwlog_info("[chip_enable] gpio %s now\n", gpio_val ? "high" : "low");
}

void mt5728_channel_select_enable(int channel)
{
	int gpio_val;
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di)
		return;

	if (channel == MT5728_HALL_KB_ID) {
		gpio_set_value(di->gpio_kb_en, di->gpio_kp_valid_val);
		gpio_val = gpio_get_value(di->gpio_kb_en);
		hwlog_info("[chip_enable] gpio kb %s now\n", gpio_val ? "high" : "low");
	}

	if (channel == MT5728_HALL_PEN_ID) {
		gpio_set_value(di->gpio_pen_en, di->gpio_kp_valid_val);
		gpio_val = gpio_get_value(di->gpio_pen_en);
		hwlog_info("[chip_enable] gpio pen %s now\n", gpio_val ? "high" : "low");
	}
}

void mt5728_channel_select_disable(int channel)
{
	int gpio_val;
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di)
		return;

	if (channel == MT5728_HALL_KB_ID) {
		gpio_set_value(di->gpio_kb_en, !di->gpio_kp_valid_val);
		gpio_val = gpio_get_value(di->gpio_kb_en);
		hwlog_info("[chip_enable] gpio kb %s now\n", gpio_val ? "high" : "low");
	}

	if (channel == MT5728_HALL_PEN_ID) {
		gpio_set_value(di->gpio_pen_en, !di->gpio_kp_valid_val);
		gpio_val = gpio_get_value(di->gpio_pen_en);
		hwlog_info("[chip_enable] gpio pen %s now\n", gpio_val ? "high" : "low");
	}
}

void mt5728_sleep_enable(int enable)
{
	int gpio_val;
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di || di->g_val.irq_abnormal_flag)
		return;
}

int mt5728_core_reset(void)
{
	int ret = 0;

	/* KEY */
	ret = mt5728_write_byte(MT5728_AHB_CLOCK_REG_BASE, MT5728_AHB_CLOCK_VAL);
	if (ret) {
		hwlog_err("remap to RAM fail\n");
		return ret;
	}

	/* run M0 */
	ret = mt5728_write_byte(MT5728_M_REG_BASE, MT5728_M_VAL);
	if (ret) {
		hwlog_err("remap to RAM fail\n");
		return ret;
	}
}

int mt5728_chip_reset(void)
{
	int ret;
	u8 data = MT5728_RST_SYS;

	ret = mt5728_write_block(MT5728_TX_CMD_ADDR, &data, BYTE_LEN);
	if (ret)
		hwlog_err("[chip_reset] ignore i2c failure\n");

	hwlog_info("[chip_reset] succ\n");
	return 0;
}

static void mt5728_irq_work(struct work_struct *work)
{
	int ret;
	int gpio_val;
	u16 mode = 0;
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di)
		goto exit;

	gpio_val = gpio_get_value(di->gpio_en);
	if (gpio_val != di->gpio_en_valid_val) {
		hwlog_err("[irq_work] gpio %s\n", gpio_val ? "high" : "low");
		goto exit;
	}

	mt5728_tx_mode_irq_handler(di);

exit:
	if (di && !di->g_val.irq_abnormal_flag)
		mt5728_enable_irq();

	mt5728_wake_unlock();
}

static irqreturn_t mt5728_interrupt(int irq, void *_di)
{
	struct mt5728_dev_info *di = _di;

	if (!di)
		return IRQ_HANDLED;

	mt5728_wake_lock();
	hwlog_info("[interrupt] ++\n");
	if (di->irq_active) {
		disable_irq_nosync(di->irq_int);
		di->irq_active = false;
		schedule_work(&di->irq_work);
	} else {
		hwlog_info("interrupt: irq is not enable\n");
		mt5728_wake_unlock();
	}
	hwlog_info("[interrupt] --\n");

	return IRQ_HANDLED;
}

static int mt5728_dev_check(struct mt5728_dev_info *di)
{
	int ret;
	u16 chip_id = 0;

	wlps_control(WLPS_TX_SW, WLPS_CTRL_ON);
	power_usleep(DT_USLEEP_10MS);
	ret = mt5728_get_hw_chip_id(&chip_id);
	if (ret) {
		hwlog_err("dev_check: failed\n");
		wlps_control(WLPS_TX_SW, WLPS_CTRL_OFF);
		return ret;
	}
	wlps_control(WLPS_TX_SW, WLPS_CTRL_OFF);
	hwlog_info("[dev_check] chip_id=0x%x\n", chip_id);
	if ((chip_id == MT5728_HW_CHIP_ID) || (chip_id == MT5728_CHIP_ID_AB))
		return 0;

	hwlog_err("dev_check: tx_chip not match\n");
	return -WLC_ERR_MISMATCH;
}

struct device_node *mt5728_dts_dev_node(void *dev_data)
{
	struct mt5728_dev_info *di = NULL;

	mt5728_get_dev_info(&di);
	if (!di || !di->dev)
		return NULL;

	return di->dev->of_node;
}

static int mt5728_gpio_init(struct mt5728_dev_info *di,
	struct device_node *np)
{
	/* gpio_en */
	if (power_gpio_config_output(np, "gpio_en", "mt5728_en",
		&di->gpio_en, di->gpio_en_valid_val))
		return -EINVAL;

	if (power_gpio_config_output(np, "gpio_pen_en", "gpio_pen_coil_en",
		&di->gpio_pen_en, !di->gpio_kp_valid_val))
		goto gpio_en_fail;

	if (power_gpio_config_output(np, "gpio_kb_en", "gpio_kb_coil_en",
		&di->gpio_kb_en, !di->gpio_kp_valid_val))
		goto gpio_en_fail;

	return 0;

gpio_en_fail:
	return -EINVAL;
}

static int mt5728_irq_init(struct mt5728_dev_info *di,
	struct device_node *np)
{
	if (power_gpio_config_interrupt(np, "gpio_int", "mt5728_int",
		&di->gpio_int, &di->irq_int))
		goto irq_init_fail_0;

	if (request_irq(di->irq_int, mt5728_interrupt,
		IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "mt5728_irq", di)) {
		hwlog_err("irq_init: request mt5728_irq failed\n");
		goto irq_init_fail_1;
	}

	enable_irq_wake(di->irq_int);
	di->irq_active = true;
	INIT_WORK(&di->irq_work, mt5728_irq_work);

	return 0;

irq_init_fail_1:
	gpio_free(di->gpio_int);
irq_init_fail_0:
	return -EINVAL;
}

static void mt5728_register_pwr_dev_info(struct mt5728_dev_info *di)
{
	int ret;
	u16 chip_id = 0;
	struct power_devices_info_data *pwr_dev_info = NULL;

	ret = mt5728_get_chip_id(&chip_id);
	if (ret)
		return;

	pwr_dev_info = power_devices_info_register();
	if (pwr_dev_info) {
		pwr_dev_info->dev_name = di->dev->driver->name;
		pwr_dev_info->dev_id = chip_id;
		pwr_dev_info->ver_id = 0;
	}
}

static int mt5728_ops_register(struct mt5728_dev_info *di)
{
	int ret;

	ret = mt5728_fw_ops_register();
	if (ret) {
		hwlog_err("ops_register: register fw_ops failed\n");
		return ret;
	}
	ret = mt5728_tx_ops_register();
	if (ret) {
		hwlog_err("ops_register: register tx_ops failed\n");
		return ret;
	}
	ret = mt5728_tx_ps_ops_register();
	if (ret) {
		hwlog_err("ops_register: register txps_ops failed\n");
		return ret;
	}
	ret = mt5728_qi_ops_register();
	if (ret) {
		hwlog_err("ops_register: register qi_ops failed\n");
		return ret;
	}
	di->g_val.qi_hdl = qi_protocol_get_handle();

	return 0;
}

static void mt5728_fw_mtp_check(struct mt5728_dev_info *di)
{
	if (power_cmdline_is_powerdown_charging_mode()) {
		di->g_val.mtp_chk_complete = true;
		return;
	}

	INIT_DELAYED_WORK(&di->mtp_check_work, mt5728_fw_mtp_check_work);
	schedule_delayed_work(&di->mtp_check_work,
		msecs_to_jiffies(WIRELESS_FW_WORK_DELAYED_TIME));
}

static ssize_t show_wls_switch(struct device *dev, struct device_attribute *attr, char *buf)
{
	buf = &g_mt_printf_buf[0];
	return sprintf_s(buf, sizeof(g_mt_printf_buf), "cps wls switch status:0x%x\n", *buf);
}

static ssize_t store_wls_switch(struct device *dev, struct device_attribute *attr,
	const char *buf, size_t count)
{
	int tmp = 0;
	int pen_int = 0;
	int pen_en = 0;
	int pen_col_en = 0;
	int vbst_en = 0;
	struct device_node *node = dev->of_node;

	pen_int = of_get_named_gpio(node, "gpio_int", 0);
	pen_en = of_get_named_gpio(node, "gpio_en", 0);
	pen_col_en = of_get_named_gpio(node, "gpio_coil_en", 0);
	vbst_en = of_get_named_gpio(node, "gpio_pwr_good", 0);

	hwlog_err("[%s] p_int = %d, p_en = %d, p_col_en = %d, vbst_en = %d.\n",
		__func__, pen_int, pen_en, pen_col_en, vbst_en);

	tmp = simple_strtoul(buf, NULL, 0);
	if (tmp != 0) {
		gpio_set_value(pen_en, 0);
		gpio_set_value(pen_col_en, 1);
		gpio_set_value(vbst_en, 1);
	} else {
		gpio_set_value(pen_en, 0);
		gpio_set_value(pen_col_en, 0);
		gpio_set_value(vbst_en, 0);
	}
	hwlog_err("[%s] p_en = %d, p_col_en = %d, vbst_en = %d, tmp = %d.\n",
		__func__, pen_en, pen_col_en, vbst_en, tmp);

	return count;
}
static DEVICE_ATTR(wls_switch, 0664, show_wls_switch, store_wls_switch);

static int mt5728_probe(struct i2c_client *client,
	const struct i2c_device_id *id)
{
	int ret;
	struct mt5728_dev_info *di = NULL;
	struct device_node *np = NULL;

	if (!client || !client->dev.of_node)
		return -ENODEV;

	di = devm_kzalloc(&client->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_mt5728_di = di;
	di->dev = &client->dev;
	np = client->dev.of_node;
	di->client = client;
	i2c_set_clientdata(client, di);

	ret = mt5728_dev_check(di);
	if (ret)
		goto dev_ck_fail;

	ret = mt5728_parse_dts(np, di);
	if (ret)
		goto parse_dts_fail;

	ret = mt5728_gpio_init(di, np);
	if (ret)
		goto gpio_init_fail;
	ret = mt5728_irq_init(di, np);
	if (ret)
		goto irq_init_fail;

	wakeup_source_init(&g_mt5728_wakelock, "mt5728_wakelock");
	mutex_init(&di->mutex_irq);

	ret = mt5728_ops_register(di);
	if (ret)
		goto ops_regist_fail;

	mt5728_fw_mtp_check(di);
	mt5728_register_pwr_dev_info(di);

	hwlog_info("wireless_chip probe ok\n");
	return 0;

ops_regist_fail:
	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
irq_init_fail:
	gpio_free(di->gpio_en);
	gpio_free(di->gpio_re_pwr_en);

gpio_init_fail:
parse_dts_fail:
dev_ck_fail:
	devm_kfree(&client->dev, di);
	g_mt5728_di = NULL;
	return ret;
}

static int mt5728_remove(struct i2c_client *client)
{
	struct mt5728_dev_info *l_dev = i2c_get_clientdata(client);

	if (!l_dev)
		return -ENODEV;

	gpio_free(l_dev->gpio_en);
	gpio_free(l_dev->gpio_int);
	gpio_free(l_dev->gpio_re_pwr_en);
	free_irq(l_dev->irq_int, l_dev);
	mutex_destroy(&l_dev->mutex_irq);
	wakeup_source_trash(&g_mt5728_wakelock);
	cancel_delayed_work(&l_dev->mtp_check_work);
	devm_kfree(&client->dev, l_dev);
	g_mt5728_di = NULL;

	return 0;
}

static void mt5728_shutdown(struct i2c_client *client)
{
	hwlog_info("[shutdown] ++\n");
	hwlog_info("[shutdown] --\n");
}

MODULE_DEVICE_TABLE(i2c, wireless_mt5728);
static const struct of_device_id mt5728_of_match[] = {
	{
		.compatible = "mt,wls-charger-mt5728",
		.data = NULL,
	},
};

static const struct i2c_device_id mt5728_i2c_id[] = {
	{ "wls-charger-mt5728", 0 },
};

static struct i2c_driver mt5728_driver = {
	.probe = mt5728_probe,
	.shutdown = mt5728_shutdown,
	.remove = mt5728_remove,
	.id_table = mt5728_i2c_id,
	.driver = {
		.owner = THIS_MODULE,
		.name = "wireless_mt5728",
		.of_match_table = of_match_ptr(mt5728_of_match),
	},
};

static int __init mt5728_init(void)
{
	return i2c_add_driver(&mt5728_driver);
}

static void __exit mt5728_exit(void)
{
	i2c_del_driver(&mt5728_driver);
}

device_initcall(mt5728_init);
module_exit(mt5728_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("mt5728 module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
