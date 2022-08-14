/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: als route source file
 * Author: wangsiwen
 * Create: 2020-10-29
 */

#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/of.h>
#include <linux/workqueue.h>
#include <securec.h>
#include <huawei_platform/oeminfo/oeminfo_def.h>
#include "sensor_scp.h"
#include "als_route.h"
#include "sensor_para.h"

#define MAX_ALS_UD_CMD_BUF_ARGC    2
#define ch_is_digit(ch) ('0' <= (ch) && (ch) <= '9')
#define ch_is_hex(ch) ((('A' <= (ch)) && ((ch) <= 'F')) || (('a' <= (ch)) && ((ch) <= 'f')))
#define ch_is_hexdigit(ch) (ch_is_digit(ch) || ch_is_hex(ch))

#define AREA_COUNT 25
#define BR_LINEAR 30
#define FORMAT_DEC 10
#define ALS_UD_CALI_LEN 4
#define CALIDATA_MAGIC1 0x414C5355
#define CALIDATA_MAGIC2 0x43414C49

#define LCD_190 "190"
#define LCD_310 "310"

#define ALS_3701 "als_s001_002"

static unsigned int g_als_undertp_calidata[ALS_UNDER_TP_CALDATA_SIZE] = {0};
static struct workqueue_struct *g_oeminfo_read_workqueue;
static struct work_struct g_oeminfo_read_work;
static int g_should_read_work;
static struct oeminfo_info_user g_user_read_info;
static struct workqueue_struct *g_oeminfo_write_workqueue;
static struct work_struct g_oeminfo_write_work;
static int g_should_write_work;
static struct oeminfo_info_user g_user_write_info;

enum {
	ALS_UD_CMD_BUFFER_UPDATE = 1,
};

enum {
	PRODUCT_AM = 8,
};

enum {
	LCD_190_TYPE = 1,
	LCD_310_TYPE,
};

enum {
	AMS3701 = 1,
};

enum {
	ANGELA_AMS3701_190 = 19,
	ANGELA_AMS3701_310 = 20,
};

struct als_ud_cmd_map_t {
	const char *str;
	int cmd;
};

struct als_under_tp_calidata {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t length;
	uint32_t a[AREA_COUNT]; // area para.
	uint32_t b[BR_LINEAR];  // algrothm para,
};

struct als_full_ud_calidata {
	uint32_t magic_num1;
	uint32_t magic_num2;
	struct als_under_tp_calidata calidata;
};

struct lcd_model {
	const char *lcd_info;
	uint8_t lcd_type;
};

struct sensor_model {
	const char *sensor_info;
	uint8_t sensor_type;
};

static struct als_ud_cmd_map_t als_ud_cmd_map[] = {
	{ "BUFF", ALS_UD_CMD_BUFFER_UPDATE },
};

static struct lcd_model lcd_model_tbl[] = {
	{ LCD_190, LCD_190_TYPE },
	{ LCD_310, LCD_310_TYPE },
};

static struct sensor_model sensor_model_tbl[] = {
	{ ALS_3701, AMS3701 },
};

void als_oeminfo_write_work(struct work_struct *work)
{
	if (oeminfo_direct_access(&g_user_write_info)) {
		pr_err("%s: oeminfo_id: %d write fail\n",
			__func__, g_user_write_info.oeminfo_id);
		g_should_write_work = -1;
		return;
	}

	g_should_write_work = 1;
}

int als_oeminfo_write(uint32_t id, const void *buf, uint32_t buf_size)
{
	errno_t ret;

	if (buf == NULL || buf_size == 0) {
		pr_err("%s: invalid params, size: %u, id: %u\n",
			__func__, buf_size, id);
		return -1;
	}

	g_user_write_info.oeminfo_operation = OEMINFO_WRITE;
	g_user_write_info.oeminfo_id = id;
	g_user_write_info.valid_size = buf_size;
	ret = memcpy_s(g_user_write_info.oeminfo_data,
		sizeof(g_user_write_info.oeminfo_data), buf,
		(sizeof(g_user_write_info.oeminfo_data) < g_user_write_info.valid_size) ?
		sizeof(g_user_write_info.oeminfo_data) : g_user_write_info.valid_size);
	if (ret != EOK) {
		pr_err("%s: oeminfo_data memcpy_s fail, ret: %d, id: %u\n",
			__func__, ret, id);
		return -1;
	}

	g_should_write_work = 0;
	if (g_oeminfo_write_workqueue) {
		queue_work(g_oeminfo_write_workqueue, &g_oeminfo_write_work);

		while (g_should_write_work == 0)
			mdelay(10);
	}
	if (g_should_write_work == -1)
		return -1;
	return 0;
}

