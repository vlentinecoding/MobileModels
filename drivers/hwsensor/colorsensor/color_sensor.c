/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2020. All rights reserved.
 * Date:    2020.07.20
 * Description: color sensor module
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


#include <linux/kernel.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/string.h>
#include <linux/mutex.h>
#include <linux/unistd.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/pm.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include <linux/kthread.h>
#include <linux/freezer.h>
#include <linux/timer.h>
#include <linux/version.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif

#include "color_sensor.h"
#include "hw_log.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif
#define HWLOG_TAG color_sensor
HWLOG_REGIST();

static struct class *color_sensor_class;
int color_report_val[MAX_REPORT_LEN] = {0};
UINT32 flicker_support;
static struct color_priv_data_t color_priv_data = {
	.rgb_support = 0,
	.rgb_absent = 0,
};

int color_sensor_get_byte(const struct i2c_client *i2c, uint8_t reg,
	uint8_t *data)
{
	int ret = -EINVAL;

	if (!i2c || !data) {
		hwlog_err("%s data or handle Pointer is NULL\n", __func__);
		return ret;
	}

	ret = i2c_smbus_read_i2c_block_data(i2c, reg, 1, data);
	if (ret < 0)
		hwlog_err("%s failed\n", __func__);

	return ret;
}

int color_sensor_set_byte(const struct i2c_client *i2c, uint8_t reg,
	uint8_t data)
{
	int ret = -EINVAL;

	if (!i2c) {
		hwlog_err("%s data or handle Pointer is NULL\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_i2c_block_data(i2c, reg, 1, &data);
	if (ret < 0)
		hwlog_err("%s failed\n", __func__);

	return ret;
}

int color_sensor_read_fifo(struct i2c_client *client, uint8_t reg,
	void *buf, size_t len)
{
	struct i2c_msg msg[2]; // one set for reg, one for read buf

	if (!client || !buf) {
		hwlog_err("%s, client buf is NULL\n", __func__);
		return -EINVAL;
	}

	msg[0].addr = client->addr;
	msg[0].flags = 0; // 0 set for read
	msg[0].len = 1;   // reg len
	msg[0].buf = &reg;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].len = len;
	msg[1].buf = buf;

	if (i2c_transfer(client->adapter, msg, 2) != 2) { // 2 case i2c err
		hwlog_err("i2c transfer failed\n");
		return -EIO;
	}

	return 0;
}

int color_sensor_write_fifo(struct i2c_client *i2c, uint8_t reg,
	const void *buf, size_t len)
{
	int ret = -EINVAL;

	if (!i2c || !buf) {
		hwlog_err("%s data or handle Pointer is NULL\n", __func__);
		return ret;
	}

	ret = i2c_smbus_write_i2c_block_data(i2c, reg, len, buf);
	if (ret < 0)
		hwlog_err("%s failed\n", __func__);

	return ret;
}

static ssize_t color_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_sensor_output_t out_data = {0};
	struct color_chip *chip = NULL;
	int size;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	hwlog_info("[%s] in\n", __func__);
	size = sizeof(struct color_sensor_output_t);
	if (chip->color_show_calibrate_state == NULL) {
		hwlog_err("[%s] color_show_calibrate_state NULL\n", __func__);
		return -1;
	}
	chip->color_show_calibrate_state(chip, &out_data);
	memcpy(buf, &out_data, size);
	return size;
}

static ssize_t color_calibrate_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct color_sensor_input_t in_data;
	struct color_chip *chip = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	hwlog_info("[%s] color_sensor store in\n", __func__);

	if (size >= sizeof(struct color_sensor_input_t))
		memcpy(&in_data, buf, sizeof(struct color_sensor_input_t));

	if (chip->color_store_calibrate_state == NULL) {
		hwlog_err("[%s] color_store_calibrate_state NULL\n", __func__);
		return -1;
	}

	hwlog_info("[%s] input enable = %d, data[%d, %d, %d, %d]\n", __func__,
		in_data.enable, in_data.tar_x, in_data.tar_y,
		in_data.tar_z, in_data.tar_ir);
	chip->color_store_calibrate_state(chip, &in_data);

	return size;
}

