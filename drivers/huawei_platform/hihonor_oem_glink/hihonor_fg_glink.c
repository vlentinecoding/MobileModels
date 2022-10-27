/*
 * hihonor_fg_glink.c
 *
 * hihonor fg glink driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/firmware.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/rpmsg.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_fg_glink.h>
#include <chipset_common/hwpower/coul_interface.h>
#include <chipset_common/hwpower/power_printk.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_cmdline.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_device_id.h>
#include <chipset_common/hwpower/direct_charge/direct_charge_ic_manager.h>

#define HWLOG_TAG fg_glink
HWLOG_REGIST();

#define HIHONOR_CHARGE_PARA_MAX_NUM  8
#define HIHONOR_SC_VTERM_COMP_PARA_MAX_NUM 20
#define HIHONOR_FG_GLINK_BATT_TEMP_K 2730
#define HIHONOR_FG_GLINK_UPDATE_TIME 10000
#define FACTORY_DEFAULT_BATT_TEMP_C 250
#define UVTOMV 1000;

enum hihonor_fg_glink_charge_info {
	HIHONOR_CHARGE_INFO_TEMP_MIN = 0,
	HIHONOR_CHARGE_INFO_TEMP_MAX,
	HIHONOR_CHARGE_INFO_MAX_CUR,
	HIHONOR_CHARGE_INFO_VTERM,
	HIHONOR_CHARGE_INFO_ITERM,
	HIHONOR_CHARGE_INFO_MAX,
};

enum hihonor_glink_sc_vterm_comp_info {
	HIHONOR_SC_ID,
	HIHONOR_SC_VTERM_COMP_UV,
	HIHONOR_SC_VTERM_COMP_INFO_MAX,
};

struct hihonor_glink_sc_vterm_comp_para {
	int sc_id;
	int vterm_comp;
};

struct hihonor_fg_glink_charge_para {
	int temp_min;
	int temp_max;
	u32 max_current;
	u32 vterm;
	u32 iterm;
};

struct hihonor_fg_glink_dev_info {
	struct device *dev;
	struct hihonor_fg_glink_charge_para charge_para[HIHONOR_CHARGE_PARA_MAX_NUM];
	struct hihonor_glink_sc_vterm_comp_para vterm_para[HIHONOR_SC_VTERM_COMP_PARA_MAX_NUM];
	u32 design_fcc;
	int vterm_comp;
	int get_vterm_comp_finish;
	u32 support_multi_ic;
	int sc_vterm_comp_len;
	struct delayed_work update_work;
};

struct hihonor_fg_glink_batt_info {
	u32 vbatt;
	int curr;
	u32 cycle;
	u32 design_fcc;
	u32 learned_fcc;
	u32 rm;
	int batt_temp;
	u32 vbat_max;
	u32 ibat_max;
	u32 iterm;
	u32 soc;
};

static u32 vterm_dec = 0;

static int hihonor_fg_glink_get_charge_para(int temp,
	u32 *vterm, u32 *max_cur, u32 *iterm, struct hihonor_fg_glink_dev_info *info)
{
	int i;

	temp = temp / 10;

	for (i = 0; i < HIHONOR_CHARGE_PARA_MAX_NUM; i ++) {
		if (info->charge_para[i].temp_max >= temp &&
			temp > info->charge_para[i].temp_min) {
			*vterm = info->charge_para[i].vterm;
			*iterm = info->charge_para[i].iterm;
			*max_cur = info->charge_para[i].max_current;
			return 0;
		}
	}

	return -1;
}

static int hihonor_get_vterm_comp(struct hihonor_fg_glink_dev_info *info)
{
	int main_ic_id = 0;
	int aux_ic_id = 0;
	int main_vterm_comp = 0;
	int aux_vterm_comp = 0;
	int vterm_comp = 0;
	int i;
	int count = 0;

	if (!info)
		return 0;

	if (info->sc_vterm_comp_len < 0)
		return 0;

	if (info->get_vterm_comp_finish)
		return info->vterm_comp;

	main_ic_id = dc_get_ic_id(SC_MODE, CHARGE_IC_MAIN);
	aux_ic_id = info->support_multi_ic ? dc_get_ic_id(SC_MODE, CHARGE_IC_AUX) : DC_DEVICE_ID_END;
	if ((main_ic_id == -1) || (aux_ic_id == -1))
		return 0;

	hwlog_info("main_ic_id is:%d, aux_ic_id is:%d\n", main_ic_id, aux_ic_id);
	for (i = 0; i < info->sc_vterm_comp_len / HIHONOR_SC_VTERM_COMP_INFO_MAX; i++) {
		if (main_ic_id == info->vterm_para[i].sc_id) {
			main_vterm_comp = info->vterm_para[i].vterm_comp;
			hwlog_info("main_vterm_comp is:%d\n", main_vterm_comp);
			count++;
		}
		if (aux_ic_id == info->vterm_para[i].sc_id)  {
			aux_vterm_comp = info->vterm_para[i].vterm_comp;
			hwlog_info("aux_vterm_comp is:%d\n", aux_vterm_comp);
			count++;
		}
		if (info->support_multi_ic && count == CHARGE_IC_TYPE_MAX)
			break;
		if (!info->support_multi_ic && count == (CHARGE_IC_TYPE_MAIN + 1))
			break;
	}
	vterm_comp = (main_vterm_comp + aux_vterm_comp) / UVTOMV;
	/* round (vterm_comp) */
	info->vterm_comp = (vterm_comp + 5) / 10 * 10;
	hwlog_info("final vterm_comp is %d\n", info->vterm_comp);
	info->get_vterm_comp_finish = 1;
	return info->vterm_comp;
}

