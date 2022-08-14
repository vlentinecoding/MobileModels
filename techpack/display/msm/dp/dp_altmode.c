// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2012-2021, The Linux Foundation. All rights reserved.
 */

#include <linux/slab.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/kthread.h>
#include <linux/soc/qcom/altmode-glink.h>
#include <linux/usb/dwc3-msm.h>
#include <linux/usb/pd_vdo.h>
#include <linux/soc/qcom/dp_cust_interface.h>

#include "dp_altmode.h"
#include "dp_debug.h"
#include "sde_dbg.h"
#ifdef CONFIG_HUAWEI_PD
#include <huawei_platform/usb/hw_pd_dev.h>
#endif

#define ALTMODE_CONFIGURE_MASK (0x3f)
#define ALTMODE_HPD_STATE_MASK (0x40)
#define ALTMODE_HPD_IRQ_MASK (0x80)

#define ALTMODE_PAYLOAD_LEN_MAX    16
#define ALTMODE_PAYLOAD_MODE_IDX   3
#define ALTMODE_PAYLOAD_MODE_MAGIC 0x6D6F6465 // mode

struct dp_altmode_private {
	bool forced_disconnect;
	struct device *dev;
	struct dp_hpd_cb *dp_cb;
	struct dp_altmode dp_altmode;
	struct altmode_client *amclient;
	struct work_struct connect_work;
	struct notifier_block source_nb;
	struct mutex lock;
	u8 payload[ALTMODE_PAYLOAD_LEN_MAX];
	bool connected;
};

enum dp_altmode_pin_assignment {
	DPAM_HPD_OUT,
	DPAM_HPD_A,
	DPAM_HPD_B,
	DPAM_HPD_C,
	DPAM_HPD_D,
	DPAM_HPD_E,
	DPAM_HPD_F,
};

static int dp_altmode_release_ss_lanes(struct dp_altmode_private *altmode,
		bool multi_func)
{
	int rc;
	struct device_node *np;
	struct device_node *usb_node;
	struct platform_device *usb_pdev;
	int timeout = 250;

	if (!altmode || !altmode->dev) {
		DP_ERR("invalid args\n");
		return -EINVAL;
	}

	np = altmode->dev->of_node;

	usb_node = of_parse_phandle(np, "usb-controller", 0);
	if (!usb_node) {
		DP_ERR("unable to get usb node\n");
		return -EINVAL;
	}

	usb_pdev = of_find_device_by_node(usb_node);
	if (!usb_pdev) {
		of_node_put(usb_node);
		DP_ERR("unable to get usb pdev\n");
		return -EINVAL;
	}

	while (timeout) {
		rc = dwc3_msm_release_ss_lane(&usb_pdev->dev, multi_func);
		if (rc != -EBUSY)
			break;

		DP_WARN("USB busy, retry\n");

		/* wait for hw recommended delay for usb */
		msleep(20);
		timeout--;
	}
	of_node_put(usb_node);
	platform_device_put(usb_pdev);

	if (rc)
		DP_ERR("Error releasing SS lanes: %d\n", rc);

	return rc;
}

#ifdef CONFIG_HUAWEI_POWER_EMBEDDED_ISOLATION
static void dp_altmode_set_dp_state(struct dp_altmode_private *altmode, bool dp_state)
{
        int rc;
        struct device_node *np = NULL;
        struct device_node *usb_node = NULL;
        struct platform_device *usb_pdev = NULL;
        int timeout = 250;

        if (!altmode || !altmode->dev) {
                DP_ERR("invalid args\n");
                return;
        }

        np = altmode->dev->of_node;
        usb_node = of_parse_phandle(np, "usb-controller", 0);
        if (!usb_node) {
                DP_ERR("unable to get usb node\n");
                return;
        }
        usb_pdev = of_find_device_by_node(usb_node);
        if (!usb_pdev) {
                of_node_put(usb_node);
                DP_ERR("unable to get usb pdev\n");
                return;
        }
        while (timeout) {
                rc = dwc3_msm_set_dp_state(&usb_pdev->dev, dp_state);
                if (rc != -EBUSY)
                        break;

                DP_WARN("USB busy, retry\n");

                /* wait for hw recommended delay for usb */
                msleep(20);
                timeout--;
        }
        of_node_put(usb_node);
        platform_device_put(usb_pdev);

        if (rc)
                DP_ERR("Error set dp_state to dwc3 msm: %d\n", rc);
}
#endif

