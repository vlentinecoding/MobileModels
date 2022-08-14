#ifndef __LED_HAPTICS_H_
#define __LED_HAPTICS_H_

void haptic_play_work(struct work_struct *work);
void haptic_stop_work(struct work_struct *work);
enum hrtimer_restart haptic_timer_func(struct hrtimer *timer);
void android_vibrator_enable(struct led_classdev *led_cdev,
				enum led_brightness value);
ssize_t state_show(struct device *dev,
		struct device_attribute *attr, char *buf);
ssize_t state_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t activate_show(struct device *dev,
		struct device_attribute *attr, char *buf);
ssize_t activate_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t duration_show(struct device *dev,
		struct device_attribute *attr, char *buf);
ssize_t duration_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t trigger_index_show(struct device *dev,
		struct device_attribute *attr, char *buf);
size_t trigger_index_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
size_t num_waves_show(struct device *dev,
		struct device_attribute *attr, char *buf);
size_t num_waves_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
ssize_t dig_scale_show(struct device *dev,
		struct device_attribute *attr, char *buf);
size_t dig_scale_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count);
int android_hal_stub_init(struct tas256x_priv *p_tas256x, struct device *dev);

#endif
