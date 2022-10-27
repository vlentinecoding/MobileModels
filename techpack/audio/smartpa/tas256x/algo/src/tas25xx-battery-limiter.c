/*
 ** ============================================================================
 ** Copyright (c) 2016  Texas Instruments Inc.
 **
 ** This program is free software; you can redistribute it and/or modify it
 ** under the terms of the GNU General Public License as published by the Free
 ** Software Foundation; version 2.
 **
 ** This program is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS
 ** FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 ** details.
 **
 ** File:
 **  tas25xx-battery-limiter.c
 ** Description:
 **  Implements the logic of battery info based algo. Supports query of battery
 ** info at regular intervals of time and send it to DSP.
 ** ============================================================================
 */
#include <sound/soc.h>
#include <linux/delay.h> /* usleep_range */
#include <linux/kernel.h>
#include <linux/types.h> /* atomic_t */
#include <linux/workqueue.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include "algo/inc/tas_smart_amp_v2.h"
#include "algo/inc/tas25xx-calib.h"
#include "tas25xx-algo-intf.h"

#include <sound/hw_audio/hw_audio_interface.h>

enum battery_algo_channel {
	CH_LEFT = 0,
	CH_RIGHT = 1,
	CH_MAX = 2,
};

enum battery_algo_property {
	SHFT_BAT_TEMP = 0,
	SHFT_BAT_CAP,
	SHFT_BAT_DURATION,
	SHFT_BAT_MAX
};

struct battery_info {
	int32_t temperature;
	int32_t capacity;
};

struct battery_algo {
	/* can be changed by mixer control */
	unsigned int query_duration;
	int32_t enable[CH_MAX];
	int32_t bat_temp_thr[CH_MAX];
	int32_t bat_cap_thr[CH_MAX];

	/* internal */
	int32_t first_set[CH_MAX];
	int32_t current_temp;
	int32_t current_cap;
	int32_t previous_temp;
	int32_t previous_cap;
	struct delayed_work wrk;
};

static struct battery_algo alg;
static atomic_t s_algo_active = ATOMIC_INIT(0);

void tas25xx_stop_battery_info_monitor(void)
{
	pr_debug("[TI-SmartPA:%s]", __func__);
	atomic_set(&s_algo_active, 0);
}

void tas25xx_start_battery_info_monitor(void)
{
	pr_debug("[TI-SmartPA:%s]", __func__);
	atomic_set(&s_algo_active, 1);

	/* reset the values */
	alg.first_set[CH_LEFT] = alg.first_set[CH_RIGHT] = 1;

	schedule_delayed_work(&alg.wrk,
			msecs_to_jiffies(100));
}

static int tas25xx_set_bat_info_common(int sa_ch)
{
	int16_t temp;
	int16_t cap;
	int32_t info;
	int ret = -EINVAL;
	int p_id = 0;

	temp = (int16_t)alg.current_temp;
	cap = (int16_t)alg.current_cap;

	info = ((temp & 0xFFFF) << 16) | cap;
	p_id = TAS_CALC_PARAM_IDX(TAS_SA_ALGO_BAT_INFO, 1, sa_ch);
	ret = tas25xx_smartamp_algo_ctrl((u8 *)&info, p_id,
			TAS_SET_PARAM, sizeof(info), TISA_MOD_RX);

	return ret;
}

static int is_notify_algo(enum battery_algo_channel ch)
{
	int call_dsp = 0;
	int positive_temp_crossover = 0;
	int positive_cap_crossover = 0;

	/* call DSP when the algorithm is enabled */
	call_dsp = alg.enable[ch];

	/* call dsp when the read values are below threshold */
	if (call_dsp && (alg.current_cap <= alg.bat_cap_thr[ch])
			&& (alg.current_temp <= alg.bat_temp_thr[ch]))
		call_dsp = 1;
	else
		call_dsp = 0;

	/* call dsp when the read values are above threshold but
	 * previous values are below threshod so that algorithm reset to
	 * normal mode of operation.
	 */
	if (!alg.first_set[ch] && (alg.current_temp > alg.bat_temp_thr[ch])
			&& (alg.previous_temp <= alg.bat_temp_thr[ch])) {
		positive_temp_crossover = 1;
		pr_info("[TI-SmartPA]: %s: positive temp crossover detected", __func__);
	}

	if (!alg.first_set[ch] && (alg.current_cap > alg.bat_cap_thr[ch])
			&& (alg.previous_cap <= alg.bat_cap_thr[ch])) {
		positive_cap_crossover = 1;
		pr_info("[TI-SmartPA]: %s: positive cap crossover detected", __func__);
	}

	if (call_dsp || positive_temp_crossover || positive_cap_crossover)
		call_dsp = 1;

	return call_dsp;
}

