/*
  * Copyright (c) Honor Technologies Co., Ltd. 2021. All rights reserved.
  * Description: DMD EVENT REPORT
  * Author: gaofeng
  * Create: 2021-05-17
===========================================================================*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>

#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>

#define BUF_SIZE	1024


static struct dsm_dev dsm_bt = {
	.name = "dsm_bt",
	.device_name = NULL,
	.ic_name = "NULL",
	.module_name = NULL,
	.fops = NULL,
	.buff_size = BUF_SIZE,
};

static struct dsm_client *bt_dsm_client = NULL;

static void bt_dsm_register_client(void)
{
	if (NULL != bt_dsm_client) {
		pr_debug(KERN_INFO "dsm_bt had been register!\n");
		return;
	}
	bt_dsm_client = dsm_register_client(&dsm_bt);
	if(NULL == bt_dsm_client) {
		pr_err("dsm_bt register failed!\n");
	}
	pr_debug("dsm_bt register success!\n");
	return;
}
static void bt_dsm_unregister_client(void)
{
	if (bt_dsm_client != NULL) {
		dsm_unregister_client(bt_dsm_client, &dsm_bt);
		bt_dsm_client = NULL;
	}
	pr_debug("dsm_bt unregister success!\n");
	return;
}
#endif

static int __init bt_dsm_init(void)
{
#ifdef CONFIG_HUAWEI_DSM
	bt_dsm_register_client();
#endif
	return 0;
}

static void __exit bt_dsm_exit(void)
{
#ifdef CONFIG_HUAWEI_DSM
	bt_dsm_unregister_client();
#endif
}


MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("BT dsm client");

module_init(bt_dsm_init);
module_exit(bt_dsm_exit);
