/*
 * hihonor_oem_glink.c
 *
 * hihonor_oem_glink driver
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
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/soc/qcom/pmic_glink.h>
#include <chipset_common/hwpower/power_printk.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>

#define HWLOG_TAG oem_glink
HWLOG_REGIST();

#define MSG_OWNER_OEM              32782
#define MSG_TYPE_OEM_REQ_RESP      1
#define MSG_TYPE_OEM_NOTIFY        2
#define OEM_OPCODE_READ_BUFFER     0x10000
#define OEM_OPCODE_WRITE_BUFFER    0x10001
#define OEM_OPCODE_NOTIFY_IND      0x10002
#define OEM_WAIT_TIME_MS           1000

struct hihonor_oem_glink_dev_info {
	struct device *dev;
	struct pmic_glink_client *client;
	struct mutex rw_lock;
	atomic_t state;
	struct completion ack;
	struct work_struct sync_work;
	bool initialized;
	atomic_t data_ready;
	u32 data_buffer[MAX_OEM_PROPERTY_DATA_SIZE];
	u32 data_size;
};

struct hihonor_oem_glink_notify_msg {
	struct pmic_glink_hdr hdr;
	u32 notification;
};

struct hihonor_oem_glink_set_msg {
	struct pmic_glink_hdr hdr;
	struct hihonor_oem_glink_msg_buffer msg_buff;
};

struct hihonor_oem_glink_get_msg {
	struct pmic_glink_hdr hdr;
	u32 oem_property_id;
	u32 data_size;
};

struct hihonor_oem_glink_rsp_msg {
	struct pmic_glink_hdr hdr;
	u32 ret_code;
};

struct hihonor_glink_ops_list_node {
	struct list_head node;
	struct hihonor_glink_ops *ops;
};

static struct hihonor_oem_glink_dev_info *g_dev_info;
static LIST_HEAD(glink_ops_listhead);

int hihonor_oem_glink_ops_register(struct hihonor_glink_ops *ops)
{
	struct hihonor_glink_ops_list_node *entry = NULL;

	if (!ops)
		return -1;

	entry = kzalloc(sizeof(*entry), GFP_KERNEL);
	if (!entry)
		return -1;

	entry->ops = ops;
	list_add_tail(&entry->node, &glink_ops_listhead);

	return 0;
}

int hihonor_oem_glink_ops_unregister(struct hihonor_glink_ops *ops)
{
	struct hihonor_glink_ops_list_node *pos = NULL;
	struct hihonor_glink_ops_list_node *pos_next = NULL;

	if (!ops)
		return -1;

	list_for_each_entry_safe(pos, pos_next, &glink_ops_listhead, node) {
		if (strcmp(pos->ops->name, ops->name))
			continue;

		list_del(&pos->node);
		kfree(pos);
		pos = NULL;
		break;
	}

	return 0;
}

static int hihonor_oem_glink_oem_write(void *msg, int len)
{
	struct hihonor_oem_glink_dev_info *info = g_dev_info;
	int rc;

	if (!info)
		return -1;

	if (atomic_read(&info->state) == PMIC_GLINK_STATE_DOWN) {
		hwlog_err("glink state is down\n");
		return 0;
	}

	mutex_lock(&info->rw_lock);
	reinit_completion(&info->ack);
	rc = pmic_glink_write(info->client, msg, len);
	if (!rc) {
		rc = wait_for_completion_timeout(&info->ack,
			msecs_to_jiffies(OEM_WAIT_TIME_MS));
		if (!rc) {
			hwlog_err("Error, timed out sending message\n");
			mutex_unlock(&info->rw_lock);
			return -ETIMEDOUT;
		}
	}

	mutex_unlock(&info->rw_lock);

	return 0;
}

int hihonor_oem_glink_oem_set_prop(u32 oem_property_id, void *data, size_t data_size)
{
	struct hihonor_oem_glink_set_msg req_msg = {0};

	if (!data || !data_size)
		return -1;

	req_msg.hdr.owner = MSG_OWNER_OEM;
	req_msg.hdr.type = MSG_TYPE_OEM_REQ_RESP;
	req_msg.hdr.opcode = OEM_OPCODE_WRITE_BUFFER;
	req_msg.msg_buff.oem_property_id = oem_property_id;
	req_msg.msg_buff.data_size = data_size;
	memcpy(req_msg.msg_buff.data_buffer, data, data_size);

	return hihonor_oem_glink_oem_write(&req_msg, sizeof(req_msg));
}

int hihonor_oem_glink_oem_get_prop(u32 oem_property_id, void *data, size_t data_size)
{
	struct hihonor_oem_glink_get_msg req_msg = {0};
	struct hihonor_oem_glink_dev_info *info = g_dev_info;
	int ret;
	int count = 10; /* wait count */

	if (!info || !data || !data_size)
		return -1;

	req_msg.oem_property_id = oem_property_id;
	req_msg.data_size = data_size;
	req_msg.hdr.owner = MSG_OWNER_OEM;
	req_msg.hdr.type = MSG_TYPE_OEM_REQ_RESP;
	req_msg.hdr.opcode = OEM_OPCODE_READ_BUFFER;

	ret = hihonor_oem_glink_oem_write(&req_msg, sizeof(req_msg));
	if (ret)
		return -1;

	while (count--) {
		if (atomic_read(&info->data_ready) == 0) {
			msleep(10); /* sleep 10ms for data ready */
			continue;
		}

		memcpy(data, info->data_buffer, data_size);
		memset(info->data_buffer, 0, sizeof(int) * MAX_OEM_PROPERTY_DATA_SIZE);
		info->data_size = 0;
		atomic_set(&info->data_ready, 0);
		break;
	}

	if (!count)
		return -1;

	return 0;
}

