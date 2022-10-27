/*
 * dp_cust_info.c
 *
 * dp custom info driver
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
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/of_platform.h>
#include <linux/usb/typec.h>
#include <linux/usb/ucsi_glink.h>
#include <linux/iio/consumer.h>
#include <drm/drm_edid.h>
#include <log/hw_log.h>
#include "dp_cust_info.h"

#define HWLOG_TAG dp_cust
HWLOG_REGIST();

#define DP_LINK_BW_1_62    0x06 // RBR
#define DP_LINK_BW_2_7     0x0a // HBR
#define DP_LINK_BW_5_4     0x14 // HBR2, dp1.2
#define DP_LINK_BW_8_1     0x1e // HBR3, dp1.4

#define DP_RECEIVER_EXT_CAP_SIZE 4
#define DP_MAX_DOWNSTREAM_PORTS  0x10
#define DP_DS_PORT0_OFFSET_0     0
#define DP_DS_PORT0_OFFSET_2     2
#define DP_HEX_DUMP_ROW_SIZE     16 // 16 bytes per line
#define DP_HEX_DUMP_BUF_SIZE     64
#define DP_SINK_NAME_LEN_MAX     20

#define DP_EDID_READ_OFFSET      0x00
#define DP_EDID_LINE_LEN         16
#define DP_EDID_LINE_NUM         \
	((DP_EDID_BLOCK_NUM_MAX * DP_EDID_BLOCK_SIZE) / DP_EDID_LINE_LEN)

#define DP_CONVERT_DATA_TO_INT8(d)  (!(d) ? 0 : *(int8_t *)(d))
#define DP_CONVERT_DATA_TO_INT(d)   (!(d) ? 0 : *(int *)(d))
#define DP_CONVERT_DATA_TO_UINT8(d) (!(d) ? 0 : *(uint8_t *)(d))
#define DP_CONVERT_DATA_TO_UINT(d)  (!(d) ? 0 : *(uint32_t *)(d))

struct dp_cust_info_priv {
	struct device *dev;
	struct dp_factory_info fac_info;
	struct dp_source_info src_info;
	struct dp_dsm_info dsm_info;
	bool connected;

	uint8_t dpcd[DP_RECEIVER_CAP_SIZE + DP_RECEIVER_EXT_CAP_SIZE + 1];
	uint8_t ds_ports[DP_MAX_DOWNSTREAM_PORTS];
	uint8_t edid[DP_EDID_BLOCK_NUM_MAX * DP_EDID_BLOCK_SIZE];
	uint32_t max_supported_bpp;
	char sink_name[DP_SINK_NAME_LEN_MAX];

	// for debug
	int debug_bpp; // bits per pixel
	int debug_v_level; // voltage swing level
	int debug_p_level; // pre-emphasis level
	int debug_revision; // dpcd revision
	int debug_edid_size;
	int debug_edid_index;
	uint8_t debug_edid[DP_EDID_BLOCK_NUM_MAX * DP_EDID_BLOCK_SIZE];
};

static struct dp_cust_info_priv *cust_info_priv;

struct dp_factory_info *dp_cust_get_factory_info(void)
{
	if (!cust_info_priv)
		return NULL;

	return &cust_info_priv->fac_info;
}

struct dp_source_info *dp_cust_get_source_info(void)
{
	if (!cust_info_priv)
		return NULL;

	return &cust_info_priv->src_info;
}

struct dp_dsm_info *dp_cust_get_dsm_info(void)
{
	if (!cust_info_priv)
		return NULL;

	return &cust_info_priv->dsm_info;
}

#ifdef DP_DEBUG_ENABLE
void dp_cust_hex_dump(const char *prefix, void *data, int size)
{
	const int rowsize = DP_HEX_DUMP_ROW_SIZE;
	int linelen;
	int remaining = size;
	uint8_t linebuf[DP_HEX_DUMP_BUF_SIZE] = {0};
	int i;

	if (!prefix || !data || (size <= 0))
		return;

	hwlog_info("%s: %s size %d\n", __func__, prefix, size);
	for (i = 0; i < size; i += rowsize) {
		linelen = min(remaining, rowsize);
		remaining -= rowsize;

		hex_dump_to_buffer(data + i, linelen, rowsize, 1,
			linebuf, sizeof(linebuf), false);
		hwlog_info("%s: %s[%02x] %s\n", __func__, prefix, i, linebuf);
	}
}

int dp_cust_get_debug_revision(int *revision)
{
	if (!cust_info_priv || !revision)
		return -EINVAL;

	*revision = cust_info_priv->debug_revision;
	return 0;
}

int dp_cust_set_debug_revision(int revision)
{
	if (!cust_info_priv)
		return -EINVAL;

	if (revision < 0)
		cust_info_priv->debug_revision = -1; // clear revision
	else
		cust_info_priv->debug_revision = revision;
	return 0;
}

void dp_cust_get_dpcd_revision(uint8_t *revision)
{
	if (!cust_info_priv || !revision)
		return;

	if (cust_info_priv->debug_revision < 0)
		return;

	*revision = (uint8_t)(cust_info_priv->debug_revision);
	hwlog_info("%s: debug revision %02x\n", __func__, *revision);
}

int dp_cust_get_debug_bpp(int *bpp)
{
	if (!cust_info_priv || !bpp)
		return -EINVAL;

	*bpp = cust_info_priv->debug_bpp;
	return 0;
}

int dp_cust_set_debug_bpp(int bpp)
{
	if (!cust_info_priv)
		return -EINVAL;

	if (bpp < 0)
		cust_info_priv->debug_bpp = -1; // clear bpp
	else
		cust_info_priv->debug_bpp = MAX(bpp, DP_BPP_SUPPORTED_MIN);
	return 0;
}

int dp_cust_get_debug_vs_pe(int *v_level, int *p_level)
{
	if (!cust_info_priv || !v_level || !p_level)
		return -EINVAL;

	*v_level = cust_info_priv->debug_v_level;
	*p_level = cust_info_priv->debug_p_level;
	return 0;
}

int dp_cust_set_debug_vs_pe(int v_level, int p_level)
{
	if (!cust_info_priv)
		return -EINVAL;

	cust_info_priv->debug_v_level = v_level;
	cust_info_priv->debug_p_level = p_level;
	return 0;
}

void dp_cust_get_vs_pe(uint8_t *v_level, uint8_t *p_level)
{
	if (!cust_info_priv || !v_level || !p_level)
		return;

	hwlog_info("%s: current v_level %u, p_level %u\n", __func__, *v_level, *p_level);
	if ((cust_info_priv->debug_v_level < 0) || (cust_info_priv->debug_p_level < 0))
		return;

	*v_level = (uint8_t)(cust_info_priv->debug_v_level);
	*p_level = (uint8_t)(cust_info_priv->debug_p_level);
	hwlog_info("%s: debug v_level %u, p_level %u\n", __func__, *v_level, *p_level);
}

int dp_cust_get_debug_edid(uint8_t **edid, int **size)
{
	if (!cust_info_priv)
		return -EINVAL;

	cust_info_priv->debug_edid_index = 0;
	cust_info_priv->debug_edid_size = sizeof(cust_info_priv->debug_edid);

	*edid = cust_info_priv->debug_edid;
	*size = &(cust_info_priv->debug_edid_size);
	return 0;
}

void dp_cust_get_edid(bool native, bool read, struct drm_dp_aux_msg *msg)
{
	u8 *edid = NULL;
	u8 *buf = NULL;
	int index_max;

	if (!cust_info_priv || !msg)
		return;

	if (cust_info_priv->debug_edid_size <= 0)
		return;

	if (native || (msg->address != DDC_ADDR))
		return;

	index_max = cust_info_priv->debug_edid_size / DP_EDID_LINE_LEN;
	edid = cust_info_priv->debug_edid;
	buf = msg->buffer;

	// I2C WR   50h( 1): 00
	// I2C WR   50h( 1): 80
	if (!read && (msg->size == 1) && (buf[0] == DP_EDID_READ_OFFSET)) {
		hwlog_info("%s: from debug edid %d,%d\n", __func__,
	  		index_max, cust_info_priv->debug_edid_size);
		cust_info_priv->debug_edid_index = 0;
	}

	if (cust_info_priv->debug_edid_index >= index_max)
		return;

	// I2C RD   50h(16)
	if (read && (msg->size == DP_EDID_LINE_LEN)) {
		memcpy(buf, &edid[cust_info_priv->debug_edid_index *
			DP_EDID_LINE_LEN], DP_EDID_LINE_LEN);
		cust_info_priv->debug_edid_index++;
	}
}
#else
void dp_cust_hex_dump(const char *prefix, void *data, int size)
{
	UNUSED(prefix);
	UNUSED(data);
	UNUSED(size);
}

void dp_cust_get_dpcd_revision(uint8_t *revision)
{
	UNUSED(revision);
}

void dp_cust_get_vs_pe(uint8_t *v_level, uint8_t *p_level)
{
	UNUSED(v_level);
	UNUSED(p_level);
}

void dp_cust_get_edid(bool native, bool read, struct drm_dp_aux_msg *msg)
{
	UNUSED(native);
	UNUSED(read);
	UNUSED(msg);
}
#endif // !DP_DEBUG_ENABLE
EXPORT_SYMBOL(dp_cust_hex_dump);
EXPORT_SYMBOL(dp_cust_get_dpcd_revision);
EXPORT_SYMBOL(dp_cust_get_vs_pe);
EXPORT_SYMBOL(dp_cust_get_edid);

static void dp_cust_dsm_report_prepare(struct dp_cust_info_priv *priv)
{
	struct dp_dsm_info *info = &priv->dsm_info;

	hwlog_info("%s: hotplug retval %d\n", __func__, info->hotplug_retval);
	hwlog_info("%s: link train retval %d\n", __func__, info->link_train_retval);
	hwlog_info("%s: aux rw failed count %d\n", __func__, info->aux_rw_count);
	hwlog_info("%s: aux rw retval %d\n", __func__, info->aux_rw_retval);
	if (info->hotplug_retval != 0 || info->link_train_retval != 0)
		dp_dsm_imonitor_report(DP_IMONITOR_LINK_TRAINING);
}

static void dp_cust_init_prams(struct dp_cust_info_priv *priv)
{
	memset(priv->dpcd, 0, sizeof(priv->dpcd));
	memset(priv->ds_ports, 0, sizeof(priv->ds_ports));
	memset(priv->edid, 0, sizeof(priv->edid));
	priv->max_supported_bpp = DP_BPP_SUPPORTED_HDR_MAX;
	memset(priv->sink_name, 0, sizeof(priv->sink_name));
}

void dp_cust_set_connect_state(bool connected)
{
	if (!cust_info_priv)
		return;

	if (connected) {
		if (cust_info_priv->connected)
			return;

		hwlog_info("%s: cable in, dp is connected\n", __func__);
		cust_info_priv->connected = true;
		dp_cust_init_prams(cust_info_priv);
		dp_dsm_init_link_status(&cust_info_priv->dsm_info);
		dp_source_send_event(DP_LINK_CABLE_IN);

		cust_info_priv->fac_info.test_running = true;
		cust_info_priv->fac_info.factory_state = DP_FACTORY_CONNECTED;
		dp_factory_init_link_status(&cust_info_priv->fac_info);
		dp_factory_send_event(DP_MANUFACTURE_LINK_CABLE_IN);
	} else { // disconnect
		if (!cust_info_priv->connected)
			return;

		hwlog_info("%s: cable out, dp is disconnected\n", __func__);
		cust_info_priv->connected = false;
		dp_source_send_event(DP_LINK_CABLE_OUT);

		cust_info_priv->fac_info.factory_state = DP_FACTORY_PENDING;
		dp_factory_send_event(DP_MANUFACTURE_LINK_CABLE_OUT);

		// dsm report
		dp_cust_dsm_report_prepare(cust_info_priv);
	}
}

static int dp_cust_get_link_rate(uint32_t bw_code)
{
	uint32_t rate;

	switch (bw_code) {
	case DP_LINK_BW_8_1:
		rate = DP_LINK_RATE_HBR3;
		break;
	case DP_LINK_BW_5_4:
		rate = DP_LINK_RATE_HBR2;
		break;
	case DP_LINK_BW_2_7:
		rate = DP_LINK_RATE_HBR;
		break;
	case DP_LINK_BW_1_62:
	default:
		rate = DP_LINK_RATE_RBR;
		break;
	}
	return rate;
}

void dp_cust_set_sink_lanes_rate(uint32_t lanes, uint32_t rate)
{
	if (!cust_info_priv)
		return;

	cust_info_priv->fac_info.sink_lanes = lanes;
	cust_info_priv->fac_info.sink_rate = dp_cust_get_link_rate(rate);
}
EXPORT_SYMBOL(dp_cust_set_sink_lanes_rate);

void dp_cust_set_init_lanes_rate(uint32_t lanes, uint32_t rate)
{
	if (!cust_info_priv)
		return;

	cust_info_priv->fac_info.init_lanes = lanes;
	cust_info_priv->fac_info.init_rate = dp_cust_get_link_rate(rate);
}
EXPORT_SYMBOL(dp_cust_set_init_lanes_rate);

void dp_cust_set_link_lanes_rate(uint32_t lanes, uint32_t rate)
{
	if (!cust_info_priv)
		return;

	cust_info_priv->fac_info.link_lanes = lanes;
	cust_info_priv->fac_info.link_rate = dp_cust_get_link_rate(rate);
}
EXPORT_SYMBOL(dp_cust_set_link_lanes_rate);

void dp_cust_set_resolution_fps(uint32_t h, uint32_t v, uint32_t fps)
{
	if (!cust_info_priv)
		return;

	cust_info_priv->fac_info.resolution_h = h;
	cust_info_priv->fac_info.resolution_v = v;
	cust_info_priv->fac_info.fps = fps;
}
EXPORT_SYMBOL(dp_cust_set_resolution_fps);

static void dp_cust_set_param_aux_rw(struct dp_cust_info_priv *priv, int err_no)
{
	priv->dsm_info.aux_rw_count++;

	// The first report of dp_aux reading and writing errors
	if (priv->dsm_info.aux_rw_count == DP_AUX_RW_COUNT_FIRST) {
		priv->dsm_info.aux_rw_retval = err_no;
	} else if (priv->dsm_info.aux_rw_count > DP_AUX_RW_COUNT_THRESHOLD) {
		priv->fac_info.aux_rw_state = DP_AUX_RW_FAILED;
	}
}

static void dp_cust_set_param_connect_timeout(struct dp_cust_info_priv *priv)
{
	priv->dsm_info.connect_timeout = true;
	priv->fac_info.connect_timeout = true;

	if (priv->fac_info.aux_rw_state == DP_AUX_RW_FAILED) {
		hwlog_err("%s: aux rw failed\n", __func__);
		dp_factory_send_event(DP_MANUFACTURE_LINK_AUX_FAILED);
		return;
	}

	hwlog_err("%s: connect timeout\n", __func__);
	dp_factory_send_event(DP_MANUFACTURE_LINK_HPD_NOT_EXISTED);
}

static void dp_cust_set_param_edid(struct dp_cust_info_priv *priv, uint8_t *data, int size)
{
	int edid_len = MIN(sizeof(priv->edid), size);
	struct edid *edid = (struct edid *)data;

	if (!data || (size <= 0))
		return;

	memset(priv->edid, 0, sizeof(priv->edid));
	memcpy(priv->edid, data, edid_len);
	dp_cust_hex_dump("edid", data, size);

	drm_edid_get_monitor_name(edid, priv->sink_name, sizeof(priv->sink_name));
#ifdef DP_DEBUG_ENABLE
	hwlog_info("%s: sink name %s\n", __func__, priv->sink_name);
#endif
}

static void dp_cust_parse_bpp(struct dp_cust_info_priv *priv)
{
	uint32_t max_bpp = DP_BPP_SUPPORTED_HDR_MAX;
	uint8_t bpc;
	uint8_t dfp_type;
	bool branch_device;

	// down stream port present
	branch_device = priv->dpcd[DP_DOWNSTREAMPORT_PRESENT] & DP_DWN_STRM_PORT_PRESENT;
	if (branch_device) {
		hwlog_info("%s: Device is a Branch device and has DFPs\n", __func__);

		dfp_type = priv->ds_ports[DP_DS_PORT0_OFFSET_0] & DP_DS_PORT_TYPE_MASK;
		hwlog_info("%s: current dfpx type %d\n", __func__, dfp_type);

		if (dfp_type != DP_DS_PORT_TYPE_DP) {
			max_bpp = DP_BPP_SUPPORTED_MAX;
		} else {
			bpc = priv->ds_ports[DP_DS_PORT0_OFFSET_2] & DP_DS_MAX_BPC_MASK;
			hwlog_info("%s: current bpc %d\n", __func__, bpc);

			if (bpc == DP_DS_8BPC)
				max_bpp = DP_BPP_SUPPORTED_MAX;
		}
	} else {
		dfp_type = (priv->dpcd[DP_DOWNSTREAMPORT_PRESENT] &
			DP_DWN_STRM_PORT_TYPE_MASK) >> 1;
		hwlog_info("%s: current dfp type %d\n", __func__, dfp_type);
	}

	priv->max_supported_bpp = max_bpp;
	hwlog_info("%s: current max bpp %d\n", __func__, max_bpp);
}

uint32_t dp_cust_get_supported_bpp(void)
{
	if (!cust_info_priv)
		return DP_BPP_SUPPORTED_HDR_MAX;

	if (cust_info_priv->debug_bpp > 0) {
		hwlog_info("%s: current debug bpp %d\n", __func__, cust_info_priv->debug_bpp);
		return (uint32_t)(cust_info_priv->debug_bpp);
	}

	return cust_info_priv->max_supported_bpp;
}
EXPORT_SYMBOL(dp_cust_get_supported_bpp);

static void dp_cust_set_param_dpcd(struct dp_cust_info_priv *priv, uint8_t *data, int size)
{
	int dpcd_len = MIN(sizeof(priv->dpcd), size);

	if (!data || (size <= 0))
		return;

	memset(priv->dpcd, 0, sizeof(priv->dpcd));
	memcpy(priv->dpcd, data, dpcd_len);

	hwlog_info("%s: dpcd version %02x\n", __func__, priv->dpcd[DP_DPCD_REV]);
	dp_cust_hex_dump("dpcd", data, size);

	dp_cust_parse_bpp(priv);
}

static void dp_cust_set_param_ds_ports(struct dp_cust_info_priv *priv, uint8_t *data, int size)
{
	int ports_len = MIN(sizeof(priv->ds_ports), size);

	if (!data || (size <= 0))
		return;

	memset(priv->ds_ports, 0, sizeof(priv->ds_ports));
	memcpy(priv->ds_ports, data, ports_len);
	hwlog_info("%s: size %d\n", __func__, ports_len);
	dp_cust_hex_dump("ds_ports", data, size);
}

void dp_cust_set_param(enum dp_cust_param param, void *data, int size)
{
	UNUSED(size);
	if (!cust_info_priv)
		return;

	if (param >= DP_PARAM_NUM) {
		hwlog_info("%s: invalid param %d\n", __func__, param);
		return;
	}

	switch (param) {
	case DP_PARAM_CABLE_TYPE:
		cust_info_priv->dsm_info.cable_type = (int)DP_CONVERT_DATA_TO_UINT8(data);
		break;
	case DP_PARAM_HOTPLUG_RETVAL:
		cust_info_priv->dsm_info.hotplug_retval = DP_CONVERT_DATA_TO_INT(data);
		cust_info_priv->fac_info.hotplug_retval = DP_CONVERT_DATA_TO_INT(data);
		if (cust_info_priv->dsm_info.hotplug_retval != 0)
			dp_source_send_event(DP_LINK_LINK_FAILED);
		break;
	case DP_PARAM_LINK_TRAIN_RETVAL:
		cust_info_priv->dsm_info.link_train_retval = DP_CONVERT_DATA_TO_INT(data);
		cust_info_priv->fac_info.link_train_retval = DP_CONVERT_DATA_TO_INT(data);
		if (cust_info_priv->dsm_info.link_train_retval != 0)
			dp_source_send_event(DP_LINK_LINK_FAILED);
		break;
	case DP_PARAM_AUX_ERR_NO:
		dp_cust_set_param_aux_rw(cust_info_priv, DP_CONVERT_DATA_TO_INT(data));
		break;
	case DP_PARAM_SAFE_MODE:
		cust_info_priv->dsm_info.safe_mode = true;
		cust_info_priv->fac_info.safe_mode = true;
		dp_source_send_event(DP_LINK_SAFE_MODE);
		break;
	case DP_PARAM_CONNECT_TIMEOUT:
		dp_cust_set_param_connect_timeout(cust_info_priv);
		break;
	case DP_PARAM_AUDIO_SUPPORTED:
		cust_info_priv->fac_info.audio_supported = true;
		break;
	case DP_PARAM_EDID:
		dp_cust_set_param_edid(cust_info_priv, data, size);
		break;
	case DP_PARAM_DPCD:
		dp_cust_set_param_dpcd(cust_info_priv, data, size);
		break;
	case DP_PARAM_DS_PORT:
		dp_cust_set_param_ds_ports(cust_info_priv, data, size);
		break;
	default:
		break;
	}
}
EXPORT_SYMBOL(dp_cust_set_param);

int dp_cust_get_property_value(struct device_node *node,
	const char *prop_name, int default_value)
{
	int value = 0;
	int ret;

	ret = of_property_read_u32(node, prop_name, &value);
	if (ret < 0)
		value = default_value;

	hwlog_info("%s: %s is %d\n", __func__, prop_name, value);
	return value;
}

bool dp_cust_get_property_bool(struct device_node *node,
	const char *prop_name, bool default_setting)
{
	bool setting = false;

	if (!of_find_property(node, prop_name, NULL))
		setting = default_setting;
	else
		setting = of_property_read_bool(node, prop_name);

	hwlog_info("%s: %s is %d\n", __func__, prop_name, setting);
	return setting;
}

static int dp_cust_info_probe(struct platform_device *pdev)
{
	struct dp_cust_info_priv *priv = NULL;
	int ret;

	priv = devm_kzalloc(&pdev->dev, sizeof(*priv), GFP_KERNEL);
	if (!priv)
		return -ENOMEM;

	priv->dev = &pdev->dev;
	platform_set_drvdata(pdev, priv);

	priv->fac_info.node = pdev->dev.of_node;
	ret = dp_factory_init(&priv->fac_info);
	if (ret < 0) {
		hwlog_err("%s: dp factory init failed\n", __func__);
		goto err_out_fac;
	}

	priv->src_info.node = pdev->dev.of_node;
	ret = dp_source_init(&priv->src_info);
	if (ret < 0) {
		hwlog_err("%s: dp source init failed\n", __func__);
		goto err_out_src;
	}

	ret = dp_dsm_init(&priv->dsm_info);
	if (ret < 0)
		hwlog_info("%s: dp dsm init failed\n", __func__);

	priv->debug_bpp = -1;
	priv->debug_v_level = -1;
	priv->debug_p_level = -1;
	priv->debug_revision = -1;
	priv->debug_edid_size = 0;
	priv->debug_edid_index = -1;

	cust_info_priv = priv;
	hwlog_info("%s: probe success\n", __func__);
	return 0;

err_out_src:
	dp_source_exit(&priv->src_info);
err_out_fac:
	devm_kfree(&pdev->dev, priv);
	return 0;
}

static int dp_cust_info_remove(struct platform_device *pdev)
{
	struct dp_cust_info_priv *priv = NULL;

	if (!pdev)
		return -EINVAL;

	priv = platform_get_drvdata(pdev);
	if (!priv)
		return -EINVAL;

	priv->connected = false;

	dp_factory_exit(&priv->fac_info);
	dp_source_exit(&priv->src_info);
	dp_dsm_exit(&priv->dsm_info);

	dev_set_drvdata(&pdev->dev, NULL);
	return 0;
}

static const struct of_device_id dp_cust_info_dt_match[] = {
	{ .compatible = "honor,dp_cust_info", },
	{}
};
MODULE_DEVICE_TABLE(of, dp_cust_info_dt_match);

static struct platform_driver dp_cust_info_driver = {
	.driver = {
		.name           = "dp_cust_info",
		.owner          = THIS_MODULE,
		.of_match_table = of_match_ptr(dp_cust_info_dt_match),
	},
	.probe  = dp_cust_info_probe,
	.remove = dp_cust_info_remove,
};

static int __init dp_cust_info_init(void)
{
	int ret;

	ret = platform_driver_register(&dp_cust_info_driver);
	if (ret < 0)
		hwlog_err("%s: Failed to register driver %d\n", __func__, ret);

	hwlog_info("%s: register driver success\n", __func__);
	return ret;
}
module_init(dp_cust_info_init);

static void __exit dp_cust_info_exit(void)
{
	platform_driver_unregister(&dp_cust_info_driver);
}
module_exit(dp_cust_info_exit);

MODULE_DESCRIPTION("dp custom info driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
