/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Team:    Huawei DIVS
 * Date:    2020.07.20
 * Description: xhub boot module
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/rtc.h>
#include <linux/delay.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
#endif
#include <securec.h>
#include "xhub_route.h"
#include "xhub_boot.h"
#include "xhub_recovery.h"
#include "xhub_pm.h"
#include "scp_helper.h"
#include "ipc_adapter.h"
#include "scp_excep.h"
#include "platform.h"

int (*api_xhub_mcu_recv)(const char *buf, unsigned int length) = 0;

#define BUFFER_SIZE 1024
#define MINS_TO_SECS 60
#define MCU_SYNC_TIMES 3000
#define CFG_ON_DDR_MAGIC_BASE 0x12345678

static int is_sensor_mcu_mode; /* mcu power mode: 0 power off;  1 power on */
struct completion iom3_reboot;
struct config_on_ddr *g_config_on_ddr;
u8 tplcd_manufacture;
unsigned long sensor_jiffies;
int xhub_reboot_reason_flag = SENSOR_POWER_DO_RESET;

#ifdef CONFIG_HUAWEI_DSM
struct dsm_client *shb_dclient;
struct dsm_client *xhub_get_shb_dclient(void)
{
	return shb_dclient;
}
#endif

int get_sensor_mcu_mode(void)
{
	return is_sensor_mcu_mode;
}
EXPORT_SYMBOL(get_sensor_mcu_mode);

static void set_sensor_mcu_mode(int mode)
{
	is_sensor_mcu_mode = mode;
}

static lcd_module lcd_info[] = {
	{ DTS_COMP_190_B, TPLCD_190 },
	{ DTS_COMP_310_V, TPLCD_310 }
};

static lcd_model lcd_model_info[] = {
	{ DTS_COMP_190, TPLCD_190 },
	{ DTS_COMP_310, TPLCD_310 },
};

static int8_t get_lcd_model(const char *lcd_model, uint8_t index)
{
	if (!strncmp(lcd_model, lcd_model_info[index].dts_comp_lcd_model,
		strlen(lcd_model_info[index].dts_comp_lcd_model)))
		return lcd_model_info[index].tplcd;
	else
		return -1;
}

static int get_lcd_module(void)
{
	uint8_t index;
	int8_t tplcd;
	struct device_node *np = NULL;
	const char *lcd_model = NULL;
	const char *lcd_compatible = NULL;

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("not find device node %s!\n", "huawei,lcd_panel_type");
		return -1;
	}
	lcd_compatible = (char *)of_get_property(np, "lcd_panel_type", NULL);
	if (!lcd_compatible) {
		hwlog_err("can not get lcd kit compatible\n");
		return -1;
	}
	for (index = 0; index < ARRAY_SIZE(lcd_info); index++) {
		if (strncmp(lcd_info[index].dts_comp_mipi,
			lcd_compatible, strlen(lcd_compatible)) == 0)
			return lcd_info[index].tplcd;
	}

	np = of_find_compatible_node(NULL, NULL, "huawei,lcd_panel_type");
	if (!np) {
		hwlog_err("not find lcd_panel_type node\n");
		return -1;
	}
	if (of_property_read_string(np, "lcd_panel_type", &lcd_model)) {
		hwlog_err("not find lcd_model in dts\n");
		return -1;
	}
	hwlog_info("find lcd_panel_type suc in dts!!\n");

	for (index = 0; index < ARRAY_SIZE(lcd_model_info); index++) {
		tplcd = get_lcd_model(lcd_model, index);
		if (tplcd > 0)
			return tplcd;
	}

	hwlog_warn("sensor kernel failed to get lcd module\n");
	return -1;
}

static int xhub_mcu_recv(const char *buf, unsigned int length)
{
	if (atomic_read(&xhub_rec_state) == SCP_RECOVERY_START) {
		hwlog_err("xhub under recovery mode, ignore all recv data\n");
		return 0;
	}

	if (!api_xhub_mcu_recv) {
		hwlog_err("---->error: api_xhub_mcu_recv == NULL\n");
		return -1;
	} else {
		return api_xhub_mcu_recv(buf, length);
	}
}

static void hw_cust_message_recv(int id, void *data, unsigned int len)
{
	hwlog_debug("%s id %d len %d data0 %d data1 %d\n",
		__func__, id, len, *((int *)data), *((int *)(data + 4)));
	xhub_mcu_recv(data, len);
}

