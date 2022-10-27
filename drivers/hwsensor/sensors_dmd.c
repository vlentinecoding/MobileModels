#include <linux/module.h>
#include <linux/kernel.h>
#ifdef CONFIG_HUAWEI_DSM
#include <dsm/dsm_pub.h>
struct dsm_dev dsm_sensorhub =
{
	.name = "dsm_sensorhub",
	.device_name = "sensor",
	.ic_name = "NNN",
	.module_name = "NNN",
	.fops = NULL,
	.buff_size = 1024,
};
struct dsm_client *sensors_dclient = NULL;

#endif

static int __init sensors_dmd_init(void)
{
#if defined (CONFIG_HUAWEI_DSM)
    if (!sensors_dclient) {
        sensors_dclient = dsm_register_client(&dsm_sensorhub);
	pr_info("dsm_register_client dsm_sensorhub init");
    }
#endif
	pr_info("dmd_init dsm_register_client dsm_sensorhub out");
	return 1;
}

static void __exit sensors_dmd_exit(void)
{
	pr_info("dmd_exit\n");
}

module_init(sensors_dmd_init);
module_exit(sensors_dmd_exit);

MODULE_AUTHOR("Honor");
MODULE_DESCRIPTION("sensors dmd init");
MODULE_LICENSE("GPL");

