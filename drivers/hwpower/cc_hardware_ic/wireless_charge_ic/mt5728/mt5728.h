/*
 * mt5728.h
 *
 * mt5728 macro, addr etc.
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
 *
 */

#ifndef _MT5728_H_
#define _MT5728_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/workqueue.h>
#include <linux/bitops.h>
#include <linux/jiffies.h>
#include <chipset_common/hwpower/power_delay.h>
#include <chipset_common/hwpower/boost_5v.h>
#include <chipset_common/hwpower/charge_pump.h>
#include <chipset_common/hwpower/wireless_fw.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_cmdline.h>
#include <chipset_common/hwpower/power_event_ne.h>
#include <chipset_common/hwpower/power_devices_info.h>
#include <chipset_common/hwpower/wireless_charge/wireless_rx_ic_intf.h>
#include <chipset_common/hwpower/wireless_charge/wireless_power_supply.h>
#include <huawei_platform/power/wireless/wireless_charger.h>
#include <huawei_platform/power/wireless/wireless_direct_charger.h>
#include <huawei_platform/power/wireless/wireless_transmitter.h>
#include <chipset_common/hwpower/wireless_charge/wireless_test_hw.h>
#include <chipset_common/hwpower/wired_channel_switch.h>
#include <chipset_common/hwpower/power_gpio.h>
#include <chipset_common/hwpower/power_i2c.h>
#include <chipset_common/hwpower/power_printk.h>

#include "mt5728_chip.h"

#define MT5728_SW2TX_SLEEP_TIME              25 /* ms */
#define MT5728_SW2TX_RETRY_TIME              500 /* ms */

#define MT5728_SHUTDOWN_SLEEP_TIME           200
#define MT5728_RCV_MSG_SLEEP_TIME            100
#define MT5728_RCV_MSG_SLEEP_CNT             10
#define MT5728_WAIT_FOR_ACK_SLEEP_TIME       100
#define MT5728_WAIT_FOR_ACK_RETRY_CNT        5
#define MT5728_SNED_MSG_RETRY_CNT            2
#define MT5728_RX_TMP_BUFF_LEN               32

/* coil test */
#define MT5728_COIL_TEST_PING_INTERVAL       0
#define MT5728_COIL_TEST_PING_FREQ           115

struct mt5728_chip_info {
	u16 chip_id;
	u8 cust_id;
	u8 hw_id;
	u16 minor_ver;
	u16 major_ver;
};

struct mt5728_global_val {
	bool mtp_chk_complete;
	bool rx_stop_chrg_flag;
	bool irq_abnormal_flag;
	struct qi_protocol_handle *qi_hdl;
};

struct mt5728_tx_init_para {
	u16 ping_interval;
	u16 ping_freq;
};

struct mt5728_dev_info {
	struct i2c_client *client;
	struct device *dev;
	struct work_struct irq_work;
	struct delayed_work mtp_check_work;
	struct mutex mutex_irq;
	struct mt5728_global_val g_val;
	struct mt5728_tx_init_para tx_init_para;
	bool irq_active;
	u8 rx_fod_5v[MT5728_RX_FOD_LEN];
	u8 rx_fod_9v[MT5728_RX_FOD_LEN];
	u8 rx_fod_15v[MT5728_RX_FOD_LEN];
	u8 rx_ldo_cfg_5v[MT5728_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_9v[MT5728_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_12v[MT5728_RX_LDO_CFG_LEN];
	u8 rx_ldo_cfg_sc[MT5728_RX_LDO_CFG_LEN];
	int rx_ss_good_lth;
	int gpio_en;
	int gpio_en_valid_val;
	int gpio_kp_valid_val;
	int gpio_pen_en;
	int gpio_kb_en;
	int gpio_sleep_en;
	int gpio_int;
	int gpio_pwr_good;
	int gpio_re_pwr_en;
	int irq_int;
	u32 irq_val;
	int irq_cnt;
	u32 ept_type;
	int mtp_status;
};

enum mt5728_dev_hall_id {
	MT5728_HALL_NULL_ID = 0,
	MT5728_HALL_PEN_ID,
	MT5728_HALL_KB_ID,
};

/* mt5728 common */
int mt5728_read_byte(u16 reg, u8 *data);
int mt5728_read_word(u16 reg, u16 *data);
int mt5728_write_byte(u16 reg, u8 data);
int mt5728_write_word(u16 reg, u16 data);
int mt5728_read_byte_mask(u16 reg, u8 mask, u8 shift, u8 *data);
int mt5728_write_byte_mask(u16 reg, u8 mask, u8 shift, u8 data);
int mt5728_read_word_mask(u16 reg, u16 mask, u16 shift, u16 *data);
int mt5728_write_word_mask(u16 reg, u16 mask, u16 shift, u16 data);
int mt5728_read_block(u16 reg, u8 *data, u8 len);
int mt5728_write_block(u16 reg, u8 *data, u8 data_len);
int mt5728_core_reset(void);
int mt5728_chip_reset(void);
void mt5728_chip_enable(int enable);
void mt5728_sleep_enable(int enable);
void mt5728_enable_irq(void);
void mt5728_disable_irq_nosync(void);
void mt5728_get_dev_info(struct mt5728_dev_info **di);
struct device_node *mt5728_dts_dev_node(void *dev_data);
int mt5728_get_mode(u16 *mode);
bool mt5728_is_pwr_good(void);
void mt5728_channel_select_enable(int channel);
void mt5728_channel_select_disable(int channel);
/* mt5728 chip_info */
int mt5728_get_chip_id(u16 *chip_id);
int mt5728_get_chip_info_str(char *info_str, int len);
int mt5728_get_chip_fw_version(u8 *data, int len, void *dev_data);

/* mt5728 rx */
int mt5728_rx_ops_register(void);
void mt5728_rx_mode_irq_handler(struct mt5728_dev_info *di);
void mt5728_rx_abnormal_irq_handler(struct mt5728_dev_info *di);
void mt5728_rx_shutdown_handler(void);
void mt5728_rx_probe_check_tx_exist(void);

/* mt5728 tx */
int mt5728_tx_ops_register(void);
int mt5728_tx_ps_ops_register(void);
void mt5728_tx_mode_irq_handler(struct mt5728_dev_info *di);
struct wireless_tx_device_ops *mt5728_get_tx_ops(void);
struct wlps_tx_ops *mt5728_get_txps_ops(void);

/* mt5728 fw */
int mt5728_fw_ops_register(void);
void mt5728_fw_mtp_check_work(struct work_struct *work);
int mt5728_fw_sram_update(enum wireless_mode sram_mode);

/* mt5728 qi */
int mt5728_qi_ops_register(void);

/* mt5728 dts */
int mt5728_parse_dts(const struct device_node *np, struct mt5728_dev_info *di);

/* mt5728 hw_test */
int mt5728_hw_test_ops_register(void);

#endif /* _MT5728_H_ */
