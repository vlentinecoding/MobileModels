/*
 * Copyright (c) Honor Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description:  headfile of sensorevent.c
 * Author: yangyang
 * Create: 2021-05-07
 * History: 2021-05-07 Creat new file
 */

#ifndef __SENSOREVENT_H__
#define __SENSOREVENT_H__

#define ENVP_LENTH            2000
#define ENVP_EXT_MEMBER       7
#define SENSOREVENT_REPORT_EVENT  _IOWR('N', 0x01, __u64)
#define SENSOREVENT_INFO_EVENT    _IOWR('N', 0x02, __u64)

#endif // __SENSOREVENT_H__