static void hihonor_oem_glink_handle_notification(struct hihonor_oem_glink_dev_info *info, void *data,
	size_t len)
{
	struct hihonor_oem_glink_notify_msg *notify_msg = data;
	struct hihonor_glink_ops_list_node *pos = NULL;

	if (len != sizeof(*notify_msg)) {
		hwlog_err("Incorrect response length %zu\n", len);
		return;
	}
	if (notify_msg->notification >= OEM_NOTIFY_EVENT_END) {
		hwlog_err("Incorrect notification %u\n", notify_msg->notification);
		return;
	}

	hwlog_info("notification: %#x\n", notify_msg->notification);
	list_for_each_entry(pos, &glink_ops_listhead, node) {
		if (pos->ops && pos->ops->notify_event)
			pos->ops->notify_event(pos->ops->dev_data, notify_msg->notification);
	}
}

static bool hihonor_oem_glink_is_valid_message(struct hihonor_oem_glink_rsp_msg *resp_msg, size_t len)
{
	if (sizeof(*resp_msg) != len)
		return false;

	if (resp_msg->ret_code)
		return false;

	return true;
}

static void hihonor_oem_glink_update_data(struct hihonor_oem_glink_dev_info *info,
	struct hihonor_oem_glink_msg_buffer *data)
{
	int count = 10; /* wait count */

	while (count--) {
		if (atomic_read(&info->data_ready) == 1) {
			msleep(10); /* sleep 10ms for read data */
			continue;
		}

		memcpy(info->data_buffer, data->data_buffer, data->data_size);
		info->data_size = data->data_size;
		atomic_set(&info->data_ready, 1);
		break;
	}
}

static void hihonor_oem_glink_handle_message(struct hihonor_oem_glink_dev_info *info, void *data,
	size_t len)
{
	struct hihonor_oem_glink_rsp_msg *resp_msg = (struct hihonor_oem_glink_rsp_msg *)data;
	struct hihonor_oem_glink_set_msg *req_msg = (struct hihonor_oem_glink_set_msg *)data;
	bool ack_set = false;

	switch (resp_msg->hdr.opcode) {
	case OEM_OPCODE_WRITE_BUFFER:
		if (hihonor_oem_glink_is_valid_message(resp_msg, len))
			ack_set = true;
		break;
	case OEM_OPCODE_READ_BUFFER:
		if (!req_msg->msg_buff.data_size ||
			req_msg->msg_buff.data_size > sizeof(int) * MAX_OEM_PROPERTY_DATA_SIZE)
			break;
		hihonor_oem_glink_update_data(info, &req_msg->msg_buff);
		ack_set = true;
		break;
	default:
		break;
	}

	if (ack_set)
		complete(&info->ack);
}