static ssize_t at_color_calibrate_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct at_color_sensor_input_t *in_data;
	struct color_chip *chip = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	if (chip->at_color_store_calibrate_state == NULL) {
		hwlog_err("[%s] func NULL\n", __func__);
		return -1;
	}
	hwlog_info("[%s] color_sensor store in buf: %s\n", __func__, buf);

	in_data = (struct at_color_sensor_input_t *)buf;

	hwlog_info("%s, target = %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d",
		__func__, in_data->enable, in_data->reserverd[BUF_DATA_0],
		in_data->reserverd[BUF_DATA_1], in_data->reserverd[BUF_DATA_2],
		in_data->reserverd[BUF_DATA_3], in_data->reserverd[BUF_DATA_4],
		in_data->reserverd[BUF_DATA_5], in_data->reserverd[BUF_DATA_6],
		in_data->reserverd[BUF_DATA_7], in_data->reserverd[BUF_DATA_8],
		in_data->reserverd[BUF_DATA_9]);
	chip->at_color_store_calibrate_state(chip, in_data);

	return size;
}

static ssize_t at_color_calibrate_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct at_color_sensor_output_t out_data = {0};
	struct color_chip *chip = NULL;
	int size;
	char *color_calibrate_result;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	if (chip->at_color_show_calibrate_state == NULL) {
		hwlog_err("[%s] at_color_show_calibrate NULL\n", __func__);
		return -1;
	}
	hwlog_info("[%s] in\n", __func__);
	size = sizeof(struct at_color_sensor_output_t);

	color_calibrate_result = chip->at_color_show_calibrate_state(chip, &out_data);

	hwlog_info("get cali result = %d, gain_arr=%d, color_array=%d\n",
		out_data.result, out_data.gain_arr, out_data.color_arr);
	hwlog_info("get cali gain = %d, %d, %d, %d %d\n", out_data.cali_gain[BUF_DATA_0],
		out_data.cali_gain[BUF_DATA_1], out_data.cali_gain[BUF_DATA_2],
		out_data.cali_gain[BUF_DATA_3], out_data.cali_gain[BUF_DATA_4]);
	return snprintf(buf, REPORT_CALI_LEN, "%s\n", color_calibrate_result);
}

static ssize_t color_enable_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int state;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	if (chip->color_enable_show_state == NULL) {
		hwlog_err("[%s] color_enable_show_state NULL\n", __func__);
		return -1;
	}
	chip->color_enable_show_state(chip, &state);

	return snprintf(buf, ONE_SHOW_LEN, "%d\n", state);
}
static ssize_t color_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct color_chip *chip = NULL;
	bool state = true;
	hwlog_err("[%s] enter\n", __func__);
	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	hwlog_err("[%s] buf :%s\n", __func__, buf);
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	if (buf[0] == '1')
		state = true;
	else
		state = false;
	if (chip->color_enable_store_state == NULL) {
		hwlog_err("[%s] color_enable_store_state NULL\n", __func__);
		return -1;
	}
	hwlog_err("[%s] state :%d\n", __func__, state);
	chip->color_enable_store_state(chip, (int)state);
	return size;
}
static ssize_t flicker_enable_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	struct color_chip *chip = NULL;
	long state = 0;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	if (kstrtoul(buf, ONE_SHOW_LEN, &state)) {
		hwlog_err("[%s] Failed to strtobool enable state.\n", __func__);
		return -EINVAL;
	}
	if (chip->flicker_enable_store_state == NULL) {
		hwlog_err("[%s] flicker_enable_store_state NULL\n", __func__);
		return -1;
	}
	if (flicker_support == 0) {
		hwlog_err("%s not support flicker\n", __func__);
		return -1;
	}
	hwlog_info("%s state = %d\n", __func__, (int)state);

	chip->flicker_enable_store_state(chip, (int)state);
	return size;
}

static ssize_t flicker_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	if (chip->get_flicker_data == NULL) {
		hwlog_err("[%s] get_flicker_data NULL\n", __func__);
		return -1;
	}

	chip->get_flicker_data(chip, buf);

	return MAX_FLICK_DATA_LEN;
}

static ssize_t calibrate_timeout_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	return snprintf(buf, PAGE_SIZE, "%d\n", TIME_OUT_DEFAULT);
}

