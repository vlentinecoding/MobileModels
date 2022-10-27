obj-$(CONFIG_LCD_QCOM_DRV) += backlight_linear_to_exp.o
obj-$(CONFIG_LCD_QCOM_DRV) += lcd_kit_backlight.o

EXTRA_CFLAGS +=-Idrivers/staging/android \
		-Idrivers/atfdriver \
		-I$(srctree)/drivers/devkit/lcdkit/qcomkit \
		-I$(srctree)/include/drm \
		-I$(srctree)/drivers/gpu/drm \
		-I$(srctree)/techpack/display/msm \
		-I$(srctree)/techpack/display/include \
		-I$(srctree)/techpack/display/include/uapi/display \
		-I$(srctree)/techpack/display/msm/dsi
