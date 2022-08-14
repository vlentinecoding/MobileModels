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
 **  tas25xx-battery-limiter.h
 ** Description:
 **  Header file for the batter info based power limiter algo control
 ** ============================================================================
 */
#ifndef __TAS25XX_BAT_INFO_LIMITER__
#define __TAS25XX_BAT_INFO_LIMITER__

void tas25xx_stop_battery_info_monitor(void);
void tas25xx_start_battery_info_monitor(void);
void tas_smartamp_battery_algo_deinitalize(void);
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
void tas_smartamp_add_battery_algo(struct snd_soc_component *codec, int ch);
#else
void tas_smartamp_add_battery_algo(struct snd_soc_codec *codec, int ch);
#endif

#endif /* __TAS25XX_BAT_INFO_LIMITER__ */

