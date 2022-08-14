/*
 * hw_audio_interface.h
 *
 * hw audio interface
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HW_AUDIO_INTERFACE_H
#define _HW_AUDIO_INTERFACE_H

#include <sound/soc.h>

static const char * const g_hac_switch_text[] = { "OFF", "ON" };
static const char * const g_simple_pa_switch_text[] = { "OFF", "ON" };
static const char * const g_simple_pa_mode_text[] = { "ZERO", "ONE", "TWO",
	"THREE", "FOUR", "FIVE", "SIX", "SEVEN", "EIGHT", "NINE", "TEN" };

static const struct soc_enum g_hw_snd_priv_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(g_hac_switch_text),
		g_hac_switch_text),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(g_simple_pa_switch_text),
		g_simple_pa_switch_text),
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(g_simple_pa_mode_text),
		g_simple_pa_mode_text),
};

enum smartpa_type {
	INVALID,
	CS35LXX,
	AW882XX,
	TFA98XX,
	TAS256X,
	CS35LXXA,
	SMARTPA_TYPE_MAX
};

enum battery_prop_type {
	BATT_TEMP,
	BATT_CAP,
	BATT_VOL,
	BATT_TYPE_MAX
};

#ifdef CONFIG_HW_AUDIO_INFO
void hw_reset_smartpa_firmware(char *str, char *suffix, char *buf, int len);
void hw_set_smartpa_type(const char *buf, int len);
enum smartpa_type hw_get_smartpa_type(void);
bool hw_check_smartpa_type(char *pa_name, char *default_name);
int hw_lock_pa_probe_state(void);
void hw_unlock_pa_probe_state(void);
void hw_add_smartpa_codec(const char* driver_name,
	const char* device_name, const char* dai_name);
void hw_get_smartpa_codecs(struct snd_soc_dai_link *dai_link, int len);

bool is_mic_differential_mode(void);
int hw_simple_pa_power_set(int gpio, int value);
int hac_switch_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
int hac_switch_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
int simple_pa_mode_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
int simple_pa_mode_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
int simple_pa_switch_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
int simple_pa_switch_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol);
int audio_get_battery_info(enum battery_prop_type type, int *val);
#else
static inline void hw_reset_smartpa_firmware(char *str, char *suffix,
	char *buf, int len) {}
static inline void hw_set_smartpa_type(const char *buf, int len) {}
static inline enum smartpa_type hw_get_smartpa_type(void) { return INVALID; }
static inline bool hw_check_smartpa_type(char *pa_name, char *default_name);
static inline int hw_lock_pa_probe_state(void);
static inline void hw_unlock_pa_probe_state(void);
static inline void hw_add_smartpa_codec(const char* driver_name,
	const char* device_name, const char* dai_name) {}
static inline void hw_get_smartpa_codecs(struct snd_soc_dai_link *dai_link,
				int len) {}
static inline bool is_mic_differential_mode(void) { return false; }
static inline int hw_simple_pa_power_set(int gpio, int value) { return 0; }
static inline int audio_get_battery_info(enum battery_prop_type type,
	int *val) { return -1; }
#endif // CONFIG_HW_AUDIO_INFO
#endif // _HW_AUDIO_INTERFACE_H_