static ssize_t color_gain_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int gain;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	if (chip->color_sensor_get_gain == NULL) {
		hwlog_err("[%s] get_flicker_data NULL\n", __func__);
		return -1;
	}

	gain = chip->color_sensor_get_gain(chip->device_ctx);

	return snprintf(buf, PAGE_SIZE, "%d\n", gain);
}
static ssize_t color_gain_store(struct device *dev,
				struct device_attribute *attr,
				const char *buf, size_t size)
{
	struct color_chip *chip = NULL;
	unsigned long value = 0L;
	int gain_value;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	if (chip->color_sensor_set_gain == NULL) {
		hwlog_err("[%s] color_sensor_setGain NULL\n", __func__);
		return -1;
	}
	if (kstrtoul(buf, ONE_SHOW_LEN, &value))
		return -EINVAL;
	gain_value = (int)value;
	chip->color_sensor_set_gain(chip->device_ctx, gain_value);
	return size;
}

static ssize_t color_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}
	// color_report_val
	memcpy(buf, color_report_val, MAX_REPORT_LEN * sizeof(int));
	hwlog_info("color_report_val = %d, %d, %d, %d, %d\n",
		color_report_val[BUF_DATA_0], color_report_val[BUF_DATA_1], color_report_val[BUF_DATA_2],
		color_report_val[BUF_DATA_3], color_report_val[BUF_DATA_4]);
	return (MAX_REPORT_LEN * sizeof(int));
}

static ssize_t report_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int report_type;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_report_type == NULL) {
		hwlog_err("%s, NULL , return invalid report type\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	report_type = chip->color_report_type();
	if ((report_type > AWB_SENSOR_RAW_SEQ_TYPE_INVALID) &&
		(report_type < AWB_SENSOR_RAW_SEQ_TYPE_MAX)) {
		hwlog_info("%s, report type = %d\n", __func__, report_type);
		return snprintf(buf, PAGE_SIZE, "%d\n", report_type);
	}

	return snprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t color_name_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	unsigned int len;
	char *color_name = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	if (!chip->color_chip_name)
		goto unsupport_rgb;

	color_name = chip->color_chip_name();
	if (color_name == NULL)
		goto unsupport_rgb;

	len = strlen(color_name);
	hwlog_debug("get color name len = %d\n", len);
	if (len >= MAX_NAME_STR_LEN)
		goto unsupport_rgb;

	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", color_name);

unsupport_rgb:
	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", "unsupport");
}

static ssize_t color_algo_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	unsigned int len;
	char *color_algo = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	if (!chip->color_algo_type)
		goto unsupport_algo;

	color_algo = chip->color_algo_type();
	if (color_algo == NULL)
		goto unsupport_algo;

	len = strlen(color_algo);
	hwlog_info("get color algo type = %s, len = %d\n", color_algo, len);
	if (len >= MAX_ALGO_TYPE_STR_LEN)
		goto unsupport_algo;

	return snprintf(buf, MAX_ALGO_TYPE_STR_LEN, "%s", color_algo);

unsupport_algo:
	return snprintf(buf, MAX_ALGO_TYPE_STR_LEN, "%s", "unsupport");
}

static ssize_t color_vendor_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	unsigned int len;
	char *color_vendor = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	if (!chip->color_chip_vendor)
		goto unsupport_rgb;

	color_vendor = chip->color_chip_vendor();
	if (color_vendor == NULL)
		goto unsupport_rgb;

	len = strlen(color_vendor);
	hwlog_info("get color vendor len = %d\n", len);
	if (len >= MAX_NAME_STR_LEN)
		goto unsupport_rgb;

	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", color_vendor);

unsupport_rgb:
	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", "unsupport");
}

static ssize_t color_version_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int color_version;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_chip_version == NULL) {
		hwlog_err("%s, NULL , return invalid color version\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	color_version = chip->color_chip_version();

	return snprintf(buf, PAGE_SIZE, "%d\n", color_version);
}

static ssize_t color_type_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int color_type;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_chip_type == NULL) {
		hwlog_err("%s, NULL , return invalid color type\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	color_type = chip->color_chip_type();

	return snprintf(buf, PAGE_SIZE, "%d\n", color_type);
}

static ssize_t color_maxRange_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	unsigned int len;
	char *color_maxRange = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	if (!chip->color_chip_max_range)
		goto unsupport_rgb;

	color_maxRange = chip->color_chip_max_range();
	if (color_maxRange == NULL)
		goto unsupport_rgb;

	len = strlen(color_maxRange);
	hwlog_info("get color maxRange len = %d\n", len);
	if (len >= MAX_NAME_STR_LEN)
		goto unsupport_rgb;

	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", color_maxRange);

unsupport_rgb:
	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", "unsupport");
}

