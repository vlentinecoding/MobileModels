#include <linux/jiffies.h>
#include <linux/module.h>

#include "dubai_qcom_plat.h"

static int __init dubai_init(void)
{
	dubai_wakeup_stats_init();

#ifdef CONFIG_HUAWEI_DUBAI_GPU_STATS
	dubai_gpu_freq_stats_init();
#endif

#ifdef CONFIG_HUAWEI_DUBAI_BATTERY_STATS
	dubai_qcom_battery_stats_init();
#endif

#ifdef CONFIG_HUAWEI_DUBAI_DDR_STATS
	dubai_qcom_ddr_stats_init();
#endif

	return 0;
}

static void __exit dubai_exit(void)
{
	dubai_wakeup_stats_exit();

#ifdef CONFIG_HUAWEI_DUBAI_GPU_STATS
	dubai_gpu_freq_stats_exit();
#endif

#ifdef CONFIG_HUAWEI_DUBAI_BATTERY_STATS
	dubai_qcom_battery_stats_exit();
#endif

#ifdef CONFIG_HUAWEI_DUBAI_DDR_STATS
	dubai_qcom_ddr_stats_exit();
#endif

	return;
}

late_initcall(dubai_init);
module_exit(dubai_exit);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_DESCRIPTION("Huawei Device Usage Big-data Analytics Initiative Driver");
