/*
 * cs35l38a.c -- CS35L38A ALSA SoC audio driver
 *
 * Copyright 2020 Cirrus Logic, Inc.
 *
 * Author: Li Xu <li.xu@cirrus.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */
#define DEBUG

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>
#include <linux/gpio/consumer.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <linux/gpio.h>
#include <sound/initval.h>
#include <sound/tlv.h>
#include <sound/cs35l38a.h>
#include <linux/of_irq.h>
#include <linux/completion.h>

#include "cs35l38a.h"

#define CS35LXX_MONITOR_WORK_ENABLE
#define SLEEP_COUNT_NUM         10

#include <sound/hw_audio/hw_audio_interface.h>

/*
 * Some fields take zero as a valid value so use a high bit flag that won't
 * get written to the device to mark those.
 */
#define CS35L38A_VALID_PDATA 0x80000000

int cs35l38a_component_set_sysclk(struct snd_soc_component *component,
				     int clk_id, int source, unsigned int freq,
				int dir);
static const char * const cs35l38a_supplies[] = {
	"VA",
	"VP",
};

struct  cs35l38a_private {
	struct device *dev;
	struct cs35l38a_platform_data pdata;
	struct regmap *regmap;
	struct regulator_bulk_data supplies[2];
	int num_supplies;
	int clksrc;
	int prev_clksrc;
	int extclk_freq;
	int extclk_cfg;
	int fll_igain;
	int sclk;
	struct gpio_desc *reset_gpio;
	struct completion global_pup_done;
	struct completion global_pdn_done;
	struct completion init_done;
	int rev_id;
};

struct cs35l38a_pll_sysclk_config {
	int freq;
	int clk_cfg;
	int fll_igain;
};

static const struct cs35l38a_pll_sysclk_config cs35l38a_pll_sysclk[] = {
	{32768,		0x00, 0x05},
	{8000,		0x01, 0x03},
	{11025,		0x02, 0x03},
	{12000,		0x03, 0x03},
	{16000,		0x04, 0x04},
	{22050,		0x05, 0x04},
	{24000,		0x06, 0x04},
	{32000,		0x07, 0x05},
	{44100,		0x08, 0x05},
	{48000,		0x09, 0x05},
	{88200,		0x0A, 0x06},
	{96000,		0x0B, 0x06},
	{128000,	0x0C, 0x07},
	{176400,	0x0D, 0x07},
	{192000,	0x0E, 0x07},
	{256000,	0x0F, 0x08},
	{352800,	0x10, 0x08},
	{384000,	0x11, 0x08},
	{512000,	0x12, 0x09},
	{705600,	0x13, 0x09},
	{750000,	0x14, 0x09},
	{768000,	0x15, 0x09},
	{1000000,	0x16, 0x0A},
	{1024000,	0x17, 0x0A},
	{1200000,	0x18, 0x0A},
	{1411200,	0x19, 0x0A},
	{1500000,	0x1A, 0x0A},
	{1536000,	0x1B, 0x0A},
	{2000000,	0x1C, 0x0A},
	{2048000,	0x1D, 0x0A},
	{2400000,	0x1E, 0x0A},
	{2822400,	0x1F, 0x0A},
	{3000000,	0x20, 0x0A},
	{3072000,	0x21, 0x0A},
	{3200000,	0x22, 0x0A},
	{4000000,	0x23, 0x0A},
	{4096000,	0x24, 0x0A},
	{4800000,	0x25, 0x0A},
	{5644800,	0x26, 0x0A},
	{6000000,	0x27, 0x0A},
	{6144000,	0x28, 0x0A},
	{6250000,	0x29, 0x08},
	{6400000,	0x2A, 0x0A},
	{6500000,	0x2B, 0x08},
	{6750000,	0x2C, 0x09},
	{7526400,	0x2D, 0x0A},
	{8000000,	0x2E, 0x0A},
	{8192000,	0x2F, 0x0A},
	{9600000,	0x30, 0x0A},
	{11289600,	0x31, 0x0A},
	{12000000,	0x32, 0x0A},
	{12288000,	0x33, 0x0A},
	{12500000,	0x34, 0x08},
	{12800000,	0x35, 0x0A},
	{13000000,	0x36, 0x0A},
	{13500000,	0x37, 0x0A},
	{19200000,	0x38, 0x0A},
	{22579200,	0x39, 0x0A},
	{24000000,	0x3A, 0x0A},
	{24576000,	0x3B, 0x0A},
	{25000000,	0x3C, 0x0A},
	{25600000,	0x3D, 0x0A},
	{26000000,	0x3E, 0x0A},
	{27000000,	0x3F, 0x0A},
};

static const struct reg_sequence cs35l38a_2terminal_load_on_patch[] = {
	{ 0x00000020,		0x00005555 },
	{ 0x00000020,		0x0000aaaa },
	{ 0x00004f34,		0x00008800 },
	{ 0x00000020,		0x0000cccc },
	{ 0x00000020,		0x00003333 },
};

static const struct reg_sequence cs35l38a_2terminal_load_off_patch[] = {
	{ 0x00000020,		0x00005555 },
	{ 0x00000020,		0x0000aaaa },
	{ 0x00004f34,		0x00000000 },
	{ 0x00000020,		0x0000cccc },
	{ 0x00000020,		0x00003333 },
};

static DECLARE_TLV_DB_SCALE(dig_vol_tlv, -10200, 25, 0);
static DECLARE_TLV_DB_SCALE(amp_gain_tlv, 0, 1, 1);

static const char * const cs35l38a_pcm_sftramp_text[] =  {
	"Off", ".5ms", "1ms", "2ms", "4ms", "8ms", "15ms", "30ms"};

static SOC_ENUM_SINGLE_DECL(pcm_sft_ramp,
			    CS35L38A_AMP_DIG_VOL_CTRL, 0,
			    cs35l38a_pcm_sftramp_text);

static const struct snd_kcontrol_new cs35l38a_aud_controls[] = {
	SOC_SINGLE_SX_TLV("Digital PCM Volume", CS35L38A_AMP_DIG_VOL_CTRL,
			  3, 0x4D0, 0x390, dig_vol_tlv),
	SOC_SINGLE_TLV("Analog PCM Volume", CS35L38A_AMP_GAIN_CTRL, 5, 0x13, 0,
		       amp_gain_tlv),
	SOC_ENUM("PCM Soft Ramp", pcm_sft_ramp),
#ifdef CS35LXX_MONITOR_WORK_ENABLE
	SOC_SINGLE_RANGE("CS35LXX Temp Thrshd 1", CS35L38A_ASP_TX1_SEL, 16,
			 0, 0xff, 0),
	SOC_SINGLE_RANGE("CS35LXX Temp Thrshd 2", CS35L38A_ASP_TX1_SEL, 24,
			 0, 0xff, 0),
	SOC_SINGLE_RANGE("CS35LXX Batt Gauge Thrshd 1", CS35L38A_ASP_TX2_SEL, 16,
			 0, 0xff, 0),
	SOC_SINGLE_RANGE("CS35LXX Batt Gauge Thrshd 2", CS35L38A_ASP_TX2_SEL, 24,
			 0, 0xff, 0),
	SOC_SINGLE_RANGE("CS35LXX Batt Gauge Thrshd 3", CS35L38A_ASP_TX3_SEL, 16,
			 0, 0xff, 0),
	SOC_SINGLE_RANGE("CS35LXX Batt Gauge Thrshd 4", CS35L38A_ASP_TX3_SEL, 24,
			 0, 0xff, 0),
	SOC_SINGLE_RANGE("CS35LXX  Atten Level 1", CS35L38A_ASP_TX4_SEL, 16,
			 0, 0xf, 0),
	SOC_SINGLE_RANGE("CS35LXX  Atten Level 2", CS35L38A_ASP_TX4_SEL, 20,
			 0, 0xf, 0),
	SOC_SINGLE_RANGE("CS35LXX  Atten Level 3", CS35L38A_ASP_TX4_SEL, 24,
			 0, 0xf, 0),
	SOC_SINGLE_RANGE("CS35LXX  Atten Level 4", CS35L38A_ASP_TX4_SEL, 28,
			 0, 0xf, 0),

	SOC_SINGLE_RANGE("CS35LXX temp battery protect disable", CS35L38A_ASP_TX5_SEL, 28,
			 0, 0xf, 0),
#endif
};
#ifdef CS35LXX_MONITOR_WORK_ENABLE
static int cs35lxx_spk_safe_release(struct cs35l38a_private *cs35l38a)
{
	int ret = 0;
	pr_info("++++>CRUS: %s begin, spk safe mode = %d.\n", __func__, cs35l38a->pdata.sub_spk_safe_mode);
	if(cs35l38a->pdata.sub_spk_safe_mode & CS35L38A_AMP_SHORT_ERR ) {
		pr_info("++++>CRUS:amp short release", __func__);
		ret = regmap_update_bits(cs35l38a->regmap,
				   	CS35L38A_PROTECT_REL_ERR,
					CS35L38A_AMP_SHORT_ERR_RLS, 0);
		ret = regmap_update_bits(cs35l38a->regmap,
				   	CS35L38A_PROTECT_REL_ERR,
					CS35L38A_AMP_SHORT_ERR_RLS,
					CS35L38A_AMP_SHORT_ERR_RLS);
		ret = regmap_update_bits(cs35l38a->regmap,
				   	CS35L38A_PROTECT_REL_ERR,
					CS35L38A_AMP_SHORT_ERR_RLS, 0);
		if (ret == 0)
			cs35l38a->pdata.sub_spk_safe_mode &= ~CS35L38A_AMP_SHORT_ERR;
	}
	if (cs35l38a->pdata.sub_spk_safe_mode & CS35L38A_TEMP_WARN) {
		pr_info("++++>CRUS:temp warning release", __func__);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_WARN_ERR_RLS, 0);
		ret = regmap_update_bits(cs35l38a->regmap,
				   	CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_WARN_ERR_RLS,
					CS35L38A_TEMP_WARN_ERR_RLS);
		ret = regmap_update_bits(cs35l38a->regmap,
				   	CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_WARN_ERR_RLS, 0);
		if (ret == 0)
			cs35l38a->pdata.sub_spk_safe_mode &= ~CS35L38A_TEMP_WARN;

	}

	if (cs35l38a->pdata.sub_spk_safe_mode & CS35L38A_TEMP_ERR) {
		pr_info("++++>CRUS:temp error release", __func__);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_ERR_RLS, 0);
		ret = regmap_update_bits(cs35l38a->regmap,
				  	CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_ERR_RLS,
					CS35L38A_TEMP_ERR_RLS);
		ret = regmap_update_bits(cs35l38a->regmap,
				    CS35L38A_PROTECT_REL_ERR,
				    CS35L38A_TEMP_ERR_RLS, 0);
		if (ret == 0)
			cs35l38a->pdata.sub_spk_safe_mode &= ~CS35L38A_TEMP_ERR;

	}

	if (cs35l38a->pdata.sub_spk_safe_mode & CS35L38A_BST_OVP_ERR) {
		pr_info("++++>CRUS:VBST Over Voltage error release", __func__);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_ERR_RLS, 0);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_ERR_RLS,
					CS35L38A_TEMP_ERR_RLS);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_TEMP_ERR_RLS, 0);
		if (ret == 0)
			cs35l38a->pdata.sub_spk_safe_mode &= ~CS35L38A_BST_OVP_ERR;

	}

	if (cs35l38a->pdata.sub_spk_safe_mode & CS35L38A_BST_DCM_UVP_ERR) {
		pr_info("++++>CRUS:DCM VBST Under Voltage Error release", __func__);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_BST_UVP_ERR_RLS, 0);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_BST_UVP_ERR_RLS,
					CS35L38A_BST_UVP_ERR_RLS);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_BST_UVP_ERR_RLS, 0);
		if (ret == 0)
			cs35l38a->pdata.sub_spk_safe_mode &= ~CS35L38A_BST_DCM_UVP_ERR;

	}

	if (cs35l38a->pdata.sub_spk_safe_mode & CS35L38A_BST_SHORT_ERR) {
		pr_info("++++>CRUS:LBST SHORT  Error release", __func__);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_BST_SHORT_ERR_RLS, 0);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_BST_SHORT_ERR_RLS,
					CS35L38A_BST_SHORT_ERR_RLS);
		ret = regmap_update_bits(cs35l38a->regmap,
					CS35L38A_PROTECT_REL_ERR,
					CS35L38A_BST_SHORT_ERR_RLS, 0);
		if (ret == 0)
			cs35l38a->pdata.sub_spk_safe_mode &= ~CS35L38A_BST_SHORT_ERR;

	}

	pr_info("++++>CRUS: %s finished, spk safe mode = %d.\n", __func__, cs35l38a->pdata.sub_spk_safe_mode);


	return ret;


}

