#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/wait.h>

struct gpio_config {
	int gpio;
	const char *prop;
	const char *label;
};

struct hardware_detector_info {
	struct device *dev;
	struct gpio_config *gipo_tab;
	int gpio_cnt;
};

static struct gpio_config g_gpio_tab[] = {
	{ 0, "gpio_detector_left", "detector_left" },
	{ 0, "gpio_detector_bottom", "detector_bottom" },
	{ 0, "gpio_sdcard_detector", "sdcard_detector" },
};

static struct proc_dir_entry *proc_hardware_detector = NULL;
static struct hardware_detector_info *g_dev_info = NULL;

#define HARDWARE_DETECTOR_FILE_DEFINE(name) \
static int name##_proc_show(struct seq_file *m, void *v) \
{ \
	int i, val; \
	if (g_dev_info == NULL || g_dev_info->dev == NULL || \
		g_dev_info->gipo_tab == NULL) { \
		printk(KERN_ERR "g_dev_info not init\n"); \
		return -EINVAL; \
	} \
	for (i = 0; i < g_dev_info->gpio_cnt; i++) { \
		if (strcmp(g_dev_info->gipo_tab[i].prop, #name) == 0) \
			break; \
	} \
	if (i == g_dev_info->gpio_cnt) { \
		dev_err(g_dev_info->dev, "gpio %s not found in table\n", #name); \
		return -EINVAL; \
	} \
	val = gpio_get_value(g_dev_info->gipo_tab[i].gpio); \
	dev_err(g_dev_info->dev, "%s GPIO%d value is %d\n", #name, \
		g_dev_info->gipo_tab[i].gpio, val); \
	seq_printf(m, "%d\n", val); \
	return 0; \
} \
static int name##_open(struct inode *inode, struct file *file) \
{ \
	return single_open(file, name##_proc_show, NULL); \
} \
\
static ssize_t name##_write(struct file *file, const char __user *buffer, \
	size_t count, loff_t *ppos) \
{ \
	return 0; \
} \
static const struct file_operations name##_proc_fops = { \
	.open		= name##_open, \
	.read		= seq_read, \
	.llseek		= seq_lseek, \
	.release	= single_release, \
	.write		= name##_write, \
}

HARDWARE_DETECTOR_FILE_DEFINE(gpio_detector_left);
HARDWARE_DETECTOR_FILE_DEFINE(gpio_detector_bottom);
HARDWARE_DETECTOR_FILE_DEFINE(gpio_sdcard_detector);

static int detector_gpio_request(struct device *dev,
	const char *prop, const char *label, int *gpio)
{
	if (dev == NULL || prop == NULL || label == NULL || gpio == NULL) {
		printk(KERN_ERR "detector_gpio_request: invalid input\n");
		return -EINVAL;
	}

	*gpio = of_get_named_gpio(dev->of_node, prop, 0);
	dev_info(dev, "%s: %s=%d\n", label, prop, *gpio);

	if (!gpio_is_valid(*gpio)) {
		dev_err(dev, "gpio %d is not valid\n", *gpio);
		return -EINVAL;
	}

	if (gpio_request(*gpio, label)) {
		dev_err(dev, "gpio %d request fail\n", *gpio);
		return -EINVAL;
	}

	return 0;
}

static int hardware_detector_config_gpio(struct device *dev,
	const char *prop, const char *label, int *gpio)
{
	if (detector_gpio_request(dev, prop, label, gpio))
		return -EINVAL;

	if (gpio_direction_input((unsigned int)(*gpio))) {
		gpio_free((unsigned int)(*gpio));
		dev_err(dev, "gpio %d set input fail\n", *gpio);
		return -EINVAL;
	}

	return 0;
}

static int hardware_detector_gpio_init(struct hardware_detector_info *info)
{
	int i, j;
	if (info == NULL || info->dev == NULL || info->gipo_tab == NULL) {
		printk(KERN_ERR "gpio table not init\n");
		return -EINVAL;
	}

	for (i = 0; i < info->gpio_cnt; i++) {
		if (hardware_detector_config_gpio(info->dev, info->gipo_tab[i].prop,
			info->gipo_tab[i].label, &info->gipo_tab[i].gpio) != 0) {
			goto config_gpio_fail;
		}
	}
	return 0;

config_gpio_fail:
	for (j = 0; j < i; j++) {
		gpio_free((unsigned int)(info->gipo_tab[j].gpio));
	}
	return -EINVAL;
}

static int init_hardware_detector(struct platform_device *pdev){
	int ret;
	proc_hardware_detector = proc_mkdir("hardware_detector", NULL);
	if (proc_hardware_detector == NULL)
		return -EINVAL;
	/* 122: gpio122 detector_left, 126: gpio126 detector_bottom */
	proc_create("122", 0644, proc_hardware_detector, &gpio_detector_left_proc_fops);
	proc_create("126", 0644, proc_hardware_detector, &gpio_detector_bottom_proc_fops);
	g_dev_info = devm_kzalloc(&pdev->dev, sizeof(*g_dev_info), GFP_KERNEL);
	if (g_dev_info == NULL)
		return -ENOMEM;
	g_dev_info->dev = &pdev->dev;
	g_dev_info->gipo_tab = g_gpio_tab;
	g_dev_info->gpio_cnt = (int)(sizeof(g_gpio_tab) / sizeof(g_gpio_tab[0]));
	if (hardware_detector_gpio_init(g_dev_info)) {
		ret = -EINVAL;
		goto gpio_init_fail;
	}
	platform_set_drvdata(pdev, g_dev_info);
	return 0;

gpio_init_fail:
	devm_kfree(&pdev->dev, g_dev_info);
	return ret;
}

static int remove_hardware_detector(struct platform_device *pdev)
{
	struct hardware_detector_info *info = dev_get_drvdata(&pdev->dev);
	if (info == NULL)
		return -ENODEV;
	devm_kfree(&pdev->dev, info);
	platform_set_drvdata(pdev, NULL);
	g_dev_info = NULL;
	return 0;
}

static struct of_device_id hardware_detector_match_table[] = {
	{
		.compatible = "honor,hardware_detector_info",
	},
	{},
};

static struct platform_driver hardware_detector_driver = {
	.probe = init_hardware_detector,
	.remove = remove_hardware_detector,
	.driver = {
		.name = "honor_hardware_detector",
		.owner = THIS_MODULE,
		.of_match_table = hardware_detector_match_table,
	},
};

static int __init hardware_detector_init(void)
{
	int ret = 0;
	ret = platform_driver_register(&hardware_detector_driver);
	if (ret){
		printk(KERN_ERR "hardware_detector_drv regiset error %d\n", ret);
	}

	return ret;
}

static void __exit hardware_detector_exit(void)
{
	platform_driver_unregister(&hardware_detector_driver);
}

MODULE_AUTHOR("honor");
MODULE_DESCRIPTION("hardware DFR detector");
MODULE_LICENSE("GPL");

module_init(hardware_detector_init);
module_exit(hardware_detector_exit);
