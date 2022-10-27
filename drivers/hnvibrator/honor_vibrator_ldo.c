/*
 * honor_vibrator_ldo.c
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
 */

#define pr_fmt(fmt)	"%s: " fmt, __func__

#include <linux/errno.h>
#include <linux/hrtimer.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/leds.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_device.h>
#include <linux/regmap.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

/*
 * Define vibration periods: default(5sec), min(50ms), max(15sec)
 */
#define HONOR_VIB_MIN_PLAY_MS			50
#define HONOR_VIB_PLAY_MS_EDFAULT		5000
#define HONOR_VIB_MAX_PLAY_MS			15000

struct vib_ldo_dev {
	struct led_classdev     cdev;
	struct mutex            lock;
	struct hrtimer          stop_timer;
	struct work_struct      vib_work;

	int                     gpio;
	int                     state;
	u64                     vib_play_ms;
	bool                    vib_enabled;
};

static void honor_vib_ldo_enable(struct vib_ldo_dev *chip, bool enable)
{
	if (chip->vib_enabled == enable)
		return;

	gpio_set_value(chip->gpio, enable);
	chip->vib_enabled = enable;
}

static void honor_vib_work(struct work_struct *work)
{
	struct vib_ldo_dev *chip = container_of(work, struct vib_ldo_dev,
		vib_work);
	int ret = 0;

	if (chip->state) {
		if (!chip->vib_enabled)
			honor_vib_ldo_enable(chip, true);

		if (ret == 0)
			hrtimer_start(&chip->stop_timer, ms_to_ktime(chip->vib_play_ms),
				HRTIMER_MODE_REL);
	} else {
		honor_vib_ldo_enable(chip, false);
	}
}

static enum hrtimer_restart honor_vib_stop_timer(struct hrtimer *timer)
{
	struct vib_ldo_dev *chip = container_of(timer, struct vib_ldo_dev,
		stop_timer);

	chip->state = 0;
	schedule_work(&chip->vib_work);
	return HRTIMER_NORESTART;
}