void als_oeminfo_read_work(struct work_struct *work)
{
	if (oeminfo_direct_access(&g_user_read_info)) {
		pr_err("%s: oeminfo_id: %d read fail\n",
			__func__, g_user_read_info.oeminfo_id);
		g_should_read_work = -1;
		return;
	}
	g_should_read_work = 1;
}

int als_oeminfo_read(uint32_t id, void *buf, uint32_t buf_size)
{
	errno_t ret;

	if (buf == NULL || buf_size == 0) {
		pr_err("%s: invalid params, size: %u, id: %u\n",
			__func__, buf_size, id);
		return -1;
	}

	memset(&g_user_read_info, 0, sizeof(g_user_read_info));

	g_user_read_info.oeminfo_operation = OEMINFO_READ;
	g_user_read_info.oeminfo_id = id;
	g_user_read_info.valid_size = buf_size;

	g_should_read_work = 0;
	if (g_oeminfo_read_workqueue) {
		queue_work(g_oeminfo_read_workqueue, &g_oeminfo_read_work);

		while (g_should_read_work == 0)
			mdelay(10);
	}
	if (g_should_read_work == -1)
		return -1;

	ret = memcpy_s(buf, buf_size,
		g_user_read_info.oeminfo_data, g_user_read_info.valid_size);

	if (ret != EOK) {
		pr_err("%s: memcpy_s fail, ret: %d, id: %u\n",
			__func__, ret, id);
		return -1;
	}
	return 0;
}

ssize_t als_under_tp_calidata_store(int32_t sensor_type, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	if (sensor_type != SENSOR_TYPE_LIGHT || !dev || !attr || !buf ||
		size != (ALS_UNDER_TP_CALDATA_SIZE * sizeof(int))) {
		pr_err("%s : invalid params, sensor_type: %d, size: %zu\n",
			__func__, sensor_type, size);
		return -1;
	}

	if (als_oeminfo_write(OEMINFO_ALS_UNDER_TP_CALIDATA,
		buf, size) != 0)
		return -1;
	return (ssize_t)size;
}

ssize_t als_under_tp_calidata_show(int32_t sensor_type, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int i;
	int ret;
	int als_undertp_calidata[ALS_UNDER_TP_CALDATA_SIZE] = {0};

	if (sensor_type != SENSOR_TYPE_LIGHT || !dev || !attr || !buf) {
		pr_err("%s : invalid params, sensor_type: %d\n",
			__func__, sensor_type);
		return -1;
	}

	if (als_oeminfo_read(OEMINFO_ALS_UNDER_TP_CALIDATA,
		als_undertp_calidata, sizeof(als_undertp_calidata)) != 0)
		return -1;

	ret = snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"%d", als_undertp_calidata[0]);
	if (ret <= 0) {
		pr_err("%s: write calidata[0] to buf fail\n", __func__);
		return -1;
	}
	for (i = 1; i < ALS_UNDER_TP_CALDATA_SIZE; i++) {
		ret = snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%s,%d",
			buf, als_undertp_calidata[i]);
		if (ret <= 0) {
			pr_info("%s: write calidata[%d] to buf fail\n",
				__func__, i);
			return -1;
		}
	}

	return (ssize_t)ret;
}

static inline bool is_space_ch(char ch)
{
	return (' ' == ch) || ('\t' == ch);
}

static bool end_of_string(char ch)
{
	bool ret = false;

	switch (ch) {
	case '\0':
	case '\r':
	case '\n':
		ret = true;
		break;
	default:
		ret = false;
		break;
	}

	return ret;
}

static bool str_fuzzy_match(const char *cmd_buf, const char *target)
{
	if (!cmd_buf || !target)
		return false;

	for (; !is_space_ch(*cmd_buf) && !end_of_string(*cmd_buf) && *target;
		++target) {
		if (*cmd_buf == *target)
			++cmd_buf;
	}

	return is_space_ch(*cmd_buf) || end_of_string(*cmd_buf);
}

bool get_arg(const char *str, int *arg)
{
	unsigned int val = 0;
	bool neg = false;
	bool hex = false;

	if ('-' == *str) {
		++str;
		neg = true;
	}

	if (('0' == *str) && (('x' == *(str + 1)) || ('X' == *(str + 1)))) {
		str += 2;
		hex = true;
	}

	if (hex) {
		for (; !is_space_ch(*str) && !end_of_string(*str); ++str) {
			if (!ch_is_hexdigit(*str))
				return false;
			val <<= 4;
			val |= (ch_is_digit(*str) ?
				(*str - '0') : (((*str | 0x20) - 'a') + 10));
		}
	} else {
		for (; !is_space_ch(*str) && !end_of_string(*str); ++str) {
			if (!ch_is_digit(*str))
				return false;
			val *= 10;
			val += *str - '0';
		}
	}

	*arg = neg ? -val : val;
	return true;
}

