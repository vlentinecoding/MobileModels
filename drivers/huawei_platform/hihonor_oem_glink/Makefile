obj-$(CONFIG_HIHONOR_OEM_GLINK) += hihonor_oem_glink.o
obj-$(CONFIG_HIHONOR_OEM_GLINK) += hihonor_fg_glink.o

ifeq ($(HLTHERM_RUNTEST), true)
    CFLAGS_KERNEL += -DCONFIG_HLTHERM_RUNTEST
endif
