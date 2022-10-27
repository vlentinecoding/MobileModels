/*
 * hn_haptics_effects wrapper
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

#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <securec.h>
#include <securectype.h>

#ifdef pr_fmt
#undef pr_fmt
#endif
#define pr_fmt(fmt) "[hapwrap] %s: " fmt, __func__

#define HAPWRAP_IOC_MAGIC		'H'
#define HAPWRAP_SET_VMAX		_IOW(HAPWRAP_IOC_MAGIC, 0x01, unsigned int)
#define HAPWRAP_GET_VMAX		_IOR(HAPWRAP_IOC_MAGIC, 0x02, unsigned int *)
#define HAPWRAP_GET_F0			_IOR(HAPWRAP_IOC_MAGIC, 0x03, unsigned int *)

/* BEGIN definitions of legacy haptics */
#define MAX_WRITE_BUF_LEN		16
#define LONG_HAPTIC_RTP_MAX_ID		4999
#define SHORT_HAPTIC_RAM_MAX_ID		309
#define LONG_TIME_AMP_DIV_COFF		100
#define SHORT_HAPTIC_AMP_DIV_COFF	10
/* END definitions of legacy haptics */

/* BEGIN definitions of qcom-hv-haptics */
enum s_period {
	T_LRA = 0,
	T_LRA_DIV_2,
	T_LRA_DIV_4,
	T_LRA_DIV_8,
	T_LRA_X_2,
	T_LRA_X_4,
	T_LRA_X_8,
	T_RESERVED,
	/* F_xKHZ definitions are for FIFO only */
	F_8KHZ,
	F_16KHZ,
	F_24KHZ,
	F_32KHZ,
	F_44P1KHZ,
	F_48KHZ,
	F_RESERVED,
};

struct fifo_cfg {
	u8				*samples;
	u32				num_s;
	enum s_period			period_per_s;
	u32				play_length_us;
};
/* END definitions of qcom-hv-haptics */

#define HAPWRAP_MAGIC			"HHAP"
#define HAPWRAP_EFFECTS_MAX		(SHORT_HAPTIC_RAM_MAX_ID / SHORT_HAPTIC_AMP_DIV_COFF)
#define HAPWRAP_DEFAULT_PLAY_HZ		24000
#define HAPWRAP_DEFAULT_VMAX		7500

#define HAPWRAP_CONST_MAX_AMP_CFG	8
#define HAPWRAP_FIFO_MAX_AMP_CFG	5
static s16 g_hapwrap_const_gains[HAPWRAP_CONST_MAX_AMP_CFG + 1] = {
	0x7fff, 0x7fff, 0x6fff, 0x5fff, 0x4fff, 0x3fff, 0x2fff, 0x1fff, 0xfff
};
static s16 g_hapwrap_fifo_gains[HAPWRAP_FIFO_MAX_AMP_CFG + 1] = {
	0x7fff, 0x7fff, 0x65ff, 0x4Bff, 0x32ff, 0x18ff
};

struct hn_hapwrap_data {
	struct device 		*dev;
	void			*hap_chip;
	struct delayed_work	stop_work;
	struct mutex		lock;
	void			*fw_cont;
	u32			play_rate_hz;
	u32			effects_cnt;
	struct fifo_cfg		fifos[HAPWRAP_EFFECTS_MAX];
};

static struct hn_hapwrap_data g_hapwrap_data;

static int hapwrap_file_open(struct inode *inode, struct file *filp)
{
	if (!filp)
		return 0;

	if (!try_module_get(THIS_MODULE))
		return -ENODEV;
	filp->private_data = &g_hapwrap_data;
	return 0;
}

static int hapwrap_file_release(struct inode *inode, struct file *filp)
{
	if (!filp)
		return 0;
	filp->private_data = NULL;
	module_put(THIS_MODULE);
	return 0;
}

static ssize_t hapwrap_file_read(struct file *filp,
	char *buff, size_t len, loff_t *offset)
{
	return 0;
}

