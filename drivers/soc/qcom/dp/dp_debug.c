/*
 * dp_debug.c
 *
 * dp debug driver
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
#include <log/hw_log.h>
#include "dp_cust_info.h"

#define HWLOG_TAG dp_cust
HWLOG_REGIST();

#define INFO_BUF_MAX       512
#define DP_DSM_VS_PE_NUM   4
#define DP_DSM_VS_PE_LINE  2
#define DP_INT_HEX_CODE    16 // hexadecimal base
#define DP_INT_DEC_CODE    0 //  decimal base
#define DP_INT_BYTE_LEN    2

static int dp_debug_get_revision(char *buffer, const struct kernel_param *kp)
{
	int revision = 0;

	UNUSED(kp);
	if (!buffer)
		return -EINVAL;

	dp_cust_get_debug_revision(&revision);
	return snprintf(buffer, (unsigned long)INFO_BUF_MAX, "%d(%x)\n",
		revision, (uint8_t)revision);
}

static int dp_debug_set_revision(const char *val, const struct kernel_param *kp)
{
	int revision = 0;

	UNUSED(kp);
	if (!val)
		return -EINVAL;

	if (kstrtoint(val, DP_INT_DEC_CODE, &revision) < 0) {
		hwlog_err("%s: invalid params %s\n", __func__, val);
		return -EINVAL;
	}

	dp_cust_set_debug_revision(revision);
	hwlog_info("%s: revision is %d(%x)\n", __func__, revision, (uint8_t)revision);
	return 0;
}

static int dp_debug_get_max_bpp(char *buffer, const struct kernel_param *kp)
{
	int bpp = 0;

	UNUSED(kp);
	if (!buffer)
		return -EINVAL;

	dp_cust_get_debug_bpp(&bpp);
	return snprintf(buffer, (unsigned long)INFO_BUF_MAX, "%d\n", bpp);
}

static int dp_debug_set_max_bpp(const char *val, const struct kernel_param *kp)
{
	int bpp = 0;

	UNUSED(kp);
	if (!val)
		return -EINVAL;

	if (kstrtoint(val, DP_INT_DEC_CODE, &bpp) < 0) {
		hwlog_err("%s: invalid params %s\n", __func__, val);
		return -EINVAL;
	}

	dp_cust_set_debug_bpp(bpp);
	hwlog_info("%s: bpp is %d\n", __func__, bpp);
	return 0;
}

static int dp_debug_get_vs_pe(char *buffer, const struct kernel_param *kp)
{
	int v_level = 0;
	int p_level = 0;

	UNUSED(kp);
	if (!buffer)
		return -EINVAL;

	dp_cust_get_debug_vs_pe(&v_level, &p_level);
	return snprintf(buffer, (unsigned long)INFO_BUF_MAX, "%d, %d\n", v_level, p_level);
}

static bool dp_debug_is_vs_pe_valid(int vs, int pe)
{
	int vs_pe_valid[DP_DSM_VS_PE_NUM * DP_DSM_VS_PE_LINE] = {
		// vs level, pe level max
		0, 3,
		1, 2,
		2, 1,
		3, 0
	};
	int i;

	if ((vs < 0) && (pe < 0))
		return true;

	for (i = 0; i < (DP_DSM_VS_PE_NUM * DP_DSM_VS_PE_LINE); i += DP_DSM_VS_PE_LINE) {
		if (vs_pe_valid[i] == vs) {
			if (pe <= vs_pe_valid[i + 1])
				return true;
			return false;
		}
	}

	return false;
}

static int dp_debug_set_vs_pe(const char *val, const struct kernel_param *kp)
{
	int v_level = 0;
	int p_level = 0;
	int ret;

	UNUSED(kp);
	if (!val)
		return -EINVAL;

	ret = sscanf(val, "%d, %d", &v_level, &p_level);
	if (ret != 2) {
		hwlog_err("%s: invalid params num %d\n", __func__, ret);
		return -EINVAL;
	}

	if (!dp_debug_is_vs_pe_valid(v_level, p_level)) {
		hwlog_err("%s: invalid v_level %d or p_level %d\n", __func__, v_level, p_level);
		return -EINVAL;
	}

	dp_cust_set_debug_vs_pe(v_level, p_level);
	hwlog_info("%s: debug v_level %d, p_level %d\n", __func__, v_level, p_level);
	return 0;
}

static int dp_debug_set_edid(const char *val, const struct kernel_param *kp)
{
	uint8_t *edid = NULL;
	int *edid_size = NULL;
	int index = 0;
	int size;
	int ret;

	UNUSED(kp);
	if (!val)
		return -EINVAL;

	size = strlen(val);
	if (size < (DP_EDID_BLOCK_SIZE * DP_INT_BYTE_LEN)) {
		hwlog_err("%s: invalid size %d\n", __func__, size);
		return -EINVAL;
	}
	size /= DP_INT_BYTE_LEN;

	ret = dp_cust_get_debug_edid(&edid, &edid_size);
	if ((ret < 0) || !edid || !edid_size)
		return -EINVAL;

	size = MIN(size, *edid_size);
	*edid_size = size;
	while (size--) {
		char t[DP_INT_BYTE_LEN + 1] = {0};
		int d = 0;

		memcpy(t, val, sizeof(char) * DP_INT_BYTE_LEN);
		t[DP_INT_BYTE_LEN] = '\0';
		if (kstrtoint(t, DP_INT_HEX_CODE, &d) < 0)
			break;

		if (index < (*edid_size))
			edid[index++] = d;

		val += DP_INT_BYTE_LEN;
	}

	dp_cust_hex_dump("debug edid", edid, *edid_size);
	return 0;
}

static struct kernel_param_ops param_ops_revision = {
	.get = dp_debug_get_revision,
	.set = dp_debug_set_revision,
};

static struct kernel_param_ops param_ops_max_bpp = {
	.get = dp_debug_get_max_bpp,
	.set = dp_debug_set_max_bpp,
};

static struct kernel_param_ops param_ops_vs_pe = {
	.get = dp_debug_get_vs_pe,
	.set = dp_debug_set_vs_pe,
};

static struct kernel_param_ops param_ops_edid = {
	.set = dp_debug_set_edid,
};

module_param_cb(revision, &param_ops_revision, NULL, 0644);
module_param_cb(max_bpp, &param_ops_max_bpp, NULL, 0644);
module_param_cb(vs_pe, &param_ops_vs_pe, NULL, 0644);
module_param_cb(edid, &param_ops_edid, NULL, 0644);

MODULE_DESCRIPTION("dp debug driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