static void update_battery_info(struct work_struct *wrk)
{
	int ret;
	int ret1;
	int sleep_us;
	int sleep_us_max;
	int sleep_ms = 0;

	if (alg.query_duration > 0)
		sleep_ms = alg.query_duration;

	if (sleep_ms < 1000) {
		pr_info("TI-SmartPA: %s duration < 1s, setting it to 1s", __func__);
		sleep_ms = 1000;
	}

	sleep_us = sleep_ms * 1000;
	sleep_us_max = sleep_us + 1000;

	pr_info("TI-SmartPA: query interval is set to %d(ms)\n", sleep_ms);

	while (atomic_read(&s_algo_active)) {
		ret = audio_get_battery_info(BATT_TEMP, &alg.current_temp);
		ret1 = audio_get_battery_info(BATT_CAP, &alg.current_cap);
		if ((ret == 0) && (ret1 == 0)) {
			pr_info("[TI-SmartPA] %s: batt temp,cap=%d,%d\n", __func__,
					alg.current_temp, alg.current_cap);

			ret = 0;
			if (is_notify_algo(CH_LEFT)) {
				ret = tas25xx_set_bat_info_common(CHANNEL0);
				if (ret) {
					pr_err("TI-SmartPA:%s error setting bat info for left\n",
						__func__);
				} else {
					if (alg.first_set[CH_LEFT])
						alg.first_set[CH_LEFT] = 0;
				}
			}

			if (!ret && is_notify_algo(CH_RIGHT)) {
				ret = tas25xx_set_bat_info_common(CHANNEL1);
				if (ret) {
					pr_err("TI-SmartPA:%s error setting bat info for right\n",
							__func__);
				} else {
					if (alg.first_set[CH_RIGHT])
						alg.first_set[CH_RIGHT] = 0;
				}
			}

			/* update only if DSP calls were successful */
			if (!ret) {
				alg.previous_temp = alg.current_temp;
				alg.previous_cap = alg.current_cap;
			}
		}

		usleep_range(sleep_us, sleep_us_max);
	}
}

static int tas25xx_bat_algo_enable_set(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *pUcontrol)
{
	int ret = 0;
	int user_data = pUcontrol->value.integer.value[0];

	alg.enable[CH_LEFT] = user_data;
	pr_info("TI-SmartPA: %s: case %d(FALSE=0,TRUE=1)\n",
			__func__, user_data);
	return ret;
}

static int tas25xx_bat_algo_enable_set_r(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *pUcontrol)
{
	int ret = 0;
	int user_data = pUcontrol->value.integer.value[0];

	alg.enable[CH_RIGHT] = user_data;
	pr_info("TI-SmartPA: %s: case %d(FALSE=0,TRUE=1)\n",
			__func__, user_data);
	return ret;
}

static int tas25xx_bat_algo_enable_get(struct snd_kcontrol *pKcontrol,
		struct snd_ctl_elem_value *pUcontrol)
{
	int value = alg.enable[CH_LEFT];

	pUcontrol->value.integer.value[0] = value;

	return 0;
}

static int tas25xx_bat_algo_enable_get_r(struct snd_kcontrol *pKcontrol,
		struct snd_ctl_elem_value *pUcontrol)
{
	int value = alg.enable[CH_RIGHT];

	pUcontrol->value.integer.value[0] = value;

	return 0;
}


static int tas25xx_bat_get_bat_info(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *pUcontrol)
{
	int ret = 0;
	struct soc_mixer_control *ctl =
		(struct soc_mixer_control*)kcontrol->private_value;

	switch (ctl->shift) {
		case SHFT_BAT_TEMP:
			if (ctl->invert == 0)
				pUcontrol->value.integer.value[0] = alg.bat_temp_thr[CH_LEFT] / -10;
			else
				pUcontrol->value.integer.value[0] = alg.bat_temp_thr[CH_RIGHT] / -10;
			break;

		case SHFT_BAT_CAP:
			if (ctl->invert == 0)
				pUcontrol->value.integer.value[0] = alg.bat_cap_thr[CH_LEFT];
			else
				pUcontrol->value.integer.value[0] = alg.bat_cap_thr[CH_RIGHT];
			break;

		case SHFT_BAT_DURATION:
			pUcontrol->value.integer.value[0] = alg.query_duration;
			break;

		case SHFT_BAT_MAX:
		default:
			ret = -EINVAL;
			break;
	}

	pr_info("TI-SmartPA: %s property=%d, channel=%d, value=%ld", __func__,
			ctl->shift, ctl->invert, pUcontrol->value.integer.value[0]);

	return ret;
}

