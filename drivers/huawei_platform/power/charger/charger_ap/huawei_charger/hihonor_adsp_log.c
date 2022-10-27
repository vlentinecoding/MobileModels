/*
 * hihonor_adsp_log.c
 *
 * hihonor_adsp_log driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/soc/qcom/pmic_glink.h>
#include <linux/kthread.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_log.h>
#include <chipset_common/hwpower/power_printk.h>
#include <chipset_common/hwpower/power_event_ne.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_sysfs.h>

#define HWLOG_TAG hihonor_adsp_log
HWLOG_REGIST();

#define MSG_OWNER_CHARGER          32778
#define MSG_TYPE_LOG_REQ_RESP      1
#define MSG_TYPE_LOG_NOTIFY        2
#define BATT_MNGR_GET_VOTE_REQ     0x17
#define BATT_MNGR_GET_ULOG_REQ     0x18
#define BATT_MNGR_GET_INFO_LOG_BUFF_REQ 0x200
#define BATT_MNGR_GET_CHG_LOG_BUFF_REQ  0x201
#define BATT_MNGR_START_LOG_REQ    0x202
#define BATT_MNGR_STOP_LOG_REQ     0x203
#define ULOG_WAIT_TIME_MS          2000
#define MAX_ULOG_READ_BUFFER_SIZE  8192
#define UPDATE_LOG_WORK_TIME       30000
#define VOTE_MAX_NUM               32
#define MAX_CLIENT_NAME_LENTH      32
#define MAX_LOG_BUFF_SIZE          8088
#define MAX_SINGLE_LOG_SIZE 1000
#define MAX_CHARGE_LOG_SIZE        145
#define CHARGE_LOG_LENGTH          1024
#define CHARGE_LOG_TITLE_SIZE      64
#define CHARGE_LOG_TIME_INTERVAL   5
#define CHARGE_LOG_MAX_READ_NUM    4

typedef enum battman_ulog_type{
	BATTMAN_LOG_TYPE_INFO        = 0,
	BATTMAN_LOG_TYPE_ERROR       = 1,
	BATTMAN_LOG_TYPE_CHARGE_LOG  = 2,
	BATTMAN_LOG_TYPE_REALTIME    = 3,
}battman_log_type;

enum soc_decimal_sysfs_type {
	ADSP_LOG_SYSFS_BEGIN = 0,
	ADSP_LOG_SYSFS_LOG = ADSP_LOG_SYSFS_BEGIN,
	ADSP_LOG_SYSFS_ON,
	ADSP_LOG_SYSFS_END,
};

/** request Message; to get ulogs from chargerPD */
struct hihonor_adsp_log_get_msg {
	struct pmic_glink_hdr header;
	u32 max_logsize;
};

/** Response Message; to get ulogs from chargerPD */
struct hihonor_adsp_log_rsp_msg {
	struct pmic_glink_hdr header;
	u32 log_type;
};

struct battman_get_log_resp_msg
{
	struct pmic_glink_hdr header;
	char read_buffer[MAX_ULOG_READ_BUFFER_SIZE];
};

struct charge_log_info
{
	unsigned short buck_vbus;
	unsigned short buck_ibus;
	short bat_temp1;
	short bat_temp2;
	short bat_temp_raw;
	unsigned short fg_vbat;
	short fg_ibat;
	unsigned short fg_soc;
	unsigned short fg_soc_raw;
	unsigned short fg_rm;
	unsigned short fg_fcc;
	short fg_ibat_avg;
	unsigned short sc_vusb1;
	unsigned short sc_vbus1;
	unsigned short sc_ibus1;
	unsigned short sc_vbat1;
	short sc_ibat1;
	unsigned short sc_vout1;
	unsigned short sc_tdie1;
	unsigned short sc_vusb2;
	unsigned short sc_vbus2;
	unsigned short sc_ibus2;
	unsigned short sc_vbat2;
	short sc_ibat2;
	unsigned short sc_vout2;
	unsigned short sc_tdie2;
	unsigned int cur_time;
};

struct charge_log_format
{
	char *title;
	int placehold;
	int value_type;
};

enum charge_log_value_type
{
	CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT,
	CHARGE_LOG_VAL_TYPE_SHORT,
};