static void dp_altmode_send_pan_ack(struct altmode_client *amclient,
		u8 port_index)
{
	int rc;
	struct altmode_pan_ack_msg ack;

	ack.cmd_type = ALTMODE_PAN_ACK;
	ack.port_index = port_index;

	rc = altmode_send_data(amclient, &ack, sizeof(ack));
	if (rc < 0) {
		DP_ERR("failed: %d\n", rc);
		return;
	}

	DP_DEBUG("port=%d\n", port_index);
}

static void dp_altmode_disconnect(struct dp_altmode_private *altmode)
{
	if (!altmode->connected)
		return;

	altmode->connected = false;
#ifdef CONFIG_HUAWEI_POWER_EMBEDDED_ISOLATION
	dp_altmode_set_dp_state(altmode, false);
#endif
	altmode->dp_altmode.base.alt_mode_cfg_done = false;
	altmode->dp_altmode.base.orientation = ORIENTATION_NONE;
	if (altmode->dp_cb && altmode->dp_cb->disconnect)
		altmode->dp_cb->disconnect(altmode->dev);
}

static void dp_altmode_save_payload(struct dp_altmode_private *altmode,
	u8 hpd_state, u8 *payload, size_t len)
{
	bool hpd_high = !!hpd_state;
	u32 *mode_magic = &((u32 *)altmode->payload)[ALTMODE_PAYLOAD_MODE_IDX];

	(void)len;
	// 1. hpd_high is false, the current state is dp disconnect
	// 2. base.hpd_high is true, the current state is dp connected
	if (!hpd_high || altmode->dp_altmode.base.hpd_high) {
		return;
	}

	memcpy(altmode->payload, payload, sizeof(altmode->payload));
	*mode_magic = ALTMODE_PAYLOAD_MODE_MAGIC;
	DP_INFO("payload with mode magic %*ph\n", sizeof(altmode->payload), altmode->payload);
}

static bool dp_altmode_notify_from_source(u8 *payload, size_t len)
{
	u32 mode_magic = ((u32 *)payload)[ALTMODE_PAYLOAD_MODE_IDX];

	(void)len;
	DP_INFO("mode magic is 0x%x\n", mode_magic);
	if (mode_magic != ALTMODE_PAYLOAD_MODE_MAGIC)
		return false;

	DP_INFO("The payload data from source switch\n");
	return true;
}

