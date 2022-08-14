/*
 * Huawei Early Detection File
 *
 * Copyright (c) 2019-2021 Huawei Technologies Co., Ltd.
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

#include <linux/sched.h>
#include <../kernel/sched/sched.h>
#include <../kernel/sched/walt/walt.h>

#ifdef CONFIG_HW_RTG_NORMALIZED_UTIL
static inline bool is_rtg_ed_task(struct rq *rq, struct task_struct *p, u64 wall, bool new_task)
{
	struct walt_related_thread_group *grp = NULL;
	struct walt_sched_cluster *prefer_cluster = NULL;
	int cpu = cpu_of(rq);

	rcu_read_lock();
	grp = task_related_thread_group(p);
	rcu_read_unlock();

	if (!grp)
		return false;
	prefer_cluster = grp->preferred_cluster;
	if (!prefer_cluster)
		return false;
	if (!grp->ed_enabled)
		return false;

	/* if the task running on perferred cluster, then igore */
	if (cpumask_test_cpu(cpu, &prefer_cluster->cpus) ||
	    capacity_orig_of(cpu) > capacity_orig_of(cpumask_first(&prefer_cluster->cpus)))
		return false;

	if (p->last_wake_wait_sum >= grp->ed_task_waiting_duration)
		return true;

	if (wall - p->wts.last_wake_ts >= grp->ed_task_running_duration)
		return true;

	if (new_task && wall - p->wts.last_wake_ts >=
				grp->ed_new_task_running_duration)
		return true;

	return false;
}
#endif /* CONFIG_HW_RTG_NORMALIZED_UTIL */


bool is_ed_task_ext(struct rq *rq, struct task_struct *p, u64 wall, bool new_task)
{
#ifdef CONFIG_SCHED_TASK_UTIL_CLAMP
	if (p->uclamp[UCLAMP_MAX].value < capacity_orig_of(cpu_of(rq))) {
		rq->skip_overload_detect = true;
		return false;
	}
#endif

#ifdef CONFIG_HW_RTG_NORMALIZED_UTIL
	/* handle rtg task */
	if (is_rtg_ed_task(rq, p, wall, new_task))
		return true;
#endif

	if (p->last_wake_wait_sum >= rq->ed_task_waiting_duration)
		return true;

	if (wall - p->wts.last_wake_ts >= rq->ed_task_running_duration)
		return true;

	if (new_task && wall - p->wts.last_wake_ts >=
				rq->ed_new_task_running_duration)
		return true;

	return false;
}