static ssize_t honor_vib_show_state(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct vib_ldo_dev *chip = container_of(cdev, struct vib_ldo_dev, cdev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", chip->vib_enabled);
}

static ssize_t honor_vib_store_state(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	/* At present, nothing to do with setting state */
	return count;
}

static ssize_t honor_vib_show_duration(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct vib_ldo_dev *chip = container_of(cdev, struct vib_ldo_dev, cdev);
	ktime_t time_rem;
	s64 time_ms = 0;

	if (hrtimer_active(&chip->stop_timer)) {
		time_rem = hrtimer_get_remaining(&chip->stop_timer);
		time_ms = ktime_to_ms(time_rem);
	}

	return scnprintf(buf, PAGE_SIZE, "%lld\n", time_ms);
}

static ssize_t honor_vib_store_duration(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct vib_ldo_dev *chip = container_of(cdev, struct vib_ldo_dev, cdev);
	u32 val;
	int ret;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	/* setting 0 on duration is NOP for now */
	if (val <= 0)
		return count;

	if (val < HONOR_VIB_MIN_PLAY_MS)
		val = HONOR_VIB_MIN_PLAY_MS;

	if (val > HONOR_VIB_MAX_PLAY_MS)
		val = HONOR_VIB_MAX_PLAY_MS;

	mutex_lock(&chip->lock);
	chip->vib_play_ms = val;
	mutex_unlock(&chip->lock);

	return count;
}

static ssize_t honor_vib_show_activate(struct device *dev,
	struct device_attribute *attr, char *buf)
{

	/* For now nothing to show */
	return scnprintf(buf, PAGE_SIZE, "%d\n", 0);
}

static ssize_t honor_vib_store_activate(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *cdev = dev_get_drvdata(dev);
	struct vib_ldo_dev *chip = container_of(cdev, struct vib_ldo_dev, cdev);
	u32 val;
	int ret;

	ret = kstrtouint(buf, 0, &val);
	if (ret < 0)
		return ret;

	if (val != 0 && val != 1)
		return count;

	mutex_lock(&chip->lock);
	hrtimer_cancel(&chip->stop_timer);
	chip->state = val;
	pr_err("%s : state = %d, time = %llums\n", __func__,
		chip->state, chip->vib_play_ms);
	mutex_unlock(&chip->lock);
	schedule_work(&chip->vib_work);

	return count;
}

static struct device_attribute honor_vib_attrs[] = {
	__ATTR(state, 0664, honor_vib_show_state, honor_vib_store_state),
	__ATTR(duration, 0664, honor_vib_show_duration, honor_vib_store_duration),
	__ATTR(activate, 0664, honor_vib_show_activate, honor_vib_store_activate),
};

static int honor_vib_parse_dt(struct device *dev, struct vib_ldo_dev *chip)
{
	chip->gpio = of_get_named_gpio(dev->of_node, "vibrator,ldo-en-gpio", 0);
	if (!gpio_is_valid(chip->gpio)) {
		pr_err("%s : fail to get ldo-en-gpio from devicetree\n",
			__func__);
		return -EINVAL;
	} else {
		pr_info("%s : ldo-en-gpio=%d\n", __func__, chip->gpio);
	}

	return 0;
}

static int honor_vibrator_ldo_suspend(struct device *dev)
{
	struct vib_ldo_dev *chip = dev_get_drvdata(dev);

	mutex_lock(&chip->lock);
	hrtimer_cancel(&chip->stop_timer);
	cancel_work_sync(&chip->vib_work);
	honor_vib_ldo_enable(chip, false);
	mutex_unlock(&chip->lock);

	return 0;
}
static SIMPLE_DEV_PM_OPS(honor_vibrator_ldo_pm_ops, honor_vibrator_ldo_suspend,
	NULL);

static int honor_vibrator_ldo_probe(struct platform_device *pdev)
{
	struct vib_ldo_dev *chip;
	int i;
	int ret;

	pr_info("%s in\n", __func__);

	chip = devm_kzalloc(&pdev->dev, sizeof(*chip), GFP_KERNEL);
	if (!chip)
		return -ENOMEM;

	ret = honor_vib_parse_dt(&pdev->dev, chip);
	if (ret < 0)
		goto parse_dt_fail;

	chip->vib_play_ms = HONOR_VIB_PLAY_MS_EDFAULT;
	mutex_init(&chip->lock);
	INIT_WORK(&chip->vib_work, honor_vib_work);

	hrtimer_init(&chip->stop_timer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	chip->stop_timer.function = honor_vib_stop_timer;
	dev_set_drvdata(&pdev->dev, chip);

	chip->cdev.name = "vibrator";
	ret = devm_led_classdev_register(&pdev->dev, &chip->cdev);
	if (ret < 0) {
		pr_err("%s : register led class device failed, ret=%d\n",
			__func__, ret);
		goto register_fail;
	}

	for (i = 0; i < ARRAY_SIZE(honor_vib_attrs); i++) {
		ret = sysfs_create_file(&chip->cdev.dev->kobj,
				&honor_vib_attrs[i].attr);
		if (ret < 0) {
			dev_err(&pdev->dev, "%s : create sysfs file failed, ret=%d\n",
				__func__, ret);
			goto sysfs_fail;
		}
	}

	ret = gpio_direction_output(chip->gpio, 0);
	if (ret < 0) {
		pr_err("%s : set gpio output high failed, ret=%d\n",
			__func__, ret);
		goto sysfs_fail;
	}

	pr_info("%s : done\n", __func__);
	return 0;

sysfs_fail:
	for (--i; i >= 0; i--)
		sysfs_remove_file(&chip->cdev.dev->kobj, &honor_vib_attrs[i].attr);
register_fail:
	dev_set_drvdata(&pdev->dev, NULL);
parse_dt_fail:
	devm_kfree(&pdev->dev, chip);

	return ret;
}

static int honor_vibrator_ldo_remove(struct platform_device *pdev)
{
	struct vib_ldo_dev *chip = dev_get_drvdata(&pdev->dev);

	hrtimer_cancel(&chip->stop_timer);
	cancel_work_sync(&chip->vib_work);
	mutex_destroy(&chip->lock);
	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static const struct of_device_id vibrator_ldo_match_table[] = {
	{ .compatible = "honor,vibrator-ldo" },
	{ /* sentinel */ },
};
MODULE_DEVICE_TABLE(of, vibrator_ldo_match_table);

static struct platform_driver honor_vibrator_ldo_driver = {
	.driver	= {
		.name		= "honor,honor-vibrator-ldo",
		.of_match_table	= vibrator_ldo_match_table,
		.pm		= &honor_vibrator_ldo_pm_ops,
	},
	.probe	= honor_vibrator_ldo_probe,
	.remove	= honor_vibrator_ldo_remove,
};
module_platform_driver(honor_vibrator_ldo_driver);

MODULE_DESCRIPTION("HONOR Vibrator-LDO driver");
MODULE_LICENSE("GPL v2");
