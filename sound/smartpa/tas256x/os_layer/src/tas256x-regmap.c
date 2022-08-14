/*
 * ALSA SoC Texas Instruments TAS256X High Performance 4W Smart Amplifier
 *
 * Copyright (C) 2016 Texas Instruments, Inc.
 *
 * Author: saiprasad
 *
 * This package is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 */

#define DEBUG 5
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/pm.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/firmware.h>
#include <linux/regmap.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/slab.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/pm.h>
#include <linux/version.h>
#include "physical_layer/inc/tas256x.h"
#include "logical_layer/inc/tas256x-logic.h"
#include "physical_layer/inc/tas256x-device.h"
#include "os_layer/inc/tas256x-codec.h"
#include "os_layer/inc/tas256x-regmap.h"
#include "misc/tas256x-misc.h"
#include <linux/kobject.h>
#include <sound/hw_audio/hw_audio_interface.h>
#if IS_ENABLED(CONFIG_TAS26XX_HAPTICS)
#include "os_layer/haptics/led/inc/led-haptics.h"
#endif

#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
#include "algo/inc/tas_smart_amp_v2.h"
#if IS_ENABLED(CONFIG_TISA_SYSFS_INTF)
#include "algo/src/smartpa-debug-common.h"
#endif /*CONFIG_TISA_SYSFS_INTF*/
#if IS_ENABLED(CONFIG_PLATFORM_QCOM)
#include <dsp/tas_qualcomm.h>
static dc_detection_data_t s_dc_detect;
#endif /*CONFIG_PLATFORM_QCOM*/
#endif /*CONFIG_TAS25XX_ALGO*/
/*For mixer_control implementation*/
#define MAX_STRING	200
#define SYS_NODE

#ifdef SYS_NODE
struct tas256x_priv *g_tas256x;
static char gSysCmdLog[MaxCmd][256];
#endif

static const char *dts_tag[][3] = {
	{
		"ti,left-channel",
		"ti,reset-gpio",
		"ti,irq-gpio"
	},
	{
		"ti,right-channel",
		"ti,reset-gpio2",
		"ti,irq-gpio2"
	}
};

static const char *reset_gpio_label[2] = {
	"TAS256X_RESET", "TAS256X_RESET2"
};

static const char *irq_gpio_label[2] = {
	"TAS256X-IRQ", "TAS256X-IRQ2"
};

static int tas256x_regmap_write(void *plat_data, unsigned int i2c_addr,
	unsigned int reg, unsigned int value)
{
	int ret = -1;
	int retry_count = TAS256X_I2C_RETRY_COUNT;
	struct linux_platform *platform_data =
		(struct linux_platform *)plat_data;

	if (platform_data->i2c_suspend)
		return ERROR_I2C_SUSPEND;

	platform_data->client->addr = i2c_addr;
	while (retry_count--) {
		ret = regmap_write(platform_data->regmap, reg,
			value);
		if (ret >= 0)
			break;
		msleep(20);
	}
	if (retry_count == -1)
		return ERROR_I2C_FAILED;
	else
		return 0;
}

static int tas256x_regmap_bulk_write(void *plat_data, unsigned int i2c_addr,
	unsigned int reg, unsigned char *pData,
	unsigned int nLength)
{
	int ret = -1;
	int retry_count = TAS256X_I2C_RETRY_COUNT;
	struct linux_platform *platform_data =
		(struct linux_platform *)plat_data;

	if (platform_data->i2c_suspend)
		return ERROR_I2C_SUSPEND;

	platform_data->client->addr = i2c_addr;
	while (retry_count--) {
		ret = regmap_bulk_write(platform_data->regmap, reg,
			 pData, nLength);
		if (ret >= 0)
			break;
		msleep(20);
	}
	if (retry_count == -1)
		return ERROR_I2C_FAILED;
	else
		return 0;
}

static int tas256x_regmap_read(void *plat_data, unsigned int i2c_addr,
	unsigned int reg, unsigned int *value)
{
	int ret = -1;
	int retry_count = TAS256X_I2C_RETRY_COUNT;
	struct linux_platform *platform_data =
		(struct linux_platform *)plat_data;

	if (platform_data->i2c_suspend)
		return ERROR_I2C_SUSPEND;

	platform_data->client->addr = i2c_addr;
	while (retry_count--) {
		ret = regmap_read(platform_data->regmap, reg,
			value);
		if (ret >= 0)
			break;
		msleep(20);
	}
	if (retry_count == -1)
		return ERROR_I2C_FAILED;
	else
		return 0;
}

static int tas256x_regmap_bulk_read(void *plat_data, unsigned int i2c_addr,
	unsigned int reg, unsigned char *pData,
	unsigned int nLength)
{
	int ret = -1;
	int retry_count = TAS256X_I2C_RETRY_COUNT;
	struct linux_platform *platform_data =
		(struct linux_platform *)plat_data;

	if (platform_data->i2c_suspend)
		return ERROR_I2C_SUSPEND;

	platform_data->client->addr = i2c_addr;
	while (retry_count--) {
		ret = regmap_bulk_read(platform_data->regmap, reg,
			 pData, nLength);
		if (ret >= 0)
			break;
		msleep(20);
	}
	if (retry_count == -1)
		return ERROR_I2C_FAILED;
	else
		return 0;
}

static int tas256x_regmap_update_bits(void *plat_data, unsigned int i2c_addr,
	unsigned int reg, unsigned int mask,
	unsigned int value)
{
	int ret = -1;
	int retry_count = TAS256X_I2C_RETRY_COUNT;
	struct linux_platform *platform_data =
		(struct linux_platform *)plat_data;

	if (platform_data->i2c_suspend)
		return ERROR_I2C_SUSPEND;

	platform_data->client->addr = i2c_addr;
	while (retry_count--) {
		ret = regmap_update_bits(platform_data->regmap, reg,
			mask, value);
		if (ret >= 0)
			break;
		msleep(20);
	}
	if (retry_count == -1)
		return ERROR_I2C_FAILED;
	else
		return 0;
}

/* REGBIN related */
/* max. length of a alsa mixer control name */
#define MAX_CONTROL_NAME        48

static char *fw_name = "tas256x_reg.bin";

const char *blocktype[5] = {
	"COEFF",
	"POST_POWER_UP",
	"PRE_SHUTDOWN",
	"PRE_POWER_UP",
	"POST_SHUTDOWN"
};

static int tas256x_process_block(void *pContext, unsigned char *data,
	unsigned char dev_idx, int sublocksize)
{
	struct tas256x_priv *p_tas256x = (struct tas256x_priv *)pContext;
	unsigned char subblk_typ = data[1];
	int subblk_offset = 2;
	enum channel chn =
		(dev_idx == 0) ? channel_both : (enum channel)dev_idx;

	switch (subblk_typ) {
	case TAS256X_CMD_SING_W: {
/*
 *		dev_idx		: one byte
 *		subblk_type	: one byte
 *		payload_len	: two bytes
 *		{
 *			book	: one byte
 *			page	: one byte
 *			reg		: one byte
 *			val		: one byte
 *		}[payload_len/4]
 */
		int i = 0;
		unsigned short len = SMS_HTONS(data[2], data[3]);

		subblk_offset += 2;
		if (subblk_offset + 4 * len > sublocksize) {
			pr_err("Out of memory %s: %u\n", __func__, __LINE__);
			break;
		}

		for (i = 0; i < len; i++) {
			p_tas256x->write(p_tas256x, chn,
				TAS256X_REG(data[subblk_offset],
					data[subblk_offset + 1],
					data[subblk_offset + 2]),
				data[subblk_offset + 3]);
			subblk_offset += 4;
		}
	}
	break;
	case TAS256X_CMD_BURST: {
/*
 *		dev_idx		: one byte
 *		subblk_type : one byte
 *		payload_len	: two bytes
 *		book		: one byte
 *		page		: one byte
 *		reg		: one byte
 *		reserve		: one byte
 *		payload		: payload_len bytes
 */
		unsigned short len = SMS_HTONS(data[2], data[3]);

		subblk_offset += 2;
		if (subblk_offset + 4 + len > sublocksize) {
			pr_err("Out of memory %s: %u\n", __func__, __LINE__);
			break;
		}
		if (len % 4) {
			pr_err("Burst len is wrong %s: %u\n", __func__,
				__LINE__);
			break;
		}

		p_tas256x->bulk_write(p_tas256x, chn,
			TAS256X_REG(
				data[subblk_offset],
				data[subblk_offset + 1],
				data[subblk_offset + 2]),
			&(data[subblk_offset + 4]), len);
		subblk_offset += (len + 4);
	}
	break;
	case TAS256X_CMD_DELAY: {
/*
 *		dev_idx		: one byte
 *		subblk_type : one byte
 *		delay_time	: two bytes
 */
		unsigned short delay_time = 0;

		if (subblk_offset + 2 > sublocksize) {
			pr_err("Out of memory %s: %u\n", __func__, __LINE__);
			break;
		}
		delay_time = SMS_HTONS(data[2], data[3]);
		usleep_range(delay_time*1000, delay_time*1000);
		subblk_offset += 2;
	}
	break;
	case TAS256X_CMD_FIELD_W:
/*
 *		dev_idx		: one byte
 *		subblk_type : one byte
 *		reserve		: one byte
 *		mask		: one byte
 *		book		: one byte
 *		page		: one byte
 *		reg		: one byte
 *		reserve		: one byte
 *		payload		: payload_len bytes
 */
	if (subblk_offset + 6 > sublocksize) {
		pr_err("Out of memory %s: %u\n", __func__, __LINE__);
		break;
	}
	p_tas256x->update_bits(p_tas256x, chn,
		TAS256X_REG(data[subblk_offset + 2], data[subblk_offset + 3],
		data[subblk_offset + 4]),
		data[subblk_offset + 1], data[subblk_offset + 5]);
	subblk_offset += 6;
	break;
	default:
	break;
	};

	return subblk_offset;
}

void tas256x_select_cfg_blk(void *pContext, int conf_no,
	unsigned char block_type)
{
	struct tas256x_priv *p_tas256x = (struct tas256x_priv *) pContext;
	struct tas256x_config_info **cfg_info = p_tas256x->cfg_info;
	int i = 0, j = 0, k = 0;

	if (conf_no > p_tas256x->ncfgs || conf_no < 0 || NULL == cfg_info) {
		pr_err("conf_no should be in range from 0 to %u\n",
			p_tas256x->ncfgs);
		goto EXIT;
	} else {
		pr_info("%s:%u:profile_conf_id = %d\n", __func__, __LINE__,
			conf_no);
	}
	conf_no--;
	if(conf_no < 0) goto EXIT;
	for (i = 0; i < p_tas256x->ncfgs; i++) {
		if (conf_no == i) {
			for (j = 0; j < (int)cfg_info[i]->real_nblocks; j++) {
				unsigned int length = 0, rc = 0;

				if (block_type > 5 || block_type < 2) {
					pr_err("ERROR!!!block_type should be in range from 2 to 5\n");
					goto EXIT;
				}
				if (block_type != cfg_info[i]->blk_data[j]->block_type)
					continue;
				pr_info("%s:%u:conf %d\n", __func__, __LINE__, i);
				pr_info("%s:%u:block type:%s\t device idx = 0x%02x\n",
					 __func__, __LINE__,
					 blocktype[cfg_info[i]->blk_data[j]->block_type - 1],
					cfg_info[i]->blk_data[j]->dev_idx);
				for (k = 0; k < (int)cfg_info[i]->blk_data[j]->nSublocks; k++) {
					rc = tas256x_process_block(p_tas256x,
						cfg_info[i]->blk_data[j]->regdata + length,
						cfg_info[i]->blk_data[j]->dev_idx,
						cfg_info[i]->blk_data[j]->block_size - length);
					length += rc;
					if (cfg_info[i]->blk_data[j]->block_size < length) {
						pr_err("%s:%u:ERROR:%u %u out of memory\n",
							__func__, __LINE__,
							length,
							cfg_info[i]->blk_data[j]->block_size);
						break;
					}
				}
				if (length != cfg_info[i]->blk_data[j]->block_size) {
					pr_err("%s:%u:ERROR: %u %u size is not same\n",
						__func__, __LINE__, length,
						cfg_info[i]->blk_data[j]->block_size);
				}
			}
		} else {
			continue;
		}
	}
EXIT:
	return;
}