static struct charge_log_format charge_log_format_tbl[] = {
	{ "buck_vbus",   15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "buck_ibus",   15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "bat_temp1",   15, CHARGE_LOG_VAL_TYPE_SHORT },
	{ "bat_temp2",   15, CHARGE_LOG_VAL_TYPE_SHORT },
	{ "bat_temp_raw",15, CHARGE_LOG_VAL_TYPE_SHORT },
	{ "fg_vbat",     15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "fg_ibat",     15, CHARGE_LOG_VAL_TYPE_SHORT          },
	{ "fg_soc",      15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "fg_soc_raw",  15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "fg_rm",       15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "fg_fcc",      15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "fg_ibat_avg", 15, CHARGE_LOG_VAL_TYPE_SHORT },
	{ "sc_vusb1",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_vbus1",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_ibus1",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_vbat1",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_ibat1",    15, CHARGE_LOG_VAL_TYPE_SHORT },
	{ "sc_vout1",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_tdie1",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_vusb2",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_vbus2",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_ibus2",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_vbat2",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_ibat2",    15, CHARGE_LOG_VAL_TYPE_SHORT },
	{ "sc_vout2",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
	{ "sc_tdie2",    15, CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT },
};

struct charge_log_buff
{
	struct pmic_glink_hdr header;
	unsigned int log_type;
	unsigned int cur_pos;
	struct charge_log_info info[MAX_CHARGE_LOG_SIZE];
};

struct hihonor_adsp_log_dev_info {
	struct device *dev;
	char client_name[MAX_CLIENT_NAME_LENTH];
	u32 client_id;
	struct pmic_glink_client *client;
	atomic_t state;
	bool initialized;
	struct charge_log_buff chg_log;
	char charge_log_head[CHARGE_LOG_LENGTH];
	int charge_log_ready;
	struct rtc_time start_chg_log_tm;
	struct rtc_time end_chg_log_tm;
	unsigned int log_start_pos;
	struct delayed_work uevent_work;
};

static struct hihonor_adsp_log_dev_info *g_adsp_log_info = NULL;

struct log_info_buff
{
	struct pmic_glink_hdr header;
	u32 log_type;
	u32 cur_pos;
	char log_buff[MAX_LOG_BUFF_SIZE];
};

static void hihonor_adsp_log_write(struct hihonor_adsp_log_dev_info *info,
	struct pmic_glink_client *client, void *msg, size_t len)
{
	if (atomic_read(&info->state) == PMIC_GLINK_STATE_DOWN) {
		pr_err("[adsp_log]glink state is down\n");
		return;
	}

