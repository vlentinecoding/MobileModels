/*
 * adsp_misc.c
 *
 * adsp misc for QC platform audio factory test
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "adsp_misc.h"
#include "ti_smartamp.h"
#include "tfa_smartamp.h"
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <log/hw_log.h>
#include <sound/adsp_misc_interface.h>

#ifdef CONFIG_HUAWEI_DSM_AUDIO_MODULE
#define CONFIG_HUAWEI_DSM_AUDIO
#endif
#ifdef CONFIG_HUAWEI_DSM_AUDIO
#include <dsm/dsm_pub.h>
#endif

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#define HWLOG_TAG adsp_misc
HWLOG_REGIST();

struct adsp_misc_info {
	struct mutex adsp_misc_mutex;
	struct smartpa_afe_interface intf;
};
static struct adsp_misc_info g_adsp_misc_info;

static int adsp_misc_open(struct inode *inode, struct file *filp)
{
	UNUSED(inode);
	UNUSED(filp);
	hwlog_debug("%s: Device opened\n", __func__);
	return 0;
}

static int adsp_misc_release(struct inode *inode, struct file *filp)
{
	UNUSED(inode);
	UNUSED(filp);
	hwlog_debug("%s: Device released\n", __func__);
	return 0;
}

void register_adsp_intf(struct smartpa_afe_interface *intf)
{
	if (intf == NULL) {
		hwlog_err("%s: input ptr is null\n", __func__);
		return;
	}
	g_adsp_misc_info.intf.afe_tisa_get_set = intf->afe_tisa_get_set;
	g_adsp_misc_info.intf.send_tfa_cal_apr = intf->send_tfa_cal_apr;
}
EXPORT_SYMBOL(register_adsp_intf);

static int afe_send_tfa_cal_apr(void *buf, int size, bool read)
{
	if (g_adsp_misc_info.intf.send_tfa_cal_apr == NULL) {
		hwlog_err("%s: function registration not done\n", __func__);
		return -EFAULT;
	}

	return g_adsp_misc_info.intf.send_tfa_cal_apr(buf, size, read);
}

// Wrapper arround set/get parameter,all set/get cmd pass through this wrapper
static int afe_tisa_algo_ctrl(u8 *user_data, uint32_t param_id,
		uint8_t get_set, uint32_t length, uint32_t module_id)
{
	if (g_adsp_misc_info.intf.afe_tisa_get_set == NULL) {
		hwlog_err("%s: function registration not done\n", __func__);
		return -EFAULT;
	}

	return g_adsp_misc_info.intf.afe_tisa_get_set(user_data, param_id,
						    get_set, length, module_id);
}

static int tisa_get_current_param(char *data, int size, int cmd)
{
	int ret;
	int param_id;

	UNUSED(size);
	switch (cmd) {
	case TAS_SA_GET_RE:
		param_id = TAS_CALC_PARAM_IDX(TAS_SA_GET_RE, LENGTH_1, CHANNEL0);
		break;
	case TAS_SA_GET_TV:
		param_id = TAS_CALC_PARAM_IDX(TAS_SA_GET_TV, LENGTH_1, CHANNEL0);
		break;
	case TAS_SA_GET_Q:
		param_id = TAS_CALC_PARAM_IDX(TAS_SA_GET_Q, LENGTH_1, CHANNEL0);
		break;
	case TAS_SA_GET_F0:
		param_id = TAS_CALC_PARAM_IDX(TAS_SA_GET_F0, LENGTH_1, CHANNEL0);
		break;
	default:
		hwlog_err("%s:unsupport type value\n", __func__);
		return -EFAULT;
	}

	ret = afe_tisa_algo_ctrl((u8 *)data, param_id, TAS_GET_PARAM,
				sizeof(uint32_t), AFE_SMARTAMP_MODULE_RX);
	if (ret < 0)
		hwlog_err("%s: Failed to get param, ret=%d", __func__, ret);

	hwlog_info("get param value=0x%x, cmd", (uint32_t)data, cmd);
	return ret;
}

static ssize_t adsp_misc_read(struct file *file, char __user *buf,
			      size_t nbytes, loff_t *pos)
{
	int ret;
	uint8_t *buffer = NULL;

	hwlog_debug("%s: Read %d bytes from adsp_misc\n", __func__, nbytes);
	if ((nbytes == 0) || (buf == NULL) || (nbytes > TFA_TUNING_RW_MAX_SIZE))
		return -EINVAL;

	UNUSED(file);
	mutex_lock(&g_adsp_misc_info.adsp_misc_mutex);

	buffer = kzalloc(nbytes, GFP_KERNEL);
	if (buffer == NULL) {
		ret = -ENOMEM;
		goto err_out;
	}

	ret = afe_send_tfa_cal_apr(buffer, nbytes, true);
	if (ret) {
		hwlog_err("dsp_msg_read error: %d\n", ret);
		ret = -EFAULT;
		goto err_out;
	}

	ret = copy_to_user(buf, buffer, nbytes);
	if (ret) {
		hwlog_err("copy_to_user error: %d\n", ret);
		ret = -EFAULT;
		goto err_out;
	}

	kfree(buffer);
	*pos += nbytes;
	mutex_unlock(&g_adsp_misc_info.adsp_misc_mutex);
	return (ssize_t)nbytes;

err_out:
	kfree(buffer);
	mutex_unlock(&g_adsp_misc_info.adsp_misc_mutex);
	return (ssize_t)ret;
}

static ssize_t adsp_misc_write(struct file *file, const char __user *buf,
			       size_t nbytes, loff_t *ppos)
{
	uint8_t *buffer = NULL;
	int ret;

	hwlog_debug("%s: Write %d bytes to adsp_misc\n", __func__, nbytes);
	if ((nbytes == 0) || (buf == NULL) || (nbytes > TFA_TUNING_RW_MAX_SIZE))
		return -EINVAL;

	UNUSED(file);
	UNUSED(ppos);
	mutex_lock(&g_adsp_misc_info.adsp_misc_mutex);

	buffer = kmalloc(nbytes, GFP_KERNEL);
	if (buffer == NULL) {
		ret = -ENOMEM;
		goto err_out;
	}

	if (copy_from_user(buffer, buf, nbytes)) {
		hwlog_err("Copy from user space err\n");
		ret = -EFAULT;
		goto err_out;
	}

	ret = afe_send_tfa_cal_apr(buffer, nbytes, false);
	if (ret) {
		hwlog_err("dsp_msg error: %d\n", ret);
		goto err_out;
	}

	kfree(buffer);
	mutex_unlock(&g_adsp_misc_info.adsp_misc_mutex);
	return (ssize_t)nbytes;

err_out:
	kfree(buffer);
	mutex_unlock(&g_adsp_misc_info.adsp_misc_mutex);
	return (ssize_t)ret;
}

static int tfa98xx_adsp_cmd(int cmd_id, uint8_t *buf, ssize_t size)
{
	int ret;

	if (buf == NULL)
		return -EINVAL;

	memset(buf, 0x00, size);
	TFA_CMD_TO_ADSP_BUF(buf, cmd_id);

	ret = afe_send_tfa_cal_apr(buf, size, false);
	mdelay(4);
	/* This is from NXP, which is adapt to the tfa algorithm */
	if ((ret == 0) && ((cmd_id & TFA_ADSP_CMD_MASK) >= 0x80))
		ret = afe_send_tfa_cal_apr(buf, size, true);

	return ret;
}

