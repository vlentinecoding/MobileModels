/*
 * npu_task_message.c
 *
 * about npu task message
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
 *
 */
#include "npu_task_message.h"

#include "npu_pm_framework.h"
#include "npu_adapter.h"
#include "bbox/npu_dfx_black_box.h"
#include "npu_common_resource.h"

void npu_set_report_timeout(struct npu_dev_ctx *dev_ctx, int *timeout_out)
{
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	cond_return_void(timeout_out == NULL, "timeout_out is null\n");

	*timeout_out = dev_ctx->pm.npu_task_time_out;
	return;
}

void npu_exception_timeout(
	struct npu_dev_ctx *dev_ctx, struct npu_proc_ctx *proc_ctx)
{
	npu_drv_warn("drv receive response wait timeout\n");
	cond_return_void(dev_ctx == NULL, "dev_ctx is null\n");
	cond_return_void(proc_ctx == NULL, "proc_ctx is null\n");

	down_write(&dev_ctx->pm.exception_lock);
	if (dev_ctx->pm.npu_exception_status == NPU_STATUS_EXCEPTION) {
		up_write(&dev_ctx->pm.exception_lock);
		return;
	}
	dev_ctx->pm.npu_exception_status = NPU_STATUS_EXCEPTION;
	dev_ctx->pm.npu_exception_task_id = 0;
	dev_ctx->pm.npu_exception_persist_stream_id = 0;
	dev_ctx->pm.npu_exception_persist_task_id = 0;
	npu_exception_timeout_record(dev_ctx);
	npu_pm_exception_powerdown(proc_ctx, dev_ctx);
	up_write(&dev_ctx->pm.exception_lock);

	if (npu_rdr_exception_is_count_exceeding(
		RDR_EXC_TYPE_TS_RUNNING_TIMEOUT) == 0) {
		npu_drv_warn("call rdr_system_error\n");
		rdr_system_error(RDR_EXC_TYPE_TS_RUNNING_TIMEOUT, 0, 0);
	}
}

int npu_task_set_init(u8 dev_ctx_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;
	struct list_head *task_set = NULL;
	struct npu_task_info *task_buffer = NULL;
	int i = 0;
	int count = 0;

	cond_return_error(dev_ctx_id >= NPU_DEV_NUM, -EINVAL,
		"invalid device id, dev_id = %u\n", dev_ctx_id);
	cur_dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	cond_return_error(cur_dev_ctx == NULL, -ENODATA,
		"cur_dev_ctx %u is null\n", dev_ctx_id);

	task_set = vmalloc(sizeof(struct list_head) * NPU_MAX_NON_SINK_STREAM_ID);
	cond_return_error(task_set == NULL, -1, "alloc memory fail\n");

	count = NPU_MAX_SQ_NUM * NPU_MAX_SQ_DEPTH + TS_TASK_BUFF_SIZE;
	task_buffer = vmalloc(sizeof(struct npu_task_info) * count);
	if (task_buffer == NULL) {
		npu_drv_err("alloc memory fail\n");
		vfree(task_set);
		return -1;
	}

	for (i = 0; i < NPU_MAX_NON_SINK_STREAM_ID; i++)
		INIT_LIST_HEAD(&task_set[i]);
	cur_dev_ctx->pm.task_set = task_set;
	INIT_LIST_HEAD(&cur_dev_ctx->pm.task_available_list);
	for (i = 0; i < count; i++)
		list_add(&task_buffer[i].node, &cur_dev_ctx->pm.task_available_list);
	cur_dev_ctx->pm.task_buffer = task_buffer;
	atomic_set(&cur_dev_ctx->pm.task_ref_cnt, 0);

	return 0;
}

int npu_task_set_destroy(u8 dev_ctx_id)
{
	struct npu_dev_ctx *cur_dev_ctx = NULL;

	npu_drv_warn("task_set_lock after destroy");
	cond_return_error(dev_ctx_id >= NPU_DEV_NUM, -EINVAL,
		"invalid device id, dev_id = %u\n", dev_ctx_id);
	cur_dev_ctx = get_dev_ctx_by_id(dev_ctx_id);
	cond_return_error(cur_dev_ctx == NULL, -ENODATA,
		"cur_dev_ctx %u is null\n", dev_ctx_id);

	vfree(cur_dev_ctx->pm.task_set);
	cur_dev_ctx->pm.task_set = NULL;
	INIT_LIST_HEAD(&cur_dev_ctx->pm.task_available_list);
	vfree(cur_dev_ctx->pm.task_buffer);
	cur_dev_ctx->pm.task_buffer = NULL;

	return 0;
}

// the func is not in lock
int npu_task_set_insert(struct npu_dev_ctx *dev_ctx, u32 stream_id, u32 task_id)
{
	struct npu_task_info *task = NULL;

	cond_return_error(stream_id >= NPU_MAX_NON_SINK_STREAM_ID, -EINVAL,
		"invalid stream id, stream_id = %u\n", stream_id);

	mutex_lock(&dev_ctx->pm.task_set_lock);
	if (list_empty(&dev_ctx->pm.task_available_list)) {
		npu_drv_err("task_available_list is empty\n");
		mutex_unlock(&dev_ctx->pm.task_set_lock);
		return -1;
	}
	task = list_first_entry(&dev_ctx->pm.task_available_list,
		struct npu_task_info, node);
	list_del(&task->node);
	task->task_id = task_id;
	task->stream_id = stream_id;
	list_add_tail(&task->node, &dev_ctx->pm.task_set[stream_id]);
	mutex_unlock(&dev_ctx->pm.task_set_lock);
	return 0;
}

// the func is not in lock
int npu_task_set_remove(struct npu_dev_ctx *dev_ctx, u32 stream_id, u32 task_id)
{
	struct npu_task_info *task = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;

	cond_return_error(stream_id >= NPU_MAX_NON_SINK_STREAM_ID, -EINVAL,
		"invalid stream id, stream_id = %u\n", stream_id);

	mutex_lock(&dev_ctx->pm.task_set_lock);
	list_for_each_safe(pos, n, &dev_ctx->pm.task_set[stream_id]) {
		task = list_entry(pos, struct npu_task_info, node);
		if (task->task_id == task_id) {
			list_del(&task->node);
			list_add(&task->node, &dev_ctx->pm.task_available_list);
			mutex_unlock(&dev_ctx->pm.task_set_lock);
			return 0;
		}
	}
	mutex_unlock(&dev_ctx->pm.task_set_lock);
	return -1;
}

// the func can not be in npu_power_up_off_mutex
int npu_task_set_clear(struct npu_dev_ctx *dev_ctx)
{
	struct npu_task_info *task = NULL;
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	int i;

	mutex_lock(&dev_ctx->pm.task_set_lock);
	for (i = 0; i < NPU_MAX_NON_SINK_STREAM_ID; i++) {
		list_for_each_safe(pos, n, &dev_ctx->pm.task_set[i]) {
			task = list_entry(pos, struct npu_task_info, node);
			list_del(&task->node);
			list_add(&task->node, &dev_ctx->pm.task_available_list);
		}
	}
	atomic_set(&dev_ctx->pm.task_ref_cnt, 0);
	mutex_unlock(&dev_ctx->pm.task_set_lock);
	return 0;
}