static int xhub_mcu_connect(void)
{
	enum scp_ipi_status status;

	status = scp_ipi_registration(IPI_HW_CUST,
			hw_cust_message_recv, "hw_cust_ipc_rx");
	if (status != SCP_IPI_DONE)
		hwlog_err("ipi register failed ret %d\n", status);

	/* connect to xhub_route */
	api_xhub_mcu_recv = xhub_route_recv_mcu_data;
	return 0;
}

#ifdef CONFIG_HUAWEI_DSM
/* extern void hisi_rdr_nmi_notify_iom3(void); */
static int xhub_img_dump(int type, void *buff, int size)
{
	return 0;
}

struct dsm_client_ops xhub_ops = {
	.poll_state = NULL,
	.dump_func = xhub_img_dump,
};

struct dsm_dev dsm_sensorhub = {
	.name = "dsm_sensorhub",
	.device_name = NULL,
	.ic_name = NULL,
	.module_name = NULL,
	.fops = &xhub_ops,
	.buff_size = BUFFER_SIZE,
};
#endif

static int mcu_sys_ready_callback(const pkt_header_t *head)
{
	int ret;

	if (((pkt_sys_statuschange_req_t *) head)->status == ST_MINSYSREADY) {
		hwlog_info("sys ready mini!\n");
		tplcd_manufacture = get_lcd_module();
		hwlog_info("sensor kernel get_lcd_module tplcd_manufacture=%d\n",
			tplcd_manufacture);

		hwlog_info("reboot_reason_flag:%d\n", xhub_reboot_reason_flag);

		ret = init_sensors_cfg_data_from_dts();
		if (ret)
			hwlog_err("get sensors cfg data from dts fail,ret %d\n", ret);
		else
			hwlog_info("get sensors cfg data from dts success!\n");
	} else if (((pkt_sys_statuschange_req_t *)head)->status ==
				ST_MCUREADY) {
		hwlog_info("mcu all ready!\n");
		ret = sensor_set_cfg_data();
		if (ret < 0)
			hwlog_err("sensor_chip_detect ret=%d\n", ret);
		ret = sensor_set_fw_load();
		if (ret < 0)
			hwlog_err("sensor fw dload err ret=%d\n", ret);
		ret = motion_set_cfg_data();
		if (ret < 0)
			hwlog_err("motion set cfg data err ret=%d\n", ret);
		unregister_mcu_event_notifier(TAG_SYS, CMD_SYS_STATUSCHANGE_REQ,
			mcu_sys_ready_callback);
		atomic_set(&xhub_rec_state, XHUB_RECOVERY_IDLE);
	} else {
		hwlog_info("other status\n");
	}
	return 0;
}

static void set_notifier(void)
{
	register_mcu_event_notifier(TAG_SYS, CMD_SYS_STATUSCHANGE_REQ,
		mcu_sys_ready_callback);
	register_mcu_event_notifier(TAG_SYS, CMD_SYS_STATUSCHANGE_REQ,
		xhub_rec_sys_callback);
	set_pm_notifier();
}
#if 0
static void read_reboot_reason_cmdline(void)
{
	char reboot_reason_buf[SENSOR_REBOOT_REASON_MAX_LEN] = { 0 };
	char *pstr = NULL;
	char *dstr = NULL;
	bool checklen = false;

	pstr = strstr(saved_command_line, "reboot_reason=");
	if (!pstr) {
		pr_err("No fastboot reboot_reason info\n");
		return;
	}
	pstr += strlen("reboot_reason=");
	dstr = strstr(pstr, " ");
	if (!dstr) {
		pr_err("No find the reboot_reason end\n");
		return;
	}
	checklen = SENSOR_REBOOT_REASON_MAX_LEN > (unsigned long)(dstr - pstr);
	if (!checklen) {
		pr_err("overrun reboot_reason_buf\n");
		return;
	}
	memcpy(reboot_reason_buf, pstr, (unsigned long)(dstr - pstr));
	reboot_reason_buf[dstr - pstr] = '\0';

	if (!strcasecmp(reboot_reason_buf, "AP_S_COLDBOOT"))
		/* reboot flag */
		xhub_reboot_reason_flag = SENSOR_POWER_NO_RESET;
	else
		/* others */
		xhub_reboot_reason_flag = SENSOR_POWER_DO_RESET;
	hwlog_info("sensorhub get reboot reason:%s length:%d flag:%d\n",
		reboot_reason_buf,
		(int)strlen(reboot_reason_buf),
		xhub_reboot_reason_flag);
}
#endif
static int write_defualt_config_info_to_sharemem(void)
{
	uint64_t start_virt;

	start_virt = scp_get_reserve_mem_virt(XHUB_CFG_MEM_ID);

	if (!g_config_on_ddr)
		g_config_on_ddr = (struct config_on_ddr *)(uintptr_t)start_virt;

	if (memset_s(g_config_on_ddr, sizeof(struct config_on_ddr),
				0, sizeof(struct config_on_ddr)) != EOK)
		hwlog_err("%s, memset_s fail\n", __func__);
	g_config_on_ddr->magic = CFG_ON_DDR_MAGIC_BASE + sizeof(struct config_on_ddr);
	g_config_on_ddr->log_buff_cb_backup.mutex = 0;
	g_config_on_ddr->log_level = INFO_LEVEL;

	return 0;
}