static ssize_t color_resolution_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	unsigned int len;
	char *color_resolution = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	if (!chip->color_chip_resolution)
		goto unsupport_rgb;

	color_resolution = chip->color_chip_resolution();
	if (color_resolution == NULL)
		goto unsupport_rgb;

	len = strlen(color_resolution);
	hwlog_info("get color resolution len = %d\n", len);
	if (len >= MAX_NAME_STR_LEN)
		goto unsupport_rgb;

	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", color_resolution);

unsupport_rgb:
	return snprintf(buf, MAX_NAME_STR_LEN, "%s\n", "unsupport");
}

static ssize_t color_report_data_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	unsigned int len;
	char *color_reportdata = NULL;

	if (!dev || !attr || !buf) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("%s input NULL\n", __func__);
		return -EFAULT;
	}

	if (!chip->color_chip_sensor_report_data)
		goto unsupport_rgb;

	color_reportdata = chip->color_chip_sensor_report_data();
	if (color_reportdata == NULL)
		goto unsupport_rgb;

	len = strlen(color_reportdata);
	hwlog_debug("get color report_data len = %d\n", len);
	if (len >= MAX_CHDATA_STR_LEN)
		goto unsupport_rgb;
	hwlog_debug("get report_data = %s\n", color_reportdata);
	return snprintf(buf, MAX_CHDATA_STR_LEN, "%s\n", color_reportdata);

unsupport_rgb:
	return snprintf(buf, MAX_CHDATA_STR_LEN, "%s\n", "unsupport");
}

static ssize_t color_minDelay_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int color_minDelay;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_chip_min_delay == NULL) {
		hwlog_err("%s, NULL , return invalid color type\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	color_minDelay = chip->color_chip_min_delay();

	return snprintf(buf, PAGE_SIZE, "%d\n", color_minDelay);
}

static ssize_t color_fifoReservedEventCount_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int color_fifoReservedEventCount;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_chip_fifo_reserved_event_count == NULL) {
		hwlog_err("%s, NULL , return invalid color type\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	color_fifoReservedEventCount = chip->color_chip_fifo_reserved_event_count();

	return snprintf(buf, PAGE_SIZE, "%d\n", color_fifoReservedEventCount);
}

static ssize_t color_fifoMaxEventCount_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int color_fifoMaxEventCount;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_chip_fifo_max_event_count == NULL) {
		hwlog_err("%s, NULL , return invalid color type\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	color_fifoMaxEventCount = chip->color_chip_fifo_max_event_count();

	return snprintf(buf, PAGE_SIZE, "%d\n", color_fifoMaxEventCount);
}

static ssize_t color_atime_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	int color_type;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_chip_get_atime == NULL) {
		hwlog_err("%s, NULL , return invalid color type\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	color_type = chip->color_chip_get_atime(chip);

	return snprintf(buf, PAGE_SIZE, "%d\n", color_type);
}

static ssize_t color_itime_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct color_chip *chip = NULL;
	UINT32 itime;

	if (!dev || !attr || !buf) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip = dev_get_drvdata(dev);
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	if (chip->color_chip_get_itime == NULL) {
		hwlog_err("%s, NULL , return invalid itime\n", __func__);
		return snprintf(buf, PAGE_SIZE, "%d\n", 0);
	}

	itime = chip->color_chip_get_itime(chip);

	return snprintf(buf, PAGE_SIZE, "%u\n", itime);
}

