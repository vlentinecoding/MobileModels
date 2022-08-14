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
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/soc/qcom/pmic_glink.h>
#include <linux/kthread.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_printk.h>

#define HWLOG_TAG hihonor_adsp_log
HWLOG_REGIST();

#define MSG_OWNER_CHARGER          32778
#define MSG_OWNER_QBG_DEBUG        32781
#define MSG_TYPE_LOG_REQ_RESP      1
#define MSG_TYPE_LOG_NOTIFY        2
#define BATT_MNGR_GET_VOTE_REQ     0x17
#define BATT_MNGR_GET_ULOG_REQ     0x18
#define ULOG_WAIT_TIME_MS          2000
#define MAX_ULOG_READ_BUFFER_SIZE  8192
#define UPDATE_LOG_WORK_TIME       30000
#define VOTE_MAX_NUM               32
#define MAX_CLIENT_NAME_LENTH      32

struct hihonor_adsp_log_dev_info {
	struct device *dev;
	char client_name[MAX_CLIENT_NAME_LENTH];
	u32 client_id;
	u32 update_work_enable;
	struct pmic_glink_client *client;
	struct mutex rw_lock;
	atomic_t state;
	struct completion ack;
	struct completion data_ready;
	bool initialized;
	struct delayed_work update_log_work;
	struct task_struct *log_task;
};

/** request Message; to get ulogs from chargerPD */
struct hihonor_adsp_log_get_msg {
	struct pmic_glink_hdr header;
	u32 max_logsize;
};

/** Response Message; to get ulogs from chargerPD */
struct hihonor_adsp_log_rsp_msg {
	struct pmic_glink_hdr header;
	char read_buffer[MAX_ULOG_READ_BUFFER_SIZE];
};

struct hihonor_adsp_log_vote_req_msg {
	struct pmic_glink_hdr header;
	u32 voteable_id;
};

/** Response Message; for get voteable information*/
struct hihonor_adsp_log_vote_rsp_msg {
struct pmic_glink_hdr header;
	u32 voteable_id;
	u32 effective_value;
	u8  voters[VOTE_MAX_NUM];
	u32 voted_values[VOTE_MAX_NUM];
	u32 voter_enable_status;
	u32 effective_voter_id;
	u32 overwrite_voter_id;
};

static int hihonor_adsp_log_write(struct hihonor_adsp_log_dev_info *info,
	struct pmic_glink_client *client, void *msg, size_t len)
{
	int rc;

	if (atomic_read(&info->state) == PMIC_GLINK_STATE_DOWN) {
		pr_err("[adsp_log]glink state is down\n");
		return 0;
	}

	mutex_lock(&info->rw_lock);
	reinit_completion(&info->ack);
	rc = pmic_glink_write(client, msg, len);
	if (!rc) {
		rc = wait_for_completion_timeout(&info->ack,
			msecs_to_jiffies(ULOG_WAIT_TIME_MS));
		if (!rc) {
			pr_err("[adsp_log]Error, timed out sending message\n");
			mutex_unlock(&info->rw_lock);
			return -ETIMEDOUT;
		}
	}

	mutex_unlock(&info->rw_lock);

	return 0;
}

static void hihonor_adsp_log_handle_notification(struct hihonor_adsp_log_dev_info *info, void *data,
	size_t len)
{
	return;
}

static void hihonor_adsp_log_handle_ulog_msg(struct hihonor_adsp_log_dev_info *info, void *data)
{
	struct hihonor_adsp_log_rsp_msg *rsp_msg = data;

	switch (rsp_msg->header.opcode) {
	case BATT_MNGR_GET_ULOG_REQ:
		pr_info("[adsp_log]%s\n", rsp_msg->read_buffer);
		break;
	default:
		break;
	}
}

static void hihonor_adsp_log_handle_vote_msg(struct hihonor_adsp_log_dev_info *info, void *data)
{
	struct hihonor_adsp_log_vote_rsp_msg *rsp_msg = data;
	int i;

	switch (rsp_msg->header.opcode) {
	case BATT_MNGR_GET_VOTE_REQ:
		pr_info("[adsp_log]voteableid=%u, effective_value=%u, voter_enable_status=%u, "
			"effective_voter_id=%u, overwrite_voter_id=%u\n",
			rsp_msg->voteable_id, rsp_msg->effective_value, rsp_msg->voter_enable_status,
			rsp_msg->effective_voter_id, rsp_msg->overwrite_voter_id);
		for (i = 0; i < VOTE_MAX_NUM; i++) {
			pr_info("voiters[%u] value is %u\n", rsp_msg->voters[i], rsp_msg->voted_values[i]);
		}
		break;
	default:
		break;
	}
}