static int dp_altmode_notify(void *priv, void *data, size_t len)
{
	int rc = 0;
	struct dp_altmode_private *altmode =
			(struct dp_altmode_private *) priv;
	u8 port_index, dp_data, orientation;
	u8 *payload = (u8 *) data;
	u8 pin, hpd_state, hpd_irq;
	bool force_multi_func = altmode->dp_altmode.base.force_multi_func;
	bool source = false;

	mutex_lock(&altmode->lock);
	port_index = payload[0];
	orientation = payload[1];
	dp_data = payload[8];

	pin = dp_data & ALTMODE_CONFIGURE_MASK;
	hpd_state = (dp_data & ALTMODE_HPD_STATE_MASK) >> 6;
	hpd_irq = (dp_data & ALTMODE_HPD_IRQ_MASK) >> 7;

	source = dp_altmode_notify_from_source(payload, len);
	dp_altmode_save_payload(altmode, hpd_state, payload, len);

	altmode->dp_altmode.base.hpd_high = !!hpd_state;
	altmode->dp_altmode.base.hpd_irq = !!hpd_irq;
	altmode->dp_altmode.base.multi_func = force_multi_func ? true :
		!(pin == DPAM_HPD_C || pin == DPAM_HPD_E ||
		pin == DPAM_HPD_OUT);

	DP_DEBUG("payload=0x%x\n", dp_data);
	DP_INFO("port_index=%d, orientation=%d, pin=%d, hpd_state=%d\n",
			port_index, orientation, pin, hpd_state);
	DP_INFO("multi_func=%d, hpd_high=%d, hpd_irq=%d\n",
			altmode->dp_altmode.base.multi_func,
			altmode->dp_altmode.base.hpd_high,
			altmode->dp_altmode.base.hpd_irq);
	DP_INFO("connected=%d\n", altmode->connected);
	SDE_EVT32_EXTERNAL(dp_data, port_index, orientation, pin, hpd_state,
			altmode->dp_altmode.base.multi_func,
			altmode->dp_altmode.base.hpd_high,
			altmode->dp_altmode.base.hpd_irq, altmode->connected);

	if (!pin) {
#ifdef CONFIG_HUAWEI_PD
		pd_dpm_send_event(DP_CABLE_OUT_EVENT);
#endif
		dp_cust_set_connect_state(false);
		memset(altmode->payload, 0, sizeof(altmode->payload));
		cancel_work_sync(&altmode->connect_work);

		/* Cable detach */
		if (altmode->connected) {
			altmode->connected = false;
#ifdef CONFIG_HUAWEI_POWER_EMBEDDED_ISOLATION
			dp_altmode_set_dp_state(altmode, false);
#endif
			altmode->dp_altmode.base.alt_mode_cfg_done = false;
			altmode->dp_altmode.base.orientation = ORIENTATION_NONE;
			if (altmode->dp_cb && altmode->dp_cb->disconnect)
				altmode->dp_cb->disconnect(altmode->dev);
		}
		goto ack;
	}
#ifdef CONFIG_HUAWEI_PD
	else {
		pd_dpm_send_event(DP_CABLE_IN_EVENT);
	}
#endif

	if (dp_factory_get_state() == DP_FACTORY_FAILED) {
		DP_ERR("factory test failed, skip\n");
		goto ack;
	}

	/* Configure */
	if (!altmode->connected) {
		dp_cust_set_connect_state(true);
		dp_cust_set_param(DP_PARAM_CABLE_TYPE, &pin, sizeof(pin));
		altmode->connected = true;
#ifdef CONFIG_HUAWEI_POWER_EMBEDDED_ISOLATION
		dp_altmode_set_dp_state(altmode, true);
#endif
		altmode->dp_altmode.base.alt_mode_cfg_done = true;
		altmode->forced_disconnect = false;

		// check source mode before dp connect
		altmode->dp_cb->check_source(altmode->dev);

		switch (orientation) {
		case 0:
			orientation = ORIENTATION_CC1;
			break;
		case 1:
			orientation = ORIENTATION_CC2;
			break;
		case 2:
			orientation = ORIENTATION_NONE;
			break;
		default:
			orientation = ORIENTATION_NONE;
			break;
		}

		altmode->dp_altmode.base.orientation = orientation;

		rc = dp_altmode_release_ss_lanes(altmode,
				altmode->dp_altmode.base.multi_func);
		if (rc)
			goto ack;

		if (altmode->dp_cb && altmode->dp_cb->configure)
			altmode->dp_cb->configure(altmode->dev);

		cancel_work_sync(&altmode->connect_work);
		queue_work(system_freezable_wq, &altmode->connect_work);
		goto ack;
	}

	/* Attention */
	if (altmode->forced_disconnect)
		goto ack;

	if (altmode->dp_cb && altmode->dp_cb->attention)
		altmode->dp_cb->attention(altmode->dev);
ack:
	if (!source)
		dp_altmode_send_pan_ack(altmode->amclient, port_index);
	mutex_unlock(&altmode->lock);
	return rc;
}

static void dp_altmode_register(void *priv)
{
	struct dp_altmode_private *altmode = priv;
	struct altmode_client_data cd = {
		.callback	= &dp_altmode_notify,
	};

	cd.name = "displayport";
	cd.svid = USB_SID_DISPLAYPORT;
	cd.priv = altmode;

	altmode->amclient = altmode_register_client(altmode->dev, &cd);
	if (IS_ERR_OR_NULL(altmode->amclient))
		DP_ERR("failed to register as client: %d\n",
				PTR_ERR(altmode->amclient));
	else
		DP_DEBUG("success\n");
}

static int dp_altmode_simulate_connect(struct dp_hpd *dp_hpd, bool hpd)
{
	struct dp_altmode *dp_altmode;
	struct dp_altmode_private *altmode;

	dp_altmode = container_of(dp_hpd, struct dp_altmode, base);
	altmode = container_of(dp_altmode, struct dp_altmode_private,
			dp_altmode);

	dp_altmode->base.hpd_high = hpd;
	altmode->forced_disconnect = !hpd;
	altmode->dp_altmode.base.alt_mode_cfg_done = hpd;

	if (hpd)
		altmode->dp_cb->configure(altmode->dev);
	else
		altmode->dp_cb->disconnect(altmode->dev);

	return 0;
}