#ifdef CONFIG_INPUT_QCOM_HV_HAPTICS
extern int hn_haptics_set_effects_vmax(void *priv, u32 vmax);
extern int hn_haptics_get_effects_vmax(void *priv, u32 *pvmax);
extern int hn_haptics_get_calib_f0(void *priv, u32 *p_f0);
extern int hn_haptics_load_effect(void *priv, struct fifo_cfg *fifo, s16 magnitude);
extern int hn_haptics_enable_play(void *priv, bool en);
#else
int hn_haptics_set_effects_vmax(void *priv, u32 vmax)
{
	return 0;
}
int hn_haptics_get_effects_vmax(void *priv, u32 *pvmax)
{
	return 0;
}
int hn_haptics_get_calib_f0(void *priv, u32 *p_f0)
{
	return 0;
}
int hn_haptics_load_effect(void *priv, struct fifo_cfg *fifo, s16 magnitude)
{
	return 0;
}

int hn_haptics_enable_play(void *priv, bool en)
{
	return 0;
}
#endif

static void hapwrap_stop_func(struct work_struct *work)
{
	pr_info("stop");
	if (hn_haptics_enable_play(g_hapwrap_data.hap_chip, false) < 0)
		pr_err("stop play failed\n");
}

static void perform_constant_haptics(u32 duration, u32 amplitude)
{
	void *chip = g_hapwrap_data.hap_chip;
	s16 magnitude;

	if (amplitude > HAPWRAP_CONST_MAX_AMP_CFG)
		amplitude = HAPWRAP_CONST_MAX_AMP_CFG;
	magnitude = g_hapwrap_const_gains[amplitude];

	pr_info("start");
	if (hn_haptics_load_effect(chip, NULL, magnitude)) {
		pr_err("load effect failed\n");
		return;
	}
	if (hn_haptics_enable_play(chip, true) < 0) {
		pr_err("start play failed\n");
		return;
	}
	cancel_delayed_work(&g_hapwrap_data.stop_work);
	schedule_delayed_work(&g_hapwrap_data.stop_work,
		msecs_to_jiffies(duration));
}

static void perform_predefined_haptics(u32 index, u32 amplitude)
{
	void *chip = g_hapwrap_data.hap_chip;
	struct fifo_cfg *fifo = NULL;
	s16 magnitude;

	if (amplitude > HAPWRAP_FIFO_MAX_AMP_CFG)
		amplitude = HAPWRAP_FIFO_MAX_AMP_CFG;
	magnitude = g_hapwrap_fifo_gains[amplitude];

	pr_info("start");
	if ((index == 0) || (index > g_hapwrap_data.effects_cnt)) {
		pr_err("invalid effect index\n");
		return;
	}
	fifo = &g_hapwrap_data.fifos[index - 1];
	if (hn_haptics_load_effect(chip, fifo, magnitude)) {
		pr_err("load effect failed\n");
		return;
	}
	if (hn_haptics_enable_play(chip, true) < 0) {
		pr_err("start play failed\n");
		return;
	}
	cancel_delayed_work(&g_hapwrap_data.stop_work);
	/* extra 4ms to complete fifo playing */
	schedule_delayed_work(&g_hapwrap_data.stop_work,
		usecs_to_jiffies(fifo->play_length_us + 4000));
}