static int cs35lxx_low_temp_low_battery_protect(struct cs35l38a_private *cs35l38a)
{
	u32 reg = 0;
	int ret = 0;
	int temp_threshold[2] = {-10,-20};
	int batt_threshold[4] = {50,35,30,20};
	int gain_atten_lv[4] = {1,2,4,6}; // dB
	int system_temp = 0;
	int system_battery = 0;
	int temp_decay_gain = 0;
	int temp_digital_gain = 0;//dB
	int delta_gain = 0;
	int i = 0;

	regmap_read(cs35l38a->regmap, CS35L38A_ASP_TX5_SEL, &reg);
	if ((reg >> 28) != 0) {
		// this feature has been disabled, it is enable by default
		pr_debug("++++>CRUS: %s., feature disabled.\n", __func__);
		ret = -1;
		return ret;
	}
	// get battery capcity and temperature infomation
	ret = audio_get_battery_info(BATT_CAP, &system_battery);
	if (ret < 0) {
		pr_debug("++++>CRUS: %s, get capcity failed.\n", __func__);
		return ret;
	}
	ret = audio_get_battery_info(BATT_TEMP, &system_temp);
	if (ret < 0) {
		pr_debug("++++>CRUS: %s, get temp failed.\n", __func__);
		return ret;
	}

	// update configuration firstly if they are re-configured by kcontrols
	regmap_read(cs35l38a->regmap, CS35L38A_ASP_TX1_SEL, &reg);
	if (reg & 0xffff0000) {
		temp_threshold[0] = 0-((reg & 0x00ff0000)>>16);
		temp_threshold[1] = 0-((reg & 0xff000000)>>24);
	}
	pr_debug("++++>CRUS: %s, temp_threshold is [1]:%d [2]:%d.\n", __func__, temp_threshold[0], temp_threshold[1]);

	regmap_read(cs35l38a->regmap, CS35L38A_ASP_TX2_SEL, &reg);
	if (reg & 0xffff0000) {
		batt_threshold[0] = (reg & 0x00ff0000)>>16;
		batt_threshold[1] = (reg & 0xff000000)>>24;
	 }
	regmap_read(cs35l38a->regmap, CS35L38A_ASP_TX3_SEL, &reg);
	if (reg & 0xffff0000) {
		batt_threshold[2] = (reg & 0x00ff0000)>>16;
		batt_threshold[3] = (reg & 0xff000000)>>24;
	}
	pr_debug("--->CRUS: %s, batt_threshold is [1]:%d [2]:%d [3]:%d [4]:%d.\n", __func__,
		batt_threshold[0], batt_threshold[1], batt_threshold[2], batt_threshold[3]);

	regmap_read(cs35l38a->regmap, CS35L38A_ASP_TX4_SEL, &reg);
	if (reg & 0xffffff00) {
		 gain_atten_lv[0] = (reg & 0x000f0000)>>16;
		 gain_atten_lv[1] = (reg & 0x00f00000)>>20;
		 gain_atten_lv[2] = (reg & 0x0f000000)>>24;
		 gain_atten_lv[3] = (reg & 0xf0000000)>>28;
	}
	 pr_debug("--->CRUS: %s, gain_atten_lv is [1]:%d [2]:%d [3]:%d [4]:%d.\n", __func__,
		 gain_atten_lv[0], gain_atten_lv[1], gain_atten_lv[2], gain_atten_lv[3]);

	// get temperature decay gain temperature is lower than the first level
	if ((temp_threshold[1] < system_temp/10) && (system_temp/10 <= temp_threshold[0])) {
		if ((batt_threshold[3] < system_battery) && (system_battery <= batt_threshold[2]))
			temp_decay_gain = gain_atten_lv[0];
		else if(system_battery <= batt_threshold[3])
			temp_decay_gain = gain_atten_lv[1];
	// temperature is lower than the second level
	} else if (system_temp/10 <= temp_threshold[1]) {
		if ((batt_threshold[1] < system_battery) && (system_battery <= batt_threshold[0]))
			temp_decay_gain = gain_atten_lv[2];
		else if(system_battery <= batt_threshold[1])
			temp_decay_gain = gain_atten_lv[3];
	}

	pr_debug("--->CRUS: %s, system_temp is %d,decay_gain :%d dB,  system_battery is %d, \n",
		__func__, system_temp/10, temp_decay_gain, system_battery);
	if (temp_decay_gain != cs35l38a->pdata.final_decay_gain) {
		//calculate the final digital gain value and then adjust it with ramping
		//0x7ff means -0.125 dB, and step size = -0.125 dB so the multipler equals 8
		temp_digital_gain = 0x800 - temp_decay_gain * 8;
		//it can be a positive or negative value compare with the old decay value
		delta_gain = temp_decay_gain - cs35l38a->pdata.final_decay_gain;

		pr_debug("--->CRUS: %s, begin set digital gain(reg 0x6000) value from %d to %d\n",
				__func__, cs35l38a->pdata.final_digital_gain, temp_digital_gain);
		if (temp_decay_gain > cs35l38a->pdata.final_decay_gain) {
			//update the final gain to register
			for(i=1; i<=delta_gain* 8; i++) {
				regmap_update_bits(cs35l38a->regmap, CS35L38A_AMP_DIG_VOL_CTRL,
							   0x3ff8, (cs35l38a->pdata.final_digital_gain - i)<<3);
				usleep_range(10000,10010);
			}

		} else  {
			//update the final gain to register
			for(i=-1; i>=delta_gain* 8; i--) {
				regmap_update_bits(cs35l38a->regmap, CS35L38A_AMP_DIG_VOL_CTRL,
							   0x3ff8, (cs35l38a->pdata.final_digital_gain - i)<<3);
				usleep_range(10000,10010);
			}
		}
		//save current decay gain and digital gain
		cs35l38a->pdata.final_digital_gain = temp_digital_gain;
		cs35l38a->pdata.final_decay_gain = temp_decay_gain;

	}

	regmap_read(cs35l38a->regmap, CS35L38A_AMP_DIG_VOL_CTRL, &reg);
	pr_debug("--->CRUS: %s, final amp digital gain value is %d .\n", __func__, (reg & 0x3ff8) >> 3);
	return ret;
}

static void cs35lxx_monitor_work(struct work_struct *wk)

{
	struct cs35l38a_private *cs35l38a;
	int ret;
	int counter = SLEEP_COUNT_NUM;

	cs35l38a = container_of(container_of(wk, struct cs35l38a_platform_data,
						   cs35l38a_monitor_work_sturct),
						   struct cs35l38a_private, pdata);

		//check temp and battery every 1 second
		while (atomic_read(&cs35l38a->pdata.monitor_enable)) {
			if (counter >= SLEEP_COUNT_NUM) {
				ret = cs35lxx_low_temp_low_battery_protect(cs35l38a);
				if (ret < 0)
					pr_err("++++>CRUS: %s,cs35lxx_low_temp_low_battery_protect failed.\n", __func__);

				ret = cs35lxx_spk_safe_release(cs35l38a);
				if (ret < 0)
					pr_err("++++>CRUS: %s, cs35lxx_spk_safe_release failed.\n", __func__);
				counter = 0;
			}
			usleep_range(100000, 100100);
			counter++;
		}

}
#endif