static void hihonor_adsp_log_handle_message(struct hihonor_adsp_log_dev_info *info, void *data,
	size_t len)
{
	struct pmic_glink_hdr *header = data;
	bool ack_set = false;

	switch (header->owner) {
	case MSG_OWNER_CHARGER:
		hihonor_adsp_log_handle_ulog_msg(info, data);
		ack_set = true;
		break;
	case MSG_OWNER_QBG_DEBUG:
		hihonor_adsp_log_handle_vote_msg(info, data);
		ack_set = true;
		break;
	default:
		break;
	}

	if (ack_set) {
		complete(&info->ack);
		/* complete(&info->data_ready); */
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

void hihonor_adsp_log_update_work(struct work_struct *work)
{
	struct hihonor_adsp_log_dev_info *info = container_of(work, struct hihonor_adsp_log_dev_info, update_log_work.work);
	struct hihonor_adsp_log_get_msg log_msg = {0};
	struct hihonor_adsp_log_vote_req_msg vote_msg = {0};
	int i;

	if (atomic_read(&info->state) == PMIC_GLINK_STATE_DOWN)
		return;

	switch (info->client_id) {
	case MSG_OWNER_CHARGER:
		log_msg.header.owner = MSG_OWNER_CHARGER;
		log_msg.header.type = MSG_TYPE_LOG_REQ_RESP;
		log_msg.header.opcode = BATT_MNGR_GET_ULOG_REQ;
		log_msg.max_logsize = MAX_ULOG_READ_BUFFER_SIZE;
		(void)hihonor_adsp_log_write(info, info->client, &log_msg, sizeof(log_msg));
		break;
	case MSG_OWNER_QBG_DEBUG:
		vote_msg.header.owner = MSG_OWNER_QBG_DEBUG;
		vote_msg.header.type = MSG_TYPE_LOG_REQ_RESP;
		vote_msg.header.opcode = BATT_MNGR_GET_VOTE_REQ;
		for (i = 0; i < 3; i++) {
			vote_msg.voteable_id = i;
			(void)hihonor_adsp_log_write(info, info->client, &vote_msg, sizeof(vote_msg));
		}
	default:
		break;
	}

	queue_delayed_work(system_power_efficient_wq, &info->update_log_work,
		   msecs_to_jiffies(UPDATE_LOG_WORK_TIME));
}

#ifdef CONFIG_PM
static int hihonor_adsp_log_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct hihonor_adsp_log_dev_info *info = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&info->update_log_work);
	return 0;
}

static int hihonor_adsp_log_resume(struct platform_device *pdev)
{
	struct hihonor_adsp_log_dev_info *info = platform_get_drvdata(pdev);

	schedule_delayed_work(&info->update_log_work, msecs_to_jiffies(UPDATE_LOG_WORK_TIME));

	return 0;
}
#endif /* CONFIG_PM */

static int hihonor_adsp_log_parse_dts(struct device_node *np, struct hihonor_adsp_log_dev_info *info)
{
	const char *client_name = NULL;

	if (!np)
		return -1;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "client_id", &info->client_id, 0))
		return -1;

	if (power_dts_read_string(power_dts_tag(HWLOG_TAG), np, "client_name", &client_name))
		return -1;

	power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "update_work_enable", &info->update_work_enable, 0);

	strncpy(info->client_name, client_name, (MAX_CLIENT_NAME_LENTH - 1));
	return 0;
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

	atomic_set(&info->state, PMIC_GLINK_STATE_UP);
	mutex_init(&info->rw_lock);
	init_completion(&info->ack);
	/* init_completion(&info->data_ready); */
	INIT_DELAYED_WORK(&info->update_log_work, hihonor_adsp_log_update_work);
	info->initialized = true;
	platform_set_drvdata(pdev, info);
	if (info->update_work_enable)
		schedule_delayed_work(&info->update_log_work, msecs_to_jiffies(UPDATE_LOG_WORK_TIME));
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
#ifdef CONFIG_PM
	.suspend = hihonor_adsp_log_suspend,
	.resume = hihonor_adsp_log_resume,
#endif
	.probe = hihonor_adsp_log_probe,
	.remove = hihonor_adsp_log_remove,
};

module_platform_driver(hihonor_adsp_log_driver);

MODULE_DESCRIPTION("hihonor adsp log driver");
MODULE_LICENSE("GPL v2");