static ssize_t hapwrap_file_write(struct file *filp,
	const char *buff, size_t len, loff_t *off)
{
	char write_buf[MAX_WRITE_BUF_LEN] = {0};
	u64 type = 0;
	u32 index, amplitude, duration;

	if (!buff || !filp || (len > (MAX_WRITE_BUF_LEN - 1))) {
		pr_err("input illegal\n");
		return len;
	}

	if (copy_from_user(write_buf, buff, len)) {
		pr_err("copy_from_user failed\n");
		return len;
	}
	if (kstrtoull(write_buf, 10, &type)) {
		pr_err("read value error\n");
		return len;
	}

	pr_info("get haptic id: %llu\n", type);
	mutex_lock(&g_hapwrap_data.lock);
	if (type > LONG_HAPTIC_RTP_MAX_ID) {
		/* long time vibration */
		duration = type / LONG_TIME_AMP_DIV_COFF;
		amplitude = type % LONG_TIME_AMP_DIV_COFF;
		perform_constant_haptics(duration, amplitude);
	} else if ((type > 0) && (type <= SHORT_HAPTIC_RAM_MAX_ID)) {
		/* short time haptic effects */
		index = type / SHORT_HAPTIC_AMP_DIV_COFF;
		amplitude = type % SHORT_HAPTIC_AMP_DIV_COFF;
		perform_predefined_haptics(index, amplitude);
	}
	mutex_unlock(&g_hapwrap_data.lock);
	return len;
}

static long hapwrap_set_vmax(unsigned long arg)
{
	u32 vmax = (u32)arg;

	if (hn_haptics_set_effects_vmax(g_hapwrap_data.hap_chip, vmax)) {
		pr_err("failed to set vmax");
		return -EFAULT;
	}

	return 0;
}

static long hapwrap_get_vmax(unsigned long arg)
{
	u32 __user *pvmax = (u32 __user *)(uintptr_t)arg;
	u32 vmax;

	if (hn_haptics_get_effects_vmax(g_hapwrap_data.hap_chip, &vmax)) {
		pr_err("failed to get vmax");
		return -EFAULT;
	}

	if (put_user(vmax, pvmax) < 0) {
		pr_err("failed to put_user");
		return -EFAULT;
	}

	return 0;
}

static long hapwrap_get_f0(unsigned long arg)
{
	u32 __user *pf0 = (u32 __user *)(uintptr_t)arg;
	u32 f0;

	if (hn_haptics_get_calib_f0(g_hapwrap_data.hap_chip, &f0)) {
		pr_err("failed to get f0");
		return -EFAULT;
	}

	if (put_user(f0, pf0) < 0) {
		pr_err("failed to put_user");
		return -EFAULT;
	}

	return 0;
}

static long hapwrap_ioctl(struct file *filp,
	unsigned int cmd, unsigned long arg)
{
	long ret = 0;

	if ((_IOC_TYPE(cmd)) != HAPWRAP_IOC_MAGIC) {
		pr_err("ioctl magic number illegal");
		return -EINVAL;
	}
	switch (cmd) {
	case HAPWRAP_SET_VMAX:
		ret = hapwrap_set_vmax(arg);
		break;
	case HAPWRAP_GET_VMAX:
		ret = hapwrap_get_vmax(arg);
		break;
	case HAPWRAP_GET_F0:
		ret = hapwrap_get_f0(arg);
		break;
	default:
		pr_err("invalid cmd");
		ret = -EINVAL;
		break;
	}

	return ret;
}

static const struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = hapwrap_file_read,
	.write = hapwrap_file_write,
	.open = hapwrap_file_open,
	.unlocked_ioctl = hapwrap_ioctl,
	.compat_ioctl = hapwrap_ioctl,
	.release = hapwrap_file_release,
};

static struct miscdevice hapwrap_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "haptics",
	.fops = &fops,
};

static inline u16 hapwrap_get_u16(const u8 *base)
{
	return ((u16)base[0] << 8) | base[1];
}

static inline u32 hapwrap_get_u32(const u8 *base)
{
	return ((u32)base[0] << 24) |
		((u32)base[1] << 16) |
		((u32)base[2] << 8) |
		(u32)base[3];
}