DEVICE_ATTR(calibrate, 0660, color_calibrate_show, color_calibrate_store);
DEVICE_ATTR(color_enable, 0660, color_enable_show, color_enable_store);
DEVICE_ATTR(gain, 0660, color_gain_show, color_gain_store);
DEVICE_ATTR(calibrate_timeout, 0440, calibrate_timeout_show, NULL);
DEVICE_ATTR(color_data, 0660, color_data_show, NULL);
DEVICE_ATTR(color_cali, 0660, at_color_calibrate_show, at_color_calibrate_store);
DEVICE_ATTR(flicker_enable, 0660, NULL, flicker_enable_store);
DEVICE_ATTR(flicker_data, 0660, flicker_data_show, NULL);
DEVICE_ATTR(report_type, 0660, report_type_show, NULL);
DEVICE_ATTR(name, 0660, color_name_show, NULL);
DEVICE_ATTR(color_algo, 0660, color_algo_type_show, NULL);
DEVICE_ATTR(vendor, 0660, color_vendor_show, NULL);
DEVICE_ATTR(version, 0660, color_version_show, NULL);
DEVICE_ATTR(type, 0660, color_type_show, NULL);
DEVICE_ATTR(maxRange, 0660, color_maxRange_show, NULL);
DEVICE_ATTR(resolution, 0660, color_resolution_show, NULL);
DEVICE_ATTR(report_data, 0660, color_report_data_show, NULL);
DEVICE_ATTR(minDelay, 0660, color_minDelay_show, NULL);
DEVICE_ATTR(fifoReservedEventCount, 0660, color_fifoReservedEventCount_show, NULL);
DEVICE_ATTR(fifoMaxEventCount, 0660, color_fifoMaxEventCount_show, NULL);
DEVICE_ATTR(atime, 0660, color_atime_show, NULL);
DEVICE_ATTR(itime, 0660, color_itime_show, NULL);

static struct attribute *color_sensor_attributes[] = {
	&dev_attr_calibrate.attr,
	&dev_attr_color_enable.attr,
	&dev_attr_gain.attr,
	&dev_attr_calibrate_timeout.attr,
	&dev_attr_color_data.attr,
	&dev_attr_color_cali.attr,
	&dev_attr_flicker_enable.attr,
	&dev_attr_flicker_data.attr,
	&dev_attr_report_type.attr,
	&dev_attr_name.attr,
	&dev_attr_color_algo.attr,
	&dev_attr_vendor.attr,
	&dev_attr_version.attr,
	&dev_attr_type.attr,
	&dev_attr_maxRange.attr,
	&dev_attr_resolution.attr,
	&dev_attr_report_data.attr,
	&dev_attr_minDelay.attr,
	&dev_attr_fifoReservedEventCount.attr,
	&dev_attr_fifoMaxEventCount.attr,
	&dev_attr_atime.attr,
	&dev_attr_itime.attr,
	NULL,
};
static const struct attribute_group color_sensor_attr_group = {
	.attrs = color_sensor_attributes,
};

static const struct attribute_group *color_sensor_attr_groups[] = {
	&color_sensor_attr_group,
	NULL,
};

int color_register(struct color_chip *chip)
{
	if (!chip) {
		hwlog_err("[%s] input NULL\n", __func__);
		return -1;
	}

	chip->dev = device_create(color_sensor_class, NULL, 0, chip,
		"color_sensor");
	if (chip->dev == NULL) {
		hwlog_err("[%s] Failed to create color_sensor dev", __func__);
		return -1;
	}

	return 0;
}
EXPORT_SYMBOL_GPL(color_register);

void color_notify_absent(void)
{
	if (color_priv_data.rgb_support == 0)
		return;

	// set ++ when rgb absent but config sensor support
	color_priv_data.rgb_absent++;
}

void color_notify_support(void)
{

	color_priv_data.rgb_support++; // support rgb sensor for this product
}

void color_unregister(struct color_chip *chip)
{
	device_destroy(color_sensor_class, 0);
}
EXPORT_SYMBOL_GPL(color_unregister);

int (*color_default_enable)(bool enable) = NULL;
int color_sensor_enable(bool enable)
{
	if (color_default_enable == NULL) {
		hwlog_err("ERR PARA\n");
		return 0;
	}
	return color_default_enable(enable);
}
EXPORT_SYMBOL_GPL(color_sensor_enable);

static int color_sensor_init(void)
{
	color_sensor_class = class_create(THIS_MODULE, "ap_sensor");
	if (IS_ERR(color_sensor_class))
		return PTR_ERR(color_sensor_class);
	color_sensor_class->dev_groups = color_sensor_attr_groups;

	hwlog_info("[%s]color_sensor init\n", __func__);
	return 0;
}

static void color_sensor_exit(void)
{
	class_destroy(color_sensor_class);
}

subsys_initcall(color_sensor_init);
module_exit(color_sensor_exit);

MODULE_AUTHOR("Huawei");
MODULE_DESCRIPTION("Color class init");
MODULE_LICENSE("GPL");