	(void)pmic_glink_write(client, msg, len);
}

static void hihonor_adsp_log_send_get_log_msg(struct hihonor_adsp_log_dev_info *info,
	int opcode)
{
	struct hihonor_adsp_log_get_msg log_msg = {0};

	log_msg.header.owner = MSG_OWNER_CHARGER;
	log_msg.header.type = MSG_TYPE_LOG_REQ_RESP;
	log_msg.header.opcode = opcode;
	log_msg.max_logsize = MAX_ULOG_READ_BUFFER_SIZE;

	hihonor_adsp_log_write(info, info->client, &log_msg, sizeof(log_msg));
}

static void hihonor_adsp_log_handle_notification(struct hihonor_adsp_log_dev_info *info, void *data,
	size_t len)
{
	return;
}

static void hihonor_adsp_log_handle_charge_log(struct hihonor_adsp_log_dev_info *info, void *data)
{
	struct charge_log_buff *log_info = (struct charge_log_buff *)data;
	struct timespec64 tv;
	struct timespec64 tv_start;
	unsigned long time_diff;
	unsigned int cur_pos;

	if (info->charge_log_ready)
		return;

	memcpy(&info->chg_log, log_info, sizeof(*log_info));
	if (info->chg_log.cur_pos == 0) {
		pr_info("hihonor_adsp_log_handle_charge_log invalid cur_pos");
		return;
	}
	cur_pos = info->chg_log.cur_pos - 1;

	ktime_get_real_ts64(&tv);
	tv.tv_sec -= sys_tz.tz_minuteswest * 60; /* GMT, 1min equal to 60s */
	rtc_time_to_tm(tv.tv_sec, &info->end_chg_log_tm);


	time_diff = info->chg_log.info[cur_pos].cur_time - info->chg_log.info[0].cur_time;
	if (time_diff < 0)
		return;
	tv_start.tv_sec = tv.tv_sec - time_diff;
	rtc_time_to_tm(tv_start.tv_sec, &info->start_chg_log_tm);

	info->charge_log_ready = 1;
}

static void hihonor_adsp_log_send_uevent(struct work_struct *work)
{
	struct power_event_notify_data n_data;

	n_data.event = "CHARGE_LOG_PRINT=";
	n_data.event_len = 17; /* length of CHARGE_LOG_PRINT= */
	power_event_report_uevent(&n_data);
}

static void hihonor_adsp_log_ulog_print(int cur_pos,
	struct log_info_buff *log_info)
{
	if (log_info->log_type == BATTMAN_LOG_TYPE_INFO)
		pr_info("[adsp_log]pos:%d, log:%s", cur_pos, &log_info->log_buff[cur_pos]);
	else
		pr_err("[adsp_log]pos:%d, log:%s", cur_pos, &log_info->log_buff[cur_pos]);
}

static void hihonor_adsp_log_ulog_handle_info(struct log_info_buff *log_info)
{
	unsigned int i;
	int cur_pos = 0;
	char *log_buf = log_info->log_buff;
	unsigned int buf_size = log_info->cur_pos;

	for (i = 0; i < buf_size; ++i) {
		if (log_buf[i] != 0xA && log_buf[i] != 0xD && log_buf[i] != '\0')
			continue;
		if (i - cur_pos < MAX_SINGLE_LOG_SIZE)
			continue;
		log_buf[i] = '\0';
		hihonor_adsp_log_ulog_print(cur_pos, log_info);
		cur_pos = i + 1;
	}

	if (cur_pos >= buf_size)
		return;

	hihonor_adsp_log_ulog_print(cur_pos, log_info);
}

static void hihonor_adsp_log_handle_ulog_msg(struct hihonor_adsp_log_dev_info *info, void *data)
{
	struct hihonor_adsp_log_rsp_msg *rsp_msg = data;
	struct log_info_buff *log_info = (struct log_info_buff *)data;
	struct battman_get_log_resp_msg *batt_log =data;

	switch (rsp_msg->header.opcode) {
	case BATT_MNGR_GET_ULOG_REQ:
		switch (rsp_msg->log_type) {
		case BATTMAN_LOG_TYPE_REALTIME:
			pr_info("[adsp_log]%s", log_info->log_buff);
			break;
		case BATTMAN_LOG_TYPE_INFO:
		case BATTMAN_LOG_TYPE_ERROR:
			hihonor_adsp_log_ulog_handle_info(log_info);
			break;
		case BATTMAN_LOG_TYPE_CHARGE_LOG:
			hihonor_adsp_log_handle_charge_log(info, data);
			schedule_delayed_work(&info->uevent_work, 0);
			break;
		default:
			break;
		}
		break;
	case BATT_MNGR_GET_INFO_LOG_BUFF_REQ:
		pr_info("%s", batt_log->read_buffer);
		break;
	default:
		break;
	}
}

static void hihonor_adsp_log_handle_message(struct hihonor_adsp_log_dev_info *info, void *data,
	size_t len)
{
	struct pmic_glink_hdr *header = data;

	switch (header->owner) {
	case MSG_OWNER_CHARGER:
		hihonor_adsp_log_handle_ulog_msg(info, data);
		break;
	default:
		break;
	}
}

static int hihonor_adsp_log_callback(void *priv, void *data, size_t len)
{
	struct pmic_glink_hdr *hdr = data;
	struct hihonor_adsp_log_dev_info *info = priv;

	if (!info || !data)
		return -1;

	if (!info->initialized) {
		pr_err("[adsp_log]Driver initialization failed: Dropping glink callback message: state %d\n",
			 info->state);
		return 0;
	}

	if (hdr->opcode == MSG_TYPE_LOG_NOTIFY)
		hihonor_adsp_log_handle_notification(info, data, len);
	else
		hihonor_adsp_log_handle_message(info, data, len);

	return 0;
}

static void hihonor_adsp_log_state_cb(void *priv, enum pmic_glink_state state)
{
	struct hihonor_adsp_log_dev_info *info = priv;

	if (!info)
		return;

	pr_info("[adsp_log]state: %d\n", state);

	atomic_set(&info->state, state);
}

static int hihonor_adsp_charge_log_head_dump(char *buffer, int size, void *dev_data)
{
	struct hihonor_adsp_log_dev_info *info = (struct hihonor_adsp_log_dev_info *)dev_data;

	if (!buffer || !info) {
		pr_err("%s info is null\n",__func__);
		return -1;
	}

	snprintf(buffer, CHARGE_LOG_LENGTH, "%s", info->charge_log_head);
	return 0;
}

static void hihonor_adsp_handle_charge_log_string(char *buffer, struct charge_log_info *info)
{
	int i;
	int len;
	int total = 0;
	int title_num;
	char temp_buf[CHARGE_LOG_TITLE_SIZE] = { 0 };
	char *pos = (char *)info;
	int value;

	if (!buffer || !info)
		return;

	title_num = sizeof(charge_log_format_tbl) / sizeof(struct charge_log_format);
	for (i = 0; i < title_num; i++) {
		len = charge_log_format_tbl[i].placehold;
		total += len;
		if (total >= CHARGE_LOG_LENGTH - 1)
			return;
		switch (charge_log_format_tbl[i].value_type) {
		case CHARGE_LOG_VAL_TYPE_UNSIGNED_SHORT:
			value = *((unsigned short *)pos);
			pos += sizeof(unsigned short);
			break;
		case CHARGE_LOG_VAL_TYPE_SHORT:
			value = *((short *)pos);
			pos += sizeof(short);
			break;
		default:
			break;
		}
		snprintf(temp_buf, CHARGE_LOG_TITLE_SIZE, "%-*d", len, value);
		strncat(buffer, temp_buf, strlen(temp_buf));
	}
}

static int hihonor_adsp_charge_log_content_dump(char *buffer, int size, void *dev_data)
{
	int i;
	char time_str[CHARGE_LOG_TITLE_SIZE] = { 0 };
	unsigned long timestamp_sec;
	struct rtc_time chg_log_tm;
	unsigned int log_end_pos;
	unsigned int time_diff;
	struct hihonor_adsp_log_dev_info *info = (struct hihonor_adsp_log_dev_info *)dev_data;

	if (!buffer || !info) {
		pr_err("%s info is null\n",__func__);
		return -EINVAL;
	}

	if (!info->charge_log_ready) {
		pr_err("%s charge log is not ready\n", __func__);
		return -1;
	}

	if (info->log_start_pos == info->chg_log.cur_pos) {
		info->charge_log_ready = 0;
		info->log_start_pos = 0;
		return -1;
	}

	log_end_pos = info->log_start_pos + CHARGE_LOG_MAX_READ_NUM;
	if (log_end_pos > info->chg_log.cur_pos)
		log_end_pos = info->chg_log.cur_pos;

	rtc_tm_to_time(&info->start_chg_log_tm, &timestamp_sec);
	for (i = info->log_start_pos; i < log_end_pos; i++) {
		time_diff = info->chg_log.info[i].cur_time - info->chg_log.info[0].cur_time;
		if (time_diff < 0) {
			info->charge_log_ready = 0;
			info->log_start_pos = 0;
			break;
		}
		rtc_time_to_tm(timestamp_sec + time_diff, &chg_log_tm);
		snprintf(time_str, CHARGE_LOG_TITLE_SIZE,
			"%d-%02d-%02d %02d:%02d:%02d >  ",
			chg_log_tm.tm_year + 1900,
			chg_log_tm.tm_mon + 1,
			chg_log_tm.tm_mday,
			chg_log_tm.tm_hour,
			chg_log_tm.tm_min,
			chg_log_tm.tm_sec);
		strncat(buffer, time_str, strlen(time_str));
		hihonor_adsp_handle_charge_log_string(buffer, &info->chg_log.info[i]);
		strncat(buffer, "\n", strlen("\n"));
		info->log_start_pos++;
	}

	return 0;
}

static struct power_log_ops hihonor_adsp_charger_log_ops = {
	.dev_name = "adsp",
	.dump_log_head = hihonor_adsp_charge_log_head_dump,
	.dump_log_content = hihonor_adsp_charge_log_content_dump,
};

static void hihonor_adsp_log_set_charge_log_head(struct hihonor_adsp_log_dev_info *info)
{
	int i;
	int len;
	int total = 0;
	int title_num;
	char temp_buf[CHARGE_LOG_TITLE_SIZE] = { 0 };

	if (!info)
		return;

	title_num = sizeof(charge_log_format_tbl) / sizeof(struct charge_log_format);
	snprintf(info->charge_log_head, CHARGE_LOG_LENGTH, "");
	for (i = 0; i < title_num; i++) {
		len = charge_log_format_tbl[i].placehold;
		total += len;
		if (total >= CHARGE_LOG_LENGTH - 1)
			return;
		snprintf(temp_buf, CHARGE_LOG_TITLE_SIZE, "%-*s", len, charge_log_format_tbl[i].title);
		strncat(info->charge_log_head, temp_buf, strlen(temp_buf));
	}
}

static int hihonor_adsp_log_parse_dts(struct device_node *np, struct hihonor_adsp_log_dev_info *info)
{
	const char *client_name = NULL;

	if (!np)
		return -1;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "client_id", &info->client_id, 0))
		return -1;

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np, "client_name", &client_name))
		return -1;

	strncpy(info->client_name, client_name, (MAX_CLIENT_NAME_LENTH - 1));
	return 0;
}