static bool hapwrap_verify_firmware(const struct firmware *fw)
{
	size_t offset = 0;
	size_t i = 0;
	u16 checksum;
	u16 sum = 0;

	if (fw->size <= strlen(HAPWRAP_MAGIC) + sizeof(u16))
		return false;
	if (strncmp(fw->data, HAPWRAP_MAGIC, strlen(HAPWRAP_MAGIC)))
		return false;
	offset = strlen(HAPWRAP_MAGIC);
	checksum = hapwrap_get_u16(fw->data + offset);
	offset += sizeof(u16);
	for (i = offset; i < fw->size; ++i) {
		sum += fw->data[i];
	}
	if (checksum != sum) {
		pr_err("checksum failed\n");
		return false;
	}
	return true;
}

static enum s_period hapwrap_fifo_get_period(void)
{
	enum s_period p = F_24KHZ;

	switch (g_hapwrap_data.play_rate_hz) {
	case 8000:
		p = F_8KHZ;
		break;
	case 16000:
		p = F_16KHZ;
		break;
	case 24000:
		p = F_24KHZ;
		break;
	case 32000:
		p = F_32KHZ;
		break;
	case 44100:
		p = F_44P1KHZ;
		break;
	case 48000:
		p = F_48KHZ;
		break;
	default:
		pr_err("not supported, using 24Khz");
		break;
	}
	return p;
}

static u32 hapwrap_fifo_length_us(struct fifo_cfg *fifo)
{
	u32 length_us;

	switch (fifo->period_per_s) {
	case F_8KHZ:
		length_us = 1000 * fifo->num_s / 8;
		break;
	case F_16KHZ:
		length_us = 1000 * fifo->num_s / 16;
		break;
	case F_24KHZ:
		length_us = 1000 * fifo->num_s / 24;
		break;
	case F_32KHZ:
		length_us = 1000 * fifo->num_s / 32;
		break;
	case F_44P1KHZ:
		length_us = 10000 * fifo->num_s / 441;
		break;
	case F_48KHZ:
		length_us = 1000 * fifo->num_s / 48;
		break;
	default:
		pr_err("fifo->period_per_s unsupported");
		length_us = 0;
		break;
	}

	return length_us;
}

static u16 hapwrap_select_freq(u16 fmin, u16 fmax)
{
	u32 f0;
	u16 freq;

	if (hn_haptics_get_calib_f0(g_hapwrap_data.hap_chip, &f0)) {
		pr_err("failed to get f0, using default\n");
		return (fmin + fmax) / 2;
	}
	/* f0 keeps tenths-place. must be divided by 10 */
	freq = (u16)((f0 + 5) / 10);
	if (freq < fmin)
		freq = fmin;
	if (freq > fmax)
		freq = fmax;
	return freq;
}

typedef struct {
	u16 freq;
	u16 len;
	u32 offset;
} fw_freq_info;

static int hapwrap_get_selected_freq_info(const u8 *fw_buf, size_t fw_len,
	fw_freq_info *pinfo)
{
	const u8 *buf = fw_buf;
	u16 fcnt;
	u16 fmin;
	u16 fmax;
	u16 fsel;

	fcnt = hapwrap_get_u16(buf);
	if (fcnt == 0) {
		pr_err("firmware freq count error\n");
		return -EFAULT;
	}
	if (fcnt * sizeof(fw_freq_info) + sizeof(u16) > fw_len) {
		pr_err("firmware length error\n");
		return -EFAULT;
	}
	buf += sizeof(u16);
	fmin = hapwrap_get_u16(buf);
	fmax = hapwrap_get_u16(buf + sizeof(fw_freq_info) * (fcnt - 1));
	if (fmin + fcnt - 1 != fmax) {
		pr_err("firmware freq missing\n");
		return -EFAULT;
	}
	fsel= hapwrap_select_freq(fmin, fmax);
	buf += sizeof(fw_freq_info) * (fsel - fmin);
	pinfo->freq = hapwrap_get_u16(buf);
	if (pinfo->freq != fsel) {
		pr_err("firmware freq not match\n");
		return -EFAULT;
	}
	pinfo->len = hapwrap_get_u16(buf + sizeof(u16));
	pinfo->offset = hapwrap_get_u32(buf + sizeof(u16) + sizeof(u16));
	if ((pinfo->len == 0) ||
		(pinfo->offset < sizeof(u16) + fcnt * sizeof(fw_freq_info)) ||
		(pinfo->offset + pinfo->len > fw_len)) {
		pr_err("firmware format error\n");
		return -EFAULT;
	}
	return 0;
}

