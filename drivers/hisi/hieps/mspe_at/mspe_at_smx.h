/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2021. All rights reserved.
 * Description: msp engine at command smx function.
 * Create: 2020/4/9
 */
#ifndef __MSPE_AT_SMX_H__
#define __MSPE_AT_SMX_H__

#include <linux/types.h>
#include <linux/kernel.h>

int32_t mspe_at_set_smx(const char *value, int32_t (*resp)(char *buf, int len));
int32_t mspe_at_get_smx(const char *value, int32_t (*resp)(char *buf, int len));

#endif /* __MSPE_AT_SMX_H__ */