static int hihonor_oem_glink_callback(void *priv, void *data, size_t len)
{
	struct pmic_glink_hdr *hdr = data;
	struct hihonor_oem_glink_dev_info *info = priv;

	if (!info || !data)
		return -1;

	hwlog_debug("owner: %u type: %u opcode: %#x len: %zu\n", hdr->owner,
		hdr->type, hdr->opcode, len);

	if (!info->initialized) {
		hwlog_info("Driver initialization failed: Dropping glink callback message: state %d\n",
			 info->state);
		return 0;
	}

	if (hdr->opcode == OEM_OPCODE_NOTIFY_IND)
		hihonor_oem_glink_handle_notification(info, data, len);
	else
		hihonor_oem_glink_handle_message(info, data, len);

	return 0;
}

static void hihonor_oem_glink_state_cb(void *priv, enum pmic_glink_state state)
{
	struct hihonor_oem_glink_dev_info *info = priv;

	if (!info)
		return;

	hwlog_info("state: %d\n", state);

	atomic_set(&info->state, state);
	if (state == PMIC_GLINK_STATE_UP)
		schedule_work(&info->sync_work);
}

static void hihonor_oem_glink_sync_work(struct work_struct *work)
{
	struct hihonor_glink_ops_list_node *pos = NULL;

	list_for_each_entry(pos, &glink_ops_listhead, node) {
		if (pos->ops && pos->ops->sync_data)
			pos->ops->sync_data(pos->ops->dev_data);
	}
}

static int hihonor_oem_glink_probe(struct platform_device *pdev)
{
	struct hihonor_oem_glink_dev_info *info = NULL;
	struct device *dev = &pdev->dev;
	struct pmic_glink_client_data client_data = {0};
	int rc;

	hwlog_info("hihonor_oem_glink_probe in\n");
	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	client_data.id = MSG_OWNER_OEM;
	client_data.name = "hihonor-oem";
	client_data.msg_cb = hihonor_oem_glink_callback;
	client_data.priv = info;
	client_data.state_cb = hihonor_oem_glink_state_cb;

	info->client = pmic_glink_register_client(dev, &client_data);
	if (IS_ERR(info->client)) {
		rc = PTR_ERR(info->client);
		if (rc != -EPROBE_DEFER)
			hwlog_err("Error in registering with pmic_glink %d\n", rc);

		return rc;
	}

	atomic_set(&info->state, PMIC_GLINK_STATE_UP);
	mutex_init(&info->rw_lock);
	init_completion(&info->ack);
	INIT_WORK(&info->sync_work, hihonor_oem_glink_sync_work);
	info->initialized = true;
	platform_set_drvdata(pdev, info);
	g_dev_info = info;
	hwlog_info("hihonor_oem_glink_probe out\n");

	return 0;
}

static int hihonor_oem_glink_remove(struct platform_device *pdev)
{
	struct hihonor_oem_glink_dev_info *info = platform_get_drvdata(pdev);
	int rc;

	rc = pmic_glink_unregister_client(info->client);
	if (rc < 0) {
		pr_err("Error unregistering from pmic_glink, rc=%d\n", rc);
		return rc;
	}

	return 0;
}

static void hihonor_oem_glink_shutdown(struct platform_device *pdev)
{
	int enable = 0; // disable debug access detect while shutdown

	if(hihonor_oem_glink_oem_set_prop(CHARGER_OEM_DEBUG_ACCESS_EN, &enable, sizeof(enable))) {
		hwlog_err("disable debug access detect fail\n");
	}
}

static const struct of_device_id hihonor_oem_match_table[] = {
	{ .compatible = "hihonor-oem-glink" },
	{},
};

static struct platform_driver hihonor_oem_driver = {
	.driver = {
		.name = "hihonor-oem-glink",
		.of_match_table = hihonor_oem_match_table,
	},
	.probe = hihonor_oem_glink_probe,
	.remove = hihonor_oem_glink_remove,
	.shutdown = hihonor_oem_glink_shutdown,
};

module_platform_driver(hihonor_oem_driver);

MODULE_DESCRIPTION("hihonor oem Glink driver");
MODULE_LICENSE("GPL v2");