static int dp_altmode_simulate_attention(struct dp_hpd *dp_hpd, int vdo)
{
	struct dp_altmode *dp_altmode;
	struct dp_altmode_private *altmode;
	struct dp_altmode *status;

	dp_altmode = container_of(dp_hpd, struct dp_altmode, base);
	altmode = container_of(dp_altmode, struct dp_altmode_private,
			dp_altmode);

	status = &altmode->dp_altmode;

	status->base.hpd_high  = (vdo & BIT(7)) ? true : false;
	status->base.hpd_irq   = (vdo & BIT(8)) ? true : false;

	if (altmode->dp_cb && altmode->dp_cb->attention)
		altmode->dp_cb->attention(altmode->dev);

	return 0;
}

static void dp_altmode_connect_work_fn(struct work_struct *work)
{
	struct dp_altmode_private *altmode = container_of(work,
		struct dp_altmode_private, connect_work);

	if (!altmode || !altmode->dp_cb)
		return;

	if (altmode->dp_cb->connect_comp(altmode->dev) < 0) {
		DP_ERR("dp wait connect_comp timeout\n");
		dp_cust_set_param(DP_PARAM_CONNECT_TIMEOUT, NULL, 0);
		return;
	}

	if (dp_factory_is_link_downgrade() || !dp_factory_is_4k_60fps()) {
		DP_ERR("factory test failed, disconnect\n");
		mutex_lock(&altmode->lock);
		dp_altmode_disconnect(altmode);
		mutex_unlock(&altmode->lock);
	}
}

static int dp_altmode_source_callback(struct notifier_block *nb, unsigned long mode, void *ptr)
{
	struct dp_altmode_private *altmode = container_of(nb, struct dp_altmode_private, source_nb);

	if (!altmode || !altmode->dp_cb)
		return -EINVAL;

	// set source mode to dp_display
	altmode->dp_cb->switch_source(altmode->dev, (int)mode);

	// dp: connect --> disconnect --> connect
	if (altmode->connected) {
		mutex_lock(&altmode->lock);
		dp_altmode_disconnect(altmode);
		mutex_unlock(&altmode->lock);

		// 1. dp configure, altmode->connected == 0
		dp_altmode_notify(altmode, altmode->payload, sizeof(altmode->payload));
		// 2. dp attention, altmode->connected == 1
		dp_altmode_notify(altmode, altmode->payload, sizeof(altmode->payload));
	}

	return 0;
}

struct dp_hpd *dp_altmode_get(struct device *dev, struct dp_hpd_cb *cb)
{
	int rc = 0;
	struct dp_altmode_private *altmode;
	struct dp_altmode *dp_altmode;

	if (!cb) {
		DP_ERR("invalid cb data\n");
		return ERR_PTR(-EINVAL);
	}

	altmode = kzalloc(sizeof(*altmode), GFP_KERNEL);
	if (!altmode)
		return ERR_PTR(-ENOMEM);

	altmode->dev = dev;
	altmode->dp_cb = cb;

	dp_altmode = &altmode->dp_altmode;
	dp_altmode->base.register_hpd = NULL;
	dp_altmode->base.simulate_connect = dp_altmode_simulate_connect;
	dp_altmode->base.simulate_attention = dp_altmode_simulate_attention;

	rc = altmode_register_notifier(dev, dp_altmode_register, altmode);
	if (rc < 0) {
		DP_ERR("altmode probe notifier registration failed: %d\n", rc);
		goto error;
	}

	mutex_init(&altmode->lock);
	INIT_WORK(&altmode->connect_work, dp_altmode_connect_work_fn);
	altmode->source_nb.notifier_call = dp_altmode_source_callback;
	altmode->source_nb.priority = 0;
	rc = dp_source_reg_notifier(&altmode->source_nb);
	// Failure cannot be returned here, otherwise the display will be abnormal
	if (rc < 0) {
		DP_INFO("altmode source notifier registration failed %d\n", rc);
	}

	DP_DEBUG("success\n");

	return &dp_altmode->base;
error:
	kfree(altmode);
	return ERR_PTR(rc);
}

void dp_altmode_put(struct dp_hpd *dp_hpd)
{
	struct dp_altmode *dp_altmode;
	struct dp_altmode_private *altmode;

	dp_altmode = container_of(dp_hpd, struct dp_altmode, base);
	if (!dp_altmode)
		return;

	altmode = container_of(dp_altmode, struct dp_altmode_private,
			dp_altmode);

	altmode_deregister_client(altmode->amclient);
	altmode_deregister_notifier(altmode->dev, altmode);

	mutex_destroy(&altmode->lock);
	cancel_work_sync(&altmode->connect_work);
	dp_source_unreg_notifier(&altmode->source_nb);

	kfree(altmode);
}