static struct tas256x_config_info *tas256x_add_config(
	void *pContext, unsigned char *config_data, unsigned int config_size)
{
	struct tas256x_config_info *cfg_info = NULL;
	struct tas256x_priv *p_tas256x = (struct tas256x_priv *)pContext;
	int config_offset = 0, i = 0;

	cfg_info = kzalloc(
			sizeof(struct tas256x_config_info), GFP_KERNEL);
	if (!cfg_info) {
		pr_err("%s:%u:Memory alloc failed!\n", __func__, __LINE__);
		goto EXIT;
	}
	if (p_tas256x->fw_hdr.binary_version_num >= 0x105) {
        if (config_offset + 64 > (int)config_size) {
            pr_err("%s:%u:Out of memory\n", __func__, __LINE__);
            goto EXIT;
        }
        memcpy(cfg_info->mpName, &config_data[config_offset], 64);
        config_offset += 64;
    }
	if (config_offset + 4 > (int)config_size) {
		pr_err("%s:%u:Out of memory\n", __func__, __LINE__);
		goto EXIT;
	}
	cfg_info->nblocks =
		SMS_HTONL(config_data[config_offset],
		config_data[config_offset+1],
	config_data[config_offset+2], config_data[config_offset+3]);
	config_offset +=  4;
	pr_info("cfg_info->num_blocks = %u\n", cfg_info->nblocks);
	cfg_info->blk_data = kcalloc(
		cfg_info->nblocks, sizeof(struct tas256x_block_data *),
		GFP_KERNEL);
	if (!cfg_info->blk_data) {
		pr_err("%s:%u:Memory alloc failed!\n", __func__, __LINE__);
		goto EXIT;
	}
	cfg_info->real_nblocks = 0;
	for (i = 0; i < (int)cfg_info->nblocks; i++) {
		if (config_offset + 12 > config_size) {
			pr_err("%s:%u:Out of memory: i = %d nblocks = %u!\n",
				__func__, __LINE__, i, cfg_info->nblocks);
			break;
		}
		cfg_info->blk_data[i] = kzalloc(
			sizeof(struct tas256x_block_data), GFP_KERNEL);
		if (!cfg_info->blk_data[i]) {
			pr_err("%s:%u:Memory alloc failed!\n",
				__func__, __LINE__);
			break;
		}
		cfg_info->blk_data[i]->dev_idx = config_data[config_offset];
		config_offset++;
		pr_info("blk_data(%d).dev_idx = 0x%02x\n", i,
			cfg_info->blk_data[i]->dev_idx);
		cfg_info->blk_data[i]->block_type = config_data[config_offset];
		config_offset++;
		pr_info("blk_data(%d).block_type = 0x%02x\n", i,
			cfg_info->blk_data[i]->block_type);
		cfg_info->blk_data[i]->yram_checksum =
			SMS_HTONS(config_data[config_offset],
			config_data[config_offset+1]);
		config_offset += 2;
		cfg_info->blk_data[i]->block_size =
			SMS_HTONL(config_data[config_offset],
			config_data[config_offset + 1],
			config_data[config_offset + 2],
		config_data[config_offset + 3]);
		config_offset += 4;
		pr_info("blk_data(%d).block_size = %u\n", i,
		cfg_info->blk_data[i]->block_size);
		cfg_info->blk_data[i]->nSublocks =
			SMS_HTONL(config_data[config_offset],
			config_data[config_offset + 1],
			config_data[config_offset + 2],
		config_data[config_offset + 3]);
		pr_info("blk_data(%d).num_subblocks = %u\n", i,
		cfg_info->blk_data[i]->nSublocks);
		config_offset += 4;
		pr_info("config_offset = %d\n", config_offset);
		cfg_info->blk_data[i]->regdata = kzalloc(
			cfg_info->blk_data[i]->block_size, GFP_KERNEL);
		if (!cfg_info->blk_data[i]->regdata) {
			pr_err("%s:%u:Memory alloc failed!\n",
				__func__, __LINE__);
			goto EXIT;
		}
		if (config_offset + cfg_info->blk_data[i]->block_size > config_size) {
			pr_err("%s:%u:Out of memory: i = %d nblocks = %u!\n",
				__func__, __LINE__, i, cfg_info->nblocks);
			break;
		}
		memcpy(cfg_info->blk_data[i]->regdata,
			&config_data[config_offset],
		cfg_info->blk_data[i]->block_size);
		config_offset += cfg_info->blk_data[i]->block_size;
		cfg_info->real_nblocks += 1;
	}
EXIT:
	return cfg_info;
}

static int tas256x_info_profile(struct snd_kcontrol *kcontrol,
			 struct snd_ctl_elem_info *uinfo)
{
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec
					= snd_soc_kcontrol_component(kcontrol);
	struct tas256x_priv *p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct tas256x_priv *p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif

	uinfo->type = SNDRV_CTL_ELEM_TYPE_INTEGER;
	/* Codec Lock Hold*/
	mutex_lock(&p_tas256x->codec_lock);
	uinfo->count = 1;
	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);

	uinfo->value.integer.min = 0;
	uinfo->value.integer.max = max(0, p_tas256x->ncfgs);
	pr_info("%s: max profile = %d\n",
		__func__, (int)uinfo->value.integer.max);

	return 0;
}

static int tas256x_get_profile_id(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec
		= snd_soc_kcontrol_component(kcontrol);
	struct tas256x_priv *p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct tas256x_priv *p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif

	/* Codec Lock Hold*/
	mutex_lock(&p_tas256x->codec_lock);
	ucontrol->value.integer.value[0] = p_tas256x->profile_cfg_id;
	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);

	return 0;
}

static int tas256x_set_profile_id(struct snd_kcontrol *kcontrol,
		   struct snd_ctl_elem_value *ucontrol)
{
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec
	  = snd_soc_kcontrol_component(kcontrol);
	struct tas256x_priv *p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct tas256x_priv *p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif

	/* Codec Lock Hold*/
	mutex_lock(&p_tas256x->codec_lock);
	p_tas256x->profile_cfg_id = ucontrol->value.integer.value[0];
	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);

	return 0;
}

static int tas256x_create_controls(struct tas256x_priv *p_tas256x)
{
	int  nr_controls = 1, ret = 0, mix_index = 0;
	char *name = NULL;
	struct linux_platform *platform_data =
		(struct linux_platform *)p_tas256x->platform_data;
	struct snd_kcontrol_new *tas256x_profile_controls = NULL;

	tas256x_profile_controls = devm_kzalloc(platform_data->dev,
			nr_controls * sizeof(tas256x_profile_controls[0]),
			GFP_KERNEL);
	if (tas256x_profile_controls == NULL) {
		ret = -ENOMEM;
		goto EXIT;
	}

	/* Create a mixer item for selecting the active profile */
	name = devm_kzalloc(platform_data->dev, MAX_CONTROL_NAME, GFP_KERNEL);
	if (!name) {
		ret = -ENOMEM;
		goto EXIT;
	}
	scnprintf(name, MAX_CONTROL_NAME, "TAS256x Profile id");
	tas256x_profile_controls[mix_index].name = name;
	tas256x_profile_controls[mix_index].iface = SNDRV_CTL_ELEM_IFACE_MIXER;
	tas256x_profile_controls[mix_index].info = tas256x_info_profile;
	tas256x_profile_controls[mix_index].get = tas256x_get_profile_id;
	tas256x_profile_controls[mix_index].put = tas256x_set_profile_id;
	mix_index++;
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	ret = snd_soc_add_component_controls(platform_data->codec,
		tas256x_profile_controls,
		nr_controls < mix_index ? nr_controls : mix_index);
#else
	ret = snd_soc_add_codec_controls(platform_data->codec,
		tas256x_profile_controls,
		nr_controls < mix_index ? nr_controls : mix_index);
#endif
EXIT:
	return ret;
}

static void tas256x_fw_ready(const struct firmware *pFW, void *pContext)
{
	struct tas256x_priv *p_tas256x = (struct tas256x_priv *) pContext;
	struct tas256x_fw_hdr *fw_hdr = &(p_tas256x->fw_hdr);
	struct tas256x_config_info **cfg_info = NULL;
	unsigned char *buf = NULL;
	int offset = 0, i = 0, config_max_size = 0;
	unsigned int total_config_sz = 0;

	p_tas256x->fw_state = TAS256X_DSP_FW_FAIL;

	if (unlikely(!pFW) || unlikely(!pFW->data)) {
		pr_info("Failed to read %s, no side-effect on driver running\n",
			fw_name);
		return;
	}
	buf = (unsigned char *)pFW->data;
	/* Codec Lock Hold */
	mutex_lock(&p_tas256x->codec_lock);
	/* misc driver file lock hold */
	mutex_lock(&p_tas256x->file_lock);

	pr_info("%s: start\n", __func__);
	fw_hdr->img_sz = SMS_HTONL(buf[offset], buf[offset + 1],
		buf[offset + 2], buf[offset + 3]);
	offset += 4;
	if (fw_hdr->img_sz != pFW->size) {
		pr_err("File size not match, %d %u", (int)pFW->size,
			fw_hdr->img_sz);
		goto EXIT;
	}

	fw_hdr->checksum = SMS_HTONL(buf[offset], buf[offset + 1],
					buf[offset + 2], buf[offset + 3]);
	offset += 4;
	fw_hdr->binary_version_num = SMS_HTONL(buf[offset], buf[offset + 1],
					buf[offset + 2], buf[offset + 3]);
	offset += 4;
	fw_hdr->drv_fw_version = SMS_HTONL(buf[offset], buf[offset + 1],
					buf[offset + 2], buf[offset + 3]);
	offset += 4;
	fw_hdr->timestamp = SMS_HTONL(buf[offset], buf[offset + 1],
					buf[offset + 2], buf[offset + 3]);
	offset += 4;
	fw_hdr->plat_type = buf[offset];
	offset += 1;
	fw_hdr->dev_family = buf[offset];
	offset += 1;
	fw_hdr->reserve = buf[offset];
	offset += 1;
	fw_hdr->ndev = buf[offset];
	offset += 1;

	pr_info("ndev = %u\n", fw_hdr->ndev);

	if (offset + TAS256X_DEVICE_SUM > fw_hdr->img_sz) {
		pr_err("%s:%u:Out of Memory!\n", __func__, __LINE__);
		goto EXIT;
	}

	for (i = 0; i < TAS256X_DEVICE_SUM; i++) {
		fw_hdr->devs[i] = buf[offset];
		offset += 1;
		pr_info("devs[%d] = %u\n", i, fw_hdr->devs[i]);
	}
	fw_hdr->nconfig = SMS_HTONL(buf[offset], buf[offset + 1],
				buf[offset + 2], buf[offset + 3]);
	offset += 4;
	config_max_size = (fw_hdr->binary_version_num == 0x102) ? TAS256X_CONFIG_SIZEV102 : TAS256X_CONFIG_SIZE;
	pr_info("nconfig = %u\n", fw_hdr->nconfig);
	if (fw_hdr->nconfig > config_max_size) {
        pr_err("%s:%u:Out of Memory!\n", __func__, __LINE__);
        goto EXIT;
    }
	for (i = 0; i < config_max_size; i++) {
		fw_hdr->config_size[i] = SMS_HTONL(buf[offset], buf[offset + 1],
					buf[offset + 2], buf[offset + 3]);
		offset += 4;
		pr_info("config_size[%d] = %u\n", i, fw_hdr->config_size[i]);
		total_config_sz += fw_hdr->config_size[i];
	}
	pr_info("img_sz = %u total_config_sz = %u offset = %d\n",
	fw_hdr->img_sz, total_config_sz, offset);
	if (fw_hdr->img_sz - total_config_sz != (unsigned int)offset) {
		pr_err("Bin file error!\n");
		goto EXIT;
	}
	cfg_info = kcalloc(
		fw_hdr->nconfig, sizeof(struct tas256x_config_info *),
		GFP_KERNEL);
	if (!cfg_info) {
		pr_err("%s:%u:Memory alloc failed!\n", __func__, __LINE__);
		goto EXIT;
	}
	p_tas256x->cfg_info = cfg_info;
	p_tas256x->ncfgs = 0;
	for (i = 0; i < (int)fw_hdr->nconfig; i++) {
		cfg_info[i] = tas256x_add_config(p_tas256x, &buf[offset],
				fw_hdr->config_size[i]);
		if (!cfg_info[i]) {
			pr_err("%s:%u:Memory alloc failed!\n",
				__func__, __LINE__);
			break;
		}
		offset += (int)fw_hdr->config_size[i];
		p_tas256x->ncfgs += 1;
	}

	p_tas256x->fw_state = TAS256X_DSP_FW_OK;
	tas256x_create_controls(p_tas256x);
EXIT:
	/* Misc driver file lock release */
	mutex_unlock(&p_tas256x->file_lock);
	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);
	release_firmware(pFW);
	pr_info("%s: Firmware init complete\n", __func__);
}

int tas256x_load_container(struct tas256x_priv *p_tas256x)
{
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	p_tas256x->fw_state = TAS256X_DSP_FW_PENDING;
	return request_firmware_nowait(THIS_MODULE, FW_ACTION_HOTPLUG,
		fw_name, plat_data->dev, GFP_KERNEL,
		p_tas256x, tas256x_fw_ready);
}

