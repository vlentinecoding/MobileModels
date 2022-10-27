/*
 * Copyright (c) Honor Device Co., Ltd. 2021-2021. All rights reserved.
 * Description: save bootloader log function declaration.
 * Author:  guanhang
 * Create:  2021-08-30
 */

#ifndef __RAWDATA_OPER_LOG_COLLECTOR_H__
#define __RAWDATA_OPER_LOG_COLLECTOR_H__

void write_rawdata_oper_log_to_storage(const char *buf, unsigned int len);

#endif