static int cs35l38a_main_amp_event(struct snd_soc_dapm_widget *w,
				   struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component =
			snd_soc_dapm_to_component(w->dapm);
	struct cs35l38a_private *cs35l38a =
			snd_soc_component_get_drvdata(component);
	u32 reg;
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		atomic_set(&cs35l38a->pdata.monitor_enable, 1);
		queue_work(cs35l38a->pdata.cs35l38a_monitor_work_queue, &cs35l38a->pdata.cs35l38a_monitor_work_sturct);
#endif
		if (!cs35l38a->pdata.extern_boost)
			regmap_update_bits(cs35l38a->regmap, CS35L38A_PWR_CTRL2,
					   CS35L38A_BST_EN_MASK,
						CS35L38A_BST_EN <<
						CS35L38A_BST_EN_SHIFT);

		//for 2terminal load
		regmap_register_patch(cs35l38a->regmap, cs35l38a_2terminal_load_on_patch,
					  ARRAY_SIZE(cs35l38a_2terminal_load_on_patch));

		regmap_update_bits(cs35l38a->regmap, CS35L38A_PWR_CTRL1,
				   CS35L38A_GLOBAL_EN_MASK,
					1 << CS35L38A_GLOBAL_EN_SHIFT);
		usleep_range(2000, 2100);

		regmap_read(cs35l38a->regmap, CS35L38A_INT4_RAW_STATUS, &reg);
		if (reg & CS35L38A_PLL_UNLOCK_MASK)
			dev_crit(cs35l38a->dev, "PLL Unlocked\n");

		regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_RX1_SEL,
				   CS35L38A_PCM_RX_SEL_MASK,
					CS35L38A_PCM_RX_SEL_PCM);
		regmap_update_bits(cs35l38a->regmap, CS35L38A_AMP_OUT_MUTE,
				   CS35L38A_AMP_MUTE_MASK,
					0 << CS35L38A_AMP_MUTE_SHIFT);
		break;
	case SND_SOC_DAPM_POST_PMD:
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		atomic_set(&cs35l38a->pdata.monitor_enable, 0);
		cancel_work_sync(&cs35l38a->pdata.cs35l38a_monitor_work_sturct);
		flush_workqueue(cs35l38a->pdata.cs35l38a_monitor_work_queue);
#endif
		regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_RX1_SEL,
				   CS35L38A_PCM_RX_SEL_MASK,
					CS35L38A_PCM_RX_SEL_ZERO);
		regmap_update_bits(cs35l38a->regmap, CS35L38A_AMP_OUT_MUTE,
				   CS35L38A_AMP_MUTE_MASK,
					1 << CS35L38A_AMP_MUTE_SHIFT);

		if (!cs35l38a->pdata.extern_boost)
			regmap_update_bits(cs35l38a->regmap, CS35L38A_PWR_CTRL2,
					   CS35L38A_BST_EN_MASK,
						CS35L38A_BST_DIS_VP <<
						CS35L38A_BST_EN_SHIFT);

		//for 2terminal load
		regmap_register_patch(cs35l38a->regmap, cs35l38a_2terminal_load_off_patch,
					  ARRAY_SIZE(cs35l38a_2terminal_load_off_patch));

		regmap_update_bits(cs35l38a->regmap, CS35L38A_PWR_CTRL1,
				   CS35L38A_GLOBAL_EN_MASK,
					0 << CS35L38A_GLOBAL_EN_SHIFT);
		usleep_range(2000, 2100);
		break;
	default:
		dev_dbg(component->dev, "Invalid event = 0x%x\n", event);
	}
	return ret;
}

static const char * const cs35l38a_chan_text[] = {
	"RX1",
	"RX2",
};

static SOC_ENUM_SINGLE_DECL(chansel_enum, CS35L38A_ASP_RX1_SLOT, 0,
		cs35l38a_chan_text);

static const struct snd_kcontrol_new cs35l38a_chan_mux[] = {
	SOC_DAPM_ENUM("Input Mux", chansel_enum),
};

static const struct snd_kcontrol_new amp_enable_ctrl =
		SOC_DAPM_SINGLE("Switch", SND_SOC_NOPM, 0, 1, 0);

static const char * const asp_tx_src_text[] = {
	"Zero Fill", "ASPRX1", "VMON", "IMON",
	"ERRVOL", "VPMON", "VBSTMON"
};

static const unsigned int asp_tx_src_values[] = {
	0x00, 0x08, 0x18, 0x19, 0x20, 0x28, 0x29
};

static SOC_VALUE_ENUM_SINGLE_DECL(asp_tx1_src_enum,
				CS35L38A_ASP_TX1_SEL, 0,
				CS35L38A_APS_TX_SEL_MASK,
				asp_tx_src_text,
				asp_tx_src_values);

static const struct snd_kcontrol_new asp_tx1_src =
	SOC_DAPM_ENUM("ASPTX1SRC", asp_tx1_src_enum);

static SOC_VALUE_ENUM_SINGLE_DECL(asp_tx2_src_enum,
				CS35L38A_ASP_TX2_SEL, 0,
				CS35L38A_APS_TX_SEL_MASK,
				asp_tx_src_text,
				asp_tx_src_values);

static const struct snd_kcontrol_new asp_tx2_src =
	SOC_DAPM_ENUM("ASPTX2SRC", asp_tx2_src_enum);

static SOC_VALUE_ENUM_SINGLE_DECL(asp_tx3_src_enum,
				CS35L38A_ASP_TX3_SEL, 0,
				CS35L38A_APS_TX_SEL_MASK,
				asp_tx_src_text,
				asp_tx_src_values);

static const struct snd_kcontrol_new asp_tx3_src =
	SOC_DAPM_ENUM("ASPTX3SRC", asp_tx3_src_enum);

static SOC_VALUE_ENUM_SINGLE_DECL(asp_tx4_src_enum,
				CS35L38A_ASP_TX4_SEL, 0,
				CS35L38A_APS_TX_SEL_MASK,
				asp_tx_src_text,
				asp_tx_src_values);

static const struct snd_kcontrol_new asp_tx4_src =
	SOC_DAPM_ENUM("ASPTX4SRC", asp_tx4_src_enum);

static SOC_VALUE_ENUM_SINGLE_DECL(asp_tx5_src_enum,
				CS35L38A_ASP_TX5_SEL, 0,
				CS35L38A_APS_TX_SEL_MASK,
				asp_tx_src_text,
				asp_tx_src_values);

static const struct snd_kcontrol_new asp_tx5_src =
	SOC_DAPM_ENUM("ASPTX5SRC", asp_tx5_src_enum);

static SOC_VALUE_ENUM_SINGLE_DECL(asp_tx6_src_enum,
				CS35L38A_ASP_TX6_SEL, 0,
				CS35L38A_APS_TX_SEL_MASK,
				asp_tx_src_text,
				asp_tx_src_values);

static const struct snd_kcontrol_new asp_tx6_src =
	SOC_DAPM_ENUM("ASPTX6SRC", asp_tx6_src_enum);

static const struct snd_soc_dapm_widget cs35l38a_dapm_widgets[] = {
	SND_SOC_DAPM_MUX("Channel Mux", SND_SOC_NOPM, 0, 0, cs35l38a_chan_mux),
	SND_SOC_DAPM_AIF_IN("SDIN", NULL, 0, CS35L38A_ASP_RX_TX_EN, 16, 0),

	SND_SOC_DAPM_OUT_DRV_E("Main AMP", CS35L38A_PWR_CTRL2, 0, 0, NULL, 0,
			       cs35l38a_main_amp_event, SND_SOC_DAPM_POST_PMD |
				SND_SOC_DAPM_POST_PMU),

	SND_SOC_DAPM_OUTPUT("SPK"),
	SND_SOC_DAPM_SWITCH("AMP Enable", SND_SOC_NOPM, 0, 1, &amp_enable_ctrl),

	SND_SOC_DAPM_AIF_OUT("ASPTX1", NULL, 0, CS35L38A_ASP_RX_TX_EN, 0, 0),
	SND_SOC_DAPM_AIF_OUT("ASPTX2", NULL, 1, CS35L38A_ASP_RX_TX_EN, 1, 0),
	SND_SOC_DAPM_AIF_OUT("ASPTX3", NULL, 2, CS35L38A_ASP_RX_TX_EN, 2, 0),
	SND_SOC_DAPM_AIF_OUT("ASPTX4", NULL, 3, CS35L38A_ASP_RX_TX_EN, 3, 0),
	SND_SOC_DAPM_AIF_OUT("ASPTX5", NULL, 4, CS35L38A_ASP_RX_TX_EN, 4, 0),
	SND_SOC_DAPM_AIF_OUT("ASPTX6", NULL, 5, CS35L38A_ASP_RX_TX_EN, 5, 0),

	SND_SOC_DAPM_MUX("ASPTX1SRC", SND_SOC_NOPM, 0, 0, &asp_tx1_src),
	SND_SOC_DAPM_MUX("ASPTX2SRC", SND_SOC_NOPM, 0, 0, &asp_tx2_src),
	SND_SOC_DAPM_MUX("ASPTX3SRC", SND_SOC_NOPM, 0, 0, &asp_tx3_src),
	SND_SOC_DAPM_MUX("ASPTX4SRC", SND_SOC_NOPM, 0, 0, &asp_tx4_src),
	SND_SOC_DAPM_MUX("ASPTX5SRC", SND_SOC_NOPM, 0, 0, &asp_tx5_src),
	SND_SOC_DAPM_MUX("ASPTX6SRC", SND_SOC_NOPM, 0, 0, &asp_tx6_src),

	SND_SOC_DAPM_ADC("VMON ADC", NULL, CS35L38A_PWR_CTRL2, 12, 0),
	SND_SOC_DAPM_ADC("IMON ADC", NULL, CS35L38A_PWR_CTRL2, 13, 0),
	SND_SOC_DAPM_ADC("VPMON ADC", NULL, CS35L38A_PWR_CTRL2, 8, 0),
	SND_SOC_DAPM_ADC("VBSTMON ADC", NULL, CS35L38A_PWR_CTRL2, 9, 0),
	SND_SOC_DAPM_ADC("CLASS H", NULL, CS35L38A_PWR_CTRL3, 4, 0),

	SND_SOC_DAPM_INPUT("VP"),
	SND_SOC_DAPM_INPUT("VBST"),
	SND_SOC_DAPM_INPUT("VSENSE"),
};

