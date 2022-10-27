/*
 * Honor Touchscreen Driver
 *
 * Copyright (c) 2021-2021 Honor Technologies Co., Ltd.
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
#include <linux/pm_runtime.h>
#include <linux/delay.h>
#include <honor_platform/log/hw_log.h>
#ifdef CONFIG_HUAWEI_HW_DEV_DCT
#include <hwmanufac/dev_detect/dev_detect.h>
#endif

#include "huawei_ts_kit.h"
#include "tpkit_platform_adapter.h"
#include "sprd_cmdline_parse.h"

#define BUSY_QUERY_TIMEOUT          200
#define BUSY_QUERY_DELAY            150 /* unit: us */

#ifdef CONFIG_HUAWEI_HW_DEV_DCT
void set_tp_dev_flag(void)
{
	set_hw_dev_flag(DEV_I2C_TOUCH_PANEL);
}
#endif

int read_tp_color_adapter(char *buf, int buf_size)
{
	hwlog_info("%s: not implemented!\n", __func__);
	return 0;
}
EXPORT_SYMBOL(read_tp_color_adapter);

int write_tp_color_adapter(const char *buf)
{
	hwlog_info("%s: not implemented!\n", __func__);
	return 0;
}
EXPORT_SYMBOL(write_tp_color_adapter);

unsigned int get_into_recovery_flag_adapter(void)
{
	return sprd_get_recovery_flag();
}

unsigned int get_pd_charge_flag_adapter(void)
{
	return sprd_get_pd_charge_flag();
}

int charger_type_notifier_register(struct notifier_block *nb)
{
	hwlog_info("%s: not implemented!\n", __func__);
	return 0;
}

int charger_type_notifier_unregister(struct notifier_block *nb)
{
	hwlog_info("%s: not implemented!\n", __func__);
	return 0;
}

int sprd_wait_spi_ready(struct spi_device *spi)
{
	int i, ret;
	int retry_timeout = BUSY_QUERY_TIMEOUT;

	if (spi == NULL) {
		TS_LOG_ERR("%s, spi is NULL\n", __func__);
		return -EINVAL;
	}
	if (spi->master == NULL) {
		TS_LOG_ERR("%s, spi master NULL\n", __func__);
		return -ENODEV;
	}
	if (!spi->master->auto_runtime_pm) {
		TS_LOG_ERR("%s, spi master auto_runtime_pm false\n", __func__);
		return -ENODEV;
	}

	for (i = 0; i < retry_timeout; i++) {
		ret = pm_runtime_get_sync(spi->master->dev.parent);
		if (ret < 0) {
			usleep_range(BUSY_QUERY_DELAY, BUSY_QUERY_DELAY + 50);
		} else {
			pm_runtime_mark_last_busy(spi->master->dev.parent);
			pm_runtime_put_autosuspend(spi->master->dev.parent);
			return 0;
		}
	}
	TS_LOG_INFO("spi was not ready: %d\n", ret);
	return -EACCES;
}