static int get_cmd(const char *str)
{
	int i = 0;

	for (; i < sizeof(als_ud_cmd_map) / sizeof(als_ud_cmd_map[0]); ++i) {
		if (str_fuzzy_match(str, als_ud_cmd_map[i].str))
			return als_ud_cmd_map[i].cmd;
	}
	return -1;
}

/* find first pos */
const char *get_str_begin(const char *cmd_buf)
{
	if (!cmd_buf)
		return NULL;

	while (is_space_ch(*cmd_buf))
		++cmd_buf;

	if (end_of_string(*cmd_buf))
		return NULL;

	return cmd_buf;
}

/* find last pos */
const char *get_str_end(const char *cmd_buf)
{
	if (!cmd_buf)
		return NULL;

	while (!is_space_ch(*cmd_buf) && !end_of_string(*cmd_buf))
		++cmd_buf;

	return cmd_buf;
}

ssize_t als_rgb_status_store(int32_t tag, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int input_cmd = 0;
	int arg = -1;
	int argc = 0;
	int para[2];
	struct custom_cmd cmd;

	for (; (buf = get_str_begin(buf)) != NULL; buf = get_str_end(buf)) {
		if (!input_cmd)
			input_cmd = get_cmd(buf);

		if (get_arg(buf, &arg)) {
			if (argc < MAX_ALS_UD_CMD_BUF_ARGC)
				para[argc++] = arg;
			else
				pr_err("%s: too many args, ignore %d\n",
					__func__, arg);
		}
	}
	switch (input_cmd) {
	case ALS_UD_CMD_BUFFER_UPDATE:
		cmd.data[0] = CUST_CMD_CONTROL;
		cmd.data[1] = 3;
		cmd.data[2] = SUB_CMD_UPDATE_RGB_DATA;
		cmd.data[3] = para[0];
		cmd.data[4] = para[1];
		send_scp_common_cmd(SENSOR_TYPE_LIGHT, &cmd);
		break;
	default:
		pr_err("%s: unspport cmd\n", __func__);
		break;
	}
	return size;
}

int store_data_to_share_mem(uint8_t *buf, size_t len)
{
	struct als_para_t *als_data =
		get_sensor_share_mem_addr(SHR_MEM_TYPE_ALS);

	if (als_data == NULL) {
		pr_info("%s: get als data fail\n", __func__);
		return -1;
	}
	return memcpy_s(als_data->para, sizeof(als_data->para), buf, len);
}

static int send_als_under_tp_calibrate_data_to_scp(int32_t tag)
{
	int ret;
	struct custom_cmd cmd;
	struct als_full_ud_calidata als_ud_calidata;

	if (als_oeminfo_read(OEMINFO_ALS_UNDER_TP_CALIDATA,
		g_als_undertp_calidata, sizeof(g_als_undertp_calidata)) != 0)
		return -1;

	als_ud_calidata.magic_num1 = CALIDATA_MAGIC1;
	als_ud_calidata.magic_num2 = CALIDATA_MAGIC2;
	als_ud_calidata.calidata.x = (uint16_t)g_als_undertp_calidata[0];
	als_ud_calidata.calidata.y = (uint16_t)g_als_undertp_calidata[1];
	als_ud_calidata.calidata.width = (uint16_t)g_als_undertp_calidata[2];
	als_ud_calidata.calidata.length = (uint16_t)g_als_undertp_calidata[3];
	if (memcpy_s(als_ud_calidata.calidata.a, sizeof(als_ud_calidata.calidata.a), g_als_undertp_calidata + 4,
		25 * sizeof(uint32_t)) != EOK) {
		pr_err("%s: als_ud_calidata.a memcpy_s fail, ret: %d\n",
			__func__, ret);
		return -1;
	}

	if (memcpy_s(als_ud_calidata.calidata.b, sizeof(als_ud_calidata.calidata.b), g_als_undertp_calidata + 29,
		30 * sizeof(uint32_t)) != EOK) {
		pr_err("%s: als_ud_calidata.b memcpy_s fail, ret: %d\n",
			__func__, ret);
		return -1;
	}

	ret = store_data_to_share_mem((uint8_t *)(&als_ud_calidata),
		sizeof(als_ud_calidata));
	if (ret != EOK) {
		pr_err("%s: memcpy_s fail, ret: %d\n", __func__, ret);
		return -1;
	}
	cmd.data[0] = CUST_CMD_CONTROL;
	cmd.data[1] = 2;
	cmd.data[2] = SUB_CMD_SET_ALS_UD_CALIB_DATA;
	cmd.data[3] = sizeof(als_ud_calidata);
	ret = send_scp_common_cmd(tag, &cmd);
	return ret;
}