static const struct snd_soc_dapm_route cs35l38a_audio_map[] = {
	{"VPMON ADC", NULL, "VP"},
	{"VBSTMON ADC", NULL, "VBST"},
	{"IMON ADC", NULL, "VSENSE"},
	{"VMON ADC", NULL, "VSENSE"},

	{"ASPTX1SRC", "IMON", "IMON ADC"},
	{"ASPTX1SRC", "VMON", "VMON ADC"},
	{"ASPTX1SRC", "VBSTMON", "VBSTMON ADC"},
	{"ASPTX1SRC", "VPMON", "VPMON ADC"},

	{"ASPTX2SRC", "IMON", "IMON ADC"},
	{"ASPTX2SRC", "VMON", "VMON ADC"},
	{"ASPTX2SRC", "VBSTMON", "VBSTMON ADC"},
	{"ASPTX2SRC", "VPMON", "VPMON ADC"},

	{"ASPTX3SRC", "IMON", "IMON ADC"},
	{"ASPTX3SRC", "VMON", "VMON ADC"},
	{"ASPTX3SRC", "VBSTMON", "VBSTMON ADC"},
	{"ASPTX3SRC", "VPMON", "VPMON ADC"},

	{"ASPTX4SRC", "IMON", "IMON ADC"},
	{"ASPTX4SRC", "VMON", "VMON ADC"},
	{"ASPTX4SRC", "VBSTMON", "VBSTMON ADC"},
	{"ASPTX4SRC", "VPMON", "VPMON ADC"},

	{"ASPTX5SRC", "IMON", "IMON ADC"},
	{"ASPTX5SRC", "VMON", "VMON ADC"},
	{"ASPTX5SRC", "VBSTMON", "VBSTMON ADC"},
	{"ASPTX5SRC", "VPMON", "VPMON ADC"},

	{"ASPTX6SRC", "IMON", "IMON ADC"},
	{"ASPTX6SRC", "VMON", "VMON ADC"},
	{"ASPTX6SRC", "VBSTMON", "VBSTMON ADC"},
	{"ASPTX6SRC", "VPMON", "VPMON ADC"},

	{"ASPTX1", NULL, "ASPTX1SRC"},
	{"ASPTX2", NULL, "ASPTX2SRC"},
	{"ASPTX3", NULL, "ASPTX3SRC"},
	{"ASPTX4", NULL, "ASPTX4SRC"},
	{"ASPTX5", NULL, "ASPTX5SRC"},
	{"ASPTX6", NULL, "ASPTX6SRC"},

	{"AMP Capture", NULL, "ASPTX1"},
	{"AMP Capture", NULL, "ASPTX2"},
	{"AMP Capture", NULL, "ASPTX3"},
	{"AMP Capture", NULL, "ASPTX4"},
	{"AMP Capture", NULL, "ASPTX5"},
	{"AMP Capture", NULL, "ASPTX6"},

	{"AMP Enable", "Switch", "AMP Playback"},
	{"SDIN", NULL, "AMP Enable"},
	{"Channel Mux", "RX1", "SDIN"},
	{"Channel Mux", "RX2", "SDIN"},
	{"CLASS H", NULL, "SDIN"},
	{"Main AMP", NULL, "CLASS H"},
	{"SPK", NULL, "Main AMP"},
};

static int cs35l38a_set_dai_fmt(struct snd_soc_dai *component_dai, unsigned int fmt)
{
	struct cs35l38a_private *cs35l38a =
			snd_soc_component_get_drvdata(component_dai->component);
	unsigned int asp_fmt, lrclk_fmt, sclk_fmt, slave_mode;

	switch (fmt & SND_SOC_DAIFMT_MASTER_MASK) {
	case SND_SOC_DAIFMT_CBM_CFM:
		slave_mode = 1;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		slave_mode = 0;
		break;
	default:
		return -EINVAL;
	}
	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_TX_PIN_CTRL,
			   CS35L38A_SCLK_MSTR_MASK,
				slave_mode << CS35L38A_SCLK_MSTR_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_RATE_CTRL,
			   CS35L38A_LRCLK_MSTR_MASK,
				slave_mode << CS35L38A_LRCLK_MSTR_SHIFT);

	switch (fmt & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_DSP_A:
		asp_fmt = 0;
		break;
	case SND_SOC_DAIFMT_I2S:
		asp_fmt = 2;
		break;
	default:
		return -EINVAL;
	}

	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_IF:
		lrclk_fmt = 1;
		sclk_fmt = 0;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		/*
		 * TDM 1.5 mode must invert bitclk
		 */
		if (asp_fmt == 0)
			asp_fmt = 4;
		lrclk_fmt = 0;
		sclk_fmt = 1;
		break;
	case SND_SOC_DAIFMT_IB_IF:
		lrclk_fmt = 1;
		sclk_fmt = 1;
		break;
	case SND_SOC_DAIFMT_NB_NF:
		lrclk_fmt = 0;
		sclk_fmt = 0;
		break;
	default:
		return -EINVAL;
	}

	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_RATE_CTRL,
			   CS35L38A_LRCLK_INV_MASK,
				lrclk_fmt << CS35L38A_LRCLK_INV_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_TX_PIN_CTRL,
			   CS35L38A_SCLK_INV_MASK,
				sclk_fmt << CS35L38A_SCLK_INV_SHIFT);

	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_FORMAT,
			   CS35L38A_ASP_FMT_MASK, asp_fmt);

	return 0;
}

struct cs35l38a_global_fs_config {
	int rate;
	int fs_cfg;
};

static struct cs35l38a_global_fs_config cs35l38a_fs_rates[] = {
	{12000, 0x01},
	{24000, 0x02},
	{48000, 0x03},
	{96000, 0x04},
	{192000, 0x05},
	{384000, 0x06},
	{11025, 0x09},
	{22050, 0x0A},
	{44100, 0x0B},
	{88200, 0x0C},
	{176400, 0x0D},
	{8000, 0x11},
	{16000, 0x12},
	{32000, 0x13},
};

static int cs35l38a_pcm_hw_params(struct snd_pcm_substream *substream,
                                       struct snd_pcm_hw_params *params,
                                     struct snd_soc_dai *dai)
{
         struct cs35l38a_private *cs35l38a =
                   snd_soc_component_get_drvdata(dai->component);
         int i;
         unsigned int global_fs = params_rate(params);
         unsigned int asp_width;
         pr_debug("++++>CRUS: dai stream=%d[rx:0,tx:1], bit_width=%d, sample_rate=%d\n",
                   substream->stream, params_width(params), global_fs);

         for (i = 0; i < ARRAY_SIZE(cs35l38a_fs_rates); i++) {
                   if (global_fs == cs35l38a_fs_rates[i].rate)
                            regmap_update_bits(cs35l38a->regmap,
                                                  CS35L38A_GLOBAL_CLK_CTRL,
                                               CS35L38A_GLOBAL_FS_MASK,
                                               cs35l38a_fs_rates[i].fs_cfg <<
                                               CS35L38A_GLOBAL_FS_SHIFT);
         }

         switch (params_width(params)) {
         case 16:
                   asp_width = CS35L38A_ASP_WIDTH_16;
                   break;
         case 24:
                   asp_width = CS35L38A_ASP_WIDTH_24;
                   break;
         case 32:
                   asp_width = CS35L38A_ASP_WIDTH_32;
                   break;
         default:
                   return -EINVAL;
         }

         //for system clock
         if (asp_width > CS35L38A_ASP_WIDTH_16) {
                   //for I2S stereo PA, if 24 bits slot width, then 12 bits for each PA TX Vmon/Imon valid data
                   //if 32 bits slot width, then 16 bits for each PA TX Vmon/Imon valid data
                   asp_width = asp_width/2;
                   regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_FRAME_CTRL,
                                        CS35L38A_ASP_TX_WIDTH_MASK,
                                        asp_width << CS35L38A_ASP_TX_WIDTH_SHIFT);
                   //for I2S stereo PA, 24 bits valid data for RX --24/32 mode
                   asp_width = CS35L38A_ASP_WIDTH_32;
                  cs35l38a_component_set_sysclk(dai->component, 0, 0, 2 * global_fs * asp_width, 0);
                   regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_FRAME_CTRL,
                                        CS35L38A_ASP_RX_WIDTH_MASK,
                                        asp_width << CS35L38A_ASP_RX_WIDTH_SHIFT);

         } else {
                            cs35l38a_component_set_sysclk(dai->component, 0, 0, 2 * global_fs * asp_width, 0);
                            regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_FRAME_CTRL,
                                                  CS35L38A_ASP_RX_WIDTH_MASK,
                                                  asp_width << CS35L38A_ASP_RX_WIDTH_SHIFT);
                            regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_FRAME_CTRL,
                                                  CS35L38A_ASP_TX_WIDTH_MASK,
                                                  asp_width << CS35L38A_ASP_TX_WIDTH_SHIFT);

         }



         return 0;
}

static int cs35l38a_dai_set_sysclk(struct snd_soc_dai *dai,
				   int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_component *component = dai->component;
	struct cs35l38a_private *cs35l38a =
			snd_soc_component_get_drvdata(component);
	int fs1_val = 0;
	int fs2_val = 0;

	/* Need the SCLK Frequency regardless of sysclk source */
	cs35l38a->sclk = freq;

	if (cs35l38a->sclk > 6000000) {
		fs1_val = 3 * 4 + 4;
		fs2_val = 8 * 4 + 4;
	}

	if (cs35l38a->sclk <= 6000000) {
		fs1_val = 3 * ((24000000 + cs35l38a->sclk - 1) /
			       cs35l38a->sclk) + 4;
		fs2_val = 5 * ((24000000 + cs35l38a->sclk - 1) /
			       cs35l38a->sclk) + 4;
	}
	regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
		     CS35L38A_TEST_UNLOCK1);
	regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
		     CS35L38A_TEST_UNLOCK2);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_TST_FS_MON0,
			   CS35L38A_FS1_WINDOW_MASK, fs1_val);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_TST_FS_MON0,
			   CS35L38A_FS2_WINDOW_MASK, fs2_val <<
		CS35L38A_FS2_WINDOW_SHIFT);
	regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
		     CS35L38A_TEST_LOCK1);
	regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
		     CS35L38A_TEST_LOCK2);
	return 0;
}

static int cs35l38a_get_clk_config(struct cs35l38a_private *cs35l38a, int freq)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(cs35l38a_pll_sysclk); i++) {
		if (cs35l38a_pll_sysclk[i].freq == freq) {
			cs35l38a->extclk_cfg = cs35l38a_pll_sysclk[i].clk_cfg;
			cs35l38a->fll_igain = cs35l38a_pll_sysclk[i].fll_igain;
			return i;
		}
	}

	return -EINVAL;
}

