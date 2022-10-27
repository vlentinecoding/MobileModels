#include "../../../../physical_layer/inc/tas256x.h"
#include "../../../../physical_layer/inc/drv2634.h"
#include <linux/cdev.h>
#include <linux/miscdevice.h>
#include <linux/hrtimer.h>
#include <linux/leds.h>

void haptic_play_work(struct work_struct *work)
{
	struct haptics_data *haptic = container_of(work, struct haptics_data, vib_play_work);
	struct tas256x_priv *p_tas256x = container_of(haptic, struct tas256x_priv, haptics);
}

void haptic_stop_work(struct work_struct *work)
{
	struct haptics_data *haptic = container_of(work, struct haptics_data, vib_stop_work);
	struct tas256x_priv *p_tas256x = container_of(haptic, struct tas256x_priv, haptics);
}

enum hrtimer_restart haptic_timer_func(struct hrtimer *timer)
{
	struct haptics_data *haptic = container_of(timer, struct haptics_data, vib_mtimer);

	schedule_work(&haptic->vib_stop_work);
	return HRTIMER_NORESTART;
}

void android_vibrator_enable(struct led_classdev *led_cdev,
		enum led_brightness value)
{
	pr_info("%s:enter!\n", __func__);
}

ssize_t state_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int nResult = 0;

	return snprintf(buf, 30, "nResult = %d\n", nResult);
}

ssize_t state_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	return count;
}

ssize_t activate_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int nResult = 0;

	return snprintf(buf, 30, "nResult = %d\n", nResult);
}

ssize_t activate_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct haptics_data *haptic = container_of(led_cdev, struct haptics_data, led_dev);
	struct tas256x_priv *p_tas256x = container_of(haptic, struct tas256x_priv, haptics);
	int channel_left = 1;
	int value = 0;
	int rc = 0;

	rc = kstrtouint(buf, 0, &value);
	if (rc < 0) {
		pr_err("%s: kstrtouint filed\n", __func__);
		return rc;
	}
	if (value == 1) {
		p_tas256x->plat_write(p_tas256x, channel_left,
				TAS256X_POWERCONTROL, 0x02);
		p_tas256x->plat_read(p_tas256x, channel_left, TAS256X_POWERCONTROL, &value);
		pr_info("%s:enter! power value = %d\n", __func__, value);
	}
	return count;
}

ssize_t duration_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	int nResult = 0;

	return snprintf(buf, 30, "nResult = %d\n", nResult);
}

ssize_t duration_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct led_classdev *led_cdev = dev_get_drvdata(dev);
	struct haptics_data *haptic = container_of(led_cdev, struct haptics_data, led_dev);
	int rc = 0;
	int value = 0;

	rc = kstrtouint(buf, 0, &value);
	if (rc < 0) {
		pr_err("%s: kstrtouint filed\n", __func__);
		return rc;
	}
	if (value > haptic->timedout)
		value = haptic->timedout;
	haptic->haptic_play_ms = value;
	pr_info("%s:enter! duration value = %d\n", __func__, value);
	return count;
}

ssize_t trigger_index_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	pr_info("%s:enter!\n", __func__);
	return snprintf(buf, 30, "index = 0\n");
}

ssize_t trigger_index_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{

	int rc = 0;
	int value = 0;

	rc = kstrtouint(buf, 0, &value);
	if (rc < 0)
		pr_err("%s: kstrtouint filed\n", __func__);
	pr_info("%s:enter! index value = %d\n", __func__, value);
	return count;
}

ssize_t num_waves_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return snprintf(buf, 30, "value = 0\n");
}

ssize_t num_waves_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{

	int rc = 0;
	int value = 0;

	rc = kstrtouint(buf, 0, &value);
	if (rc < 0)
		pr_err("%s: kstrtouint filed\n", __func__);
	pr_info("%s:enter! waves value = %d\n", __func__, value);
	return count;
}

ssize_t dig_scale_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	pr_info("%s:enter!\n", __func__);
	return snprintf(buf, 30, "vaule = 0\n");
}

ssize_t dig_scale_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{

	int rc = 0;
	int value = 0;

	rc = kstrtouint(buf, 0, &value);
	if (rc < 0)
		pr_err("%s: kstrtouint filed\n", __func__);
	pr_info("%s:enter! dig_scale value = %d\n", __func__, value);
	return count;
}

DEVICE_ATTR(state, 0644, state_show, state_store);
DEVICE_ATTR(activate, 0644, activate_show, activate_store);
DEVICE_ATTR(duration, 0644, duration_show, duration_store);
DEVICE_ATTR(cp_trigger_index, 0644, trigger_index_show, trigger_index_store);
DEVICE_ATTR(num_waves, 0644, num_waves_show, num_waves_store);
DEVICE_ATTR(dig_scale, 0644, dig_scale_show, dig_scale_store);

struct attribute *android_led_dev_fs_attrs[] = {
	&dev_attr_state.attr,
	&dev_attr_activate.attr,
	&dev_attr_duration.attr,
	&dev_attr_cp_trigger_index.attr,
	&dev_attr_num_waves.attr,
	&dev_attr_dig_scale.attr,
	NULL,
};

struct attribute_group android_led_dev_fs_attr_group = {
	.attrs = android_led_dev_fs_attrs,
};

const struct attribute_group *android_led_dev_fs_attr_groups[] = {
	&android_led_dev_fs_attr_group,
	NULL,
};

/*
 * led device register
 *
 */
int android_hal_stub_init(struct tas256x_priv *p_tas256x, struct device *dev)
{
	/* struct tas256x_priv *p_tas256x = NULL;
	 */
	struct haptics_data *haptic = &p_tas256x->haptics;
	int ret;

	haptic->haptics_playing = false;
	haptic->led_dev.name = "vibrators";
	haptic->led_dev.max_brightness = LED_FULL;
	haptic->led_dev.brightness_set = android_vibrator_enable;
	haptic->led_dev.flags = LED_BRIGHTNESS_FAST;
	haptic->led_dev.groups = android_led_dev_fs_attr_groups;
	haptic->timedout = VIB_DEFAULT_TIMEOUT;

	INIT_WORK(&haptic->vib_play_work, haptic_play_work);
	INIT_WORK(&haptic->vib_stop_work, haptic_stop_work);

	hrtimer_init(&haptic->vib_mtimer, CLOCK_MONOTONIC, HRTIMER_MODE_REL);
	haptic->vib_mtimer.function = haptic_timer_func;
	ret = led_classdev_register(dev, &haptic->led_dev);
	if (ret) {
		pr_err("Failed to create led classdev: %d\n", ret);
		return ret;
	}
	return 0;
}