static void hihonor_fg_glink_update_work(struct work_struct *work)
{
	struct hihonor_fg_glink_batt_info batt_info = {0};
	struct hihonor_fg_glink_dev_info *info = container_of(work, struct hihonor_fg_glink_dev_info, update_work.work);
	int ret;

	batt_info.batt_temp = coul_interface_get_battery_temperature(COUL_TYPE_MAIN);

#ifndef CONFIG_HLTHERM_RUNTEST
	if (power_cmdline_is_factory_mode() && batt_info.batt_temp < 0)
		batt_info.batt_temp = FACTORY_DEFAULT_BATT_TEMP_C;
#endif /* #ifndef CONFIG_HLTHERM_RUNTEST */

	ret = hihonor_fg_glink_get_charge_para(batt_info.batt_temp, &(batt_info.vbat_max),
		&(batt_info.ibat_max), &(batt_info.iterm), info);
	if (ret) {
		hwlog_err("get charge para err\n");
		goto next;
	}
	if (vterm_dec > 0 && batt_info.vbat_max > vterm_dec) {
		batt_info.vbat_max = batt_info.vbat_max - vterm_dec;
	}
	batt_info.vbat_max -= hihonor_get_vterm_comp(info);
	batt_info.batt_temp += HIHONOR_FG_GLINK_BATT_TEMP_K;
	batt_info.vbatt = coul_interface_get_battery_voltage(COUL_TYPE_MAIN);
	batt_info.curr = -coul_interface_get_battery_current(COUL_TYPE_MAIN);
	batt_info.cycle = coul_interface_get_battery_cycle(COUL_TYPE_MAIN);
	batt_info.learned_fcc = coul_interface_get_battery_fcc(COUL_TYPE_MAIN);
	batt_info.rm = coul_interface_get_battery_rm(COUL_TYPE_MAIN);
	batt_info.design_fcc = info->design_fcc;
	batt_info.soc = coul_interface_get_battery_capacity(COUL_TYPE_MAIN);

	ret = hihonor_oem_glink_oem_set_prop(BATTERY_OEM_BATCH_INFO, &batt_info, sizeof(batt_info));
	if (ret)
		hwlog_err("send charge para to adsp fail\n");

next:
	schedule_delayed_work(&info->update_work, msecs_to_jiffies(HIHONOR_FG_GLINK_UPDATE_TIME));

	return;
}

static void hihonor_fg_glink_notification(void *dev_data, u32 notification, void *data)
{
	struct hihonor_fg_glink_dev_info *info = (struct hihonor_fg_glink_dev_info *)dev_data;

	if (!info || notification != OEM_NOTIFY_SYNC_BATTINFO)
		return;

	mod_delayed_work(system_wq, &info->update_work, msecs_to_jiffies(100)); /* wait for i2c ready */
}

static struct hihonor_glink_ops fg_glink_ops = {
	.notify_event = hihonor_fg_glink_notification,
};

