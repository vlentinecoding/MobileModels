/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Header file for mspc flash(otp/efuse) process.
 * Create: 2020/03/05
 */

#ifndef __MSPC_FLASH_H__
#define __MSPC_FLASH_H__

#include <linux/types.h>

#ifdef CONFIG_MSPC
void creat_flash_otp_thread(void);
void mspc_reinit_flash_complete(void);
void mspc_release_flash_complete(void);
bool mspc_flash_is_task_started(void);
void mspc_flash_register_func(int32_t (*fn_ptr) (void));
int32_t efuse_check_secdebug_disable(bool *b_disabled);
int32_t mspc_write_otp_image(void);
#else
static inline void creat_flash_otp_thread(void)
{
}
#endif

#endif