static ssize_t adsp_log_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf);
static ssize_t adsp_log_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count);

static struct power_sysfs_attr_info adsp_log_sysfs_field_tbl[] = {
	power_sysfs_attr_rw(adsp_log, 0660, ADSP_LOG_SYSFS_LOG, dump_log),
	power_sysfs_attr_rw(adsp_log, 0660, ADSP_LOG_SYSFS_ON, on),
};

#define ADSP_LOG_SYSFS_ATTRS_SIZE  ARRAY_SIZE(adsp_log_sysfs_field_tbl)

static struct attribute *adsp_log_sysfs_attrs[ADSP_LOG_SYSFS_ATTRS_SIZE + 1];

static const struct attribute_group adsp_log_sysfs_attr_group = {
	.attrs = adsp_log_sysfs_attrs,
};

static ssize_t adsp_log_sysfs_show(struct device *dev,
	struct device_attribute *attr, char *buf)
{
	struct hihonor_adsp_log_dev_info *dev_info = g_adsp_log_info;
	struct power_sysfs_attr_info *info = NULL;

	pr_info("adsp_log_sysfs_show +\n");
	if (!dev_info) {
		pr_info("adsp_log_sysfs_show, invalid para\n");
		return -EINVAL;
	}

	info = power_sysfs_lookup_attr(attr->attr.name,
		adsp_log_sysfs_field_tbl, ADSP_LOG_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case ADSP_LOG_SYSFS_LOG:
		pr_info("adsp_log_sysfs_show send get log cmd\n");
		hihonor_adsp_log_send_get_log_msg(dev_info, BATT_MNGR_GET_CHG_LOG_BUFF_REQ);
		hihonor_adsp_log_send_get_log_msg(dev_info, BATT_MNGR_GET_INFO_LOG_BUFF_REQ);
		break;
	default:
		pr_info("adsp_log_sysfs_show invalid info name\n");
		break;
	}
	return 0;
}

