obj-$(CONFIG_HUAWEI_DUBAI) += \
	dubai.o \
	dubai_wakeup_stats.o

obj-$(CONFIG_HUAWEI_DUBAI_BATTERY_STATS) += dubai_battery_stats.o

obj-$(CONFIG_HUAWEI_DUBAI_GPU_STATS) += dubai_gpu_freq_stats.o

ifeq ($(CONFIG_HUAWEI_DUBAI_DDR_STATS),y)
obj-y += dubai_ddr_stats.o
subdir-ccflags-y += -I$(srctree)/drivers/soc/qcom
endif
