ifneq ($(TARGET_BUILD_VARIANT),user)
EXTRA_CFLAGS += -DLCD_DEBUG_ENABLE
obj-$(CONFIG_LCD_QCOM_DRV)	+= lcd_debug.o
endif
ccflags-y += -I$(srctree)/techpack/display/include
ccflags-y += -I$(srctree)/techpack/display/include/uapi/display
ccflags-y += -I$(srctree)/techpack/display/msm
ccflags-y += -I$(srctree)/techpack/display/msm/dsi
ccflags-y += -I$(srctree)/drivers/devkit/lcdkit/lcdkit3.0/kernel/common/include
obj-$(CONFIG_LCD_QCOM_DRV)	+= lcd_panel.o \
				   lcd_bl.o \
				   lcd_bias.o \
				   lcd_notifier.o
obj-$(CONFIG_LCD_QCOM_DRV)	+= lcd_sysfs.o
obj-$(CONFIG_LCD_QCOM_DRV)	+= ../lcdkit3.0/kernel/common/src/lcd_kit_core.o
obj-$(CONFIG_LCD_FACTORY)	+= lcd_factory.o
obj-$(CONFIG_LCD_QCOM_DRV)	+= backlight/