static int hapwrap_select_firmware_section(const u8 *fw_buf, size_t fw_len)
{
	fw_freq_info finfo = { 0 };

	if (hapwrap_get_selected_freq_info(fw_buf, fw_len, &finfo)) {
		g_hapwrap_data.fw_cont = NULL;
		return -EFAULT;
	}

	g_hapwrap_data.fw_cont = kzalloc(finfo.len, GFP_KERNEL);
	if (g_hapwrap_data.fw_cont == NULL) {
		pr_err("alloc fw_cont failed\n");
		return -ENOMEM;
	}

	if (memcpy_s(g_hapwrap_data.fw_cont, finfo.len, fw_buf + finfo.offset, finfo.len))
		pr_err("%s : memcpy_s failed\n", __func__);
	pr_info("select firmware with freq-%u successfully", finfo.freq);
	return 0;
}

typedef struct {
	u16 start;
	u16 len;
} fw_desc_item;

static void hapwrap_prepare_effects(void)
{
	u8 *fw_cont = g_hapwrap_data.fw_cont;
	u16 data_start;
	u16 effects_count;
	fw_desc_item desc;
	u8 *cur = NULL;
	u16 i;

	if (fw_cont == NULL) {
		pr_err("select nothing from firmware\n");
		return;
	}
	data_start = hapwrap_get_u16(fw_cont);
	if ((data_start % sizeof(fw_desc_item) != 0) ||
		(data_start / sizeof(fw_desc_item) > HAPWRAP_EFFECTS_MAX)) {
		pr_err("firmware header error\n");
		return;
	}
	effects_count = data_start / sizeof(fw_desc_item);

	for (i = 0; i < effects_count; ++i) {
		cur = fw_cont + i * sizeof(fw_desc_item);
		desc.start = hapwrap_get_u16(cur);
		desc.len = hapwrap_get_u16(cur + sizeof(u16));
		g_hapwrap_data.fifos[i].samples = fw_cont + desc.start;
		g_hapwrap_data.fifos[i].num_s = desc.len;
		g_hapwrap_data.fifos[i].period_per_s =
			hapwrap_fifo_get_period();
		g_hapwrap_data.fifos[i].play_length_us =
			hapwrap_fifo_length_us(&g_hapwrap_data.fifos[i]);
	}
	g_hapwrap_data.effects_cnt = effects_count;
	hn_haptics_set_effects_vmax(g_hapwrap_data.hap_chip,
		HAPWRAP_DEFAULT_VMAX);
}

/*
 * firmware format:
 * | HAPWRAP_MAGIC(4 bytes) |
 * | checksum(u16) |
 * | freq_cnt(u16) |
 * | freq_infos_segment |
 * | freq_conts_segment |
 *
 * freq_infos_segment format:
 * | freq(u16) | cont_len(u16) | cont_offset(u32) |
 * | freq(u16) | cont_len(u16) | cont_offset(u32) |
 * | ... |
 *
 * freq_conts_segment format:
 * | start(u16) | len(u16) | start(u16) | len(u16) | ... |
 * | data_field |
 * | start(u16) | len(u16) | start(u16) | len(u16) | ... |
 * | data_field |
 * | ... |
 */
static void hapwrap_firmware_loaded(const struct firmware *fw, void *context)
{
	size_t offset;

	if ((fw == NULL) || (context == NULL)) {
		pr_err("illegal input\n");
		return;
	}
	pr_info("loaded size %u\n", fw->size);
	if (!hapwrap_verify_firmware(fw)) {
		pr_err("firmware verify failed\n");
		release_firmware(fw);
		return;
	}
	offset = strlen(HAPWRAP_MAGIC) + sizeof(u16);
	hapwrap_select_firmware_section(fw->data + offset, fw->size - offset);
	release_firmware(fw);
	mutex_lock(&g_hapwrap_data.lock);
	hapwrap_prepare_effects();
	mutex_unlock(&g_hapwrap_data.lock);
}

