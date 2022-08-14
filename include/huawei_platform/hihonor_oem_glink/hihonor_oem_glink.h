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

#define MAX_OEM_PROPERTY_DATA_SIZE 16
#define CURRENT_MATOUA 1000

struct hihonor_oem_glink_msg_buffer {
	u32 oem_property_id;
	u32 data_buffer[MAX_OEM_PROPERTY_DATA_SIZE];
	u32 data_size;
};

struct hihonor_glink_ops {
	void *dev_data;
	char *name;
	void (*sync_data)(void *);
	void (*update_data)(void *, void *);  /* do not block */
	void (*notify_event)(void *, u32); /* do not block */
};

enum hihonor_glink_notify_event {
	OEM_NOTIFY_EVENT_START = 0,
	OEM_NOTIFY_SYNC_BATTINFO,
	OEM_NOTIFY_VBUS_ON,
	OEM_NOTIFY_VBUS_OFF,
	OEM_NOTIFY_QUICK_ICON,
	OEM_NOTIFY_CHARGER_DONE,
	OEM_NOTIFY_WIRED_CHANNEL_RESTORE,
	OEM_NOTIFY_WIRED_CHANNEL_CUTOFF,
	OEM_NOTIFY_EVENT_END,
};

enum hihonor_glink_oem_property_id {
	BATTERY_OEM_START = 0,
	BATTERY_OEM_BATCH_INFO = BATTERY_OEM_START,
	CHARGER_OEM_START = 0x100,
	CHARGER_OEM_HIZ_EN = 0x100,
	CHARGER_OEM_CHARGE_EN,
	CHARGER_OEM_INPUT_CURRENT,
	CHARGER_OEM_SDP_INPUT_CURRENT,
	CHARGER_OEM_CHARGE_CURRENT,
	CHARGER_OEM_CHARGER_TYPE,
	CHARGER_OEM_CABLE_TYPE,
	CHARGER_OEM_BUCK_INFO,
	CHARGER_OEM_WLC_SRC,
	CHARGER_OEM_INSERT_IDENTIFY,
	CHARGER_OEM_CHECK_SBU_VBUS_SHORT,
	CHARGER_OEM_CHECK_CC_VBUS_SHORT,
	CHARGER_OEM_BOOST5V,
	CHARGER_OEM_TYPEC_SM_STATUS,
	CHARGER_OEM_SUSPEND_COLLAPSE_EN,
	CHARGER_OEM_DEBUG_ACCESS_EN,
	USB_OEM_START = 0x200,
	USB_OEM_OTG_TYPE,
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

int hihonor_oem_glink_oem_set_prop(u32 oem_property_id, void *data, size_t data_size);
int hihonor_oem_glink_oem_get_prop(u32 oem_property_id, void *data, size_t data_size);
int hihonor_oem_glink_ops_register(struct hihonor_glink_ops *ops);
int hihonor_oem_glink_ops_unregister(struct hihonor_glink_ops * ops);
#else
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

#endif /* CONFIG_HIHONOR_OEM_GLINK */
#endif /* _HIHONOR_OEM_GLINK_H_ */
