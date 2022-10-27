/*
 * honor_pmic_ocp_debug.c
 *
 * honor pmic debug driver
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
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/workqueue.h>
#include <linux/jiffies.h>
#include <linux/printk.h>
#include <chipset_common/hwpower/power_dts.h>
#include <chipset_common/hwpower/power_dsm.h>
#include <chipset_common/hwpower/power_printk.h>

#define HWLOG_TAG pmic_ocp_debug
HWLOG_REGIST();

#define PMIC_OCP_REPORT_TIME       50000
#define BUF_LEN_MAX                128
#define SPMI_SLAVE_DEV_MAX         5
#define SPMI_SLAVE_DEV_NAME_MAX    32
#define POWER_BASE_DEC             10
#define PMIC_OCP_TYPE_NAME_MAX     3

enum pmic_ocp_info {
	PMIC_INDEX,
	PMIC_TYPE,
	PMIC_NUMBER,
	PMIC_INFO_END,
};

enum spmi_slave_para {
	SPMI_SLAVE_ID,
	SPMI_SLAVE_NAME,
	SPMI_SLAVE_INFO_TOTAL,
};

struct spmi_slave_dev_info {
	int sid;
	char name[SPMI_SLAVE_DEV_NAME_MAX];
};

struct pmic_ocp_debug_dev       {
	struct device *dev;
	struct delayed_work pmic_ocp_report_work;
	struct spmi_slave_dev_info spmi_slave_info[SPMI_SLAVE_DEV_MAX];
	int stage;
};

struct pmic_ocp_debug_t {
	char name[SPMI_SLAVE_DEV_NAME_MAX];
	int dmd;
};

struct pmic_ocp_debug_t pmic_ocp_debug[] = {
	{"PM7325_VREG_S1"   ,   DSM_PM7325_VREG_S1B_OVER_LOAD  },
	{"PM7325_VREG_S2"   ,   DSM_PM7325_VREG_S2B_OVER_LOAD  },
	{"PM7325_VREG_S3"   ,   DSM_PM7325_VREG_S3B_OVER_LOAD  },
	{"PM7325_VREG_S4"   ,   DSM_PM7325_VREG_S4B_OVER_LOAD  },
	{"PM7325_VREG_S5"   ,   DSM_PM7325_VREG_S5B_OVER_LOAD  },
	{"PM7325_VREG_S6"   ,   DSM_PM7325_VREG_S6B_OVER_LOAD  },
	{"PM7325_VREG_S7"   ,   DSM_PM7325_VREG_S7B_OVER_LOAD  },
	{"PM7325_VREG_S8"   ,   DSM_PM7325_VREG_S8B_OVER_LOAD  },
	{"PM7325_VREG_L1"   ,   DSM_PM7325_VREG_L1B_OVER_LOAD  },
	{"PM7325_VREG_L2"   ,   DSM_PM7325_VREG_L2B_OVER_LOAD  },
	{"PM7325_VREG_L3"   ,   DSM_PM7325_VREG_L3B_OVER_LOAD  },
	{"PM7325_VREG_L4"   ,   DSM_PM7325_VREG_L4B_OVER_LOAD  },
	{"PM7325_VREG_L5"   ,   DSM_PM7325_VREG_L5B_OVER_LOAD  },
	{"PM7325_VREG_L6"   ,   DSM_PM7325_VREG_L6B_OVER_LOAD  },
	{"PM7325_VREG_L7"   ,   DSM_PM7325_VREG_L7B_OVER_LOAD  },
	{"PM7325_VREG_L8"   ,   DSM_PM7325_VREG_L8B_OVER_LOAD  },
	{"PM7325_VREG_L9"   ,   DSM_PM7325_VREG_L9B_OVER_LOAD  },
	{"PM7325_VREG_L10"  ,   DSM_PM7325_VREG_L10B_OVER_LOAD },
	{"PM7325_VREG_L11"  ,   DSM_PM7325_VREG_L11B_OVER_LOAD },
	{"PM7325_VREG_L12"  ,   DSM_PM7325_VREG_L12B_OVER_LOAD },
	{"PM7325_VREG_L13"  ,   DSM_PM7325_VREG_L13B_OVER_LOAD },
	{"PM7325_VREG_L14"  ,   DSM_PM7325_VREG_L14B_OVER_LOAD },
	{"PM7325_VREG_L15"  ,   DSM_PM7325_VREG_L15B_OVER_LOAD },
	{"PM7325_VREG_L16"  ,   DSM_PM7325_VREG_L16B_OVER_LOAD },
	{"PM7325_VREG_L17"  ,   DSM_PM7325_VREG_L17B_OVER_LOAD },
	{"PM7325_VREG_L18"  ,   DSM_PM7325_VREG_L18B_OVER_LOAD },
	{"PM7325_VREG_L19"  ,   DSM_PM7325_VREG_L19B_OVER_LOAD },
	{"PM7350C_VREG_S1"  ,   DSM_PM7350C_VREG_S1C_OVER_LOAD },
	{"PM7350C_VREG_S2"  ,   DSM_PM7350C_VREG_S2C_OVER_LOAD },
	{"PM7350C_VREG_S3"  ,   DSM_PM7350C_VREG_S3C_OVER_LOAD },
	{"PM7350C_VREG_S4"  ,   DSM_PM7350C_VREG_S4C_OVER_LOAD },
	{"PM7350C_VREG_S5"  ,   DSM_PM7350C_VREG_S5C_OVER_LOAD },
	{"PM7350C_VREG_S6"  ,   DSM_PM7350C_VREG_S6C_OVER_LOAD },
	{"PM7350C_VREG_S7"  ,   DSM_PM7350C_VREG_S7C_OVER_LOAD },
	{"PM7350C_VREG_S8"  ,   DSM_PM7350C_VREG_S8C_OVER_LOAD },
	{"PM7350C_VREG_S9"  ,   DSM_PM7350C_VREG_S9C_OVER_LOAD },
	{"PM7350C_VREG_S10" ,   DSM_PM7350C_VREG_S10C_OVER_LOAD},
	{"PM7350C_VREG_L1"  ,   DSM_PM7350C_VREG_L1C_OVER_LOAD },
	{"PM7350C_VREG_L2"  ,   DSM_PM7350C_VREG_L2C_OVER_LOAD },
	{"PM7350C_VREG_L3"  ,   DSM_PM7350C_VREG_L3C_OVER_LOAD },
	{"PM7350C_VREG_L4"  ,   DSM_PM7350C_VREG_L4C_OVER_LOAD },
	{"PM7350C_VREG_L5"  ,   DSM_PM7350C_VREG_L5C_OVER_LOAD },
	{"PM7350C_VREG_L6"  ,   DSM_PM7350C_VREG_L6C_OVER_LOAD },
	{"PM7350C_VREG_L7"  ,   DSM_PM7350C_VREG_L7C_OVER_LOAD },
	{"PM7350C_VREG_L8"  ,   DSM_PM7350C_VREG_L8C_OVER_LOAD },
	{"PM7350C_VREG_L9"  ,   DSM_PM7350C_VREG_L9C_OVER_LOAD },
	{"PM7350C_VREG_L10" ,   DSM_PM7350C_VREG_L10C_OVER_LOAD},
	{"PM7350C_VREG_L11" ,   DSM_PM7350C_VREG_L11C_OVER_LOAD},
	{"PM7350C_VREG_L12" ,   DSM_PM7350C_VREG_L12C_OVER_LOAD},
	{"PM7350C_VREG_L13" ,   DSM_PM7350C_VREG_L13C_OVER_LOAD},
	{"PM8350_VREG_S1"   ,   DSM_PM8350_VREG_S1B_OVER_LOAD  },
	{"PM8350_VREG_S2"   ,   DSM_PM8350_VREG_S2B_OVER_LOAD  },
	{"PM8350_VREG_S3"   ,   DSM_PM8350_VREG_S3B_OVER_LOAD  },
	{"PM8350_VREG_S4"   ,   DSM_PM8350_VREG_S4B_OVER_LOAD  },
	{"PM8350_VREG_S5"   ,   DSM_PM8350_VREG_S5B_OVER_LOAD  },
	{"PM8350_VREG_S6"   ,   DSM_PM8350_VREG_S6B_OVER_LOAD  },
	{"PM8350_VREG_S7"   ,   DSM_PM8350_VREG_S7B_OVER_LOAD  },
	{"PM8350_VREG_S8"   ,   DSM_PM8350_VREG_S8B_OVER_LOAD  },
	{"PM8350_VREG_S9"   ,   DSM_PM8350_VREG_S9B_OVER_LOAD  },
	{"PM8350_VREG_S10"  ,   DSM_PM8350_VREG_S10B_OVER_LOAD },
	{"PM8350_VREG_L1"   ,   DSM_PM8350_VREG_L1B_OVER_LOAD  },
	{"PM8350_VREG_L2"   ,   DSM_PM8350_VREG_L2B_OVER_LOAD  },
	{"PM8350_VREG_L3"   ,   DSM_PM8350_VREG_L3B_OVER_LOAD  },
	{"PM8350_VREG_L4"   ,   DSM_PM8350_VREG_L4B_OVER_LOAD  },
	{"PM8350_VREG_L5"   ,   DSM_PM8350_VREG_L5B_OVER_LOAD  },
	{"PM8350_VREG_L6"   ,   DSM_PM8350_VREG_L6B_OVER_LOAD  },
	{"PM8350_VREG_L7"   ,   DSM_PM8350_VREG_L7B_OVER_LOAD  },
	{"PM8350_VREG_L8"   ,   DSM_PM8350_VREG_L8B_OVER_LOAD  },
	{"PM8350_VREG_L9"   ,   DSM_PM8350_VREG_L9B_OVER_LOAD  },
	{"PM8350_VREG_L10"  ,   DSM_PM8350_VREG_L10B_OVER_LOAD },
	{"PM8350C_VREG_S1"  ,   DSM_PM8350C_VREG_S1C_OVER_LOAD },
	{"PM8350C_VREG_S2"  ,   DSM_PM8350C_VREG_S2C_OVER_LOAD },
	{"PM8350C_VREG_S3"  ,   DSM_PM8350C_VREG_S3C_OVER_LOAD },
	{"PM8350C_VREG_S4"  ,   DSM_PM8350C_VREG_S4C_OVER_LOAD },
	{"PM8350C_VREG_S5"  ,   DSM_PM8350C_VREG_S5C_OVER_LOAD },
	{"PM8350C_VREG_S6"  ,   DSM_PM8350C_VREG_S6C_OVER_LOAD },
	{"PM8350C_VREG_S7"  ,   DSM_PM8350C_VREG_S7C_OVER_LOAD },
	{"PM8350C_VREG_S8"  ,   DSM_PM8350C_VREG_S8C_OVER_LOAD },
	{"PM8350C_VREG_S9"  ,   DSM_PM8350C_VREG_S9C_OVER_LOAD },
	{"PM8350C_VREG_S10" ,   DSM_PM8350C_VREG_S10C_OVER_LOAD},
	{"PM8350C_VREG_L1"  ,   DSM_PM8350C_VREG_L1C_OVER_LOAD },
	{"PM8350C_VREG_L2"  ,   DSM_PM8350C_VREG_L2C_OVER_LOAD },
	{"PM8350C_VREG_L3"  ,   DSM_PM8350C_VREG_L3C_OVER_LOAD },
	{"PM8350C_VREG_L4"  ,   DSM_PM8350C_VREG_L4C_OVER_LOAD },
	{"PM8350C_VREG_L5"  ,   DSM_PM8350C_VREG_L5C_OVER_LOAD },
	{"PM8350C_VREG_L6"  ,   DSM_PM8350C_VREG_L6C_OVER_LOAD },
	{"PM8350C_VREG_L7"  ,   DSM_PM8350C_VREG_L7C_OVER_LOAD },
	{"PM8350C_VREG_L8"  ,   DSM_PM8350C_VREG_L8C_OVER_LOAD },
	{"PM8350C_VREG_L9"  ,   DSM_PM8350C_VREG_L9C_OVER_LOAD },
	{"PM8350C_VREG_L10" ,   DSM_PM8350C_VREG_L10C_OVER_LOAD},
	{"PM8350C_VREG_L11" ,   DSM_PM8350C_VREG_L11C_OVER_LOAD},
	{"PM8350C_VREG_L12" ,   DSM_PM8350C_VREG_L12C_OVER_LOAD},
	{"PM8350C_VREG_L13" ,   DSM_PM8350C_VREG_L13C_OVER_LOAD},
};

static int g_pmic_ocp_info[PMIC_INFO_END];
static int g_pmic_ocp_status;

static void pmic_ocp_report_dmd_work(struct work_struct *work)
{
	char buf[BUF_LEN_MAX] = {0};
	char name_buf[SPMI_SLAVE_DEV_NAME_MAX] = { 0 };
	char ocp_type_name[PMIC_OCP_TYPE_NAME_MAX] = {'S', 'L', 'B'};// SMPS, LDO, BOB
	int dmd = 0;
	int i;
	int size = sizeof(pmic_ocp_debug) / sizeof(struct pmic_ocp_debug_t);
	struct pmic_ocp_debug_dev *di = container_of(work, struct pmic_ocp_debug_dev, pmic_ocp_report_work.work);

	hwlog_info("pmic_ocp_report_dmd_work start\n");
	if (!g_pmic_ocp_status || !di) {
		hwlog_err("No Pmic Ocp Info, cancel report dmd work\n");
		return;
	}

	for(i = 0; i < di->stage; i++) {
		if (di->spmi_slave_info[i].sid == g_pmic_ocp_info[PMIC_INDEX]) {
			snprintf(name_buf, SPMI_SLAVE_DEV_NAME_MAX - 1, "%s_VREG_%c%d",
				di->spmi_slave_info[i].name,
				ocp_type_name[g_pmic_ocp_info[PMIC_TYPE]],
				g_pmic_ocp_info[PMIC_NUMBER]);
			break;
		}
	}
	hwlog_info("%s\n", name_buf);
	if (i == di->stage) {
		hwlog_err("dts and xbl_ocp_info[%d_%d_%d] not match, please check\n",
			g_pmic_ocp_info[PMIC_INDEX],
			g_pmic_ocp_info[PMIC_TYPE],
			g_pmic_ocp_info[PMIC_NUMBER]);
		return;
	}

	for (i = 0; i < size; i++) {
		if (!strcasecmp(pmic_ocp_debug[i].name, name_buf)) {
			dmd = pmic_ocp_debug[i].dmd;
			break;
		}
	}

	if (i == size) {
		hwlog_err("no match dmd_list info\n");
		return;
	}

	snprintf(buf, BUF_LEN_MAX - 1, "%s over-load happend\n", name_buf);
	power_dsm_dmd_report(POWER_DSM_PMU_OCP, dmd, buf);
	return;
}

static int __init pmic_ocp_info_parse_cmd(char *p)
{
	int ret;

	if (!p)
		return 0;

	ret = sscanf(p, "%d,%d,%d", &g_pmic_ocp_info[PMIC_INDEX],
		&g_pmic_ocp_info[PMIC_TYPE], &g_pmic_ocp_info[PMIC_NUMBER]);
	hwlog_info("%s ret=%d OCP=%d,%d,%d\n", __func__, ret,
		g_pmic_ocp_info[PMIC_INDEX],
		g_pmic_ocp_info[PMIC_TYPE],
		g_pmic_ocp_info[PMIC_NUMBER]);
	g_pmic_ocp_status = 1;
	return ret;
}
early_param("OCP", pmic_ocp_info_parse_cmd);

static void pmic_debug_parse_dts(struct device_node *np, struct pmic_ocp_debug_dev *di)
{
	int len;
	int col;
	int row;
	int idata;
	int i;
	const char *tmp_string = NULL;

	len = power_dts_read_count_strings(power_dts_tag(HWLOG_TAG), np,
		"spmi_slave_info", SPMI_SLAVE_DEV_MAX, SPMI_SLAVE_INFO_TOTAL);
	if (len < 0)
		return;

	for (i = 0; i < len; i++) {
		if (power_dts_read_string_index(power_dts_tag(HWLOG_TAG),
 			np, "spmi_slave_info", i, &tmp_string))
 			return;
		row = i / SPMI_SLAVE_INFO_TOTAL;
		col = i % SPMI_SLAVE_INFO_TOTAL;
		switch (col) {
		case SPMI_SLAVE_ID:
			if (!kstrtoint(tmp_string, POWER_BASE_DEC, &idata))
				di->spmi_slave_info[row].sid = idata;
			break;
		case SPMI_SLAVE_NAME:
			strncpy(di->spmi_slave_info[row].name, tmp_string,
				SPMI_SLAVE_DEV_NAME_MAX - 1);
			break;
		default:
				break;
		}
	}
	di->stage = len / SPMI_SLAVE_INFO_TOTAL;
	for (i = 0; i < di->stage; i++)
		hwlog_info("spmi_slave_info[%d] = %d %s\n", i,
			di->spmi_slave_info[i].sid,
			di->spmi_slave_info[i].name);
	return;
}

static int pmic_ocp_debug_probe(struct platform_device *pdev)
{
	struct pmic_ocp_debug_dev *l_dev = NULL;
	struct device_node *np = NULL;

	hwlog_info("pmic_ocp_debug_probe in");
	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	l_dev = kzalloc(sizeof(*l_dev), GFP_KERNEL);
	if (!l_dev)
		return -ENOMEM;

	np = pdev->dev.of_node;

	pmic_debug_parse_dts(np, l_dev);
	platform_set_drvdata(pdev, l_dev);
	if (g_pmic_ocp_status) {
		INIT_DELAYED_WORK(&l_dev->pmic_ocp_report_work, pmic_ocp_report_dmd_work);
		schedule_delayed_work(&l_dev->pmic_ocp_report_work, msecs_to_jiffies(PMIC_OCP_REPORT_TIME));
	}
	return 0;
}

static int pmic_ocp_debug_remove(struct platform_device *pdev)
{
	struct pmic_ocp_debug_dev *l_dev = platform_get_drvdata(pdev);

	if (!l_dev)
		return -ENODEV;

	if (g_pmic_ocp_status)
                cancel_delayed_work_sync(&l_dev->pmic_ocp_report_work);

	platform_set_drvdata(pdev, NULL);
 	kfree(l_dev);
	return 0;
}


static const struct of_device_id pmic_ocp_debug_match_table[] = {
	{
		.compatible = "qcom-pmic-ocp-debug",
		.data = NULL
	},
	{},
};

static struct platform_driver pmic_ocp_debug_driver = {
	.driver = {
		.name = "qcom-pmic-ocp-debug",
		.owner = THIS_MODULE,
		.of_match_table = pmic_ocp_debug_match_table,
	},
	.probe = pmic_ocp_debug_probe,
	.remove = pmic_ocp_debug_remove,
};

static int __init pmic_ocp_debug_init(void)
{
	return platform_driver_register(&pmic_ocp_debug_driver);
}

static void __exit pmic_ocp_debug_exit(void)
{
	return platform_driver_unregister(&pmic_ocp_debug_driver);
}

device_initcall_sync(pmic_ocp_debug_init);
module_exit(pmic_ocp_debug_exit);

MODULE_DESCRIPTION("hihonor Pmic Ocp debug driver");
MODULE_LICENSE("GPL v2");