static void get_als_under_tp_calibrate_data_by_tag(int32_t tag)
{
	int ret = send_als_under_tp_calibrate_data_to_scp(tag);

	if (ret)
		pr_err("%s send calidata fail, ret: %d", __func__, ret);
	else
		pr_info("%s send calidata success\n", __func__);
}

ssize_t als_ud_rgbl_status_show(int32_t tag, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	int size = 0;

	get_als_under_tp_calibrate_data_by_tag(tag);
	if (g_als_undertp_calidata[0] == 0 && g_als_undertp_calidata[1] == 0) {
		size = snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
			"%d,%d,%d,%d\n", 823, 7, 913, 97);
		return size;
	}

	size = snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1, "%u,%u,%u,%u\n",
		g_als_undertp_calidata[0] > HALF_LENGTH ? g_als_undertp_calidata[0] - HALF_LENGTH : 0,
		g_als_undertp_calidata[1] > HALF_LENGTH ? g_als_undertp_calidata[1] - HALF_LENGTH : 0,
		g_als_undertp_calidata[0] + HALF_LENGTH, g_als_undertp_calidata[1] + HALF_LENGTH);
	return size;
}

static int get_lcd_type(void)
{
	uint8_t index;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		pr_err("%s: not find device node %s\n", __func__, "huawei,lcd_panel_type");
		return -1;
	}
	if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
		pr_err("%s: not find lcd_model\n", __func__);
	}

	for (index = 0; index < ARRAY_SIZE(lcd_model_tbl); index++) {
		if (strncmp(lcd_model_tbl[index].lcd_info,
			lcd_model, strlen(lcd_model_tbl[index].lcd_info)) == 0)
			return lcd_model_tbl[index].lcd_type;
	}

	pr_err("%s: sensor kernel failed to get lcd type\n", __func__);
	return -1;
}

static int get_sensor_type(void)
{
	uint8_t index;
	struct sensor_info sensor_name;
	int ret;

	ret = scp_get_sensor_info(SENSOR_TYPE_LIGHT, &sensor_name);
	if (ret < 0) {
		pr_err("%s: get sensor id fail\n", __func__);
		return -1;
	}

	for (index = 0; index < ARRAY_SIZE(sensor_model_tbl); index++) {
		if (strncmp(sensor_model_tbl[index].sensor_info,
			sensor_name.name, strlen(sensor_model_tbl[index].sensor_info)) == 0)
			return sensor_model_tbl[index].sensor_type;
	}

	pr_err("%s: sensor kernel failed to get sensor type\n", __func__);
	return -1;
}

ssize_t als_calibrate_after_sale_show(int32_t sensor_type, struct device *dev,
	struct device_attribute *attr, char *buf)
{
	uint32_t als_ud_cali_xy[ALS_UD_CALI_LEN] = {0};
	int ret;
	uint32_t product_id = 0;
	int lcd_id = 0;
	int sensor_id = 0;
	int after_sale_proid;
	struct device_node *sensor_node =
		of_find_compatible_node(NULL, NULL, "huawei,huawei_sensor_info");

	if (sensor_type != SENSOR_TYPE_LIGHT || !dev || !attr || !buf) {
		pr_err("%s : invalid params, sensor_type: %d\n",
			__func__, sensor_type);
		return -1;
	}

	/* get product id */
	if (sensor_node == NULL) {
		pr_err("%s : load huawei_sensor_info failed\n", __func__);
		return -1;
	}
	ret = of_property_read_u32(sensor_node, "product_number", &product_id);
	if (ret != 0 ) {
		pr_err("%s: get product id failed\n", __func__);
		return -1;
	}

	/* get lcd id */
	lcd_id = get_lcd_type();
	if (lcd_id < 0) {
		pr_err("%s : not find lcd_id: %d\n", __func__, lcd_id);
		return -1;
	}

	/* get sensor id */
	sensor_id = get_sensor_type();
	if (sensor_id < 0) {
		pr_err("%s : not find sensor id: %d\n", __func__, sensor_id);
		return -1;
	}