static ssize_t adsp_log_sysfs_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct hihonor_adsp_log_dev_info *dev_info = g_adsp_log_info;
	struct power_sysfs_attr_info *info = NULL;
	int value = 0;

	pr_info("adsp_log_sysfs_store +\n");
	if (!dev_info) {
		pr_info("adsp_log_sysfs_store, invalid para\n");
		return -EINVAL;
	}

	info = power_sysfs_lookup_attr(attr->attr.name,
		adsp_log_sysfs_field_tbl, ADSP_LOG_SYSFS_ATTRS_SIZE);
	if (!info)
		return -EINVAL;

	switch (info->name) {
	case ADSP_LOG_SYSFS_LOG:
		pr_info("adsp_log_sysfs_store send get log cmd\n");
		hihonor_adsp_log_send_get_log_msg(dev_info, BATT_MNGR_GET_CHG_LOG_BUFF_REQ);
		hihonor_adsp_log_send_get_log_msg(dev_info, BATT_MNGR_GET_INFO_LOG_BUFF_REQ);
		break;
	case ADSP_LOG_SYSFS_ON:
		if ((kstrtoint(buf, 10, &value) < 0) || (value != 0 && value != 1))
			return -EINVAL;

		if (value) {
			pr_info("open adsp log\n");
			hihonor_adsp_log_send_get_log_msg(dev_info, BATT_MNGR_START_LOG_REQ);
		} else {
			pr_info("close adsp log\n");
			hihonor_adsp_log_send_get_log_msg(dev_info, BATT_MNGR_STOP_LOG_REQ);
		}
		break;
	default:
		pr_info("adsp_log_sysfs_store invalid info name\n");
		break;
	}
	return count;
}

