/*
 * Copyright (c) Honor Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description:  sensorevent.c
 * Author: yangyang
 * Create: 2021-05-07
 * History: NA
 */
#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/interrupt.h>
#include <linux/ioctl.h>
#include <linux/io.h>
#include <linux/irq.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/miscdevice.h>
#include <linux/workqueue.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <sound/jack.h>
#include <linux/fs.h>
#include <linux/regmap.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <securec.h>
#include "sensorevent.h"
#include <linux/string.h>

#define HWLOG_TAG sensorevent

struct sensorevent_data {
	struct mutex notifier_lock;
};

static struct sensorevent_data *g_event_pdata;

static const struct of_device_id g_sensorevent_of_match[] = {
	{
		.compatible = "honor,sensorevent",
	},
	{ },
};

MODULE_DEVICE_TABLE(of, g_sensorevent_of_match);

static int sensorevent_notifier_call(unsigned long event, char *str);
char g_envp_hal[ENVP_LENTH + 1] = {0};

static ssize_t event_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)
{
	size_t retval = 0;
	size_t len = strlen(g_envp_hal);
	printk("%s: enter count:%zu, *f_pos:%zu\n", __func__, count, *f_pos);
	if ( *f_pos >= ENVP_LENTH ) {
		printk("%s: *f_pos >= ENVP_LENTH\n", __func__);
		goto out;
	}
	if ( *f_pos + count > ENVP_LENTH ) {
		count = ENVP_LENTH - *f_pos;
	}
	count = count > len ? len : count;
	if ( copy_to_user(buf, g_envp_hal, count) ) {
		printk("%s: copy to user failed!\n", __func__);
		return -EINVAL;
	}
	memset(g_envp_hal, 0, count);
	*f_pos += count;
	retval = count;
	printk("%s: read to user %s, count:%zu, *f_pos:%zu\n", __func__, g_envp_hal, count, *f_pos);
out:
	return retval;
}

static ssize_t event_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos)
{
	ssize_t ret = 0;
	printk("%s: enter\n", __func__);
	memset(g_envp_hal, 0, ENVP_LENTH);
	count = count > ENVP_LENTH ? ENVP_LENTH : count;
	ret = copy_from_user(g_envp_hal, buf, count);
	if ( ret ) {
		printk("%s: copy from user failed!\n", __func__);
		return -EINVAL;
	}
	if(strcmp(g_envp_hal, "receiver_status=on") != 0 && strcmp(g_envp_hal, "receiver_status=off") != 0 && strcmp(g_envp_hal, "tof_status=on") != 0 && strcmp(g_envp_hal, "tof_status=off") != 0)
	{
		memset(g_envp_hal, 0, ENVP_LENTH);
		printk("%s: write sensorevent failed\n", __func__);
		return -1;
	}
	*f_pos += count;
	ret = count;
	printk("%s: write from user %s, %zu\n", __func__, g_envp_hal, ret);
	sensorevent_notifier_call(SENSOREVENT_INFO_EVENT, g_envp_hal);
	printk("%s: notify SENSOREVENT_INFO_EVENT:\n", __func__);
	return ret;
}

static long event_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	char *wakeInfo = NULL;

	printk("%s: sensor event ioctl enter, data addr: %ld\n", __func__, arg);
	if (g_event_pdata == NULL) {
		printk("%s: sensor event ioctl error: g_event_pdata = null\n", __func__);
		return -EBUSY;
	}

	switch (cmd) {
		case SENSOREVENT_REPORT_EVENT:
			printk("%s: sensor event report event\n", __func__);
			ret = sensorevent_notifier_call(SENSOREVENT_REPORT_EVENT, "sensorevent_report=true");
			break;
		case SENSOREVENT_INFO_EVENT:
			wakeInfo = (char *)(uintptr_t)arg;
			copy_from_user(g_envp_hal, wakeInfo, ENVP_LENTH);
			printk("%s: sensor event info event(%s)\n", __func__, g_envp_hal);
			ret = sensorevent_notifier_call(SENSOREVENT_INFO_EVENT, g_envp_hal);
			break;
		default:
			printk("%s: unsupport cmd\n", __func__);
			ret = -EINVAL;
			break;
	}

	return (long)ret;
}

static const struct file_operations g_event_fops = {
	.owner           = THIS_MODULE,
	.open            = simple_open,
	.unlocked_ioctl  = event_ioctl,
	.compat_ioctl    = event_ioctl,
	.read		 = event_read,
	.write           = event_write,
};

static struct miscdevice g_event_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "sensorevent",
	.fops  = &g_event_fops,
};

static int sensorevent_notifier_call(unsigned long event, char *str)
{
	int ret;
	char envp_ext0[ENVP_LENTH];
	char *envp_ext[ENVP_EXT_MEMBER] = { envp_ext0, NULL };

	printk("%s: sensorevent_notifier_call: sensor event is %lu, str is %s\n", __func__, event, str);
	mutex_lock(&g_event_pdata->notifier_lock);
	ret = snprintf_s(envp_ext0, ENVP_LENTH, (ENVP_LENTH - 1), str);
	if (ret < 0) {
		printk("%s: snprintf failed, ret = %d\n", __func__, ret);
		mutex_unlock(&g_event_pdata->notifier_lock);
		return ret;
	}

	kobject_uevent_env(&g_event_miscdev.this_device->kobj, KOBJ_CHANGE, envp_ext);
	mutex_unlock(&g_event_pdata->notifier_lock);

	return 0;
}

static int sensorevent_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	printk("%s: sensor event probe enter\n", __func__);
	g_event_pdata = devm_kzalloc(dev, sizeof(*g_event_pdata), GFP_KERNEL);
	if (g_event_pdata == NULL) {
		printk("%s: cannot allocate usb_audio_common data\n", __func__);
		return -ENOMEM;
	}
	mutex_init(&g_event_pdata->notifier_lock);

	ret = misc_register(&g_event_miscdev);
	if (ret != 0) {
		printk("%s: can't register sensor event miscdev, ret:%d\n", __func__, ret);
		goto err_out;
	}
	printk("%s: sensor event probe success\n", __func__);

	return 0;

err_out:
	misc_deregister(&g_event_miscdev);
	devm_kfree(dev, g_event_pdata);
	g_event_pdata = NULL;

	return ret;
}

static int sensorevent_remove(struct platform_device *pdev)
{
	if (g_event_pdata != NULL) {
		printk("%s: sensor event free\n", __func__);
		devm_kfree(&pdev->dev, g_event_pdata);
		g_event_pdata = NULL;
	}

	misc_deregister(&g_event_miscdev);

	printk("%s: exit\n", __func__);

	return 0;
}

static struct platform_driver g_event_driver = {
	.driver = {
		.name           = "sensorevent",
		.owner          = THIS_MODULE,
		.of_match_table = g_sensorevent_of_match,
	},
	.probe  = sensorevent_probe,
	.remove = sensorevent_remove,
};

static int __init sensorevent_init(void)
{
	int ret;
	ret = platform_driver_register(&g_event_driver);
	if (ret > 0) {
		printk("%s: sensor event driver register failed\n", __func__);
		return ret;
	}
	printk("%s: sensor event driver register succeed\n", __func__);
	return ret;
}

static void __exit sensorevent_exit(void)
{
	platform_driver_unregister(&g_event_driver);
}

module_init(sensorevent_init);
module_exit(sensorevent_exit);

MODULE_DESCRIPTION("sensorevent control driver");
MODULE_LICENSE("GPL v2");