static const unsigned int cs35l38a_src_rates[] = {
	8000, 12000, 11025, 16000, 22050, 24000, 32000,
	44100, 48000, 88200, 96000, 176400, 192000, 384000
};

static const struct snd_pcm_hw_constraint_list cs35l38a_constraints = {
	.count  = ARRAY_SIZE(cs35l38a_src_rates),
	.list   = cs35l38a_src_rates,
};

static int cs35l38a_pcm_startup(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	if (!substream->runtime)
		return 0;

	snd_pcm_hw_constraint_list(substream->runtime, 0,
				   SNDRV_PCM_HW_PARAM_RATE,
				   &cs35l38a_constraints);
	return 0;
}

static const struct snd_soc_dai_ops cs35l38a_ops = {
	.startup = cs35l38a_pcm_startup,
	.set_fmt = cs35l38a_set_dai_fmt,
	.hw_params = cs35l38a_pcm_hw_params,
	.set_sysclk = cs35l38a_dai_set_sysclk,
};

static struct snd_soc_dai_driver cs35l38a_dai[] = {
	{
		.name = "cs35l38a-pcm",
		.id = 0,
		.playback = {
			.stream_name = "AMP Playback",
			.channels_min = 1,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_KNOT,
			.formats = CS35L38A_RX_FORMATS,
		},
		.capture = {
			.stream_name = "AMP Capture",
			.channels_min = 1,
			.channels_max = 8,
			.rates = SNDRV_PCM_RATE_KNOT,
			.formats = CS35L38A_TX_FORMATS,
		},
		.ops = &cs35l38a_ops,
		.symmetric_rates = 1,
	},
};

int cs35l38a_component_set_sysclk(struct snd_soc_component *component,
				     int clk_id, int source, unsigned int freq,
				int dir)
{
	struct cs35l38a_private *cs35l38a =
			snd_soc_component_get_drvdata(component);
	int ret;

	cs35l38a->extclk_freq = freq;

	cs35l38a->prev_clksrc = cs35l38a->clksrc;

	switch (clk_id) {
	case 0:
		cs35l38a->clksrc = CS35L38A_PLLSRC_SCLK;
		break;
	case 1:
		cs35l38a->clksrc = CS35L38A_PLLSRC_LRCLK;
		break;
	case 2:
		cs35l38a->clksrc = CS35L38A_PLLSRC_PDMCLK;
		break;
	case 3:
		cs35l38a->clksrc = CS35L38A_PLLSRC_SELF;
		break;
	case 4:
		cs35l38a->clksrc = CS35L38A_PLLSRC_MCLK;
		break;
	default:
		return -EINVAL;
	}

	ret = cs35l38a_get_clk_config(cs35l38a, freq);

	if (ret < 0) {
		dev_err(component->dev,
			"Invalid CLK Config Freq: %d\n", freq);
		return -EINVAL;
	}

	regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_CLK_CTRL,
			   CS35L38A_PLL_OPENLOOP_MASK,
					1 << CS35L38A_PLL_OPENLOOP_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_CLK_CTRL,
			   CS35L38A_REFCLK_FREQ_MASK,
			cs35l38a->extclk_cfg << CS35L38A_REFCLK_FREQ_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_CLK_CTRL,
			   CS35L38A_PLL_REFCLK_EN_MASK,
				0 << CS35L38A_PLL_REFCLK_EN_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_CLK_CTRL,
			   CS35L38A_PLL_CLK_SEL_MASK, cs35l38a->clksrc);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_CLK_CTRL,
			   CS35L38A_PLL_OPENLOOP_MASK,
					0 << CS35L38A_PLL_OPENLOOP_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_CLK_CTRL,
			   CS35L38A_PLL_REFCLK_EN_MASK,
				1 << CS35L38A_PLL_REFCLK_EN_SHIFT);

	if (cs35l38a->rev_id == CS35L38A_REV_A0) {
		regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
			     CS35L38A_TEST_UNLOCK1);
		regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
			     CS35L38A_TEST_UNLOCK2);
		regmap_write(cs35l38a->regmap, CS35L38A_DCO_CTRL, 0x00036DA8);
		regmap_write(cs35l38a->regmap, CS35L38A_MISC_CTRL, 0x0100EE0E);
		regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_LOOP_PARAMS,
				   CS35L38A_PLL_IGAIN_MASK,
					CS35L38A_PLL_IGAIN <<
					CS35L38A_PLL_IGAIN_SHIFT);
		regmap_update_bits(cs35l38a->regmap, CS35L38A_PLL_LOOP_PARAMS,
				   CS35L38A_PLL_FFL_IGAIN_MASK,
					cs35l38a->fll_igain);
		regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
			     CS35L38A_TEST_LOCK1);
		regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
			     CS35L38A_TEST_LOCK2);
	}

	if (cs35l38a->clksrc == CS35L38A_PLLSRC_PDMCLK) {
		if (cs35l38a->pdata.ldm_mode_sel) {
			if (cs35l38a->prev_clksrc != CS35L38A_PLLSRC_PDMCLK)
				regmap_update_bits(cs35l38a->regmap,
						   CS35L38A_NG_CFG,
						CS35L38A_NG_DELAY_MASK,
						0 << CS35L38A_NG_DELAY_SHIFT);
		}
		regmap_update_bits(cs35l38a->regmap, CS35L38A_DAC_MSM_CFG,
				   CS35L38A_PDM_MODE_MASK,
					1 << CS35L38A_PDM_MODE_SHIFT);
		if (cs35l38a->pdata.ldm_mode_sel) {
			if (cs35l38a->prev_clksrc != CS35L38A_PLLSRC_PDMCLK) {
				regmap_update_bits(cs35l38a->regmap,
						   CS35L38A_NG_CFG,
						CS35L38A_NG_DELAY_MASK,
						3 << CS35L38A_NG_DELAY_SHIFT);
			}
		}
	} else {
		if (cs35l38a->pdata.ldm_mode_sel) {
			if (cs35l38a->prev_clksrc == CS35L38A_PLLSRC_PDMCLK)
				regmap_update_bits(cs35l38a->regmap,
						   CS35L38A_NG_CFG,
						CS35L38A_NG_DELAY_MASK,
						0 << CS35L38A_NG_DELAY_SHIFT);
		}
		regmap_update_bits(cs35l38a->regmap, CS35L38A_DAC_MSM_CFG,
				   CS35L38A_PDM_MODE_MASK,
					0 << CS35L38A_PDM_MODE_SHIFT);
		if (cs35l38a->pdata.ldm_mode_sel) {
			if (cs35l38a->prev_clksrc == CS35L38A_PLLSRC_PDMCLK) {
				regmap_update_bits(cs35l38a->regmap,
						   CS35L38A_NG_CFG,
						CS35L38A_NG_DELAY_MASK,
						3 << CS35L38A_NG_DELAY_SHIFT);
			}
		}
	}

	return 0;
}

static int cs35l38a_boost_inductor(struct cs35l38a_private *cs35l38a,
				   int inductor)
{
	regmap_update_bits(cs35l38a->regmap, CS35L38A_BSTCVRT_COEFF,
			   CS35L38A_BSTCVRT_K1_MASK, 0x24);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_BSTCVRT_COEFF,
			   CS35L38A_BSTCVRT_K2_MASK,
					0x24 << CS35L38A_BSTCVRT_K2_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_BSTCVRT_SW_FREQ,
			   CS35L38A_BSTCVRT_CCMFREQ_MASK, 0x00);

	switch (inductor) {
	case 1000: /* 1 uH */
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_BSTCVRT_SLOPE_LBST,
				   CS35L38A_BSTCVRT_SLOPE_MASK,
					0x75 << CS35L38A_BSTCVRT_SLOPE_SHIFT);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_BSTCVRT_SLOPE_LBST,
				   CS35L38A_BSTCVRT_LBSTVAL_MASK, 0x00);
		break;
	case 1200: /* 1.2 uH */
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_BSTCVRT_SLOPE_LBST,
				   CS35L38A_BSTCVRT_SLOPE_MASK,
					0x6B << CS35L38A_BSTCVRT_SLOPE_SHIFT);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_BSTCVRT_SLOPE_LBST,
				   CS35L38A_BSTCVRT_LBSTVAL_MASK, 0x01);
		break;
	default:
		dev_err(cs35l38a->dev, "%s Invalid Inductor Value %d uH\n",
			__func__, inductor);
		return -EINVAL;
	}
	return 0;
}