static void xhub_init_before_boot(void)
{
	//read_tp_color_cmdline();
	//read_reboot_reason_cmdline();
	xhub_io_driver_init();

#ifdef CONFIG_HUAWEI_DSM
	shb_dclient = dsm_register_client(&dsm_sensorhub);
#endif
	init_completion(&iom3_reboot);
	xhub_recovery_init();
	sensor_redetect_init();
	xhub_route_init();
	set_notifier();
}

void sync_time_to_xhub(void)
{
	write_info_t winfo;
	pkt_sync_time_req_t pkt;
	int ret;
	unsigned long secs = get_seconds();
	struct rtc_time tm;

	pkt.ap_rtc = secs;
	pkt.ap_timestamp = ktime_get_boot_ns();
	pkt.arch_counter = arch_counter_get_cntvct();
	pkt.timezone = sys_tz.tz_minuteswest * MINS_TO_SECS;

	rtc_time_to_tm(secs, &tm);

	hwlog_info("[%s]ts[%llu]secs[%llu]time[%d-%d-%d %d:%d:%d],timezone[%llu]\n", __func__, pkt.ap_timestamp,
		secs, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, pkt.timezone);
	winfo.tag = TAG_SYS;
	winfo.cmd = CMD_SYS_SYNC_TIME_REQ;
	winfo.wr_len = sizeof(pkt) - sizeof(pkt.hd);
	winfo.wr_buf = &pkt.arch_counter;
	ret = write_customize_cmd(&winfo, NULL, true);
	if (ret) {
		hwlog_err("smartplt:[%s]ret[%d]\n", __func__, ret);
	}
}

static void xhub_init_after_boot(void)
{
	set_sensor_mcu_mode(1);
	//mag_current_notify();
	init_write_nv_work();
	sync_time_to_xhub();
}

int send_status_req_to_mcu(void)
{
	write_info_t pkg_ap = { 0 };
	int ret;

	pkg_ap.tag = TAG_SYS;
	pkg_ap.cmd = CMD_SYS_STATUSREADY_REQ;
	pkg_ap.wr_buf = NULL;
	pkg_ap.wr_len = 0;

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret) {
		hwlog_err("send ready status req to mcu fail,ret=%d\n", ret);
		return -1;
	}
	hwlog_info("send ready status req to mcu success\n");
	return 0;
}

static int wait_xhub_mcu_ready(void)
{
	unsigned int scp_status = is_scp_ready(SCP_A_ID);
	unsigned int try_times = MCU_SYNC_TIMES;

	hwlog_info("mcu status sync start\n");
	while (scp_status == 0) {
		if (try_times > 0) {
			mdelay(1);
			scp_status = is_scp_ready(SCP_A_ID);
			--try_times;
		} else {
			break;
		}
	}
	hwlog_info("sync cost %u\n", MCU_SYNC_TIMES - try_times);
	if (scp_status == 0 && try_times == 0)
		return -1;

	return 0;
}

static int xhub_mcu_init(void)
{
	if (wait_xhub_mcu_ready()) {
		hwlog_err("scp is not ready\n");
		return -1;
	}

	if (write_defualt_config_info_to_sharemem())
		return -1;

	xhub_init_before_boot();
	xhub_mcu_connect();
	register_platform_notify();
	xhub_init_after_boot();

	hwlog_info("----%s--- end\n", __func__);
	return 0;
}

static void __exit xhub_mcu_exit(void)
{
	xhub_route_exit();
	close_nv_workqueue();
}

late_initcall(xhub_mcu_init);
module_exit(xhub_mcu_exit);

MODULE_AUTHOR("xhub <smartphone@huawei.com>");
MODULE_DESCRIPTION("xhub boot");
MODULE_LICENSE("GPL");
