/*
 * sensor_sysfs.c
 *
 * code for sensor debug sysfs
 *
 * Copyright (c) 2021- 2021 Huawei Technologies Co., Ltd.
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

#include <linux/delay.h>
#include <linux/of.h>
#include <linux/kthread.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/time.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/ctype.h>
#include <linux/rwsem.h>
#include <linux/of_device.h>
#include <linux/input.h>

#define ALL_INFO_SIZE                        7
#define DATA_SIZE                            256
#define FS_INPUT_DEVICE_NAME                 "fingersense"
#define TP_SENSOR_NEAR                       0
#define TP_SENSOR_FAR                        1
#define SUCCESS_RET                          0

static char sensor_all_info[ALL_INFO_SIZE + 1] = {0};
static char sensor_enable[]                = "0";
static char sensor_req[]                   = "0";
static int  sensor_ready;
static char sensor_data[DATA_SIZE]         = {0};

struct input_dev *input_dev_fs;

static ssize_t sensor_show_ACC_info(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	pr_err("%s start input :%s", __func__, sensor_all_info);
	return snprintf(buf, ALL_INFO_SIZE + 1, "%s\n", sensor_all_info);
}

static ssize_t sensor_store_ACC_info(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	snprintf(sensor_all_info, ALL_INFO_SIZE + 1, "%s\n", buf);
	return size;
}

static ssize_t store_fg_sense_enable(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{

	int value = 0;
	if(buf[0] == '1')
		value = 1;
	if (input_dev_fs) {
		pr_err("%s start input :%d\n", __func__, value);
		input_event(input_dev_fs, EV_MSC, MSC_RAW, value); // 0x03
		input_sync(input_dev_fs);
	}
	snprintf(sensor_enable, 2, "%s\n", buf);
	pr_err("%s: store sensor_enable:%s  buf:%s\n", __func__, sensor_enable, buf);
	return size;
}

static ssize_t show_fg_sensor_enable(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	size_t size;
	size = snprintf(buf, 2, "%s\n", sensor_enable);
	pr_err("%s: show sensor_enable:%s  buf:%s\n", __func__, sensor_enable, buf);
	return size;
}

static ssize_t store_fg_req_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int value = 0;
	if(buf[0] == '1')
		value = 1000;
	if (input_dev_fs) {
		pr_err("%s start input :%d\n", __func__, value);
		input_event(input_dev_fs, EV_MSC, MSC_SCAN, value); // 0x04
		input_sync(input_dev_fs);
	}
	snprintf(sensor_req, 2, "%s\n", buf);
	return size;
}

static ssize_t show_fg_req_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, 2, "%s\n", sensor_req);

}

static ssize_t store_fg_data_ready(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (buf[0] == '0')
		sensor_ready = 0; // data not ready
	if (buf[0] == '1')
		sensor_ready = 1; // data ready
	return size;
}

static ssize_t show_fg_data_ready(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, 2, "%d\n", sensor_ready);
}

static ssize_t store_fg_latch_data(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	memcpy(sensor_data, buf, DATA_SIZE);
	return size;
}

static ssize_t show_fg_latch_data(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	memcpy(buf, sensor_data, DATA_SIZE);
	return DATA_SIZE;
}

static DEVICE_ATTR(acc_info, 0660, sensor_show_ACC_info, sensor_store_ACC_info);
static DEVICE_ATTR(set_fingersense_enable, 0660, show_fg_sensor_enable,
	store_fg_sense_enable);
static DEVICE_ATTR(fingersense_req_data, 0660, show_fg_req_data, store_fg_req_data);
static DEVICE_ATTR(fingersense_data_ready, 0660, show_fg_data_ready, store_fg_data_ready);
static DEVICE_ATTR(fingersense_latch_data, 0660, show_fg_latch_data, store_fg_latch_data);

static struct attribute *sensor_attributes[] = {
	&dev_attr_acc_info.attr,
	&dev_attr_set_fingersense_enable.attr,
	&dev_attr_fingersense_req_data.attr,
	&dev_attr_fingersense_data_ready.attr,
	&dev_attr_fingersense_latch_data.attr,
	NULL
};

static const struct attribute_group sensor_node = {
	.attrs = sensor_attributes,
};

static struct platform_device sensor_input_info = {
	.name = "huawei_sensor",
	.id = -1,
};

static int tp_sensor_input_init(void)
{
	int ret = -EINVAL;

	pr_info("%s:start\n", __func__);
	input_dev_fs = input_allocate_device();
	if (!input_dev_fs) {
		pr_err("%s:input_allocate_device failed\n", __func__);
		return ret;
	}
	set_bit(EV_MSC, input_dev_fs->evbit);
	set_bit(MSC_RAW, input_dev_fs->mscbit);
	set_bit(MSC_SCAN, input_dev_fs->mscbit);
	input_dev_fs->name = FS_INPUT_DEVICE_NAME;
	ret = input_register_device(input_dev_fs);
	if (ret != SUCCESS_RET) {
		ret = -ENOMEM;
		pr_err("%s:input_register_device failed\n", __func__);
		input_free_device(input_dev_fs);
		return ret;
	}
	pr_info("%s:success\n", __func__);
	return SUCCESS_RET;
}

static int __init sensor_scp_sysfs_init(void)
{
	int ret;

	pr_info("%s\n", __func__);

	ret = platform_device_register(&sensor_input_info);
	if (ret) {
		pr_err("%s: register failed, ret:%d\n", __func__, ret);
		return -1;
	}

	ret = sysfs_create_group(&sensor_input_info.dev.kobj, &sensor_node);
	if (ret) {
		pr_err("sysfs_create_group error ret =%d\n", ret);
		goto sysfs_create_fail;
	}
	ret = tp_sensor_input_init();

	return 0;

sysfs_create_fail:
	platform_device_unregister(&sensor_input_info);
	return -1;
}

static void sensors_unregister(void)
{
	platform_device_unregister(&sensor_input_info);
}

static void sensor_scp_sysfs_exit(void)
{
	sensors_unregister();
}

module_init(sensor_scp_sysfs_init);
module_exit(sensor_scp_sysfs_exit);

MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_DESCRIPTION("fingersense driver");
MODULE_LICENSE("GPL v2");
