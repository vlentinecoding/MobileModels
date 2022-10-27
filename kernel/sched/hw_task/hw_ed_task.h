/*
 * hw_ed_task.h
 *
 * huawei early detection task header file
 *
 * Copyright (c) 2019, Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef __HW_ED_TASK_H__
#define __HW_ED_TASK_H__

#ifdef CONFIG_HW_ED_TASK
#define EARLY_DETECTION_TASK_WAITING_DURATION 11500000
#define EARLY_DETECTION_TASK_RUNNING_DURATION 120000000
#define EARLY_DETECTION_NEW_TASK_RUNNING_DURATION 100000000

bool is_ed_task_ext(struct rq *rq, struct task_struct *p, u64 wall, bool new_task);
#else
bool is_ed_task_ext(struct rq *rq, struct task_struct *p, u64 wall, bool new_task)
{
	return false;
}
#endif

#endif
