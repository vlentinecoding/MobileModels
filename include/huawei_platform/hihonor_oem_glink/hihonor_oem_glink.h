/*
 * hihonor_oem_glink.h
 *
 * hihonor_oem_glink driver
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

#ifndef _HIHONOR_OEM_GLINK_H_
#define _HIHONOR_OEM_GLINK_H_

#define MAX_OEM_PROPERTY_DATA_SIZE 64
#define MAX_OEM_CONFIG_DATA_SIZE 8000
#define CURRENT_MATOUA 1000

struct hihonor_oem_glink_msg_buffer {
	u32 oem_property_id;
	u32 data_buffer[MAX_OEM_PROPERTY_DATA_SIZE];
	u32 data_size;
};

struct hihonor_oem_glink_config_buffer {
	u32 oem_config_id;
	u8 data_buffer[MAX_OEM_CONFIG_DATA_SIZE];
	u32 data_size;
};

struct hihonor_glink_ops {
	void *dev_data;
	char *name;
	void (*sync_data)(void *);
	void (*update_data)(void *, void *);  /* do not block */
	void (*notify_event)(void *, u32, void *); /* do not block */
};

enum hihonor_glink_notify_event {
	OEM_NOTIFY_EVENT_START = 0,
	OEM_NOTIFY_SYNC_BATTINFO = OEM_NOTIFY_EVENT_START,
	OEM_NOTIFY_VBUS_ON,
	OEM_NOTIFY_VBUS_OFF,
	OEM_NOTIFY_CHARGER_DONE,
	OEM_NOTIFY_WIRED_CHANNEL_RESTORE,
	OEM_NOTIFY_WIRED_CHANNEL_CUTOFF,
	OEM_NOTIFY_FG_TYPE_BQ27Z561,
	OEM_NOTIFY_FG_TYPE_RT9426A,
	OEM_NOTIFY_FG_TYPE_CW2217,
	OEM_NOTIFY_FG_TYPE_RT9426,
	OEM_NOTIFY_BATT_LOW_VOLT,
	OEM_NOTIFY_USB_PLUG_IN,
	OEM_NOTIFY_CHARGER_PLUG_OUT,
	OEM_NOTIFY_ICON_TYPE_SUPER,
	OEM_NOTIFY_ICON_TYPE_QUICK,
	OEM_NOTIFY_START_CHARGING,
	OEM_NOTIFY_STOP_CHARGING,
	OEM_NOTIFY_DC_PLUG_OUT,
	OEM_NOTIFY_DC_SOC_DECIMAL,
	OEM_NOTIFY_FG_READY,
	OEM_NOTIFY_AP_SUSPEND_STATE,
	OEM_NOTIFY_AP_SHUTDOWN,
	OEM_NOTIFY_THERMAL_READY,
	OEM_NOTIFY_UCSI_RELEASE_LOCK,
	OEM_NOTIFY_USB_ONLINE,
	OEM_NOTIFY_DSM_INFO,
	OEM_NOTIFY_BAT_CAPACITY_CHANGED,
	OEM_NOTIFY_ADAPTER_INFO,
	OEM_NOTIFY_IC_READY,
	OEM_NOTIFY_EVENT_END,
};