static void* hapwrap_get_hapchip(struct device *dev)
{
	struct device_node *hapchip_node = NULL;
	struct platform_device *hapchip_pdev = NULL;

	hapchip_node = of_parse_phandle(dev->of_node, "qhaptics_handle", 0);
	if (hapchip_node == NULL) {
		pr_err("parse qhaptics_handle failed\n");
		return NULL;
	}
	hapchip_pdev = of_find_device_by_node(hapchip_node);
	if (hapchip_pdev == NULL) {
		pr_err("find hapchip device failed\n");
		return NULL;
	}
	return dev_get_drvdata(&hapchip_pdev->dev);
}

static int hapwrap_misc_init(void)
{
	int ret;

	ret = misc_register(&hapwrap_misc);
	if (ret != 0) {
		pr_err("misc register failed\n");
		return -ENODEV;
	}

	INIT_DELAYED_WORK(&g_hapwrap_data.stop_work, hapwrap_stop_func);
	mutex_init(&g_hapwrap_data.lock);
	return 0;
}

static int hapwrap_probe(struct platform_device *pdev)
{
	void *hap_chip = NULL;

	hap_chip = hapwrap_get_hapchip(&pdev->dev);
	if (hap_chip == NULL) {
		pr_err("get hapchip failed\n");
		return -ENODEV;
	}
	g_hapwrap_data.dev = &pdev->dev;
	g_hapwrap_data.hap_chip = hap_chip;
	g_hapwrap_data.effects_cnt = 0;

	g_hapwrap_data.play_rate_hz = HAPWRAP_DEFAULT_PLAY_HZ;
	of_property_read_u32(pdev->dev.of_node,
		"play_rate_hz", &g_hapwrap_data.play_rate_hz);

	if (hapwrap_misc_init())
		return -ENODEV;

	request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
		"hapwrap.bin", &pdev->dev, GFP_KERNEL,
		&g_hapwrap_data, hapwrap_firmware_loaded);

	dev_set_drvdata(&pdev->dev, &g_hapwrap_data);
	return 0;
}

static int hapwrap_remove(struct platform_device *pdev)
{
	misc_deregister(&hapwrap_misc);
	cancel_delayed_work(&g_hapwrap_data.stop_work);
	hn_haptics_enable_play(g_hapwrap_data.hap_chip, false);
	g_hapwrap_data.effects_cnt = 0;
	if (g_hapwrap_data.fw_cont != NULL) {
		kfree(g_hapwrap_data.fw_cont);
		g_hapwrap_data.fw_cont = NULL;
	}
	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int hapwrap_suspend(struct device *dev)
{
	cancel_delayed_work(&g_hapwrap_data.stop_work);
	hn_haptics_enable_play(g_hapwrap_data.hap_chip, false);
	return 0;
}

static int hapwrap_resume(struct device *dev)
{
	return 0;
}
#endif

static const struct dev_pm_ops hapwrap_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hapwrap_suspend, hapwrap_resume)
};

static const struct of_device_id hapwrap_match_table[] = {
	{ .compatible = "honor,haptics_wrapper" },
	{},
};

static struct platform_driver hapwrap_driver = {
	.driver		= {
		.name = "hn_haptics_wrapper",
		.of_match_table = hapwrap_match_table,
		.pm = &hapwrap_pm_ops,
	},
	.probe		= hapwrap_probe,
	.remove		= hapwrap_remove,
};
module_platform_driver(hapwrap_driver);

MODULE_DESCRIPTION("Honor Technologies, Inc. Wrapper of qcom-hv-haptics");
MODULE_LICENSE("GPL v2");