static int cs35l38a_component_probe(struct snd_soc_component *component)
{
	struct cs35l38a_private *cs35l38a =
			snd_soc_component_get_drvdata(component);
	int ret = 0;
	struct snd_soc_dapm_context *dapm = snd_soc_component_get_dapm(component);

	if (cs35l38a->pdata.sclk_frc)
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_ASP_TX_PIN_CTRL,
				CS35L38A_SCLK_FRC_MASK,
				cs35l38a->pdata.sclk_frc <<
				CS35L38A_SCLK_FRC_SHIFT);

	if (cs35l38a->pdata.lrclk_frc)
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_ASP_RATE_CTRL,
				CS35L38A_LRCLK_FRC_MASK,
				cs35l38a->pdata.lrclk_frc <<
				CS35L38A_LRCLK_FRC_SHIFT);

	if (cs35l38a->rev_id == CS35L38A_REV_A0) {
		if (cs35l38a->pdata.dcm_mode) {
			regmap_update_bits(cs35l38a->regmap,
					   CS35L38A_BSTCVRT_DCM_CTRL,
						CS35L38A_DCM_AUTO_MASK,
						CS35L38A_DCM_AUTO_MASK);
			regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
				     CS35L38A_TEST_UNLOCK1);
			regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
				     CS35L38A_TEST_UNLOCK2);
			regmap_update_bits(cs35l38a->regmap,
					   CS35L38A_BST_TST_MANUAL,
					CS35L38A_BST_MAN_IPKCOMP_MASK,
					0 << CS35L38A_BST_MAN_IPKCOMP_SHIFT);
			regmap_update_bits(cs35l38a->regmap,
					   CS35L38A_BST_TST_MANUAL,
					CS35L38A_BST_MAN_IPKCOMP_EN_MASK,
					CS35L38A_BST_MAN_IPKCOMP_EN_MASK);
			regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
				     CS35L38A_TEST_LOCK1);
			regmap_write(cs35l38a->regmap, CS35L38A_TESTKEY_CTRL,
				     CS35L38A_TEST_LOCK2);
		}
	}

	if (cs35l38a->pdata.amp_gain_zc)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_AMP_GAIN_CTRL,
				   CS35L38A_AMP_ZC_MASK,
					CS35L38A_AMP_ZC_MASK);

	if (cs35l38a->pdata.amp_pcm_inv)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_AMP_DIG_VOL_CTRL,
				   CS35L38A_AMP_PCM_INV_MASK,
					CS35L38A_AMP_PCM_INV_MASK);

	if (cs35l38a->pdata.ldm_mode_sel)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_NG_CFG,
				   CS35L38A_NG_AMP_EN_MASK,
					CS35L38A_NG_AMP_EN_MASK);

	if (cs35l38a->pdata.multi_amp_mode)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_TX_PIN_CTRL,
				   CS35L38A_ASP_TX_HIZ_MASK,
					CS35L38A_ASP_TX_HIZ_MASK);

	if (cs35l38a->pdata.pdm_ldm_enter)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_DAC_MSM_CFG,
				   CS35L38A_PDM_LDM_ENTER_MASK,
					CS35L38A_PDM_LDM_ENTER_MASK);

	if (cs35l38a->pdata.pdm_ldm_exit)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_DAC_MSM_CFG,
				   CS35L38A_PDM_LDM_EXIT_MASK,
					CS35L38A_PDM_LDM_EXIT_MASK);

	if (cs35l38a->pdata.imon_pol_inv)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_VI_SPKMON_FILT,
				   CS35L38A_IMON_POL_MASK, 0);

	if (cs35l38a->pdata.vmon_pol_inv)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_VI_SPKMON_FILT,
				   CS35L38A_VMON_POL_MASK, 0);

	if (cs35l38a->pdata.bst_vctl)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_BSTCVRT_VCTRL1,
				   CS35L35_BSTCVRT_CTL_MASK,
				cs35l38a->pdata.bst_vctl);

	if (cs35l38a->pdata.bst_vctl_sel)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_BSTCVRT_VCTRL2,
				   CS35L35_BSTCVRT_CTL_SEL_MASK,
				cs35l38a->pdata.bst_vctl_sel);

	if (cs35l38a->pdata.bst_ipk)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_BSTCVRT_PEAK_CUR,
				   CS35L38A_BST_IPK_MASK,
				cs35l38a->pdata.bst_ipk);

	if (cs35l38a->pdata.boost_ind)
		ret = cs35l38a_boost_inductor(cs35l38a,
					      cs35l38a->pdata.boost_ind);

	if (cs35l38a->pdata.temp_warn_thld)
		regmap_update_bits(cs35l38a->regmap, CS35L38A_DTEMP_WARN_THLD,
				   CS35L38A_TEMP_THLD_MASK,
					cs35l38a->pdata.temp_warn_thld);

	//for mono spk case
	snd_soc_dapm_ignore_suspend(dapm, "AMP Playback");
	snd_soc_dapm_ignore_suspend(dapm, "AMP Capture");
	snd_soc_dapm_ignore_suspend(dapm, "Main AMP");
	snd_soc_dapm_ignore_suspend(dapm, "VP");
	snd_soc_dapm_ignore_suspend(dapm, "VBST");
	snd_soc_dapm_ignore_suspend(dapm, "SDIN");
	snd_soc_dapm_ignore_suspend(dapm, "SDOUT");
	snd_soc_dapm_ignore_suspend(dapm, "SPK");
	snd_soc_dapm_ignore_suspend(dapm, "ISENSE");
	snd_soc_dapm_ignore_suspend(dapm, "VSENSE");
	if (cs35l38a->pdata.multi_amp_mode) {
		//current for stereo case
		snd_soc_dapm_ignore_suspend(dapm, "R AMP Playback");
		snd_soc_dapm_ignore_suspend(dapm, "R AMP Capture");
		snd_soc_dapm_ignore_suspend(dapm, "R Main AMP");
		snd_soc_dapm_ignore_suspend(dapm, "R VP");
		snd_soc_dapm_ignore_suspend(dapm, "R VBST");
		snd_soc_dapm_ignore_suspend(dapm, "R SDIN");
		snd_soc_dapm_ignore_suspend(dapm, "R SDOUT");
		snd_soc_dapm_ignore_suspend(dapm, "R SPK");
		snd_soc_dapm_ignore_suspend(dapm, "R ISENSE");
		snd_soc_dapm_ignore_suspend(dapm, "R VSENSE");
	}
	snd_soc_dapm_sync(dapm);

#ifdef CS35LXX_MONITOR_WORK_ENABLE
	// default Temp Threshold 1 = -10, Temp Threshold 2 = -20
	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_TX1_SEL,
			   0xffff0000, 0x140a0000);
	// default battery gauge Threshold 1 = 50%, Threshold 2 = 35%,
	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_TX2_SEL,
		   0xffff0000, 0x23320000);
	// default battery gauge Threshold 3 = 30%, Threshold 4 = 20%,
	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_TX3_SEL,
		   0xffff0000, 0x141e0000);
	// default battery attenuation level 1, 2, 3, 4= 1, 2, 4,6 dB
	regmap_update_bits(cs35l38a->regmap, CS35L38A_ASP_TX4_SEL,
		   0xffffff00, 0x64210000);
	cs35l38a->pdata.sub_spk_safe_mode = 0;
	cs35l38a->pdata.final_digital_gain = 0;
	atomic_set(&cs35l38a->pdata.monitor_enable, 0);
	cs35l38a->pdata.cs35l38a_monitor_work_queue =
		create_singlethread_workqueue("cs35lxx_monitor_work");
	INIT_WORK(&cs35l38a->pdata.cs35l38a_monitor_work_sturct, cs35lxx_monitor_work);
#endif
	return 0;
}

static struct snd_soc_component_driver soc_component_dev_cs35l38a = {
	.probe = &cs35l38a_component_probe,
	.set_sysclk = cs35l38a_component_set_sysclk,
		.dapm_widgets = cs35l38a_dapm_widgets,
		.num_dapm_widgets = ARRAY_SIZE(cs35l38a_dapm_widgets),

		.dapm_routes = cs35l38a_audio_map,
		.num_dapm_routes = ARRAY_SIZE(cs35l38a_audio_map),
		.controls = cs35l38a_aud_controls,
		.num_controls = ARRAY_SIZE(cs35l38a_aud_controls),
	//.ignore_pmdown_time = true,
};

static struct regmap_config cs35l38a_regmap = {
	.reg_bits = 32,
	.val_bits = 32,
	.reg_stride = 4,
	.max_register = CS35L38A_PAC_PMEM_WORD1023,
	.reg_defaults = cs35l38a_reg,
	.num_reg_defaults = ARRAY_SIZE(cs35l38a_reg),
	.volatile_reg = cs35l38a_volatile_reg,
	.readable_reg = cs35l38a_readable_reg,
	.cache_type = REGCACHE_RBTREE,
};

static irqreturn_t cs35l38a_irq(int irq, void *data)
{
	struct cs35l38a_private *cs35l38a = data;
	unsigned int status[4];
	unsigned int masks[4];
	unsigned int i;

	/* ack the irq by reading all status registers */
	for (i = 0; i < ARRAY_SIZE(status); i++) {
		regmap_read(cs35l38a->regmap, CS35L38A_INT1_STATUS + i * 4,
			    &status[i]);
		regmap_read(cs35l38a->regmap, CS35L38A_INT1_MASK + i * 4,
			    &masks[i]);
	}

	/* Check to see if unmasked bits are active */
	if (!(status[0] & ~masks[0]) && !(status[1] & ~masks[1]) &&
	    !(status[2] & ~masks[2]) && !(status[3] & ~masks[3])) {
		return IRQ_NONE;
	}

	if (status[3] & 1 << CS35L38A_INT4_INIT_SHFT) {
		complete(&cs35l38a->init_done);
		return IRQ_HANDLED;
	}

	/*
	 * The following interrupts require a
	 * protection release cycle to get the
	 * speaker out of Safe-Mode.
	 */
	if (status[2] & CS35L38A_AMP_SHORT_ERR) {
		dev_crit(cs35l38a->dev, "Amp short error\n");
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_AMP_SHORT_ERR_RLS, 0);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_AMP_SHORT_ERR_RLS,
				CS35L38A_AMP_SHORT_ERR_RLS);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_AMP_SHORT_ERR_RLS, 0);
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		cs35l38a->pdata.sub_spk_safe_mode |= CS35L38A_AMP_SHORT_ERR;
#endif
		regmap_write(cs35l38a->regmap, CS35L38A_INT3_STATUS,
			     CS35L38A_AMP_SHORT_ERR);
	}

	if (status[0] & CS35L38A_TEMP_WARN) {
		dev_crit(cs35l38a->dev, "Over temperature warning\n");
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_WARN_ERR_RLS, 0);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_WARN_ERR_RLS,
				CS35L38A_TEMP_WARN_ERR_RLS);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_WARN_ERR_RLS, 0);
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		cs35l38a->pdata.sub_spk_safe_mode |= CS35L38A_TEMP_WARN;
#endif
		regmap_write(cs35l38a->regmap, CS35L38A_INT1_STATUS,
			     CS35L38A_TEMP_WARN);
	}

	if (status[0] & CS35L38A_TEMP_ERR) {
		dev_crit(cs35l38a->dev, "Over temperature error\n");
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_ERR_RLS, 0);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_ERR_RLS,
				CS35L38A_TEMP_ERR_RLS);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_ERR_RLS, 0);
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		cs35l38a->pdata.sub_spk_safe_mode |= CS35L38A_TEMP_ERR;
#endif
		regmap_write(cs35l38a->regmap, CS35L38A_INT1_STATUS,
			     CS35L38A_TEMP_ERR);
	}

	if (status[0] & CS35L38A_BST_OVP_ERR) {
		dev_crit(cs35l38a->dev, "VBST Over Voltage error\n");
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_ERR_RLS, 0);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_ERR_RLS,
				CS35L38A_TEMP_ERR_RLS);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_TEMP_ERR_RLS, 0);
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		cs35l38a->pdata.sub_spk_safe_mode |= CS35L38A_BST_OVP_ERR;
#endif
		regmap_write(cs35l38a->regmap, CS35L38A_INT1_STATUS,
			     CS35L38A_BST_OVP_ERR);
	}

	if (status[0] & CS35L38A_BST_DCM_UVP_ERR) {
		dev_crit(cs35l38a->dev, "DCM VBST Under Voltage Error\n");
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_BST_UVP_ERR_RLS, 0);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_BST_UVP_ERR_RLS,
				CS35L38A_BST_UVP_ERR_RLS);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_BST_UVP_ERR_RLS, 0);
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		cs35l38a->pdata.sub_spk_safe_mode |= CS35L38A_BST_DCM_UVP_ERR;
#endif
		regmap_write(cs35l38a->regmap, CS35L38A_INT1_STATUS,
			     CS35L38A_BST_DCM_UVP_ERR);
	}

	if (status[0] & CS35L38A_BST_SHORT_ERR) {
		dev_crit(cs35l38a->dev, "LBST SHORT error!\n");
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_BST_SHORT_ERR_RLS, 0);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_BST_SHORT_ERR_RLS,
				CS35L38A_BST_SHORT_ERR_RLS);
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PROTECT_REL_ERR,
				CS35L38A_BST_SHORT_ERR_RLS, 0);
