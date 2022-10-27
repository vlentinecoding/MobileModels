/*
 * adsp_fuelgauge_dts.c
 *
 * adsp fuelgauge dts parse driver
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

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>
#include <log/hw_log.h>
#include <chipset_common/hwpower/power_debug.h>
#include <chipset_common/hwpower/power_sysfs.h>
#include <chipset_common/hwpower/coul_interface.h>
#include <chipset_common/hwpower/coul_calibration.h>
#include <huawei_platform/power/adsp/adsp_fuelgauge.h>
#include <huawei_platform/hihonor_oem_glink/hihonor_oem_glink.h>
#include <chipset_common/hwpower/battery_model_public.h>
#include <huawei_platform/power/adsp/fuelgauge_ic/rt9426a.h>
#include <huawei_platform/power/adsp/fuelgauge_ic/bq27z561.h>
#include <huawei_platform/power/adsp/fuelgauge_ic/cw2217.h>
#include <chipset_common/hwpower/power_dts.h>

#define HWLOG_TAG adsp_fuelgauge
HWLOG_REGIST();

static struct adsp_fg_device *g_di = NULL;
extern struct coul_cali_ops rt9426a_cali_ops;


static int fuelgauge_glink_set_para(struct adsp_fg_device *di)
{
	int ret;

	if (!di)
		return -EINVAL;

	ret = hihonor_oem_glink_oem_update_config(FG_CONFIG_ID, di->fuel_para, di->para_size);
	if (ret) {
		hwlog_err("fail to set fg config\n");
		return -1;
	}
	return 0;
}

static struct fuelgauge_info fuel_info_tbl[FUELGAUGE_TYPE_END] = {
	{
		.fuelgauge_type = FUELGAUGE_TYPE_RT9426A,
		.fuelgauge_name = "rt9426a",
		.parse_para = rt9426a_parse_para,
		.parse_aging_para = rt9426a_parse_aging_para,
		.cali_ops = &rt9426a_cali_ops,
	},
	{
		.fuelgauge_type = FUELGAUGE_TYPE_BQ27Z561,
		.fuelgauge_name = "bq27z561",
		.parse_para = bq27z561_parse_para,
		.parse_aging_para = NULL,
		.cali_ops = NULL,
	},
	{
		.fuelgauge_type = FUELGAUGE_TYPE_CW2217,
		.fuelgauge_name = "cw2217",
		.parse_para = cw2217_parse_para,
		.parse_aging_para = NULL,
		.cali_ops = NULL,
	}
};

static void adsp_fg_sync_notify_callback(void *dev_data)
{
	struct adsp_fg_device *di = (struct adsp_fg_device *)dev_data;

	if (!di)
		return;

	schedule_delayed_work(&di->sync_work, 0);
}

static void adsp_fg_event_notify_callback(void *dev_data, u32 notification, void *data)
{
	struct adsp_fg_device *di = (struct adsp_fg_device *)dev_data;

	if (!di)
		return;

	switch (notification)
	{
	case OEM_NOTIFY_FG_TYPE_RT9426A:
		di->fuelgauge_type = FUELGAUGE_TYPE_RT9426A;
		break;
	case OEM_NOTIFY_FG_TYPE_BQ27Z561:
		di->fuelgauge_type = FUELGAUGE_TYPE_BQ27Z561;
		break;
	case OEM_NOTIFY_FG_TYPE_CW2217:
		di->fuelgauge_type = FUELGAUGE_TYPE_CW2217;
		break;
	default:
		return;
	}

	schedule_delayed_work(&di->event_work, 0);
}

static void adsp_fg_event_work(struct work_struct *work)
{
	int i;
	int ret;
	struct adsp_fg_device *di = container_of(work, struct adsp_fg_device, event_work.work);
	struct device_node *child_node = NULL;
	struct device_node *np = NULL;
	struct fuelgauge_info *info = NULL;
	const char *status = NULL;

	if (!di)
		return;

	for (i = FUELGAUGE_TYPE_BEGIN; i < FUELGAUGE_TYPE_END; i++) {
		if (di->fuelgauge_type != fuel_info_tbl[i].fuelgauge_type)
			continue;
		info = &fuel_info_tbl[i];
		break;
	}

	if (!info)
		return;

	np = di->dev->of_node;
	for_each_child_of_node(np, child_node) {
		if (power_dts_read_string(power_dts_tag(HWLOG_TAG), 
			child_node, "status", &status)) {
			hwlog_err("childnode without status property\n");
			continue;
		}
		if (!status)
			continue;

		if (strcmp(child_node->name, info->fuelgauge_name))
			continue;

		break;
	}

	hwlog_info("find fuelgauge name: %s\n", info->fuelgauge_name);
	if (info->parse_para)
		info->parse_para(child_node, 
			di->batt_model_name, &di->fuel_para, &di->para_size);

	if (info->parse_aging_para)
		info->parse_aging_para(child_node, 
			di->batt_model_name, &di->fuel_para, &di->para_size);

	if (info->cali_ops)
		coul_cali_ops_register(info->cali_ops);

	ret = fuelgauge_glink_set_para(di);
	if (ret)
		goto next_event_work;
	return;

next_event_work:
	schedule_delayed_work(&di->event_work, msecs_to_jiffies(ADSP_BATTERY_WORK_INTERVAL));
}

static void adsp_fg_glink_sync_work(struct work_struct *work)
{
	int ret;
	struct adsp_fg_device *di = container_of(work, struct adsp_fg_device, sync_work.work);

	if (!di)
		return;

	hwlog_info("sync fg instance id to adsp\n");
	ret = hihonor_oem_glink_oem_set_prop(BATTERY_OEM_FG_INS_INDEX, 
		&di->ins_index, sizeof(di->ins_index));
	if (ret) {
		hwlog_err("sync fg instance id to adsp fail\n");
		goto next_sync_work;
	}
	return;

next_sync_work:
	schedule_delayed_work(&di->sync_work, msecs_to_jiffies(ADSP_BATTERY_WORK_INTERVAL));
}

static struct hihonor_glink_ops adsp_fg_glink_ops = {
	.sync_data = adsp_fg_sync_notify_callback,
	.notify_event = adsp_fg_event_notify_callback,
};

static ssize_t adsp_fg_send_conf_store(void *dev_data, const char *buf, size_t size)
{
	struct adsp_fg_device *di = (struct adsp_fg_device *)dev_data;

	if (!di)
		return -1;

	schedule_delayed_work(&di->sync_work, 0);
	return size;
}

static int adsp_fg_parse_dts(struct device_node *np,
	struct adsp_fg_device *di)
{
	int i, row, col, len, ret;
	const char *batt_model_name = NULL;
	int idata[MAX_SE_INSTANCE_NUM * SE_INS_MAP_TOTAL] = { 0 };

	hwlog_info("%s\n", __func__);
	batt_model_name = bat_model_name();
	strlcpy(di->batt_model_name, batt_model_name, sizeof(di->batt_model_name));
	ret = power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np, "qup_index", &di->qup_index, -1);
	if (ret)
		return -EINVAL;

	ret = power_dts_read_str2int(power_dts_tag(HWLOG_TAG), np, "se_index", &di->se_index, -1);
	if (ret)
		return -EINVAL;

	len = power_dts_read_string_array(power_dts_tag(HWLOG_TAG), np,
		"se_ins_map", idata, MAX_SE_INSTANCE_NUM, SE_INS_MAP_TOTAL);
	if (len < 0)
		return -EINVAL;

	for (row = 0; row < len / SE_INS_MAP_TOTAL; row++) {
		col = row * SE_INS_MAP_TOTAL + SE_INS_MAP_QUP_INDEX;
		di->se_ins_map[row].qup_index = idata[col];
		col = row * SE_INS_MAP_TOTAL + SE_INS_MAP_SE_INDEX;
		di->se_ins_map[row].se_index = idata[col];
		col = row * SE_INS_MAP_TOTAL + SE_INS_MAP_INS_INDEX;
		di->se_ins_map[row].ins_index = idata[col];
	}

	di->ins_index = -1;
	for (i = 0; i < MAX_SE_INSTANCE_NUM; i++) {
		if ((di->se_ins_map[i].qup_index != di->qup_index) ||
			(di->se_ins_map[i].se_index != di->se_index))
			continue;
		di->ins_index = di->se_ins_map[i].ins_index;
		break;
	}

	if (di->ins_index < 0) {
		hwlog_err("%s, failed to find instance index\n", __func__);
		return -EINVAL;
	}

	hwlog_info("%s, find fg instance index: %d\n", __func__, di->ins_index);
	return 0;
}

static int adsp_fg_probe(struct platform_device *pdev)
{
	int ret;
	struct adsp_fg_device *di = NULL;
	struct device_node *np = NULL;

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;
	hwlog_info("%s %d\n", __func__, __LINE__);
	di = devm_kzalloc(&pdev->dev, sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	di->dev = &pdev->dev;
	np = di->dev->of_node;
	g_di = di;

	ret = adsp_fg_parse_dts(np, di);
	if (ret)
		goto fail_free_mem;

	adsp_fg_glink_ops.dev_data = di;
	ret = hihonor_oem_glink_ops_register(&adsp_fg_glink_ops);
	if (ret) {
		hwlog_err("%s fail to register glink ops\n", __func__);
		goto fail_free_mem;
	}

	INIT_DELAYED_WORK(&di->sync_work, adsp_fg_glink_sync_work);
	INIT_DELAYED_WORK(&di->event_work, adsp_fg_event_work);

	power_dbg_ops_register("adsp_fg_send_conf", di,
		NULL,
		(power_dbg_store)adsp_fg_send_conf_store);
	return 0;
	
fail_free_mem:
	devm_kfree(&pdev->dev, di);
	di = NULL;
	g_di = NULL;
	return ret;
}

static int adsp_fg_remove(struct platform_device *pdev)
{
	struct adsp_fg_device *di = platform_get_drvdata(pdev);

	if (!di)
		return -ENODEV;

	devm_kfree(&pdev->dev, di);
	return 0;
}

static int adsp_fg_suspend(struct platform_device *pdev, pm_message_t state)
{
	return 0;
}

static int adsp_fg_resume(struct platform_device *pdev)
{
	return 0;
}

static const struct of_device_id adsp_fg_match_table[] = {
	{
		.compatible = "honor,adsp_fuelgauge",
		.data = NULL,
	},
	{},
};

static struct platform_driver adsp_fg_driver = {
	.probe = adsp_fg_probe,
	.remove = adsp_fg_remove,
#ifdef CONFIG_PM
	.suspend = adsp_fg_suspend,
	.resume = adsp_fg_resume,
#endif
	.driver = {
		.name = "honor,adsp_fuelgauge",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(adsp_fg_match_table),
	},
};

static int __init adsp_fg_init(void)
{
	return platform_driver_register(&adsp_fg_driver);
}

static void __exit adsp_fg_exit(void)
{
	platform_driver_unregister(&adsp_fg_driver);
}

device_initcall_sync(adsp_fg_init);
module_exit(adsp_fg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("adsp fuelgauge module driver");
MODULE_AUTHOR("Honor Technologies Co., Ltd.");