static int tfa_get_current_param(unsigned char *data, unsigned int len,
				 enum smartpa_cmd type)
{
	int ret;
	uint8_t *buffer = NULL;
	int count = TFA_ADSP_CMD_SIZE_LIMIT;
	int cur_param[2] = {0};

	if ((len < 4) || (data == NULL))
		return -EINVAL;

	buffer = kzalloc(count, GFP_KERNEL);
	if (buffer == NULL)
		return -ENOMEM;

	ret = tfa98xx_adsp_cmd(TFA_ADSP_CMD_PARAM, buffer, count);
	if (ret) {
		hwlog_err("Get param error\n");
		goto exit;
	}

	switch (type) {
	case GET_CURRENT_R0:
		cur_param[0] = TFA_CALC_PARAM(TFA_CURRENT_R0_IDX_0, buffer);
		cur_param[1] = TFA_CALC_PARAM(TFA_CURRENT_R0_IDX_1, buffer);
		hwlog_info("%s:Get current R0L = %d R0R = %d\n", __func__,
			cur_param[0], cur_param[1]);
		break;
	case GET_CURRENT_TEMPRATURE:
		cur_param[0] = TFA_CALC_PARAM(TFA_CURRENT_TEMP_IDX_0, buffer);
		cur_param[1] = TFA_CALC_PARAM(TFA_CURRENT_TEMP_IDX_1, buffer);
		hwlog_info("%s:Get current TempL = %d, TempR = %d\n", __func__,
			cur_param[0], cur_param[1]);
		break;
	case GET_CURRENT_F0:
		cur_param[0] = TFA_CALC_PARAM(TFA_CURRENT_F0_IDX_0, buffer);
		cur_param[1] = TFA_CALC_PARAM(TFA_CURRENT_F0_IDX_1, buffer);
		hwlog_info("%s:Get current F0_L = %d, F0_R = %d\n", __func__,
			cur_param[0], cur_param[1]);
		break;
	default:
		hwlog_info("%s:unsupport type value\n", __func__);
		break;
	}
	/* Following part is from NXP, which is adapt to the tfa algorithm */
	if (len <= 8 && len >= 4)
		memcpy(data, cur_param, len);

exit:
	kfree(buffer);
	return ret;
}