#ifdef CS35LXX_MONITOR_WORK_ENABLE
		cs35l38a->pdata.sub_spk_safe_mode |= CS35L38A_BST_SHORT_ERR;
#endif
		regmap_write(cs35l38a->regmap, CS35L38A_INT1_STATUS,
			     CS35L38A_BST_SHORT_ERR);
	}

	return IRQ_HANDLED;
}

static int cs35l38a_handle_of_data(struct i2c_client *i2c_client,
				   struct cs35l38a_platform_data *pdata)
{
	struct device_node *np = i2c_client->dev.of_node;
	struct irq_cfg *irq_gpio_config = &pdata->irq_config;
	struct device_node *irq_gpio;
	unsigned int val, ret;

	if (!np)
		return 0;

	ret = of_property_read_u32(np, "cirrus,boost-ctl-millivolt", &val);
	if (ret >= 0) {
		if (val < 2550 || val > 10000) {
			dev_err(&i2c_client->dev,
				"Invalid Boost Voltage %d mV\n", val);
			return -EINVAL;
		}
		pdata->bst_vctl = (((val - 2550) / 100) + 1) << 1;
	}

	ret = of_property_read_u32(np, "cirrus,boost-ctl-select", &val);
	if (!ret)
		pdata->bst_vctl_sel = val | CS35L38A_VALID_PDATA;

	ret = of_property_read_u32(np, "cirrus,boost-peak-milliamp", &val);
	if (ret >= 0) {
		if (val < 1600 || val > 4500) {
			dev_err(&i2c_client->dev,
				"Invalid Boost Peak Current %u mA\n", val);
			return -EINVAL;
		}

		//pdata->bst_ipk = (val - 1600) / 50;
		pdata->bst_ipk = (val - 1600) / 50 + 0x10;
	}

	pdata->multi_amp_mode = of_property_read_bool(np,
					"cirrus,multi-amp-mode");

	pdata->sclk_frc = of_property_read_bool(np,
					"cirrus,sclk-force-output");

	pdata->lrclk_frc = of_property_read_bool(np,
					"cirrus,lrclk-force-output");

	pdata->dcm_mode = of_property_read_bool(np,
					"cirrus,dcm-mode-enable");

	pdata->amp_gain_zc = of_property_read_bool(np,
					"cirrus,amp-gain-zc");

	pdata->amp_pcm_inv = of_property_read_bool(np,
					"cirrus,amp-pcm-inv");

	ret = of_property_read_u32(np, "cirrus,ldm-mode-select", &val);
	if (!ret)
		pdata->ldm_mode_sel = val;

	pdata->pdm_ldm_exit = of_property_read_bool(np,
					"cirrus,pdm-ldm-exit");

	pdata->pdm_ldm_enter = of_property_read_bool(np,
					"cirrus,pdm-ldm-enter");

	pdata->imon_pol_inv = of_property_read_bool(np,
					"cirrus,imon-pol-inv");

	pdata->vmon_pol_inv = of_property_read_bool(np,
					"cirrus,vmon-pol-inv");

	if (of_property_read_u32(np, "cirrus,temp-warn-threshold", &val) >= 0)
		pdata->temp_warn_thld = val | CS35L38A_VALID_PDATA;

	if (of_property_read_u32(np, "cirrus,boost-ind-nanohenry", &val) >= 0) {
		pdata->boost_ind = val;
	} else {
		dev_err(&i2c_client->dev, "Inductor not specified.\n");
		return -EINVAL;
	}

	/* INT/GPIO Pin Config */
	irq_gpio = of_get_child_by_name(np, "cirrus,irq-config");
	irq_gpio_config->is_present = irq_gpio ? true : false;
	if (irq_gpio_config->is_present) {
		if (of_property_read_u32(irq_gpio, "cirrus,irq-drive-select",
					 &val) >= 0)
			irq_gpio_config->irq_drv_sel = val;
		if (of_property_read_u32(irq_gpio, "cirrus,irq-polarity",
					 &val) >= 0)
			irq_gpio_config->irq_pol = val;
		if (of_property_read_u32(irq_gpio, "cirrus,irq-gpio-select",
					 &val) >= 0)
			irq_gpio_config->irq_gpio_sel = val;
		if (of_property_read_u32(irq_gpio, "cirrus,irq-output-enable",
					 &val) >= 0)
			irq_gpio_config->irq_out_en = val;
		if (of_property_read_u32(irq_gpio, "cirrus,irq-src-select",
					 &val) >= 0)
			irq_gpio_config->irq_src_sel = val;
	}
	of_node_put(irq_gpio);

	return 0;
}

static const struct reg_sequence cs35l38a_unlocktest[] = {
	{ CS35L38A_TESTKEY_CTRL,	CS35L38A_TEST_UNLOCK1 },
	{ CS35L38A_TESTKEY_CTRL,	CS35L38A_TEST_UNLOCK2 },
};

static const struct reg_sequence cs35l38a_locktest[] = {
	{ CS35L38A_TESTKEY_CTRL,		CS35L38A_TEST_LOCK1 },
	{ CS35L38A_TESTKEY_CTRL,		CS35L38A_TEST_LOCK2 },
};

static const struct reg_sequence cs35l38a_init_patch[] = {
	{ CS35L38A_INT4_MASK,		0x00FEFFFF },
	{ CS35L38A_PAD_INTERFACE,	0x00000039 },
	{ CS35L38A_PAC_CTL1,		0x00000000 },
	{ CS35L38A_PAC_CTL3,		0x00000001 },
	{ CS35L38A_PAC_PMEM_WORD0,	0x00DD0102 },
	{ CS35L38A_PAC_CTL3,		0x00000000 },
	{ CS35L38A_PAC_CTL1,		0x00000001 },
};

static const struct reg_sequence cs35l38a_init2_patch[] = {
	{ CS35L38A_INT4_STATUS,			0x00010000 },
	{ CS35L38A_PAD_INTERFACE,		0x00000039 },
	{ CS35L38A_PAC_CTL1,			0x00000000 },
};

static const struct reg_sequence cs35l38a_errata_patch[] = {
	{ 0x00007064,		0x0929A800 },
	{ 0x00007850,		0x00002FA9 },
	{ 0x00007854,		0x0003F1D5 },
	{ 0x00007858,		0x0003F5E3 },
	{ 0x0000785C,		0x00001137 },
	{ 0x00007860,		0x0001A7A5 },
	{ 0x00007864,		0x0002F16A },
	{ 0x00007868,		0x00003E21 },
	{ 0x00007848,		0x00000001 },
	{ 0x00002020,		0x00000000 },
};

static const struct reg_sequence cs35l38a_errata2_patch[] = {
	{ 0x00007418,		0x909001C8 },
	{ 0x0000394C,		0x028764B7 },
	{ 0x00003810,		0x00003C3C },
	{ 0x0000381C,		0x00000051 },
	{ 0x00003854,		0x05180240 },
};

static int cs35l38a_init(struct cs35l38a_private *cs35l38a)
{
	unsigned int max_try = 100;
	unsigned int i;
	int ret = 0;
	unsigned int val;

	i = 0;
	while (1) {
		if (i >= max_try) {
			dev_err(cs35l38a->dev,
				"Oscillator trim failed to set\n");
			ret = -1;
			goto exit;
		}
		ret = regmap_read(cs35l38a->regmap, CS35L38A_OSC_TRIM, &val);
		if (ret < 0) {
			dev_err(cs35l38a->dev,
				"Failed to read OSC_TRIM:%d\n", ret);
			goto exit;
		}
		if (((val >> CS35L38A_OSC_TRIM_INIT_SHFT) & 1) == 1)
			break;
		usleep_range(10000, 10500);
		i++;
	}
	/* Oscillator trim bit set */
	i = 0;
	while (1) {
		if (i >= max_try) {
			dev_err(cs35l38a->dev,
				"OTP Control5 failed to set\n");
			ret = -1;
			goto exit;
		}
		ret = regmap_read(cs35l38a->regmap, CS35L38A_OTP_CTRL5, &val);
		if (ret < 0) {
			dev_err(cs35l38a->dev,
				"Failed to read OTP CTRL5:%d\n", ret);
			goto exit;
		}
		if (((val >> CS35L38A_OTP_CTRL5_INIT_SHFT) & 1) == 1)
			break;
		usleep_range(10000, 10500);
		i++;
	}
	/* OTP Control 5 set */

	ret = regmap_register_patch(cs35l38a->regmap, cs35l38a_unlocktest,
				    ARRAY_SIZE(cs35l38a_unlocktest));
	if (ret < 0) {
		dev_err(cs35l38a->dev, "Failed to unlock test space\n");
		goto exit;
	}
	ret = regmap_register_patch(cs35l38a->regmap, cs35l38a_init_patch,
				    ARRAY_SIZE(cs35l38a_init_patch));
	if (ret < 0) {
		dev_err(cs35l38a->dev, "Failed to apply init patch\n");
		goto exit;
	}
	ret = wait_for_completion_timeout(&cs35l38a->init_done,
					  msecs_to_jiffies(1000));
	regmap_write(cs35l38a->regmap, CS35L38A_INT4_MASK, 0xFFFFFFFF);
	if (ret == 0) {
		dev_err(cs35l38a->dev,
			"Timeout waiting for initialization\n");
		ret = -ETIMEDOUT;
		goto exit;
	}
	ret = regmap_read(cs35l38a->regmap, CS35L38A_INIT_STS, &val);
	if (ret < 0) {
		dev_err(cs35l38a->dev, "Failed to read INIT_STS\n");
		goto exit;
	}
	if ((val & 0x7) != 0x3) {
		dev_err(cs35l38a->dev, "Initialization failed\n");
		ret = -1;
		goto exit;
	}
	regmap_register_patch(cs35l38a->regmap, cs35l38a_init2_patch,
			      ARRAY_SIZE(cs35l38a_init2_patch));
	regmap_register_patch(cs35l38a->regmap, cs35l38a_errata_patch,
			      ARRAY_SIZE(cs35l38a_errata_patch));
	regmap_register_patch(cs35l38a->regmap, cs35l38a_errata2_patch,
			      ARRAY_SIZE(cs35l38a_errata2_patch));
exit:
	regmap_register_patch(cs35l38a->regmap, cs35l38a_locktest,
			      ARRAY_SIZE(cs35l38a_locktest));
	return ret;
}