enum hihonor_glink_oem_property_id {
	BATTERY_OEM_START = 0,
	BATTERY_OEM_BATCH_INFO = BATTERY_OEM_START,
	BATTERY_OEM_FG_INS_INDEX,
	BATTERY_OEM_BATT_FAULT_CONF,
	BATTERY_OEM_BATT_MODEL_CONF,
	BATTERY_OEM_FG_CALI_INFO,
	BATTERY_OEM_FG_CALI_MODE,
	BATTERY_OEM_PSY_INFO,
	BATTERY_OEM_SOC_DECIMAL,
	CHARGER_OEM_START = 0x100,
	CHARGER_OEM_SET_HIZ_EN = CHARGER_OEM_START,
	CHARGER_OEM_SET_CHARGE_EN,
	CHARGER_OEM_SET_INPUT_CURRENT,
	CHARGER_OEM_SET_SDP_INPUT_CURRENT,
	CHARGER_OEM_SET_CHARGE_CURRENT,
	CHARGER_OEM_CHARGER_TYPE,
	CHARGER_OEM_BUCK_INFO,
	CHARGER_OEM_WLC_SRC,
	CHARGER_OEM_INSERT_IDENTIFY,
	CHARGER_OEM_BOOST5V,
	CHARGER_OEM_SUSPEND_COLLAPSE_EN,
	CHARGER_OEM_DEBUG_ACCESS_EN,
	USB_TYPEC_OEM_START = 0x200,
	USB_TYPEC_OEM_OTG_TYPE = USB_TYPEC_OEM_START,
	USB_TYPEC_OEM_CABLE_TYPE,
	USB_TYPEC_OEM_CHECK_SBU_VBUS_SHORT,
	USB_TYPEC_OEM_CHECK_CC_VBUS_SHORT,
	USB_TYPEC_OEM_TYPEC_SM,
	DIRECT_CHARGE_OEM_START = 0x300,
	DIRECT_CHARGE_OEM_CHARGE_EN = DIRECT_CHARGE_OEM_START,
	DIRECT_CHARGE_OEM_MAINSC_CHARGE_EN,
	DIRECT_CHARGE_OEM_AUXSC_CHARGE_EN,
	DIRECT_CHARGE_OEM_IIN_THERMAL,
	DIRECT_CHARGE_OEM_ADAPOR_DETECT,
	DIRECT_CHARGE_OEM_IADAPTOR,
	DIRECT_CHARGE_OEM_DC_CHARGE_SUCC,
	DIRECT_CHARGE_OEM_MULTI_CUR,
	DIRECT_CHARGE_OEM_VBUS,
	DIRECT_CHARGE_OEM_CUR_MODE,
	DIRECT_CHARGE_OEM_VTERM_DEC,
	DIRECT_CHARGE_OEM_ICHG_RATIO,
	DIRECT_CHARGE_OEM_RT_TEST_INFO,
	DIRECT_CHARGE_OEM_CABLE_TYPE,
	DIRECT_CHARGE_OEM_MAX_POWER,
	DIRECT_CHARGE_OEM_MMI_TEST_FLAG,
	DIRECT_CHARGE_OEM_MMI_RESULT,
	DIRECT_CHARGE_OEM_RT_RESULT,
	DIRECT_CHARGE_OEM_GET_CHARGE_INFO,
	OEM_INFO_MAX,
};

enum hihonor_glink_oem_config_id {
	FG_CONFIG_ID = 0,
	BUCK_CHARGER_ID,
	DC_LVC_CONFIG_ID,
	DC_SC_CONFIG_ID,
	DC_HSC_CONFIG_ID,
	SWITCH_CONFIG_ID,
	SC_MAIN_CONFIG_ID,
	SC_AUX_CONFIG_ID,
	BATT_UI_CAP_CONFIG_ID,
	USCP_CONFIG_ID,
	DC_LVC_SYSFS_DATA_ID,
	DC_SC_SYSFS_DATA_ID,
	DC_HSC_SYSFS_DATA_ID,
	DC_BTB_CK_CONFIG_ID,
	THERMAL_BASIC_CONFIG_ID,
	THERMAL_CONFIG_ID,
	FG_JEITA_CONFIG_ID,
	CONFIG_ID_MAX,
};

typedef struct {
	u32 buck_vbus;
	u32 buck_ibus;
	bool chg_done;
} batt_mngr_get_buck_info;

#ifdef CONFIG_HIHONOR_OEM_GLINK

typedef struct {
	u32 vsbu1;
	u32 vsbu2;
	bool sbu_vbus_status;
} batt_mngr_get_usb_typec_info;

int hihonor_oem_glink_oem_update_config(u32 oem_config_id, void *data, size_t data_size);
int hihonor_oem_glink_oem_set_prop(u32 oem_property_id, void *data, size_t data_size);
int hihonor_oem_glink_oem_get_prop(u32 oem_property_id, void *data, size_t data_size);
int hihonor_oem_glink_ops_register(struct hihonor_glink_ops *ops);
int hihonor_oem_glink_ops_unregister(struct hihonor_glink_ops * ops);
int hihonor_oem_glink_notify_state(u32 notification, void *data, size_t data_size);
#else
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(5, 4, 0))
static inline int hihonor_oem_glink_oem_update_config(u32 oem_config_id, void *data, size_t data_size)
{
	return 0;
}

static inline int hihonor_oem_glink_oem_set_prop(u32 oem_property_id, void *data, size_t data_size)
{
	return 0;
}

static int hihonor_oem_glink_oem_get_prop(u32 oem_property_id, void *data, size_t data_size)
{
	return 0;
}

static int hihonor_oem_glink_ops_register(struct hihonor_glink_ops *ops)
{
	return -1;
}

static int hihonor_oem_glink_ops_unregister(struct hihonor_glink_ops * ops)
{
	return -1;
}

static int hihonor_oem_glink_notify_state(u32 notification, void *data, size_t data_size)
{
	return -1;
}

#endif /* KERNEL_VERSION(5, 4, 0) */
#endif /* CONFIG_HIHONOR_OEM_GLINK */
#endif /* _HIHONOR_OEM_GLINK_H_ */