	/* get after sale product id */
	if (product_id == PRODUCT_AM && lcd_id == LCD_190_TYPE && sensor_id == AMS3701) {
		after_sale_proid = ANGELA_AMS3701_190;
	} else if (product_id == PRODUCT_AM && lcd_id == LCD_310_TYPE && sensor_id == AMS3701) {
		after_sale_proid = ANGELA_AMS3701_310;
	} else {
		pr_err("%s : not find after_sale_proid\n", __func__);
		return -1;
	}

	sensor_node =
		of_find_compatible_node(NULL, NULL, "huawei,huawei_sensor_info");

	ret = of_property_read_u32_array(sensor_node, "als_cali_after_sale_xy", als_ud_cali_xy, ALS_UD_CALI_LEN);

	if (ret != 0) {
		pr_err("%s: als_ud_cali_xy get data failed\n", __func__, ret);
		return -1;
	}

	return snprintf_s(buf, MAX_STR_SIZE, MAX_STR_SIZE - 1,
		"%d, %d, %d, %d, %d\n",
		after_sale_proid, als_ud_cali_xy[0], als_ud_cali_xy[1],
		als_ud_cali_xy[2], als_ud_cali_xy[3]);
}

void save_light_to_scp(uint32_t mipi_level, uint32_t bl_level)
{
	struct bright_data para;
	struct custom_cmd cmd;
	static uint32_t als_ud_algo_enable;
	struct als_para_t *als_data = NULL;
	int ret;
	struct device_node *sensor_node =
		of_find_compatible_node(NULL, NULL, "huawei,huawei_sensor_info");

	ret = of_property_read_u32(sensor_node, "als_ud_algo",
		&als_ud_algo_enable);

	if (ret != 0 || als_ud_algo_enable != 1) {
		pr_err("%s: als_ud_algo disable, ret: %d, als_ud_algo_enable: %u\n", __func__, ret, als_ud_algo_enable);
		return;
	}

	para.mipi_data = mipi_level;
	para.bright_data = bl_level;
	para.time_stamp = (uint64_t)ktime_to_ms(ktime_get_boot_ns());
	als_data = get_sensor_share_mem_addr(SHR_MEM_TYPE_ALS);
	if (als_data == NULL) {
		pr_info("%s: get als data fail\n", __func__);
		return;
	}
	ret = memcpy_s(als_data->light_data, sizeof(als_data->light_data), (uint8_t *)(&para), sizeof(para));
	if (ret != EOK) {
		pr_err("%s: memcpy_s fail, ret: %d\n", __func__, ret);
		return;
	}
	cmd.data[0] = CUST_CMD_CONTROL;
	cmd.data[1] = 2;
	cmd.data[2] = SUB_CMD_UPDATE_BL_LEVEL;
	cmd.data[3] = sizeof(para);
	send_scp_common_cmd(SENSOR_TYPE_LIGHT, &cmd);
}

ssize_t als_always_on_store(int32_t sensor_type, struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	int als_always_on;
	struct custom_cmd cmd = {0};

	if (sensor_type != SENSOR_TYPE_LIGHT || !dev || !attr || !buf) {
		pr_err("%s : invalid params, sensor_type: %d\n",
			__func__, sensor_type);
		return -1;
	}

	als_always_on = simple_strtol(buf, NULL, FORMAT_DEC);
	cmd.data[0] = CUST_CMD_CONTROL;
	cmd.data[1] = 2;
	cmd.data[2] = SUB_CMD_CHANGE_ALWAYS_ON_STATUS;
	cmd.data[3] = als_always_on;
	pr_info("set always on info = %d\n", als_always_on);
	if (send_scp_common_cmd(sensor_type, &cmd))
		return -1;
	return (ssize_t)size;
}

void init_oeminfo_work(void)
{
	g_oeminfo_read_workqueue = alloc_workqueue("g_oeminfo_read_workqueue", 0, 0);
	if (!g_oeminfo_read_workqueue) {
		pr_err("%s: alloc read workqueue error\n", __func__);
		return;
	}

	INIT_WORK(&g_oeminfo_read_work, als_oeminfo_read_work);

	g_oeminfo_write_workqueue = alloc_workqueue("g_oeminfo_write_workqueue", 0, 0);
	if (!g_oeminfo_write_workqueue) {
		pr_err("%s: alloc write workqueue error\n", __func__);
		return;
	}

	INIT_WORK(&g_oeminfo_write_work, als_oeminfo_write_work);
}

void close_oeminfo_workqueue(void)
{
	if (g_oeminfo_read_workqueue) {
		destroy_workqueue(g_oeminfo_read_workqueue);
		g_oeminfo_read_workqueue = NULL;
	}
	if (g_oeminfo_write_workqueue) {
		destroy_workqueue(g_oeminfo_write_workqueue);
		g_oeminfo_write_workqueue = NULL;
	}
}