void tas256x_config_info_remove(void *pContext)
{
	struct tas256x_priv *p_tas256x = (struct tas256x_priv *) pContext;
	struct tas256x_config_info **cfg_info = p_tas256x->cfg_info;
	int i = 0, j = 0;

	if (cfg_info) {
		for (i = 0; i < p_tas256x->ncfgs; i++) {
			if (cfg_info[i]) {
				for (j = 0; j < (int)cfg_info[i]->real_nblocks; j++) {
					if (cfg_info[i]->blk_data[j]->regdata)
						kfree(cfg_info[i]->blk_data[j]->regdata);
					if (cfg_info[i]->blk_data[j])
						kfree(cfg_info[i]->blk_data[j]);
				}
				if (cfg_info[i]->blk_data)
					kfree(cfg_info[i]->blk_data);
				kfree(cfg_info[i]);
			}
		}
		kfree(cfg_info);
	}
}

static char const *tas2564_rx_mode_text[] = {"Speaker", "Receiver"};

static const struct soc_enum tas2564_rx_mode_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(tas2564_rx_mode_text),
		tas2564_rx_mode_text),
};

static int tas2564_put(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec = snd_soc_kcontrol_component(kcontrol);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
#endif
	struct tas256x_priv *p_tas256x = NULL;
	struct linux_platform *plat_data = NULL;
	int ret = -1;

	if (codec == NULL) {
		pr_err("%s:codec is NULL\n", __func__);
		return ret;
	}

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif
	if (p_tas256x == NULL) {
		pr_err("%s:p_tas256x is NULL\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	if (strnstr(ucontrol->id.name, "LEFT", MAX_STRING))
		ret = tas2564_rx_mode_update(p_tas256x,
			ucontrol->value.integer.value[0], channel_left);
	else if (strnstr(ucontrol->id.name, "RIGHT", MAX_STRING))
		ret = tas2564_rx_mode_update(p_tas256x,
			ucontrol->value.integer.value[0], channel_right);
	else
		dev_err(plat_data->dev, "Invalid Channel %s\n",
			ucontrol->id.name);

	return ret;
}

static int tas2564_get(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	int ret = -1;
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec = snd_soc_kcontrol_component(kcontrol);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
#endif
	struct linux_platform *plat_data = NULL;
	struct tas256x_priv *p_tas256x = NULL;

	if (codec == NULL) {
		pr_err("%s:codec is NULL\n", __func__);
		return ret;
	}

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif
	if (p_tas256x == NULL) {
		pr_err("%s:p_tas256x is NULL\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	if (strnstr(ucontrol->id.name, "LEFT", MAX_STRING))
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[0]->rx_mode;
	else if (strnstr(ucontrol->id.name, "RIGHT", MAX_STRING))
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[1]->rx_mode;
	else
		dev_err(plat_data->dev, "Invalid Channel %s\n",
			ucontrol->id.name);

	return 0;
}

static int tas256x_put(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec = snd_soc_kcontrol_component(kcontrol);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
#endif
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct tas256x_priv *p_tas256x = NULL;
	struct linux_platform *plat_data = NULL;
	int ret = -1;

	if ((codec == NULL) || (mc == NULL)) {
		pr_err("%s:codec or control is NULL\n", __func__);
		return ret;
	}

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif
	if (p_tas256x == NULL) {
		pr_err("%s:p_tas256x is NULL\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	if (ucontrol->value.integer.value[0] > mc->max)
		return ret;

	switch (mc->reg) {
	case DVC_PCM:
		ret = tas256x_update_playback_volume(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIM_MAX_ATN:
		ret = tas256x_update_lim_max_attenuation(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_INF_PT:
		ret = tas256x_update_lim_inflection_point(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_SLOPE:
		ret = tas256x_update_lim_slope(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_ATK_RT:
		ret = tas256x_update_limiter_attack_rate(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_RLS_RT:
		ret = tas256x_update_limiter_release_rate(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_RLS_ST:
		ret = tas256x_update_limiter_release_step_size(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_ATK_ST:
		ret = tas256x_update_limiter_attack_step_size(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case BOP_ATK_RT:
		ret = tas256x_update_bop_attack_rate(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case BOP_ATK_ST:
		ret = tas256x_update_bop_attack_step_size(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case BOP_HLD_TM:
		ret = tas256x_update_bop_hold_time(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case BST_VREG:
		ret = tas256x_update_boost_voltage(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case BST_ILIM:
		ret = tas256x_update_current_limit(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_TH_MAX:
		ret = tas256x_update_lim_max_thr(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case LIMB_TH_MIN:
		ret = tas256x_update_lim_min_thr(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case BOP_TH:
		ret = tas256x_update_bop_thr(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case BOSD_TH:
		ret = tas256x_update_bosd_thr(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case CLASSH_TIMER:
		ret = tas256x_update_classh_timer(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case AMPOUTPUT_LVL:
		ret = tas256x_update_ampoutput_level(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case ICN_THR:
		ret = tas256x_update_icn_threshold(p_tas256x,
			ucontrol->value.integer.value[0], mc->shift);
	break;
	case ICN_HYST:
		ret = tas256x_update_icn_hysterisis(p_tas256x,
			ucontrol->value.integer.value[0],
			p_tas256x->mn_sampling_rate, mc->shift);
	break;
	case PWR_UP_DELAY:
		p_tas256x->devs[mc->shift-1]->pwrup_delay =
			ucontrol->value.integer.value[0];
		/* Its just an assignment */
		ret = 0;
	break;
	}
	return ret;
}

static int tas256x_get(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec = snd_soc_kcontrol_component(kcontrol);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
#endif
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct tas256x_priv *p_tas256x = NULL;
	struct linux_platform *plat_data = NULL;
	int ret = -1;

	if ((codec == NULL) || (mc == NULL)) {
		pr_err("%s:codec or control is NULL\n", __func__);
		return ret;
	}

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif
	if (p_tas256x == NULL) {
		pr_err("%s:p_tas256x is NULL\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	switch (mc->reg) {
	case DVC_PCM:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->dvc_pcm;
	break;
	case LIM_MAX_ATN:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_max_attn;
	break;
	case LIMB_INF_PT:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_infl_pt;
	break;
	case LIMB_SLOPE:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_trk_slp;
	break;
	case LIMB_ATK_RT:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_att_rate;
	break;
	case LIMB_RLS_RT:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_rel_rate;
	break;
	case LIMB_RLS_ST:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_rel_stp_size;
	break;
	case LIMB_ATK_ST:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_att_stp_size;
	break;
	case BOP_ATK_RT:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->bop_att_rate;
	break;
	case BOP_ATK_ST:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->bop_att_stp_size;
	break;
	case BOP_HLD_TM:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->bop_hld_time;
	break;
	case BST_VREG:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->bst_vltg;
	break;
	case BST_ILIM:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->bst_ilm;
	break;
	case LIMB_TH_MAX:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_thr_max;
	break;
	case LIMB_TH_MIN:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->lim_thr_min;
	break;
	case BOP_TH:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->bop_thd;
	break;
	case BOSD_TH:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->bosd_thd;
	break;
	case CLASSH_TIMER:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->classh_timer;
	break;
	case AMPOUTPUT_LVL:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->ampoutput_lvl;
	break;
	case ICN_THR:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->icn_thr;
	break;
	case ICN_HYST:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->icn_hyst;
	break;
	case PWR_UP_DELAY:
		ucontrol->value.integer.value[0] =
			p_tas256x->devs[mc->shift-1]->pwrup_delay;
	break;
	}
	return 0;
}

static int tas256x_enum_get(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
	int ret = -1;
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec = snd_soc_kcontrol_component(kcontrol);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
#endif
	struct tas256x_priv *p_tas256x = NULL;
	struct linux_platform *plat_data = NULL;

	if (codec == NULL) {
		pr_err("%s:codec is NULL\n", __func__);
		return ret;
	}

	if (ucontrol == NULL) {
		pr_err("%s:ucontrol is NULL\n", __func__);
		return ret;
	}

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif
	if (p_tas256x == NULL) {
		pr_err("%s:p_tas256x is NULL\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	if (strnstr(ucontrol->id.name, "Version", MAX_STRING)) {
		ucontrol->value.integer.value[0] = 0;
	} else if (strnstr(ucontrol->id.name, "LEFT", MAX_STRING)) {
		if (strnstr(ucontrol->id.name, "LIMITER SWITCH",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[0]->lim_switch;
		else if (strnstr(ucontrol->id.name, "BOP ENABLE",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[0]->bop_enable;
		else if (strnstr(ucontrol->id.name, "BOP MUTE",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[0]->bop_mute;
		else if (strnstr(ucontrol->id.name, "BROWNOUT SHUTDOWN",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[0]->bosd_enable;
		else if (strnstr(ucontrol->id.name, "VBAT LPF",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[0]->vbat_lpf;
		else if (strnstr(ucontrol->id.name, "RECIEVER ENABLE",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[0]->reciever_enable;
		else if (strnstr(ucontrol->id.name, "ICN SWITCH",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[0]->icn_sw;
		else
			dev_err(plat_data->dev, "Invalid controll %s\n",
				ucontrol->id.name);
	} else if (strnstr(ucontrol->id.name, "RIGHT", MAX_STRING)) {
		if (strnstr(ucontrol->id.name, "LIMITER SWITCH",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[1]->lim_switch;
		else if (strnstr(ucontrol->id.name, "BOP ENABLE",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[1]->bop_enable;
		else if (strnstr(ucontrol->id.name, "BOP MUTE",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[1]->bop_mute;
		else if (strnstr(ucontrol->id.name, "BROWNOUT SHUTDOWN",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[1]->bosd_enable;
		else if (strnstr(ucontrol->id.name, "VBAT LPF",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[1]->vbat_lpf;
		else if (strnstr(ucontrol->id.name, "RECIEVER ENABLE",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[1]->reciever_enable;
		else if (strnstr(ucontrol->id.name, "ICN SWITCH",
			MAX_STRING))
			ucontrol->value.integer.value[0] =
				p_tas256x->devs[1]->icn_sw;
		else
			dev_err(plat_data->dev, "Invalid controll %s\n",
				ucontrol->id.name);
	} else {
		dev_err(plat_data->dev, "Invalid Channel %s\n",
			ucontrol->id.name);
	}

	return 0;
}

static int tas256x_enum_put(struct snd_kcontrol *kcontrol,
				  struct snd_ctl_elem_value *ucontrol)
{
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	struct snd_soc_component *codec = snd_soc_kcontrol_component(kcontrol);
#else
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
#endif
	struct soc_mixer_control *mc =
		(struct soc_mixer_control *)kcontrol->private_value;
	struct tas256x_priv *p_tas256x = NULL;
	struct linux_platform *plat_data = NULL;
	int ret = -1;

	if ((codec == NULL) || (mc == NULL)) {
		pr_err("%s:codec or control is NULL\n", __func__);
		return ret;
	}

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	p_tas256x = snd_soc_component_get_drvdata(codec);
#else
	p_tas256x = snd_soc_codec_get_drvdata(codec);
#endif
	if (p_tas256x == NULL) {
		pr_err("%s:p_tas256x is NULL\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	if (strnstr(ucontrol->id.name, "LEFT", MAX_STRING)) {
		if (strnstr(ucontrol->id.name, "LIMITER SWITCH",
			MAX_STRING))
			ret = tas256x_update_limiter_enable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_left);
		else if (strnstr(ucontrol->id.name, "BOP ENABLE",
			MAX_STRING))
			ret = tas256x_update_bop_enable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_left);
		else if (strnstr(ucontrol->id.name, "BOP MUTE",
			MAX_STRING))
			ret = tas256x_update_bop_mute(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_left);
		else if (strnstr(ucontrol->id.name, "BROWNOUT SHUTDOWN",
			MAX_STRING))
			ret = tas256x_update_bop_shutdown_enable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_left);
		else if (strnstr(ucontrol->id.name, "VBAT LPF",
			MAX_STRING))
			ret = tas256x_update_vbat_lpf(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_left);
		else if (strnstr(ucontrol->id.name, "RECIEVER ENABLE",
			MAX_STRING))
			ret = tas256x_enable_reciever_mode(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_left);
		else if (strnstr(ucontrol->id.name, "ICN SWITCH",
			MAX_STRING))
			ret = tas256x_icn_disable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_left);
		else
			dev_err(plat_data->dev, "Invalid Control %s\n",
				ucontrol->id.name);
	} else if (strnstr(ucontrol->id.name, "RIGHT", MAX_STRING)) {
		if (strnstr(ucontrol->id.name, "LIMITER SWITCH",
			MAX_STRING))
			ret = tas256x_update_limiter_enable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_right);
		else if (strnstr(ucontrol->id.name, "BOP ENABLE",
				MAX_STRING))
			ret = tas256x_update_bop_enable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_right);
		else if (strnstr(ucontrol->id.name, "BOP MUTE",
				MAX_STRING))
			ret = tas256x_update_bop_mute(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_right);
		else if (strnstr(ucontrol->id.name, "BROWNOUT SHUTDOWN",
				MAX_STRING))
			ret = tas256x_update_bop_shutdown_enable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_right);
		else if (strnstr(ucontrol->id.name, "VBAT LPF",
				MAX_STRING))
			ret = tas256x_update_vbat_lpf(p_tas256x,
					ucontrol->value.integer.value[0],
					channel_right);
		else if (strnstr(ucontrol->id.name, "RECIEVER ENABLE",
			MAX_STRING))
			ret = tas256x_enable_reciever_mode(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_right);
		else if (strnstr(ucontrol->id.name, "ICN SWITCH",
			MAX_STRING))
			ret = tas256x_icn_disable(p_tas256x,
				ucontrol->value.integer.value[0],
				channel_right);
		else
			dev_err(plat_data->dev, "Invalid control %s\n",
				ucontrol->id.name);
	} else {
		dev_err(plat_data->dev, "Invalid Channel %s\n",
			ucontrol->id.name);
	}

	return ret;
}

static char const *tas256x_switch_text[] = {"DISABLE", "ENABLE"};
static const struct soc_enum tas256x_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(tas256x_switch_text),
		tas256x_switch_text),
};

static char const *tas256x_version_text[] = {TAS256X_DRIVER_TAG};
static const struct soc_enum tas256x_version_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(tas256x_version_text),
		tas256x_version_text),
};

static const struct snd_kcontrol_new tas256x_left_controls[] = {
	SOC_ENUM_EXT("TAS256X Version", tas256x_version_enum[0],
		tas256x_enum_get, NULL),
	SOC_SINGLE_EXT("TAS256X PLAYBACK VOLUME LEFT", DVC_PCM, 1, 56, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM MAX ATTN LEFT", LIM_MAX_ATN, 1, 15, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM THR MAX LEFT", LIMB_TH_MAX,
		1, 26, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM THR MIN LEFT", LIMB_TH_MIN,
		1, 26, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM INFLECTION POINT LEFT", LIMB_INF_PT,
		1, 40, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM SLOPE LEFT", LIMB_SLOPE, 1, 6, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP THR LEFT", BOP_TH,
		1, 15, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOSD THR LEFT", BOSD_TH,
		1, 15, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM ATTACT RATE LEFT", LIMB_ATK_RT, 1, 7, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM RELEASE RATE LEFT", LIMB_RLS_RT, 1, 7, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM ATTACK STEP LEFT", LIMB_ATK_ST, 1, 3, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM RELEASE STEP LEFT", LIMB_RLS_ST, 1, 3, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP ATTACK RATE LEFT", BOP_ATK_RT, 1, 7, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP ATTACK STEP LEFT", BOP_ATK_ST, 1, 3, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP HOLD TIME LEFT", BOP_HLD_TM, 1, 7, 0,
		tas256x_get, tas256x_put),
	SOC_ENUM_EXT("TAS256X LIMITER SWITCH LEFT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X BOP ENABLE LEFT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X BOP MUTE LEFT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X BROWNOUT SHUTDOWN LEFT",
		tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X CLASSH TIMER LEFT", CLASSH_TIMER, 1, 22, 0,
		tas256x_get, tas256x_put),
	SOC_ENUM_EXT("TAS256X RECIEVER ENABLE LEFT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X ICN SWITCH LEFT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X ICN THRESHOLD LEFT", ICN_THR, 1, 84, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X ICN HYSTERISIS LEFT", ICN_HYST, 1, 19, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X SET POWERUP DELAY LEFT", PWR_UP_DELAY, 1, 10000, 0,
		tas256x_get, tas256x_put),
};

static const struct snd_kcontrol_new tas256x_right_controls[] = {
	SOC_SINGLE_EXT("TAS256X PLAYBACK VOLUME RIGHT", DVC_PCM, 2, 56, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM MAX ATTN RIGHT", LIM_MAX_ATN, 2, 15, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM THR MAX RIGHT", LIMB_TH_MAX,
		2, 26, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM THR MIN RIGHT", LIMB_TH_MIN,
		2, 26, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM INFLECTION POINT RIGHT", LIMB_INF_PT,
		2, 40, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM SLOPE RIGHT", LIMB_SLOPE, 2, 6, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP THR RIGHT", BOP_TH,
		2, 15, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOSD THR RIGHT", BOSD_TH,
		2, 15, 0, tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM ATTACT RATE RIGHT", LIMB_ATK_RT, 2, 7, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM RELEASE RATE RIGHT", LIMB_RLS_RT, 2, 7, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM ATTACK STEP RIGHT", LIMB_ATK_ST, 2, 3, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X LIM RELEASE STEP RIGHT", LIMB_RLS_ST, 2, 3, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP ATTACK RATE RIGHT", BOP_ATK_RT, 2, 7, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP ATTACK STEP RIGHT", BOP_ATK_ST, 2, 3, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOP HOLD TIME RIGHT", BOP_HLD_TM, 2, 7, 0,
		tas256x_get, tas256x_put),
	SOC_ENUM_EXT("TAS256X LIMITER SWITCH RIGHT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X BOP ENABLE RIGHT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X BOP MUTE RIGHT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X BROWNOUT SHUTDOWN RIGHT",
		tas256x_switch_enum[0], tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X CLASSH TIMER RIGHT", CLASSH_TIMER, 2, 22, 0,
		tas256x_get, tas256x_put),
	SOC_ENUM_EXT("TAS256X RECIEVER ENABLE RIGHT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_ENUM_EXT("TAS256X ICN SWITCH RIGHT", tas256x_switch_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X ICN THRESHOLD RIGHT", ICN_THR, 2, 84, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X ICN HYSTERISIS RIGHT", ICN_HYST, 2, 19, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X SET POWERUP DELAY RIGHT", PWR_UP_DELAY, 2, 10000, 0,
		tas256x_get, tas256x_put),
};

static char const *tas2564_vbat_lpf_text[] = {
	"DISABLE", "HZ_10", "HZ_100", "KHZ_1"};
static const struct soc_enum tas2564_vbat_lpf_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(tas2564_vbat_lpf_text),
		tas2564_vbat_lpf_text),
};

static char const *tas2562_vbat_lpf_text[] = {
	"DISABLE", "HZ_100", "KHZ_1", "KHZ_10"};
static const struct soc_enum tas2562_vbat_lpf_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(tas2562_vbat_lpf_text),
		tas2562_vbat_lpf_text),
};

static const struct snd_kcontrol_new tas2564_left_controls[] = {
	SOC_ENUM_EXT("TAS256X RX MODE LEFT", tas2564_rx_mode_enum[0],
		tas2564_get, tas2564_put),
	SOC_ENUM_EXT("TAS256X VBAT LPF LEFT", tas2564_vbat_lpf_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X BOOST VOLTAGE LEFT", BST_VREG, 1, 15, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOOST CURRENT LEFT", BST_ILIM, 1, 63, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X AMP OUTPUT LVL LEFT", AMPOUTPUT_LVL, 1, 0x1C, 0,
		tas256x_get, tas256x_put),
};

static const struct snd_kcontrol_new tas2564_right_controls[] = {
	SOC_ENUM_EXT("TAS256X RX MODE RIGHT", tas2564_rx_mode_enum[0],
		tas2564_get, tas2564_put),
	SOC_ENUM_EXT("TAS256X VBAT LPF RIGHT", tas2564_vbat_lpf_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X BOOST VOLTAGE RIGHT", BST_VREG, 2, 15, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOOST CURRENT RIGHT", BST_ILIM, 2, 63, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X AMP OUTPUT LVL RIGHT", AMPOUTPUT_LVL, 2,
		0x1C, 0,
		tas256x_get, tas256x_put),
};

static const struct snd_kcontrol_new tas2562_left_controls[] = {
	SOC_ENUM_EXT("TAS256X VBAT LPF LEFT", tas2562_vbat_lpf_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X BOOST VOLTAGE LEFT", BST_VREG, 1, 12, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOOST CURRENT LEFT", BST_ILIM, 1, 55, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X AMP OUTPUT LVL LEFT", AMPOUTPUT_LVL, 1,
		0x1C, 0,
		tas256x_get, tas256x_put),
};

static const struct snd_kcontrol_new tas2562_right_controls[] = {
	SOC_ENUM_EXT("TAS256X VBAT LPF RIGHT", tas2562_vbat_lpf_enum[0],
		tas256x_enum_get, tas256x_enum_put),
	SOC_SINGLE_EXT("TAS256X BOOST VOLTAGE RIGHT", BST_VREG, 2, 12, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X BOOST CURRENT RIGHT", BST_ILIM, 2, 55, 0,
		tas256x_get, tas256x_put),
	SOC_SINGLE_EXT("TAS256X AMP OUTPUT LVL RIGHT", AMPOUTPUT_LVL, 2,
		0x1C, 0,
		tas256x_get, tas256x_put),
};

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
static int tas2564_probe(struct tas256x_priv *p_tas256x,
	struct snd_soc_component *codec, int chn)
{
	int ret = -1;
	struct linux_platform *plat_data = NULL;

	if ((!p_tas256x) || (!codec)) {
		pr_err("tas256x:%s p_tas256x or codec is Null\n", __func__);
		return ret;
	}

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_dbg(plat_data->dev, "%s channel %d", __func__, chn);

	tas256x_update_default_params(p_tas256x, chn);
	if (chn == channel_left) {
		ret = snd_soc_add_component_controls(codec,
			tas256x_left_controls,
			ARRAY_SIZE(tas256x_left_controls));
		ret = snd_soc_add_component_controls(codec,
			tas2564_left_controls,
			ARRAY_SIZE(tas2564_left_controls));
	} else if (chn == channel_right) {
		ret = snd_soc_add_component_controls(codec,
			tas256x_right_controls,
			ARRAY_SIZE(tas256x_right_controls));
		ret = snd_soc_add_component_controls(codec,
			tas2564_right_controls,
			ARRAY_SIZE(tas2564_right_controls));
	} else {
		dev_err(plat_data->dev, "Invalid Channel %d\n", chn);
	}

	return ret;
}
#else
static int tas2564_probe(struct tas256x_priv *p_tas256x,
	struct snd_soc_codec *codec, int chn)
{
	int ret = -1;
	struct linux_platform *plat_data = NULL;

	if ((!p_tas256x) || (!codec)) {
		pr_err("tas256x:%s p_tas256x or codec is Null\n", __func__);
		return ret;
	}

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_dbg(plat_data->dev, "%s channel %d", __func__, chn);

	tas256x_update_default_params(p_tas256x, chn);
	if (chn == channel_left) {
		ret = snd_soc_add_codec_controls(codec, tas256x_left_controls,
			ARRAY_SIZE(tas256x_left_controls));
		ret = snd_soc_add_codec_controls(codec, tas2564_left_controls,
			ARRAY_SIZE(tas2564_left_controls));
	} else if (chn == channel_right) {
		ret = snd_soc_add_codec_controls(codec, tas256x_right_controls,
			ARRAY_SIZE(tas256x_right_controls));
		ret = snd_soc_add_codec_controls(codec, tas2564_right_controls,
			ARRAY_SIZE(tas2564_right_controls));
	} else {
		dev_err(plat_data->dev, "Invalid Channel %d\n", chn);
	}

	return ret;
}
#endif

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
static int tas2562_probe(struct tas256x_priv *p_tas256x,
	struct snd_soc_component *codec, int chn)
{
	struct linux_platform *plat_data = NULL;
	int ret = -1;

	if ((!p_tas256x) || (!codec)) {
		pr_err("tas256x:%s p_tas256x or codec is Null\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_dbg(plat_data->dev, "%s channel %d", __func__, chn);

	tas256x_update_default_params(p_tas256x, chn);
	if (chn == channel_left) {
		ret = snd_soc_add_component_controls(codec,
			tas256x_left_controls,
			ARRAY_SIZE(tas256x_left_controls));
		ret |= snd_soc_add_component_controls(codec,
			tas2562_left_controls,
			ARRAY_SIZE(tas2562_left_controls));
	} else if (chn == channel_right) {
		ret = snd_soc_add_component_controls(codec,
			tas256x_right_controls,
			ARRAY_SIZE(tas256x_right_controls));
		ret |= snd_soc_add_component_controls(codec,
			tas2562_right_controls,
			ARRAY_SIZE(tas2562_right_controls));
	} else {
		dev_err(plat_data->dev, "Invalid Channel %d\n", chn);
	}

	return ret;
}
#else
static int tas2562_probe(struct tas256x_priv *p_tas256x,
	struct snd_soc_codec *codec, int chn)
{
	struct linux_platform *plat_data = NULL;
	int ret = -1;

	if ((!p_tas256x) || (!codec)) {
		pr_err("tas256x:%s p_tas256x or codec is Null\n", __func__);
		return ret;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_dbg(plat_data->dev, "%s channel %d", __func__, chn);

	tas256x_update_default_params(p_tas256x, chn);
	if (chn == channel_left) {
		ret = snd_soc_add_codec_controls(codec, tas256x_left_controls,
			ARRAY_SIZE(tas256x_left_controls));
		ret |= snd_soc_add_codec_controls(codec, tas2562_left_controls,
			ARRAY_SIZE(tas2562_left_controls));
	} else if (chn == channel_right) {
		ret = snd_soc_add_codec_controls(codec, tas256x_right_controls,
			ARRAY_SIZE(tas256x_right_controls));
		ret |= snd_soc_add_codec_controls(codec, tas2562_right_controls,
			ARRAY_SIZE(tas2562_right_controls));
	} else {
		dev_err(plat_data->dev, "Invalid Channel %d\n", chn);
	}

	return ret;
}
#endif

static bool tas256x_volatile(struct device *dev, unsigned int reg)
{
	return true;
}

static bool tas256x_writeable(struct device *dev, unsigned int reg)
{
	return true;
}
static const struct regmap_config tas256x_i2c_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.writeable_reg = tas256x_writeable,
	.volatile_reg = tas256x_volatile,
	.cache_type = REGCACHE_NONE,
	.max_register = 1 * 128,
};

static void tas256x_hw_reset(struct tas256x_priv *p_tas256x)
{
	struct linux_platform *plat_data = NULL;
	int i = 0;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (gpio_is_valid(p_tas256x->devs[i]->mn_reset_gpio)) {
			gpio_direction_output(
				p_tas256x->devs[i]->mn_reset_gpio, 0);
		}
	}
	msleep(20);

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (gpio_is_valid(p_tas256x->devs[i]->mn_reset_gpio)) {
			gpio_direction_output(
				p_tas256x->devs[i]->mn_reset_gpio, 1);
		}
		p_tas256x->devs[i]->mn_current_book = -1;
		p_tas256x->devs[i]->mn_current_page = -1;
	}
	msleep(20);

	dev_info(plat_data->dev, "reset gpio up !!\n");
}

void tas256x_enable_irq(struct tas256x_priv *p_tas256x, bool enable)
{
	static int irq_enabled[2] = {0};
	struct irq_desc *desc = NULL;
	struct linux_platform *plat_data = NULL;
	int i = 0;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	if (enable) {
		if (p_tas256x->mb_irq_eable)
			return;
		for (i = 0; i < p_tas256x->mn_channels; i++) {
			if (gpio_is_valid(p_tas256x->devs[i]->mn_irq_gpio) &&
				irq_enabled[i] == 0) {
				if (i == 0) {
					desc = irq_to_desc(
						p_tas256x->devs[i]->mn_irq);
					if (desc && desc->depth > 0)
						enable_irq(
							p_tas256x->devs[i]->mn_irq);
					else
						dev_info(plat_data->dev,
							"### irq already enabled");
				} else {
					enable_irq(p_tas256x->devs[i]->mn_irq);
				}
				irq_enabled[i] = 1;
			}
		}
		p_tas256x->mb_irq_eable = true;
	} else {
		for (i = 0; i < p_tas256x->mn_channels; i++) {
			if (gpio_is_valid(p_tas256x->devs[i]->mn_irq_gpio)
					&& irq_enabled[i] == 1) {
				disable_irq_nosync(p_tas256x->devs[i]->mn_irq);
				irq_enabled[i] = 0;
			}
		}
		p_tas256x->mb_irq_eable = false;
	}
}

static void irq_work_routine(struct work_struct *work)
{
	struct tas256x_priv *p_tas256x =
		container_of(work, struct tas256x_priv, irq_work.work);
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_info(plat_data->dev, "%s\n", __func__);

	/* Codec Lock Hold*/
	mutex_lock(&p_tas256x->codec_lock);
	if (plat_data->mb_runtime_suspend) {
		pr_info("%s, Runtime Suspended\n", __func__);
		goto end;
	}
	/*Logical Layer IRQ function, return is ignored*/
	tas256x_irq_work_func(p_tas256x);

	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);
end:
	return;
}

static void init_work_routine(struct work_struct *work)
{
	struct tas_device *dev_tas256x =
		container_of(work, struct tas_device, init_work.work);

	/*Init Work Function. return is ignored*/
	tas256x_init_work_func(dev_tas256x->prv_data, dev_tas256x);
}

static irqreturn_t tas256x_irq_handler(int irq, void *dev_id)
{
	struct tas256x_priv *p_tas256x = (struct tas256x_priv *)dev_id;
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	/* get IRQ status after 100 ms */
	schedule_delayed_work(&p_tas256x->irq_work, msecs_to_jiffies(100));
	return IRQ_HANDLED;
}

static int tas256x_runtime_suspend(struct tas256x_priv *p_tas256x)
{
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_dbg(plat_data->dev, "%s\n", __func__);

	plat_data->mb_runtime_suspend = true;

	if (delayed_work_pending(&p_tas256x->irq_work)) {
		dev_dbg(plat_data->dev, "cancel IRQ work\n");
		cancel_delayed_work_sync(&p_tas256x->irq_work);
	}

	return 0;
}

static int tas256x_runtime_resume(struct tas256x_priv *p_tas256x)
{
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_dbg(plat_data->dev, "%s\n", __func__);

	plat_data->mb_runtime_suspend = false;

	return 0;
}

static int tas256x_pm_suspend(struct device *dev)
{
	struct tas256x_priv *p_tas256x = dev_get_drvdata(dev);
	struct linux_platform *plat_data = NULL;

	if (!p_tas256x) {
		pr_err("%s drvdata is NULL\n", __func__);
		return 0;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	/* Codec Lock Hold*/
	mutex_lock(&p_tas256x->codec_lock);
	plat_data->i2c_suspend = true;
	tas256x_runtime_suspend(p_tas256x);
	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);
	return 0;
}

static int tas256x_pm_resume(struct device *dev)
{
	struct tas256x_priv *p_tas256x = dev_get_drvdata(dev);
	struct linux_platform *plat_data = NULL;

	if (!p_tas256x) {
		pr_err("%s drvdata is NULL\n", __func__);
		return 0;
	}
	plat_data = (struct linux_platform *) p_tas256x->platform_data;

	/* Codec Lock Hold*/
	mutex_lock(&p_tas256x->codec_lock);
	plat_data->i2c_suspend = false;
	tas256x_runtime_resume(p_tas256x);
	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);
	return 0;
}

#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
#if IS_ENABLED(CONFIG_PLATFORM_QCOM)
struct tas256x_priv *g_p_tas256x;

void tas256x_software_reset(void *prv_data)
{
	pr_info("[TI-SmartPA:%s]\n", __func__);
	schedule_delayed_work(&g_p_tas256x->dc_work, msecs_to_jiffies(10));
}

static void dc_work_routine(struct work_struct *work)
{
	struct tas256x_priv *p_tas256x =
		container_of(work, struct tas256x_priv, dc_work.work);
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	/* Codec Lock Hold*/
	mutex_lock(&p_tas256x->codec_lock);
	tas256x_dc_work_func(p_tas256x, s_dc_detect.channel);

	/* Codec Lock Release*/
	mutex_unlock(&p_tas256x->codec_lock);

}
#endif/*CONFIG_PLATFORM_QCOM*/
#endif /*CONFIG_TAS25XX_ALGO*/

void schedule_init_work(struct tas256x_priv *p_tas256x, int ch)
{
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	schedule_delayed_work(&p_tas256x->devs[ch-1]->init_work,
		msecs_to_jiffies(200));
}

void cancel_init_work(struct tas256x_priv *p_tas256x, int ch)
{
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	cancel_delayed_work_sync(&p_tas256x->devs[ch-1]->init_work);

}

static void pwrup_work_routine(struct work_struct *work)
{
	struct tas_device *dev_tas256x =
		container_of(work, struct tas_device, pwrup_work.work);

	/*Power Up Work Function. return is ignored*/
	tas256x_pwrup_work_func(dev_tas256x->prv_data, dev_tas256x);
}

void schedule_pwrup_work(struct tas256x_priv *p_tas256x, int ch)
{
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	schedule_delayed_work(&p_tas256x->devs[ch-1]->pwrup_work,
		msecs_to_jiffies(p_tas256x->devs[ch-1]->pwrup_delay));
}

bool cancel_pwrup_work(struct tas256x_priv *p_tas256x, int ch)
{
	struct linux_platform *plat_data = NULL;
	bool ret = 0;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	ret = cancel_delayed_work(&p_tas256x->devs[ch-1]->pwrup_work);
	p_tas256x->devs[ch-1]->pwrup_delay = 0;

	return ret;
}

static int tas256x_parse_dt(struct device *dev,
					struct tas256x_priv *p_tas256x)
{
	struct device_node *np = dev->of_node;
	int rc = 0, i = 0;
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	rc = of_property_read_u32(np, "ti,channels", &p_tas256x->mn_channels);
	if (rc) {
		dev_err(plat_data->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,channels", np->full_name, rc);
		goto EXIT;
	} else {
		dev_dbg(plat_data->dev, "ti,channels=%d",
			p_tas256x->mn_channels);
	}

	/*the device structures array*/
	p_tas256x->devs =
		kmalloc(p_tas256x->mn_channels * sizeof(struct tas_device *),
			GFP_KERNEL);
	for (i = 0; i < p_tas256x->mn_channels; i++) {
		p_tas256x->devs[i] = kmalloc(sizeof(struct tas_device),
					GFP_KERNEL);
		if (p_tas256x->devs[i] == NULL) {
			dev_err(plat_data->dev,
			"%s:%u:kmalloc failed!\n", __func__, __LINE__);
			rc = -1;
			break;
		}

		rc = of_property_read_u32(np, dts_tag[i][0],
			&p_tas256x->devs[i]->mn_addr);
		if (rc) {
			dev_err(plat_data->dev,
				"Looking up %s property in node %s failed %d\n",
				dts_tag[i][0], np->full_name, rc);
			break;
		} else {
			dev_dbg(plat_data->dev, "%s = 0x%02x",
				dts_tag[i][0], p_tas256x->devs[i]->mn_addr);
		}

		p_tas256x->devs[i]->mn_reset_gpio =
			of_get_named_gpio(np, dts_tag[i][1], 0);
		if (!gpio_is_valid(p_tas256x->devs[i]->mn_reset_gpio))
			dev_err(plat_data->dev,
				"Looking up %s property in node %s failed %d\n",
				dts_tag[i][1], np->full_name,
				p_tas256x->devs[i]->mn_reset_gpio);
		else
			dev_dbg(plat_data->dev, "%s = %d",
				dts_tag[i][1],
				p_tas256x->devs[i]->mn_reset_gpio);

		p_tas256x->devs[i]->mn_irq_gpio =
			of_get_named_gpio(np, dts_tag[i][2], 0);
		if (!gpio_is_valid(p_tas256x->devs[i]->mn_irq_gpio)) {
			dev_err(plat_data->dev,
				"Looking up %s property in node %s failed %d\n",
				dts_tag[i][2], np->full_name,
				p_tas256x->devs[i]->mn_irq_gpio);
		} else {
			dev_dbg(plat_data->dev, "%s = %d",
				dts_tag[i][2],
				p_tas256x->devs[i]->mn_irq_gpio);
		}
		p_tas256x->devs[i]->spk_control = 1;
	}

	if (rc)
		goto EXIT;

	rc = of_property_read_u32(np, "ti,frame-start",
		&p_tas256x->mn_frame_start);
	if (rc) {
		dev_info(plat_data->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,frame-start", np->full_name, rc);
		p_tas256x->mn_frame_start = 1;
	} else {
		dev_dbg(plat_data->dev, "ti,frame-start=0x%x",
			p_tas256x->mn_frame_start);
	}

	rc = of_property_read_u32(np, "ti,rx-offset", &p_tas256x->mn_rx_offset);
	if (rc) {
		dev_info(plat_data->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,rx-offset", np->full_name, rc);
		p_tas256x->mn_rx_offset = 1;
	} else {
		dev_dbg(plat_data->dev, "ti,rx-offset=0x%x",
			p_tas256x->mn_rx_offset);
	}

	rc = of_property_read_u32(np, "ti,rx-edge", &p_tas256x->mn_rx_edge);
	if (rc) {
		dev_info(plat_data->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,rx-edge", np->full_name, rc);
		p_tas256x->mn_rx_edge = 0;
	} else {
		dev_dbg(plat_data->dev, "ti,rx-edge=0x%x",
			p_tas256x->mn_rx_edge);
	}

	rc = of_property_read_u32(np, "ti,tx-offset", &p_tas256x->mn_tx_offset);
	if (rc) {
		dev_info(plat_data->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,tx-offset", np->full_name, rc);
		p_tas256x->mn_tx_offset = 1;
	} else {
		dev_dbg(plat_data->dev, "ti,tx-offset=0x%x",
			p_tas256x->mn_tx_offset);
	}

	rc = of_property_read_u32(np, "ti,tx-edge", &p_tas256x->mn_tx_edge);
	if (rc) {
		dev_info(plat_data->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,tx-edge", np->full_name, rc);
		p_tas256x->mn_tx_edge = 1;
	} else {
		dev_dbg(plat_data->dev, "ti,tx-edge=0x%x",
			p_tas256x->mn_tx_edge);
	}

	rc = of_property_read_u32(np, "ti,iv-width", &p_tas256x->mn_iv_width);
	if (rc) {
		dev_err(plat_data->dev,
			"Looking up %s property in node %s failed %d\n",
			"ti,iv-width", np->full_name, rc);
	} else {
		dev_dbg(plat_data->dev, "ti,iv-width=0x%x",
			p_tas256x->mn_iv_width);
		p_tas256x->curr_mn_iv_width = p_tas256x->mn_iv_width;
	}

	rc = of_property_read_u32(np, "ti,vbat-mon", &p_tas256x->mn_vbat);
	if (rc) {
		dev_err(plat_data->dev,
				"Looking up %s property in node %s failed %d\n",
			"ti,vbat-mon", np->full_name, rc);
	} else {
		dev_dbg(plat_data->dev, "ti,vbat-mon=0x%x",
			p_tas256x->mn_vbat);
		p_tas256x->curr_mn_vbat = p_tas256x->mn_vbat;
	}
#if IS_ENABLED(CONFIG_TAS26XX_HAPTICS)
	rc = of_property_read_u32(np, "ti,haptics", &p_tas256x->mn_haptics);
	if (rc) {
		dev_err(plat_data->dev,
				"Looking up %s property in node %s failed %d\n",
			"ti,haptics", np->full_name, rc);
	} else {
		dev_dbg(plat_data->dev, "ti,haptics=0x%x",
			p_tas256x->mn_haptics);
	}
#endif
#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
	/* Needs to be enabled always */
	tas25xx_parse_algo_dt(np);
#endif /*CONFIG_TAS25XX_ALGO*/
EXIT:
	return rc;
}
#ifdef SYS_NODE
#define DBGFS_REG_COUNT 15
static int dbgfs_reg[DBGFS_REG_COUNT] = {
	TAS256X_REG(0x00, 0x01, 0x21),
	TAS256X_REG(0x00, 0x02, 0x64),
	TAS256X_REG(0x00, 0x02, 0x65),
	TAS256X_REG(0x00, 0x02, 0x66),
	TAS256X_REG(0x00, 0x02, 0x67),
	TAS256X_REG(0x00, 0x02, 0x6c),
	TAS256X_REG(0x00, 0x02, 0x6d),
	TAS256X_REG(0x00, 0x02, 0x6e),
	TAS256X_REG(0x00, 0x02, 0x6f),
	TAS256X_REG(0x00, 0xfd, 0x0d),
	TAS256X_REG(0x00, 0xfd, 0x12),
	TAS256X_REG(0x00, 0xfd, 0x3d),
	TAS256X_REG(0x00, 0xfd, 0x46),
	TAS256X_REG(0x00, 0xfd, 0x47),
	TAS256X_REG(0x00, 0xfd, 0x64),
};

int tas256x_process_block_show(void *pContext, unsigned char *data,
	unsigned char dev_idx, int sublocksize, char *buf, ssize_t *length)
{
	struct tas256x_priv *p_tas256x = (struct tas256x_priv *)pContext;
	unsigned char subblk_typ = data[1];
	int subblk_offset = 2;
	int chn = 0, chnend = 0;

	if(dev_idx) {
		chn = dev_idx - 1;
		chnend = dev_idx;
	} else {
		chn = 0;
		chnend = p_tas256x->mn_channels;
	}

	for(;chn < chnend;chn++) {
		subblk_offset = 2;
		switch (subblk_typ) {
			case TAS256X_CMD_SING_W: {
		/*
		 *		dev_idx 	: one byte
		 *		subblk_type : one byte
		 *		payload_len : two bytes
		 *		{
		 *			book	: one byte
		 *			page	: one byte
		 *			reg 	: one byte
		 *			val 	: one byte
		 *		}[payload_len/4]
		 */
				int i = 0;
				unsigned short len = SMS_HTONS(data[2], data[3]);

				subblk_offset += 2;
				*length += scnprintf(buf+*length, PAGE_SIZE-*length, "\t\tSINGLE BYTE:\n");
				if (subblk_offset + 4 * len > sublocksize) {
					*length += scnprintf(buf+*length, PAGE_SIZE-*length, "Out of memory %s: %u\n",
						__func__, __LINE__);
					break;
				}

				for (i = 0; i < len; i++) {
					*length += scnprintf(buf+*length, PAGE_SIZE-*length,
						"\t\t\tBOOK0x%02x PAGE0x%02x REG0x%02x VALUE = 0x%02x\n",
		                data[subblk_offset], data[subblk_offset + 1],
		                data[subblk_offset + 2], data[subblk_offset + 3]);
					subblk_offset += 4;
				}
			}
			break;
			case TAS256X_CMD_BURST: {
		/*
		 *		dev_idx 	: one byte
		 *		subblk_type : one byte
		 *		payload_len : two bytes
		 *		book		: one byte
		 *		page		: one byte
		 *		reg 	: one byte
		 *		reserve 	: one byte
		 *		payload 	: payload_len bytes
		 */
				int i = 0;
				unsigned short len = SMS_HTONS(data[2], data[3]);
				unsigned char reg = 0;
				subblk_offset += 2;
				*length += scnprintf(buf+*length, PAGE_SIZE-*length, "\t\tBURST:\n");
				if (subblk_offset + 4 + len > sublocksize) {
					*length += scnprintf(buf+*length, PAGE_SIZE-*length,
						"Out of memory %s: %u\n", __func__, __LINE__);
					break;
				}
				if (len % 4) {
					*length += scnprintf(buf+*length, PAGE_SIZE-*length,
						"Burst len is wrong %s: %u\n", __func__, __LINE__);
					break;
				}
				reg = data[subblk_offset + 2];
				*length += scnprintf(buf+*length, PAGE_SIZE-*length,"\t\t\tBOOK0x%02x PAGE0x%02x\n",
					data[subblk_offset], data[subblk_offset + 1]);
		        subblk_offset += 4;
		        for (i = 0; i < len / 4; i++) {
		            *length += scnprintf(buf+*length, PAGE_SIZE-*length,
		                "\t\t\tREG0x%02x = 0x%02x REG0x%02x = 0x%02x REG0x%02x = 0x%02x REG0x%02x = 0x%02x\n",
		                reg + i*4, data[subblk_offset + 0],
		                reg + i*4+1, data[subblk_offset + 1],
		                reg + i * 4 + 2, data[subblk_offset + 2],
		                reg + i * 4 + 3, data[subblk_offset + 3]);
		            subblk_offset += 4;
		        }
			}
			break;
			case TAS256X_CMD_DELAY: {
		/*
		 *		dev_idx 	: one byte
		 *		subblk_type : one byte
		 *		delay_time	: two bytes
		 */
				unsigned short delay_time = 0;

				if (subblk_offset + 2 > sublocksize) {
					*length += scnprintf(buf+*length, PAGE_SIZE-*length,
						"Out of memory %s: %u\n", __func__, __LINE__);
					break;
				}
				delay_time = SMS_HTONS(data[2], data[3]);
				*length += scnprintf(buf+*length, PAGE_SIZE-*length,
					"\t\tDELAY = %ums\n", delay_time);
				subblk_offset += 2;
			}
			break;
			case TAS256X_CMD_FIELD_W:
		/*
		 *		dev_idx 	: one byte
		 *		subblk_type : one byte
		 *		reserve 	: one byte
		 *		mask		: one byte
		 *		book		: one byte
		 *		page		: one byte
		 *		reg 	: one byte
		 *		reserve 	: one byte
		 *		payload 	: payload_len bytes
		 */
			if (subblk_offset + 6 > sublocksize) {
				*length += scnprintf(buf+*length, PAGE_SIZE-*length,
					"Out of memory %s: %u\n", __func__, __LINE__);
				break;
			}
			*length += scnprintf(buf+*length, PAGE_SIZE-*length,"\t\tFIELD:\n");
			*length += scnprintf(buf+*length, PAGE_SIZE-*length,
				"\t\t\tBOOK0x%02x PAGE0x%02x REG0x%02x MASK0x%02x VALUE = 0x%02x\n",
	            data[subblk_offset + 2], data[subblk_offset + 3],
	            data[subblk_offset + 4], data[subblk_offset + 1],
	            data[subblk_offset + 5]);

			subblk_offset += 6;
			break;
			default:
			break;
		};
	}
	return subblk_offset;
}

static ssize_t i2c_show  (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
    struct tas256x_priv *p_tas256x = g_tas256x;
	const int size = 20;
	int n = 0;

	pr_info("[SmartPA-%d]%s enter.\n", __LINE__, __func__);

	n += scnprintf(buf, size, "SmartPA-0x%02x %s\n", p_tas256x->devs[0]->mn_addr,
			(1) ? "OK" : "ERROR");

	return n;

}

static ssize_t reg_l_show  (struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct tas256x_priv *p_tas256x = g_tas256x;
	int i;
	ssize_t len = 0;
	const int size = PAGE_SIZE;
	int data = 0;
	int n_result = 0;

	if( p_tas256x != NULL) {
		//15 bytes
		len += scnprintf(buf+len, size-len, "i2c-addr: 0x%02x\n", p_tas256x->devs[0]->mn_addr);
		//2560 bytes
		for (i = 0; i < 128; i++) {
			n_result = p_tas256x->read(p_tas256x, channel_left, i, &data);
			if (n_result < 0) {
				pr_info("[SmartPA-%d]%s: read register failed\n", __LINE__, __func__);
			}
			//20 bytes
			if(len + 20 <= size) {
				len += scnprintf(buf+len, size-len, "B0x%02xP0x%02xR0x%02x:0x%02x\n",
					TAS256X_BOOK_ID(i), TAS256X_PAGE_ID(i),
					TAS256X_PAGE_REG(i), data);
			} else {
				pr_info("[SmartPA-%u]%s: mem is not enough: PAGE_SIZE = %lu\n",
					__LINE__, __func__, PAGE_SIZE);
				break;
			}
		}
		//300 bytes
		for (i = 0; i < DBGFS_REG_COUNT; i++) {
			p_tas256x->read(p_tas256x, channel_left, dbgfs_reg[i], &data);
			//20 bytes
			if(len + 20 <= size) {
				len += scnprintf(buf+len, size-len, "B0x%02xP0x%02xR0x%02x:0x%02x\n",
					TAS256X_BOOK_ID(dbgfs_reg[i]), TAS256X_PAGE_ID(dbgfs_reg[i]),
					TAS256X_PAGE_REG(dbgfs_reg[i]), data);
			} else {
				pr_info("[SmartPA-%u]%s: mem is not enough: PAGE_SIZE = %lu\n",
					__LINE__, __func__, PAGE_SIZE);
				break;
			}
		}
	}
	pr_info("[SmartPA-%d]%s: ======caught smartpa reg end: len = %lu ======\n",
		__LINE__, __func__, len);
	return len;
}

static ssize_t reg_l_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t size)
{
	struct tas256x_priv *p_tas256x = g_tas256x;
	int ret = 0;
	unsigned int kbuf[2];
	char *temp;
	int n_result = 0;
	pr_info("[SmartPA-%d]%s: enter!\n", __LINE__, __func__);
	pr_info("[SmartPA-%d]%s: cnt %d\n", __LINE__, __func__, (int)size);
	if( p_tas256x == NULL)
		return size;

	if (size > 2) {
		temp = kmalloc(size, GFP_KERNEL);
		if (!temp) {
			return -ENOMEM;
		}
		memcpy(temp, buf, size);
		ret = sscanf(temp, "%x %x", &kbuf[0], &kbuf[1]);
		if (!ret) {
			kfree(temp);
			return -EFAULT;
		}
		pr_info("[SmartPA-%d]:%s: kbuf[0]=0x%02x, kbuf[0]=%d, kbuf[1]=0x%02x cnt=%d\n",
				__LINE__, __func__, kbuf[0], kbuf[0], kbuf[1], (int)size);
		n_result = p_tas256x->write(p_tas256x, 1, kbuf[0], kbuf[1]);
		if (n_result < 0){
			pr_err("[SmartPA-%d]:%s write reg[0x%02x] failed.\n",__LINE__, __func__, kbuf[0]);
		}
		kfree(temp);
	} else {
			pr_err("[SmartPA-%d]:%s count error.\n",__LINE__, __func__);
	}
	 return size;

}
static ssize_t
reg_r_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct tas256x_priv *p_tas256x = g_tas256x;
	ssize_t len = 0;
	if( p_tas256x != NULL) {
		if (p_tas256x->mn_channels == 2) {
			int i;
			const int size = PAGE_SIZE;
			int data = 0;
			int n_result = 0;
			//15 bytes
			len += scnprintf(buf+len, size-len, "i2c-addr: 0x%02x\n", p_tas256x->devs[1]->mn_addr);
			//2560 bytes
			for (i = 0; i < 128; i++) {
				n_result = p_tas256x->read(p_tas256x, channel_right, i, &data);
				if (n_result < 0) {
					pr_info("[SmartPA-%d]%s: read register failed\n", __LINE__, __func__);
				}
				//20 bytes
				if(len + 20 <= size) {
					len += scnprintf(buf+len, size-len, "B0x%02xP0x%02xR0x%02x:0x%02x\n",
						TAS256X_BOOK_ID(i), TAS256X_PAGE_ID(i),
						TAS256X_PAGE_REG(i), data);
				} else {
					pr_info("[SmartPA-%u]%s: mem is not enough: PAGE_SIZE = %lu\n",
						__LINE__, __func__, PAGE_SIZE);
					break;
				}
			}
			//300 bytes
			for (i = 0; i < DBGFS_REG_COUNT; i++) {
				p_tas256x->read(p_tas256x, channel_right, dbgfs_reg[i], &data);
				//20 bytes
				if(len + 20 <= size) {
					len += scnprintf(buf+len, size-len, "B0x%02xP0x%02xR0x%02x:0x%02x\n",
						TAS256X_BOOK_ID(dbgfs_reg[i]), TAS256X_PAGE_ID(dbgfs_reg[i]),
						TAS256X_PAGE_REG(dbgfs_reg[i]), data);
				} else {
					pr_info("[SmartPA-%u]%s: mem is not enough: PAGE_SIZE = %lu\n",
						__LINE__, __func__, PAGE_SIZE);
					break;
				}
			}
		} else {
				pr_info("[SmartPA-%d]%s: reg_r write only suport stereo!\n", __LINE__, __func__);
		}
	}
	pr_info("[SmartPA-%d]%s: ======caught smartpa reg end ======\n", __LINE__, __func__);
	return len;
}
static ssize_t reg_r_store(struct kobject *kobj, struct kobj_attribute *attr,
			     const char *buf, size_t size)
{
	struct tas256x_priv *p_tas256x = g_tas256x;
	if( p_tas256x == NULL)
		return size;

	if (p_tas256x->mn_channels == 2) {
		int ret = 0;
		unsigned int kbuf[2];
		char *temp;
		pr_info("[SmartPA-%d]%s: enter!\n", __LINE__, __func__);
		pr_info("[SmartPA-%d]%s: cnt %d\n", __LINE__, __func__, (int)size);
		if (size > 2) {
			temp = kmalloc(size, GFP_KERNEL);
			if (!temp) {
				return -ENOMEM;
			}
				memcpy(temp, buf, size);
				ret = sscanf(temp, "%x %x", &kbuf[0], &kbuf[1]);
			if (!ret) {
				kfree(temp);
				return -EFAULT;
			}
				pr_info("[SmartPA-%d]:%s: kbuf[0]=%x, kbuf[0]=%d, kbuf[1]=%x cnt=%d\n",
					__LINE__, __func__, kbuf[0], kbuf[0], kbuf[1], (int)size);
			p_tas256x->write(p_tas256x, 2, kbuf[0], kbuf[1]);
			kfree(temp);
		} else {
			pr_err("[SmartPA-%d]:%s count error.\n",__LINE__, __func__);
		}
	} else {
		pr_info("[SmartPA-%d]%s: reg_r show only support stereo!\n", __LINE__, __func__);
	}
	return size;
}

static ssize_t regbininfo_list_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct tas256x_priv *pTAS256X = g_tas256x;
	struct tas256x_config_info **cfg_info = pTAS256X->cfg_info;
	int n = 0, i = 0;

	mutex_lock(&pTAS256X->file_lock);
	if(pTAS256X == NULL) {
		if(n + 42 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE -n, "ERROR: Can't find tas256x_priv handle!\n\r");
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n\r",__LINE__, __func__);
			n = PAGE_SIZE;
		}
		goto EXIT;
	}

	if(n + 64 < PAGE_SIZE) {
		n += scnprintf(buf+n, PAGE_SIZE -n, "Regbin File Version: 0X%04X\n\r",
			pTAS256X->fw_hdr.binary_version_num);
	} else {
		scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n\r",__LINE__, __func__);
		n = PAGE_SIZE;
		goto EXIT;
	}

	for(i = 0; i < pTAS256X->ncfgs; i++) {
		if(n + 16 < PAGE_SIZE) {
			n += scnprintf(buf+n, PAGE_SIZE -n, "conf %d", i);
		} else {
			scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n\r",__LINE__, __func__);
			n = PAGE_SIZE;
			break;
		}
		if (pTAS256X->fw_hdr.binary_version_num >= 0x105) {
			if(n + 100 < PAGE_SIZE) {
				n += scnprintf(buf+n, PAGE_SIZE-n, ": %s\n\r", cfg_info[i]->mpName);
			} else {
				scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n\r",__LINE__, __func__);
				n = PAGE_SIZE;
				break;
			}
		} else {
			n += scnprintf(buf+n, PAGE_SIZE-n, "\n\r");
		}
	}
EXIT:
	mutex_unlock(&pTAS256X->file_lock);
	return n;
}

static ssize_t regcfg_list_store(struct kobject *kobj, struct kobj_attribute *attr,
				   const char *buf, size_t count)
{
	struct tas256x_priv *pTAS256X = g_tas256x;
	int ret = 0;
	char *temp = NULL;
	struct Tsyscmd *pSysCmd = NULL;

	if( pTAS256X == NULL)
		return count;
	pSysCmd = &pTAS256X->nSysCmd[RegCfgListCmd];
	pSysCmd->bufLen = snprintf(gSysCmdLog[RegCfgListCmd], 256, "command: echo CG > NODE\n"
		"CG is conf NO, it should be 2-digital decimal\n"
		"eg: echo 01 > NODE\n\r");

	if (count >= 1) {
		temp = kmalloc(count, GFP_KERNEL);
		if (!temp) {
			pSysCmd->bCmdErr = true;
			pSysCmd->bufLen += snprintf(gSysCmdLog[RegCfgListCmd], 15, "No memory!\n");
			return -ENOMEM;
		}
		memcpy(temp, buf, count);
		ret = sscanf(temp, "%hhd", &(pSysCmd->mnBook));
		if (!ret) {
			pSysCmd->bCmdErr = true;
			kfree(temp);
			return -EFAULT;
		}
		pr_info("[SmartPA-%d]:%s:cfg=%2d, cnt=%d\n",  __LINE__, __func__, pSysCmd->mnBook, (int)count);
		if(pSysCmd->mnBook >= (unsigned char)pTAS256X->ncfgs) {
			pSysCmd->bCmdErr = true;
			pSysCmd->bufLen += snprintf(gSysCmdLog[RegCfgListCmd], 30, "Wrong conf NO!\n\r");
		} else {
			pSysCmd->bCmdErr = false;
			gSysCmdLog[RegCfgListCmd][0] = '\0';
			pSysCmd->bufLen = 0;
		}
		kfree(temp);
	} else {
		pSysCmd->bCmdErr = true;
		ret = -1;
		pr_err("[SmartPA-%d]:%s count error.\n",__LINE__, __func__);
	}

	return count;
}

static ssize_t regcfg_list_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	struct tas256x_priv *p_tas256x = g_tas256x;
	ssize_t len = 0;
	int j = 0, k = 0;

	if( p_tas256x != NULL) {
		struct Tsyscmd *pSysCmd = &p_tas256x->nSysCmd[RegCfgListCmd];
		struct tas256x_config_info **cfg_info = p_tas256x->cfg_info;
		if(pSysCmd->bCmdErr == true || pSysCmd->mnBook >= p_tas256x->ncfgs) {
			len += scnprintf(buf, pSysCmd->bufLen, gSysCmdLog[RegCfgListCmd]);
			goto EXIT;
		}

		len += scnprintf(buf+len, PAGE_SIZE-len, "Conf %02d", pSysCmd->mnBook);
		if (p_tas256x->fw_hdr.binary_version_num >= 0x105) {
			if(len + 100 < PAGE_SIZE) {
				len += scnprintf(buf+len, PAGE_SIZE-len, ": %s\n\r", cfg_info[pSysCmd->mnBook]->mpName);
			} else {
				scnprintf(buf+PAGE_SIZE-100, 100, "\n[SmartPA-%d]:%s Out of memory!\n\r",__LINE__, __func__);
				len = PAGE_SIZE;
				goto EXIT;
			}
		} else {
			len += scnprintf(buf+len, PAGE_SIZE-len, "\n\r");
		}
		for (j = 0; j < (int)cfg_info[pSysCmd->mnBook]->real_nblocks; j++) {
			unsigned int length = 0, rc = 0;
			len += scnprintf(buf+len, PAGE_SIZE-len, "block type:%s\t device idx = 0x%02x\n",
				blocktype[cfg_info[pSysCmd->mnBook]->blk_data[j]->block_type - 1],
				cfg_info[pSysCmd->mnBook]->blk_data[j]->dev_idx);
			for (k = 0; k < (int)cfg_info[pSysCmd->mnBook]->blk_data[j]->nSublocks; k++) {
				rc = tas256x_process_block_show(p_tas256x,
					cfg_info[pSysCmd->mnBook]->blk_data[j]->regdata + length,
					cfg_info[pSysCmd->mnBook]->blk_data[j]->dev_idx,
					cfg_info[pSysCmd->mnBook]->blk_data[j]->block_size - length, buf, &len);
				length += rc;
				if (cfg_info[pSysCmd->mnBook]->blk_data[j]->block_size < length) {
					len += scnprintf(buf+len, PAGE_SIZE-len, "%s:%u:ERROR:%u %u out of memory\n",
						__func__, __LINE__, length,
						cfg_info[pSysCmd->mnBook]->blk_data[j]->block_size);
					break;
				}
			}
			if (length != cfg_info[pSysCmd->mnBook]->blk_data[j]->block_size) {
				len += scnprintf(buf+len, PAGE_SIZE-len, "%s:%u:ERROR: %u %u size is not same\n",
					__func__, __LINE__, length,
					cfg_info[pSysCmd->mnBook]->blk_data[j]->block_size);
			}
		}
	}
EXIT:
	return len;
}
static struct kobj_attribute dev_attr_reg_l =
	__ATTR(reg_l, 0664, reg_l_show, reg_l_store);
static struct kobj_attribute dev_attr_reg_r =
	__ATTR(reg_r, 0664, reg_r_show, reg_r_store);
static struct kobj_attribute dev_attr_i2c =
	__ATTR(i2c, 0664, i2c_show, NULL);
static struct kobj_attribute dev_attr_regbininfo_list =
	__ATTR(regbininfo_list, 0664, regbininfo_list_show, NULL);
static struct kobj_attribute dev_attr_regcfg_list =
	__ATTR(regcfg_list, 0664, regcfg_list_show, regcfg_list_store);

static struct attribute *sys_node_attributes[] = {
	&dev_attr_reg_l.attr,
	&dev_attr_reg_r.attr,
	&dev_attr_i2c.attr,
	&dev_attr_regbininfo_list.attr,
	&dev_attr_regcfg_list.attr,
	NULL
};
static struct attribute_group tas256x_node_attribute_group = {
	.name = NULL,		/* put in device directory */
	.attrs = sys_node_attributes
};


static int class_attr_create(struct tas256x_priv *p_tas256x)
{
	int ret = -1;
	if( p_tas256x == NULL)
	{
		return ret;
	}
	p_tas256x->k_obj = kobject_create_and_add("audio-tas2562",kernel_kobj);
	if( !p_tas256x->k_obj )
	{
		pr_info("kobject_create_and_add audio-tas2562 file faild\n");
		return 0;
	}

	ret = sysfs_create_group(p_tas256x->k_obj, &tas256x_node_attribute_group);
	if( ret )
	{
		pr_info("sysfs_create_group audio-tas2562 file faild\n");
	}

	return ret;
}

static int class_attr_remove(struct tas256x_priv *p_tas256x)
{

	if (p_tas256x != NULL && p_tas256x->k_obj) {
		sysfs_remove_group(p_tas256x->k_obj, &tas256x_node_attribute_group);
		kobject_del(p_tas256x->k_obj);
		p_tas256x->k_obj = NULL;
	}
	return 0;
}

#endif

static int tas256x_i2c_probe(struct i2c_client *p_client,
			const struct i2c_device_id *id)
{
	struct tas256x_priv *p_tas256x;
	struct linux_platform *plat_data;
	int ret = 0;
	int i = 0;
	struct regulator *ldo = NULL;

	dev_info(&p_client->dev, "Driver Tag: %s\n", TAS256X_DRIVER_TAG);
	dev_info(&p_client->dev, "%s enter\n", __func__);
	
	if (hw_check_smartpa_type("tas256x", "none") == false) {
		dev_info(&p_client->dev, "%s other smartpa driver probe succ, no need probe\n", __func__);
		return 0;
	}

	if (hw_lock_pa_probe_state() == 0) {
		dev_info(&p_client->dev, "%s get lock fail, other smartpa driver processing, probe later\n", __func__);
		return -EPROBE_DEFER;
	}

	ldo = devm_regulator_get(&p_client->dev, "smartpa-vdd");
	if (IS_ERR_OR_NULL(ldo)) {
		dev_err(&p_client->dev, "regulator get fail\n");
	}
	ret = regulator_enable(ldo);
	if (ret != 0) {
		dev_err(&p_client->dev, "regulator enable fail\n");
	}

	p_tas256x = devm_kzalloc(&p_client->dev,
		sizeof(struct tas256x_priv), GFP_KERNEL);
	if (p_tas256x == NULL) {
		dev_err(&p_client->dev, "failed to allocate memory\n");
		ret = -ENOMEM;
		goto err;
	}

	plat_data = (struct linux_platform *)devm_kzalloc(&p_client->dev,
		sizeof(struct linux_platform), GFP_KERNEL);
	if (plat_data == NULL) {
		dev_err(&p_client->dev, "failed to allocate plat_data memory\n");
		ret = -ENOMEM;
		goto err;
	}

	p_tas256x->platform_data = plat_data;
	/* REGBIN related */
	p_tas256x->profile_cfg_id = -1;
	p_tas256x->plat_write = tas256x_regmap_write;
	p_tas256x->plat_read = tas256x_regmap_read;
	p_tas256x->plat_bulk_write = tas256x_regmap_bulk_write;
	p_tas256x->plat_bulk_read = tas256x_regmap_bulk_read;
	p_tas256x->plat_update_bits = tas256x_regmap_update_bits;

	plat_data->client = p_client;
	plat_data->dev = &p_client->dev;
	i2c_set_clientdata(p_client, p_tas256x);
	dev_set_drvdata(&p_client->dev, p_tas256x);
	plat_data->regmap = devm_regmap_init_i2c(p_client,
				&tas256x_i2c_regmap);
	if (IS_ERR(plat_data->regmap)) {
		ret = PTR_ERR(plat_data->regmap);
		dev_err(&p_client->dev,
			"Failed to allocate register map: %d\n",
			ret);
		goto err;
	}

	mutex_init(&p_tas256x->dev_lock);
	p_tas256x->hw_reset = tas256x_hw_reset;
	p_tas256x->enable_irq = tas256x_enable_irq;
	p_tas256x->schedule_init_work = schedule_init_work;
	p_tas256x->cancel_init_work = cancel_init_work;
	p_tas256x->schedule_pwrup_work = schedule_pwrup_work;
	p_tas256x->cancel_pwrup_work = cancel_pwrup_work;
#if IS_ENABLED(CODEC_PM)
	plat_data->runtime_suspend = tas256x_runtime_suspend;
	plat_data->runtime_resume = tas256x_runtime_resume;
	plat_data->mn_power_state = TAS256X_POWER_SHUTDOWN;
#endif

	if (p_client->dev.of_node)
		tas256x_parse_dt(&p_client->dev, p_tas256x);

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (gpio_is_valid(p_tas256x->devs[i]->mn_reset_gpio)) {
			ret = gpio_request(
					p_tas256x->devs[i]->mn_reset_gpio,
					reset_gpio_label[i]);
			if (ret) {
				dev_err(plat_data->dev,
					"%s: Failed to request gpio %d\n",
					__func__,
					p_tas256x->devs[i]->mn_reset_gpio);
				ret = -EINVAL;
				goto err;
			}
		}
	}

	ret = tas256x_register_device(p_tas256x);
	if (ret < 0) {
		goto err;
	}

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		switch (p_tas256x->devs[i]->device_id) {
		case DEVICE_TAS2562:
			p_tas256x->devs[i]->dev_ops.tas_probe = tas2562_probe;
			break;
		case DEVICE_TAS2564:
			p_tas256x->devs[i]->dev_ops.tas_probe = tas2564_probe;
			break;
		default:
			p_tas256x->devs[i]->dev_ops.tas_probe = NULL;
			break;
		}
	}

	INIT_DELAYED_WORK(&p_tas256x->irq_work, irq_work_routine);

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (gpio_is_valid(p_tas256x->devs[i]->mn_irq_gpio)) {
			ret =
				gpio_request(
					p_tas256x->devs[i]->mn_irq_gpio,
					irq_gpio_label[i]);
			if (ret < 0) {
				dev_err(plat_data->dev,
					"%s:%u: ch 0x%02x: GPIO %d request error\n",
					__func__, __LINE__,
					p_tas256x->devs[i]->mn_addr,
					p_tas256x->devs[i]->mn_irq_gpio);
				goto err;
			}
			gpio_direction_input(p_tas256x->devs[i]->mn_irq_gpio);
			/*tas256x_dev_write(p_tas256x,
			 *	(i == 0)? channel_left : channel_right,
			 *	TAS256X_MISCCONFIGURATIONREG0, 0xce);
			 */

			p_tas256x->devs[i]->mn_irq =
				gpio_to_irq(p_tas256x->devs[i]->mn_irq_gpio);
			dev_info(plat_data->dev, "irq = %d\n",
				p_tas256x->devs[i]->mn_irq);

			ret = request_threaded_irq(
					p_tas256x->devs[i]->mn_irq,
					tas256x_irq_handler,
					NULL,
					IRQF_TRIGGER_FALLING|IRQF_ONESHOT,
					p_client->name, p_tas256x);
			if (ret < 0) {
				dev_err(plat_data->dev,
					"request_irq failed, %d\n", ret);
				goto err;
			}
			disable_irq_nosync(p_tas256x->devs[i]->mn_irq);
		}
		INIT_DELAYED_WORK(&p_tas256x->devs[i]->init_work, init_work_routine);
		p_tas256x->devs[i]->prv_data = p_tas256x;
		/* Added to fix "handset"->"vopi-speaker-lowpower" issue */
		INIT_DELAYED_WORK(&p_tas256x->devs[i]->pwrup_work, pwrup_work_routine);
	}

	tas256x_enable_irq(p_tas256x, true);

#if IS_ENABLED(CONFIG_TAS26XX_HAPTICS)
	ret = android_hal_stub_init(p_tas256x, plat_data->dev);
	if (ret) {
		dev_err(plat_data->dev, "Failed to init android hal stub: %d\n", ret);
		goto err;
	}
#endif

#ifdef SYS_NODE
	g_tas256x=p_tas256x;
	class_attr_create(p_tas256x);
#endif

	mutex_init(&p_tas256x->codec_lock);
	ret = tas256x_register_codec(p_tas256x);
	if (ret < 0) {
		dev_err(plat_data->dev,
			"register codec failed, %d\n", ret);
		goto err;
	}

	mutex_init(&p_tas256x->file_lock);
	ret = tas256x_register_misc(p_tas256x);
	if (ret < 0) {
		dev_err(plat_data->dev, "register codec failed %d\n",
			ret);
		goto err;
	}

	hw_set_smartpa_type("tas256x", strlen("tas256x"));
#if IS_ENABLED(CONFIG_TAS25XX_ALGO) && IS_ENABLED(CONFIG_TISA_SYSFS_INTF)
	/*add smartpa node*/
	ret = smartpa_debug_probe(plat_data->client);
	if (ret < 0) {
		dev_err(plat_data->dev, "register smartpa node failed %d\n",
			ret);
		goto err;
	}
#endif

#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
#if IS_ENABLED(CONFIG_PLATFORM_QCOM)
	INIT_DELAYED_WORK(&p_tas256x->dc_work, dc_work_routine);
	g_p_tas256x = p_tas256x;
	register_tas256x_reset_func(tas256x_software_reset, &s_dc_detect);
#endif /*CONFIG_PLATFORM_QCOM*/
#endif /*CONFIG_TAS25XX_ALGO*/
	goto succ;

err:
	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (gpio_is_valid(p_tas256x->devs[i]->mn_reset_gpio))
			gpio_free(p_tas256x->devs[i]->mn_reset_gpio);
		if (gpio_is_valid(p_tas256x->devs[i]->mn_irq_gpio))
			gpio_free(p_tas256x->devs[i]->mn_irq_gpio);
		if (p_tas256x->devs[i])
			kfree(p_tas256x->devs[i]);
	}
succ:
	hw_unlock_pa_probe_state();
	return ret;
}

static int tas256x_i2c_remove(struct i2c_client *p_client)
{
	int i = 0;
	struct tas256x_priv *p_tas256x = i2c_get_clientdata(p_client);
	struct linux_platform *plat_data = NULL;

	plat_data = (struct linux_platform *) p_tas256x->platform_data;
	dev_info(plat_data->dev, "%s\n", __func__);

	/*Cancel all the work routine before exiting*/
	for (i = 0; i < p_tas256x->mn_channels; i++) {
		cancel_delayed_work_sync(&p_tas256x->devs[i]->init_work);
	}

	cancel_delayed_work_sync(&p_tas256x->irq_work);
	cancel_delayed_work_sync(&p_tas256x->dc_work);

	tas256x_deregister_codec(p_tas256x);
	mutex_destroy(&p_tas256x->codec_lock);

	tas256x_deregister_misc(p_tas256x);
	mutex_destroy(&p_tas256x->file_lock);

	mutex_destroy(&p_tas256x->dev_lock);

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (gpio_is_valid(p_tas256x->devs[i]->mn_reset_gpio))
			gpio_free(p_tas256x->devs[i]->mn_reset_gpio);
		if (gpio_is_valid(p_tas256x->devs[i]->mn_irq_gpio))
			gpio_free(p_tas256x->devs[i]->mn_irq_gpio);
		if (p_tas256x->devs[i])
			kfree(p_tas256x->devs[i]);
	}
#ifdef SYS_NODE
	class_attr_remove(p_tas256x);
#endif

	if (p_tas256x->devs)
		kfree(p_tas256x->devs);

	return 0;
}


static const struct i2c_device_id tas256x_i2c_id[] = {
	{ "tas256x", 0},
	{ "tas2558", 1},
	{ "tas2562", 2},
	{ }
};
MODULE_DEVICE_TABLE(i2c, tas256x_i2c_id);

#if defined(CONFIG_OF)
static const struct of_device_id tas256x_of_match[] = {
	{ .compatible = "ti, tas256x" },
	{ .compatible = "ti, tas2558" },
	{ .compatible = "ti, tas2562" },
	{ .compatible = "ti, tas2564" },
	{ .compatible = "ti,tas256x" },
	{ .compatible = "ti,tas2558" },
	{ .compatible = "ti,tas2562" },
	{ .compatible = "ti,tas2564" },
	{},
};
MODULE_DEVICE_TABLE(of, tas256x_of_match);
#endif

static const struct dev_pm_ops tas256x_pm_ops = {
	.suspend = tas256x_pm_suspend,
	.resume = tas256x_pm_resume
};

static struct i2c_driver tas256x_i2c_driver = {
	.driver = {
		.name   = "tas256x",
		.owner  = THIS_MODULE,
#if defined(CONFIG_OF)
		.of_match_table = of_match_ptr(tas256x_of_match),
#endif
		.pm = &tas256x_pm_ops,
	},
	.probe      = tas256x_i2c_probe,
	.remove     = tas256x_i2c_remove,
	.id_table   = tas256x_i2c_id,
};

module_i2c_driver(tas256x_i2c_driver);

MODULE_AUTHOR("Texas Instruments Inc.");
MODULE_DESCRIPTION("TAS256X I2C Smart Amplifier driver");
MODULE_LICENSE("GPL v2");