static int tas25xx_bat_set_bat_info(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *pUcontrol)
{
	int ret = 0;
	int value = pUcontrol->value.integer.value[0];
	struct soc_mixer_control *ctl =
		(struct soc_mixer_control*)kcontrol->private_value;

	pr_info("TI-SmartPA: %s property=%d, channel=%d, value=%d\n", __func__,
			ctl->shift, ctl->invert, value);

	switch (ctl->shift) {
		case SHFT_BAT_TEMP:
			/* Assumption: Threshold will be always -ve.
			 * user entered value will be +ve number.
			 * This needs to be x by -10 to get the temperature
			 * threshold understood by the API.
			 */
			if (ctl->invert == 0)
				alg.bat_temp_thr[CH_LEFT] = value * -10;
			else
				alg.bat_temp_thr[CH_RIGHT] = value * -10;
			break;

		case SHFT_BAT_CAP:
			if (ctl->invert == 0)
				alg.bat_cap_thr[CH_LEFT] = value;
			else
				alg.bat_cap_thr[CH_RIGHT] = value;
			break;

		case SHFT_BAT_DURATION:
			alg.query_duration = value;
			break;

		case SHFT_BAT_MAX:
		default:
			ret = -EINVAL;
			break;
	}

	return ret;
}

static const char *tas25xx_smartamp_enable_text[] = {
	"DISABLE",
	"ENABLE"
};

static const struct soc_enum tas25xx_smartamp_enable_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(tas25xx_smartamp_enable_text),
			tas25xx_smartamp_enable_text),
};

/* left channel controls */
static const struct snd_kcontrol_new left_controls[] = {
	SOC_ENUM_EXT("TAS25XX_BATT_ALGO_EN_L",
			tas25xx_smartamp_enable_enum[0],
			tas25xx_bat_algo_enable_get,
			tas25xx_bat_algo_enable_set),
	SOC_SINGLE_EXT("TAS25XX_BATT_ALGO_TEMP_LOW_L", SND_SOC_NOPM, SHFT_BAT_TEMP,
			50, CH_LEFT, tas25xx_bat_get_bat_info, tas25xx_bat_set_bat_info),
	SOC_SINGLE_EXT("TAS25XX_BATT_ALGO_PCENT_LOW_L", SND_SOC_NOPM, SHFT_BAT_CAP,
			100, CH_LEFT, tas25xx_bat_get_bat_info, tas25xx_bat_set_bat_info),

	/* common for both channels */
	SOC_SINGLE_EXT("TAS25XX_BATT_ALGO_INFO_QRY_T", SND_SOC_NOPM, SHFT_BAT_DURATION,
			20000, CH_LEFT, tas25xx_bat_get_bat_info, tas25xx_bat_set_bat_info),
};

/* right channel controls */
static const struct snd_kcontrol_new right_controls[] = {
	SOC_ENUM_EXT("TAS25XX_BATT_ALGO_EN_R",
			tas25xx_smartamp_enable_enum[0],
			tas25xx_bat_algo_enable_get_r,
			tas25xx_bat_algo_enable_set_r),
	SOC_SINGLE_EXT("TAS25XX_BATT_ALGO_TEMP_LOW_R", SND_SOC_NOPM, SHFT_BAT_TEMP,
			50, CH_RIGHT, tas25xx_bat_get_bat_info, tas25xx_bat_set_bat_info),
	SOC_SINGLE_EXT("TAS25XX_BATT_ALGO_PCENT_LOW_R", SND_SOC_NOPM, SHFT_BAT_CAP,
			100, CH_RIGHT, tas25xx_bat_get_bat_info, tas25xx_bat_set_bat_info),
};

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
void tas_smartamp_add_battery_algo(struct snd_soc_component *codec, int ch)
#else
void tas_smartamp_add_battery_algo(struct snd_soc_codec *codec, int ch)
#endif
{
	pr_info("TI-SmartPA: Adding TI Battery Algo controls, ch=%d\n", ch);

#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
	snd_soc_add_component_controls(codec, left_controls, ARRAY_SIZE(left_controls));
	if (ch == 2)
		snd_soc_add_component_controls(codec, right_controls, ARRAY_SIZE(right_controls));

#else
	snd_soc_add_codec_controls(codec, left_controls, ARRAY_SIZE(left_controls));
	if (ch == 2)
		snd_soc_add_codec_controls(codec, right_controls, ARRAY_SIZE(right_controls));
#endif

	/* initialize the algorithm structre with initial values*/
	alg.enable[CH_LEFT] = 1;

	if (ch == 2)
		alg.enable[CH_RIGHT] = 1;
	else
		alg.enable[CH_RIGHT] = 0;

	alg.bat_temp_thr[CH_LEFT] = -100;
	alg.bat_cap_thr[CH_LEFT] = 50;

	alg.bat_temp_thr[CH_RIGHT] = -100;
	alg.bat_cap_thr[CH_RIGHT] = 50;

	alg.query_duration = 2000; /* 2 seconds */

	alg.first_set[CH_LEFT] = alg.first_set[CH_RIGHT] = 1;

	INIT_DELAYED_WORK(&alg.wrk, update_battery_info);
}
EXPORT_SYMBOL(tas_smartamp_add_battery_algo);

void tas_smartamp_battery_algo_deinitalize(void)
{
	tas25xx_stop_battery_info_monitor();
	cancel_delayed_work_sync(&alg.wrk);
}
EXPORT_SYMBOL(tas_smartamp_battery_algo_deinitalize);

