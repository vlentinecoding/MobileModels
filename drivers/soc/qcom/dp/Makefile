
DP_CUST_OBJS += dp_cust_info.o
DP_CUST_OBJS += dp_source.o
DP_CUST_OBJS += dp_dsm.o

ifdef CONFIG_HONOR_DP_FACTORY
	DP_CUST_OBJS += dp_factory.o
endif

ifndef CONFIG_FINAL_RELEASE
	ccflags-y += -DDP_DEBUG_ENABLE
	DP_CUST_OBJS += dp_debug.o
endif

dp_cust_dlkm-y := $(DP_CUST_OBJS)
obj-$(CONFIG_HONOR_DP_CUST) += dp_cust_dlkm.o