static int cs35l38a_irq_gpio_config(struct cs35l38a_private *cs35l38a)
{
	struct cs35l38a_platform_data *pdata = &cs35l38a->pdata;
	struct irq_cfg *irq_config = &pdata->irq_config;
	int irq_polarity;

	/* setup the Interrupt Pin (INT | GPIO) */
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PAD_INTERFACE,
			   CS35L38A_INT_OUTPUT_EN_MASK,
				irq_config->irq_out_en);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PAD_INTERFACE,
			   CS35L38A_INT_GPIO_SEL_MASK,
				irq_config->irq_gpio_sel <<
				CS35L38A_INT_GPIO_SEL_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PAD_INTERFACE,
			   CS35L38A_INT_POL_SEL_MASK,
				irq_config->irq_pol <<
				CS35L38A_INT_POL_SEL_SHIFT);
	if (cs35l38a->rev_id == CS35L38A_REV_A0)
		regmap_update_bits(cs35l38a->regmap,
				   CS35L38A_PAD_INTERFACE,
				CS35L38A_IRQ_SRC_MASK,
				irq_config->irq_src_sel <<
				CS35L38A_IRQ_SRC_SHIFT);
	regmap_update_bits(cs35l38a->regmap, CS35L38A_PAD_INTERFACE,
			   CS35L38A_INT_DRV_SEL_MASK,
				irq_config->irq_drv_sel <<
				CS35L38A_INT_DRV_SEL_SHIFT);

	if (irq_config->irq_pol)
		irq_polarity = IRQF_TRIGGER_HIGH;
	else
		irq_polarity = IRQF_TRIGGER_LOW;

	return irq_polarity;
}

static int cs35l38a_i2c_probe(struct i2c_client *i2c_client,
			      const struct i2c_device_id *id)
{
	struct cs35l38a_private *cs35l38a;
	struct device *dev = &i2c_client->dev;
	struct cs35l38a_platform_data *pdata = dev_get_platdata(dev);

	int i;
	int ret;
	u32 reg_id, reg_revid;
	int irq_pol = IRQF_TRIGGER_LOW;

	cs35l38a = devm_kzalloc(dev, sizeof(struct cs35l38a_private),
				GFP_KERNEL);
	if (!cs35l38a)
		return -ENOMEM;

	cs35l38a->dev = dev;

	i2c_set_clientdata(i2c_client, cs35l38a);
	cs35l38a->regmap = devm_regmap_init_i2c(i2c_client, &cs35l38a_regmap);
	if (IS_ERR(cs35l38a->regmap)) {
		ret = PTR_ERR(cs35l38a->regmap);
		dev_err(dev, "regmap_init() failed: %d\n", ret);
		goto err;
	}

	for (i = 0; i < ARRAY_SIZE(cs35l38a_supplies); i++)
		cs35l38a->supplies[i].supply = cs35l38a_supplies[i];
	cs35l38a->num_supplies = ARRAY_SIZE(cs35l38a_supplies);

	ret = devm_regulator_bulk_get(dev, cs35l38a->num_supplies,
				      cs35l38a->supplies);
	if (ret != 0) {
		dev_err(dev, "Failed to request core supplies: %d\n", ret);
		return ret;
	}

	if (pdata) {
		cs35l38a->pdata = *pdata;
	} else {
		pdata = devm_kzalloc(dev, sizeof(struct cs35l38a_platform_data),
				     GFP_KERNEL);
		if (!pdata)
			return -ENOMEM;
		if (i2c_client->dev.of_node) {
			ret = cs35l38a_handle_of_data(i2c_client, pdata);
			if (ret != 0)
				return ret;
		}
		cs35l38a->pdata = *pdata;
	}

	ret = regulator_bulk_enable(cs35l38a->num_supplies,
				    cs35l38a->supplies);
	if (ret != 0) {
		dev_err(dev, "Failed to enable core supplies: %d\n", ret);
		return ret;
	}

	/* returning NULL can be an option if in stereo mode */
	cs35l38a->reset_gpio = devm_gpiod_get_optional(dev, "reset",
							GPIOD_OUT_LOW);
	if (IS_ERR(cs35l38a->reset_gpio)) {
		ret = PTR_ERR(cs35l38a->reset_gpio);
		cs35l38a->reset_gpio = NULL;
		if (ret == -EBUSY) {
			dev_info(dev, "Reset line busy, assuming shared reset\n");
		} else {
			dev_err(dev, "Failed to get reset GPIO: %d\n", ret);
			goto err;
		}
	}
	if (cs35l38a->reset_gpio)
		gpiod_set_value_cansleep(cs35l38a->reset_gpio, 1);

	usleep_range(2000, 2100);

	/* initialize codec */
	ret = regmap_read(cs35l38a->regmap, CS35L38A_SW_RESET, &reg_id);
	if (ret < 0) {
		dev_err(dev, "Get Device ID failed %d\n", ret);
		goto err;
	}

	if (reg_id != CS35L38A_CHIP_ID) {
		dev_err(dev, "CS35L38A Device ID (%X). Expected ID %X\n",
			reg_id, CS35L38A_CHIP_ID);
		ret = -ENODEV;
		goto err;
	}

	ret = regmap_read(cs35l38a->regmap, CS35L38A_REV_ID, &reg_revid);
	if (ret < 0) {
		dev_err(&i2c_client->dev, "Get Revision ID failed %d\n", ret);
		goto err;
	}

	cs35l38a->rev_id = reg_revid >> 8;

	if (pdata->irq_config.is_present)
		irq_pol = cs35l38a_irq_gpio_config(cs35l38a);

	init_completion(&cs35l38a->init_done);
	ret = devm_request_threaded_irq(dev, i2c_client->irq, NULL,
					cs35l38a_irq,
					IRQF_ONESHOT |
					irq_pol,
					"cs35l38a", cs35l38a);

	if (ret != 0) {
		dev_err(dev, "Failed to request IRQ: %d\n", ret);
		goto err;
	}

	if (cs35l38a_init(cs35l38a) < 0)
		goto err;

	/* Set interrupt masks for critical errors */
	regmap_write(cs35l38a->regmap, CS35L38A_INT1_MASK,
		     CS35L38A_INT1_MASK_DEFAULT);
	regmap_write(cs35l38a->regmap, CS35L38A_INT3_MASK,
		     CS35L38A_INT3_MASK_DEFAULT);

	dev_info(&i2c_client->dev,
		 "Cirrus Logic CS35L38A (%x), Revision: %02X\n", reg_id,
			reg_revid >> 8);

	hw_set_smartpa_type("cs35lxxa",sizeof("cs35lxxa"));
	for (i = 0; i < ARRAY_SIZE(cs35l38a_dai); i++)
		dev_err(dev, "%s: cs35l38a_dai[%d].name %s\n", __func__, i, cs35l38a_dai[i].name);
	hw_add_smartpa_codec(dev->driver->name, dev_name(dev), cs35l38a_dai[0].name);
	ret =  snd_soc_register_component(dev, &soc_component_dev_cs35l38a,
				      cs35l38a_dai, ARRAY_SIZE(cs35l38a_dai));
	if (ret < 0) {
		dev_err(dev, "%s: Register codec failed %d\n", __func__, ret);
		goto err;
	}

	return 0;

err:
	regulator_bulk_disable(cs35l38a->num_supplies, cs35l38a->supplies);
	gpiod_set_value_cansleep(cs35l38a->reset_gpio, 0);
	return ret;
}

static int cs35l38a_i2c_remove(struct i2c_client *client)
{
	struct cs35l38a_private *cs35l38a = i2c_get_clientdata(client);

	/* Reset interrupt masks for device removal */
	regmap_write(cs35l38a->regmap, CS35L38A_INT1_MASK,
		     CS35L38A_INT1_MASK_RESET);
	regmap_write(cs35l38a->regmap, CS35L38A_INT3_MASK,
		     CS35L38A_INT3_MASK_RESET);

	if (cs35l38a->reset_gpio)
		gpiod_set_value_cansleep(cs35l38a->reset_gpio, 0);

	regulator_bulk_disable(cs35l38a->num_supplies, cs35l38a->supplies);

	snd_soc_unregister_component(&client->dev);

	return 0;
}

static const struct of_device_id cs35l38a_of_match[] = {
	{.compatible = "cirrus,cs35lxxa"},
	{},
};
MODULE_DEVICE_TABLE(of, cs35l38a_of_match);

static const struct i2c_device_id cs35l38a_id[] = {
	{"cs35lxxa", 0},
	{}
};

MODULE_DEVICE_TABLE(i2c, cs35l38a_id);

static struct i2c_driver cs35l38a_i2c_driver = {
	.driver = {
		.name = "cs35lxxa",
		.of_match_table = cs35l38a_of_match,
	},
	.id_table = cs35l38a_id,
	.probe = cs35l38a_i2c_probe,
	.remove = cs35l38a_i2c_remove,
};
module_i2c_driver(cs35l38a_i2c_driver);

MODULE_DESCRIPTION("ASoC CS35L38A driver");
MODULE_AUTHOR("Li Xu, Cirrus Logic Inc, <li.xu@cirrus.com>");
MODULE_LICENSE("GPL");
