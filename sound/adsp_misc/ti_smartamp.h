/*
 * ti_smartamp.h
 *
 * ti smartpa related definition for adsp misc
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef _TI_SMART_AMP_H
#define _TI_SMART_AMP_H

#include <linux/types.h>
#include <sound/soc.h>

/* Below 3 should be same as in aDSP code */
#define AFE_PARAM_ID_SMARTAMP_DEFAULT   0x10001166
#define AFE_SMARTAMP_MODULE_RX          0x1000C003  /* Rx module */
#define AFE_SMARTAMP_MODULE_TX          0x1000C002  /* Tx module */

#define CAPI_V2_TAS_TX_ENABLE           0x10012D14
#define CAPI_V2_TAS_TX_CFG              0x10012D16
#define CAPI_V2_TAS_RX_ENABLE           0x10012D13
#define CAPI_V2_TAS_RX_CFG              0x10012D15

#define MAX_DSP_PARAM_INDEX             600

#define TAS_PAYLOAD_SIZE        14
#define TAS_GET_PARAM           1
#define TAS_SET_PARAM           0

#define TAS_RX_PORT             AFE_PORT_ID_SECONDARY_MI2S_RX
#define TAS_TX_PORT             AFE_PORT_ID_SECONDARY_MI2S_TX

#define CHANNEL0        1
#define CHANNEL1        2
#define MAX_CHANNELS    2

#define TRUE            1
#define FALSE           0

#define TAS_SA_GET_F0          3810
#define TAS_SA_GET_Q           3811
#define TAS_SA_GET_TV          3812
#define TAS_SA_GET_RE          3813
#define TAS_SA_CALIB_INIT      3814
#define TAS_SA_CALIB_DEINIT    3815
#define TAS_SA_SET_RE          3816
#define TAS_SA_SET_PROFILE     3819
#define TAS_SA_GET_STATUS      3821
#define TAS_SA_SET_TCAL        3823

#define CALIB_START            1
#define CALIB_STOP             2
#define TEST_START             3
#define TEST_STOP              4

#define SLAVE1          0x98
#define SLAVE2          0x9A
#define SLAVE3          0x9C
#define SLAVE4          0x9E

#define TAS_SA_IS_SPL_IDX(x)  ((((x) >= 3810) && ((x) < 3899)) ? 1 : 0)
#define LENGTH_1        1
#define TAS_CALC_PARAM_IDX(index, length, channel) \
	((index) | (length << 16) | (channel << 24))

/* Random Numbers is to handle global data corruption */
#define TAS_FALSE       0x01010101
#define TAS_TRUE        0x10101010

#endif // _TI_SMART_AMP_H