static int soc_adsp_send_param(struct adsp_misc_ctl_info *param,
			       unsigned char *data, unsigned int len)
{
	int ret;
	int cmd;
	int chip_vendor;

	if (param == NULL) {
		hwlog_err("%s,invalid input param\n", __func__);
		return -EINVAL;
	}

	cmd = param->cmd;
	chip_vendor = param->pa_info.chip_vendor;
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: enter, cmd = %d\n", __func__, cmd);
#endif
	switch (cmd) {
	case CALIBRATE_MODE_START:
	case CALIBRATE_MODE_STOP:
		break;
	case GET_CURRENT_R0:
		if (chip_vendor == CHIP_VENDOR_TI)
			ret = tisa_get_current_param(data, len, TAS_SA_GET_RE);
		else
			ret = tfa_get_current_param(data, len, GET_CURRENT_R0);
		break;
	case GET_CURRENT_TEMPRATURE:
		if (chip_vendor == CHIP_VENDOR_TI)
			ret = tisa_get_current_param(data, len, TAS_SA_GET_TV);
		else
			ret = tfa_get_current_param(data, len,
						    GET_CURRENT_TEMPRATURE);
		break;
	case GET_CURRENT_F0:
		if (chip_vendor == CHIP_VENDOR_TI)
			ret = tisa_get_current_param(data, len, TAS_SA_GET_F0);
		else
			ret = tfa_get_current_param(data, len, GET_CURRENT_F0);
		break;
	case GET_CURRENT_Q:
		if (chip_vendor == CHIP_VENDOR_TI)
			ret = tisa_get_current_param(data, len, TAS_SA_GET_Q);
		break;
	default:
		break;
	}

	if (ret)
		hwlog_info("%s: send cmd = %d, ret = %d\n", __func__, cmd, ret);

	return ret;
}

static int soc_adsp_get_inparam(struct adsp_ctl_param *adsp_param,
	struct adsp_misc_ctl_info **param_in)
{
	unsigned int par_len;
	struct adsp_misc_ctl_info *par_in = NULL;

	par_len = adsp_param->in_len;
#ifndef CONFIG_FINAL_RELEASE
	hwlog_info("%s: param in len is %d, in_param is %p",
		__func__, par_len, adsp_param->param_in);
#endif
	if ((par_len < MIN_PARAM_IN) || (adsp_param->param_in == NULL)) {
		hwlog_err("%s,param_in from user space is error\n", __func__);
		return -EINVAL;
	}

	par_in = kzalloc(par_len, GFP_KERNEL);
	if (par_in == NULL)
		return -ENOMEM;

	if (copy_from_user(par_in, adsp_param->param_in, par_len)) {
		hwlog_err("%s: get param copy_from_user fail\n", __func__);
		return -EFAULT;
	}
	*param_in = par_in;
	return 0;
}

static int soc_adsp_parse_param_info(void __user *arg, int compat_mode,
	struct adsp_ctl_param *adsp_param)
{
	memset(adsp_param, 0, sizeof(*adsp_param));

#ifdef CONFIG_COMPAT
	if (compat_mode == 0) {
#endif // CONFIG_COMPAT
		struct misc_io_sync_param par;

		memset(&par, 0, sizeof(par));
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: copy_from_user b64 %p\n", __func__, arg);
#endif
		if (copy_from_user(&par, arg, sizeof(par))) {
			hwlog_err("%s: copy_from_user fail\n", __func__);
			return -EFAULT;
		}

		adsp_param->out_len = par.out_len;
		adsp_param->param_out = (void __user *)par.out_param;
		adsp_param->in_len = par.in_len;
		adsp_param->param_in = (void __user *)par.in_param;

#ifdef CONFIG_COMPAT
	} else {
		struct misc_io_sync_param_compat par_compat;

		memset(&par_compat, 0, sizeof(par_compat));
#ifndef CONFIG_FINAL_RELEASE
		hwlog_info("%s: copy_from_user b32 %p\n", __func__, arg);
#endif
		if (copy_from_user(&par_compat, arg, sizeof(par_compat))) {
			hwlog_err("%s: copy_from_user fail\n", __func__);
			return -EFAULT;
		}

		adsp_param->out_len = par_compat.out_len;
		adsp_param->param_out = compat_ptr(par_compat.out_param);
		adsp_param->in_len = par_compat.in_len;
		adsp_param->param_in = compat_ptr(par_compat.in_param);
	}
#endif //CONFIG_COMPAT
	return 0;
}