static struct device *adsp_log_sysfs_create_group(void)
{
	power_sysfs_init_attrs(adsp_log_sysfs_attrs,
		adsp_log_sysfs_field_tbl, ADSP_LOG_SYSFS_ATTRS_SIZE);
	return power_sysfs_create_group("hw_power", "adsp_log",
		&adsp_log_sysfs_attr_group);
}

static void adsp_log_sysfs_remove_group(struct device *dev)
{
	power_sysfs_remove_group(dev, &adsp_log_sysfs_attr_group);
}

static int hihonor_adsp_log_probe(struct platform_device *pdev)
{
	struct hihonor_adsp_log_dev_info *info = NULL;
	struct device *dev = &pdev->dev;
	struct pmic_glink_client_data client_data = {0};
	int rc = -1;

	pr_info("[adsp_log]hihonor_adsp_log_probe in\n");
	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	if (hihonor_adsp_log_parse_dts(pdev->dev.of_node, info))
		goto FREE_MEM;

	hihonor_adsp_log_set_charge_log_head(info);
	client_data.id = info->client_id;
	client_data.name = info->client_name;
	client_data.msg_cb = hihonor_adsp_log_callback;
	client_data.priv = info;
	client_data.state_cb = hihonor_adsp_log_state_cb;

	info->client = pmic_glink_register_client(dev, &client_data);
	if (IS_ERR(info->client)) {
		rc = PTR_ERR(info->client);
		if (rc != -EPROBE_DEFER)
			dev_err(dev, "[adsp_log]Error in registering with pmic_glink %d\n", rc);

		return rc;
	}

	INIT_DELAYED_WORK(&info->uevent_work, hihonor_adsp_log_send_uevent);
	atomic_set(&info->state, PMIC_GLINK_STATE_UP);
	info->initialized = true;
	platform_set_drvdata(pdev, info);
	hihonor_adsp_log_send_get_log_msg(info, BATT_MNGR_GET_INFO_LOG_BUFF_REQ);
	hihonor_adsp_charger_log_ops.dev_data = info;
	power_log_ops_register(&hihonor_adsp_charger_log_ops);
	info->dev = adsp_log_sysfs_create_group();
	g_adsp_log_info = info;
	pr_info("hihonor_adsp_log_probe out\n");

	return 0;

FREE_MEM:
	(void)pmic_glink_unregister_client(info->client);
	devm_kfree(&pdev->dev, info);
	return rc;
}

static int hihonor_adsp_log_remove(struct platform_device *pdev)
{
	struct hihonor_adsp_log_dev_info *info = platform_get_drvdata(pdev);
	int rc;

	rc = pmic_glink_unregister_client(info->client);
	if (rc < 0) {
		pr_err("[adsp_log]Error unregistering from pmic_glink, rc=%d\n", rc);
		return rc;
	}
	adsp_log_sysfs_remove_group(info->dev);
	return 0;
}

static const struct of_device_id hihonor_oem_match_table[] = {
	{ .compatible = "hihonor-adsp-log" },
	{},
};

static struct platform_driver hihonor_adsp_log_driver = {
	.driver = {
		.name = "hihonor-adsp-log",
		.of_match_table = hihonor_oem_match_table,
	},
	.probe = hihonor_adsp_log_probe,
	.remove = hihonor_adsp_log_remove,
};

module_platform_driver(hihonor_adsp_log_driver);

MODULE_DESCRIPTION("hihonor adsp log driver");
MODULE_LICENSE("GPL v2");
