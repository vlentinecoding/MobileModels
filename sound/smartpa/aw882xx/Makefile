# TOP_PATH = $(shell pwd)
# ccflags-y += -I$(TOP_PATH)/../../../../../../../../kernel/msm-5.4/techpack/audio/include
# ccflags-y += -I$(TOP_PATH)/../../../../../../../../kernel/msm-5.4/techpack/audio/include/uapi
ccflags-y += -I$(srctree)/techpack/audio/include
ccflags-y += -I$(srctree)/techpack/audio/include/uapi

obj-$(CONFIG_SND_SOC_AW882XX) += aw882xx.o
obj-$(CONFIG_SND_SOC_AW882XX) += aw_calib.o
obj-$(CONFIG_SND_SOC_AW882XX) += aw_monitor.o
obj-$(CONFIG_SND_SOC_AW882XX) += aw_dsp.o
obj-$(CONFIG_SND_SOC_AW882XX) += aw_bin_parse.o
obj-$(CONFIG_SND_SOC_AW882XX) += aw_init.o
obj-$(CONFIG_SND_SOC_AW882XX) += aw_device.o
