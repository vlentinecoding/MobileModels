#ifndef BOOTDEVICE_H
#define BOOTDEVICE_H
#include <linux/device.h>

enum bootdevice_type { BOOT_DEVICE_EMMC = 0, BOOT_DEVICE_UFS = 1 };

enum bootdevice_type get_bootdevice_type(void);

#endif

