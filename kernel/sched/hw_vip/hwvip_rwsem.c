/*
 * hwvip_rwsem.c
 *
 * rwsem schedule implementation
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
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

#ifdef CONFIG_HW_VIP_THREAD
/*lint -save -e578 -e695 -e571*/
#include <linux/list.h>
#include <linux/sched/hw_vip/hwvip_common.h>
#include <linux/sched/hw_vip/hwvip_rwsem.h>

enum rwsem_waiter_type {
	RWSEM_WAITING_FOR_WRITE,
	RWSEM_WAITING_FOR_READ
};

struct rwsem_waiter {
	struct list_head list;
	struct task_struct *task;
	enum rwsem_waiter_type type;
};

#define RWSEM_READER_OWNED ((struct task_struct *)1UL)

static inline bool rwsem_owner_is_writer(struct task_struct *owner)
{
	return owner && owner != RWSEM_READER_OWNED;
}

static void rwsem_list_add_vip(struct list_head *entry, struct list_head *head)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	struct rwsem_waiter *waiter = NULL;

	list_for_each_safe(pos, n, head) {
		waiter = list_entry(pos, struct rwsem_waiter, list);
		if (!test_task_vip(waiter->task)) {
			list_add(entry, waiter->list.prev);
			return;
		}
	}
	if (pos == head)
		list_add_tail(entry, head);
}

void rwsem_list_add(struct task_struct *tsk,
	struct list_head *entry, struct list_head *head)
{
	bool is_vip = test_set_dynamic_vip(tsk);

	if (!entry || !head)
		return;

	if (is_vip)
		rwsem_list_add_vip(entry, head);
	else
		list_add_tail(entry, head);
}

void rwsem_dynamic_vip_enqueue(
	struct task_struct *tsk, struct task_struct *waiter_task,
	struct task_struct *owner, struct rw_semaphore *sem)
{
	bool is_vip = test_set_dynamic_vip(tsk);

	if (waiter_task && is_vip) {
		if (rwsem_owner_is_writer(owner) &&
			!test_task_vip(owner) && sem && !sem->vip_dep_task) {
			dynamic_vip_enqueue(owner, DYNAMIC_VIP_RWSEM, tsk->vip_depth);
			sem->vip_dep_task = owner;
		}
	}
}

void rwsem_dynamic_vip_dequeue(struct rw_semaphore *sem,
	struct task_struct *tsk)
{
	if (tsk && sem && sem->vip_dep_task == tsk) {
		dynamic_vip_dequeue(tsk, DYNAMIC_VIP_RWSEM);
		sem->vip_dep_task = NULL;
	}
}

/*lint -restore*/
#endif

