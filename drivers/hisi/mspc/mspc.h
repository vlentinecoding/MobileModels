/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Header file for MSPC module.
 * Create: 2020/04/03
 */

#ifndef MSPC_H
#define MSPC_H

#include <linux/types.h>

#define MSPC_MODULE_UNREADY             0xC8723B6D
#define MSPC_MODULE_READY               0x378DC492

int32_t mspc_get_init_status(void);

#endif /*  MSPC_H */
