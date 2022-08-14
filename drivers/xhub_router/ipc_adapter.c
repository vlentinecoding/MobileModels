/*
 *  drivers/misc/inputhub/xhub_route.c
 *  Sensor Hub Channel driver
 *
 *  Copyright (C) 2012 Huawei, Inc.
 *  Author: qindiwen <inputhub@huawei.com>
 *
 */
#include "scp_mbox_layout.h"
#include "ipc_adapter.h"

#include <log/hw_log.h>

#define HWLOG_TAG sensorhub
HWLOG_REGIST();

/*
 * mbox slot size definition
 * 1 slot for 4 bytes
 */
#define MBOX_SLOT_SIZE 4

int ipc_adapter_send(const char *buf, unsigned int length)
{
#ifdef HW_CUST_IPC
	enum scp_ipi_status status;

	if (length > (PIN_OUT_SIZE_SCP_MPOOL - 2) * MBOX_SLOT_SIZE) {
		hwlog_warn("ipc msg len(%d) is too long, should use share memory\n", length);
		return 0;
	}
	/* for mtk */
	status = scp_ipi_send(IPI_HW_CUST, (void *)buf, length, 0, SCP_A_ID);
	if (status != SCP_IPI_DONE) {
		hwlog_err("scp_ipi_send fail, status %d\n", status);
		return -1;
	}
	hwlog_info("%s success\n", __func__);
#endif

	return 0;
}
