#ifndef DUBAI_SPRD_PLAT_H
#define DUBAI_SPRD_PLAT_H

void dubai_wakeup_stats_init(void);
void dubai_wakeup_stats_exit(void);

#ifdef CONFIG_HUAWEI_DUBAI_GPU_STATS
void dubai_gpu_freq_stats_init(void);
void dubai_gpu_freq_stats_exit(void);
#endif

#ifdef CONFIG_HUAWEI_DUBAI_BATTERY_STATS
void dubai_qcom_battery_stats_init(void);
void dubai_qcom_battery_stats_exit(void);
#endif

#ifdef CONFIG_HUAWEI_DUBAI_DDR_STATS
void dubai_qcom_ddr_stats_init(void);
void dubai_qcom_ddr_stats_exit(void);
#endif

#endif // DUBAI_SPRD_PLAT_H