static int hihonor_fg_glink_parse_dts(struct device_node *np, struct hihonor_fg_glink_dev_info *info)
{
	int row, col, len;
	int idata[HIHONOR_CHARGE_PARA_MAX_NUM * HIHONOR_CHARGE_INFO_MAX] = { 0 };

	hwlog_info("hihonor_fg_glink_parse_dts in\n");

	if (!np)
		return -1;

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "design_fcc", &info->design_fcc, 0))
		return -1;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"charge_para", idata, HIHONOR_CHARGE_PARA_MAX_NUM, HIHONOR_CHARGE_INFO_MAX);
	if (len < 0)
		return -1;

	for (row = 0; row < len / HIHONOR_CHARGE_INFO_MAX; row++) {
		col = row * HIHONOR_CHARGE_INFO_MAX + HIHONOR_CHARGE_INFO_TEMP_MIN;
		info->charge_para[row].temp_min = idata[col];
		col = row * HIHONOR_CHARGE_INFO_MAX + HIHONOR_CHARGE_INFO_TEMP_MAX;
		info->charge_para[row].temp_max = idata[col];
		col = row * HIHONOR_CHARGE_INFO_MAX + HIHONOR_CHARGE_INFO_MAX_CUR;
		info->charge_para[row].max_current = idata[col];
		col = row * HIHONOR_CHARGE_INFO_MAX + HIHONOR_CHARGE_INFO_VTERM;
		info->charge_para[row].vterm = idata[col];
		col = row * HIHONOR_CHARGE_INFO_MAX + HIHONOR_CHARGE_INFO_ITERM;
		info->charge_para[row].iterm = idata[col];
	}

	info->sc_vterm_comp_len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"sc_vterm_comp", idata, HIHONOR_SC_VTERM_COMP_PARA_MAX_NUM, HIHONOR_SC_VTERM_COMP_INFO_MAX);
	if (info->sc_vterm_comp_len < 0)
		hwlog_info("get sc_vterm_comp para fail\n");

	for (row = 0; row < info->sc_vterm_comp_len / HIHONOR_SC_VTERM_COMP_INFO_MAX; row++) {
		col = row * HIHONOR_SC_VTERM_COMP_INFO_MAX + HIHONOR_SC_ID;
		info->vterm_para[row].sc_id = idata[col];
		col = row *  HIHONOR_SC_VTERM_COMP_INFO_MAX + HIHONOR_SC_VTERM_COMP_UV;
		info->vterm_para[row].vterm_comp = idata[col];
		hwlog_info("sc_vterm_comp[%d]:%d %d\n", row, info->vterm_para[row].sc_id, info->vterm_para[row].vterm_comp);
	}

	if (power_dts_read_u32(power_dts_tag(HWLOG_TAG), np, "support_multi_ic", &info->support_multi_ic, 1))
		hwlog_info("get support_multi_ic para fail\n");

	return 0;
}

void hihonor_glink_set_vterm_dec(unsigned int val)
{
	vterm_dec = val;
	hwlog_info("hihonor_glink_set_vterm_dec vterm_dec=%d\n", vterm_dec);
}

static int hihonor_fg_glink_probe(struct platform_device *pdev)
{
	struct hihonor_fg_glink_dev_info *info = NULL;
	struct device *dev = &pdev->dev;
	int ret;

	hwlog_info("hihonor_fg_glink_probe in\n");
	info = devm_kzalloc(&pdev->dev, sizeof(*info), GFP_KERNEL);
	if (!info)
		return -ENOMEM;

	fg_glink_ops.dev_data = info;
	(void)hihonor_oem_glink_ops_register(&fg_glink_ops);

	info->dev = dev;
	platform_set_drvdata(pdev, info);

	ret = hihonor_fg_glink_parse_dts(dev->of_node, info);
	if (ret)
		goto ERROR_PARSE;

	INIT_DELAYED_WORK(&info->update_work, hihonor_fg_glink_update_work);
	schedule_delayed_work(&info->update_work, msecs_to_jiffies(HIHONOR_FG_GLINK_UPDATE_TIME));
	hwlog_info("hihonor_fg_glink_probe out\n");
	return 0;

ERROR_PARSE:
	hwlog_info("hihonor_fg_glink_probe err\n");
	platform_set_drvdata(pdev, NULL);
	devm_kfree(&pdev->dev, info);
	return -1;
}

static int hihonor_fg_glink_remove(struct platform_device *pdev)
{
	struct hihonor_fg_glink_dev_info *info = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&info->update_work);
	return 0;
}

#ifdef CONFIG_PM
static int hihonor_fg_glink_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct hihonor_fg_glink_dev_info *info = platform_get_drvdata(pdev);

	cancel_delayed_work_sync(&info->update_work);
	return 0;
}

static int hihonor_fg_glink_resume(struct platform_device *pdev)
{
	struct hihonor_fg_glink_dev_info *info = platform_get_drvdata(pdev);

	schedule_delayed_work(&info->update_work, msecs_to_jiffies(HIHONOR_FG_GLINK_UPDATE_TIME));

	return 0;
}
#endif /* CONFIG_PM */

static const struct of_device_id hihonor_fg_glink_match_table[] = {
	{ .compatible = "hihonor-fg-glink" },
	{},
};

static struct platform_driver hihonor_fg_glink_driver = {
	.driver = {
		.name = "hihonor-fg-glink",
		.of_match_table = hihonor_fg_glink_match_table,
	},
	.probe = hihonor_fg_glink_probe,
	.remove = hihonor_fg_glink_remove,
#ifdef CONFIG_PM
	.suspend = hihonor_fg_glink_suspend,
	.resume = hihonor_fg_glink_resume,
#endif

};

module_platform_driver(hihonor_fg_glink_driver);

MODULE_DESCRIPTION("hihonor fg Glink driver");
MODULE_LICENSE("GPL v2");
