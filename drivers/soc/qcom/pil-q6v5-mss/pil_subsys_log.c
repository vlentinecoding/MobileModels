/*
 * Copyright (C) 2021 Honor Device Co.Ltd
 * License terms: GNU General Public License (GPL) version 2
 *
 */

#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <log/hiview_hievent.h>
#include "pil_subsys_log.h"

#define REASON_LEN (128U)

/* define work data sturct */
struct work_data {
	char *name;
	struct workqueue_struct *log_subsys_crash_work_queue; // WORK QUEUE
	struct work_struct log_subsys_crash_work; // WORK
	char reset_reason[REASON_LEN];
};

static struct work_data *g_work_data = NULL;

static void upload_subsys_crash_log(struct work_data *work)
{
	int ret, event_id;
	struct hiview_hievent *subsys_hiview_hievent = NULL;

	if (strcmp(work->name, "modem") == 0) {
		event_id = 901002000; // Modem Crash code
	} else {
		event_id = 901002005; // Wcnss/Adsp/Venus Crash code
	}

	subsys_hiview_hievent = hiview_hievent_create(event_id);
	if (!subsys_hiview_hievent) {
		pr_err("create subsystem_crash hievent fail, crash event_id:%d\n", event_id);
		return;
	}

	ret = hiview_hievent_put_string(subsys_hiview_hievent, "PNAME", work->name);
	if (ret < 0)
		pr_err("hievent put string failed, crash event_id:%d, error_num:%d\n", event_id, ret);

	ret = hiview_hievent_put_string(subsys_hiview_hievent, "F1NAME", work->reset_reason);
	if (ret < 0)
		pr_err("hievent put string failed, crash event_id:%d, error_num:%d\n", event_id, ret);

	ret = hiview_hievent_report(subsys_hiview_hievent);
	if (ret < 0)
		pr_err("subsys_crash hiview report failed, crash event_id:%d, error_num:%d\n", event_id, ret);

	hiview_hievent_destroy(subsys_hiview_hievent);
}

static void log_subsys_crash_work_func(struct work_struct *work)
{
	struct work_data *work_data_self = container_of(work, struct work_data, log_subsys_crash_work);
	if (!work_data_self) {
		pr_err("[log_subsys_reset]work_data_self is NULL\n");
		return;
	}

	upload_subsys_crash_log(work_data_self);
	pr_info("[log_subsys_reset]log_subsys_crash_work_func reset_reason=%s\n", work_data_self->reset_reason);
}

void report_subsys_crash_log(const char *name, const char *reason, size_t reason_len)
{
	int is_workqueue_pending;
	if (!name || !reason || reason_len == 0) {
		pr_err("log subsystem reset work para invalid, reason_len = %d\n", reason_len);
		return;
	}

	is_workqueue_pending = work_pending(&g_work_data->log_subsys_crash_work);
	if (is_workqueue_pending) {
		pr_err("log subsystem reset work queue is pending, ignore current ones\n");
		return;
	}

	g_work_data->name = name;
	memset(g_work_data->reset_reason, 0, REASON_LEN);
	(void)strncpy_s(g_work_data->reset_reason, REASON_LEN, reason, min(reason_len, REASON_LEN - 1));

	if (g_work_data->log_subsys_crash_work_queue) {
		queue_work(g_work_data->log_subsys_crash_work_queue, &(g_work_data->log_subsys_crash_work));
		pr_err("log_subsys_crash_work is inserted\n");
	}
}

void create_subsys_crash_log_queue(void)
{
	if (g_work_data) {
		return;
	}
	g_work_data = kzalloc(sizeof(struct work_data), GFP_KERNEL);
	if (!g_work_data) {
		pr_err("[log_subsys_reset]work_data_temp is NULL, Don't log this\n");
		return;
	}

	memset(g_work_data, 0, sizeof(struct work_data));
	INIT_WORK(&(g_work_data->log_subsys_crash_work), log_subsys_crash_work_func);
	g_work_data->log_subsys_crash_work_queue = create_singlethread_workqueue("log_subsystem_reset");
	if (!g_work_data->log_subsys_crash_work_queue) {
		pr_err("[log_subsys_reset]log subsystem reset queue created failed\n");
		kfree(g_work_data);
		g_work_data = NULL;
		return;
	}

	pr_info("[log_subsys_reset]log subsystem reset queue created success\n");
}

void destroy_subsys_crash_log_queue(void)
{
	if (g_work_data) {
		kfree(g_work_data);
		g_work_data = NULL;
	}
}