static int soc_adsp_handle_sync_param(void __user *arg, int compat_mode)
{
	int ret;
	struct adsp_misc_ctl_info *param_in = NULL;
	unsigned int param_out_len;
	struct adsp_misc_data_pkg *result = NULL;
	struct adsp_ctl_param adsp_param;

	if (arg == NULL) {
		hwlog_err("%s: Invalid input arg, exit\n", __func__);
		return -EINVAL;
	}

	ret = soc_adsp_parse_param_info(arg, compat_mode, &adsp_param);
	if (ret < 0)
		return ret;

	ret = soc_adsp_get_inparam(&adsp_param, &param_in);
	if (ret < 0)
		goto err;

	param_out_len = adsp_param.out_len;
	if ((param_out_len > MIN_PARAM_OUT) && (adsp_param.param_out != NULL)) {
		result = kzalloc(param_out_len, GFP_KERNEL);
		if (result == NULL) {
			ret = -ENOMEM;
			goto err;
		}
		result->size = param_out_len - sizeof(*result);
		ret = soc_adsp_send_param(param_in, result->data, result->size);
		if (ret < 0)
			goto err;

		if (copy_to_user(adsp_param.param_out, result, param_out_len)) {
			hwlog_err("%s:copy_to_user fail\n", __func__);
			ret = -EFAULT;
		}
	} else {
		ret = soc_adsp_send_param(param_in, NULL, 0);
	}

err:
	if (param_in != NULL)
		kfree(param_in);
	if (result != NULL)
		kfree(result);
	return ret;
}

static int adsp_misc_do_ioctl(struct file *file, unsigned int command,
			      void __user *arg, int compat_mode)
{
	int ret;

	hwlog_debug("%s: enter, cmd:0x%x compat_mode=%d\n",
		__func__, command, compat_mode);
	if (file == NULL) {
		hwlog_err("%s: invalid argument\n", __func__);
		ret = -EFAULT;
		goto out;
	}

	switch (command) {
	case ADSP_MISC_IOCTL_ASYNCMSG:
		ret = -EFAULT;
		break;
	case ADSP_MISC_IOCTL_SYNCMSG:
		mutex_lock(&g_adsp_misc_info.adsp_misc_mutex);
		ret = soc_adsp_handle_sync_param(arg, compat_mode);
		mutex_unlock(&g_adsp_misc_info.adsp_misc_mutex);
		break;
	default:
		ret = -EFAULT;
		break;
	}

out:
	return ret;
}

static long adsp_misc_ioctl(struct file *file, unsigned int command,
			    unsigned long arg)
{
	return adsp_misc_do_ioctl(file, command, (void __user *)arg, 0);
}

#ifdef CONFIG_COMPAT
static long adsp_misc_ioctl_compat(struct file *file, unsigned int command,
				   unsigned long arg)
{
	switch (command) {
	case ADSP_MISC_IOCTL_ASYNCMSG_COMPAT:
		command = ADSP_MISC_IOCTL_ASYNCMSG;
		break;
	case ADSP_MISC_IOCTL_SYNCMSG_COMPAT:
		command = ADSP_MISC_IOCTL_SYNCMSG;
		break;
	default:
		break;
	}
	return adsp_misc_do_ioctl(file, command,
				  compat_ptr((unsigned int)arg), 1);
}
#else
#define adsp_misc_ioctl_compat NULL
#endif  // CONFIG_COMPAT

static const struct file_operations adsp_misc_fops = {
	.owner          = THIS_MODULE,
	.open           = adsp_misc_open,
	.release        = adsp_misc_release,
#ifndef CONFIG_FINAL_RELEASE
	.read           = adsp_misc_read, // 24bits BIG ENDING DATA
	.write          = adsp_misc_write, // 24bits BIG ENDING DATA
#endif
	.unlocked_ioctl = adsp_misc_ioctl, // 32bits LITTLE ENDING DATA
#ifdef CONFIG_COMPAT
	.compat_ioctl   = adsp_misc_ioctl_compat, // 32bits LITTLE ENDING DATA
#endif
};

static struct miscdevice adsp_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name  = "adsp_misc",
	.fops  = &adsp_misc_fops,
};

static int __init adsp_misc_init(void)
{
	int ret;

	ret = misc_register(&adsp_misc_dev);
	if (ret != 0) {
		hwlog_err("%s: register miscdev failed:%d\n", __func__, ret);
		goto err_out;
	}

	memset(&g_adsp_misc_info, 0, sizeof(g_adsp_misc_info));
	mutex_init(&g_adsp_misc_info.adsp_misc_mutex);
	hwlog_info("%s: misc dev registed success\n", __func__);

	return 0;
err_out:
	return ret;
}

static void __exit adsp_misc_exit(void)
{
	misc_deregister(&adsp_misc_dev);
}

module_init(adsp_misc_init);
module_exit(adsp_misc_exit);

/* lint -e753 */
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei adsp_misc driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");

