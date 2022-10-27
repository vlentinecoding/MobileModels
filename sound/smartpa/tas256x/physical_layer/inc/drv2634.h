#ifndef __DRV2634_H_
#define __DRV2634_H_
#include <linux/leds.h>

#define LED_BRIGHTNESS_FAST  120
#define VIB_DEFAULT_TIMEOUT    15000

struct haptics_data  {
	int timedout;
	int haptic_play_ms;
	bool haptics_playing;
	struct led_classdev led_dev;
	struct workqueue_struct *wq;
	struct work_struct vib_play_work;
	struct work_struct vib_stop_work;
	struct hrtimer vib_mtimer;
};

#define DRV2634_BOOST_CFG1 TAS256X_REG(0x0, 0x0, 0x33)
#define DRV2634_NEW_04 TAS256X_REG(0x0, 0x01, 0x04)
#define DRV2634_TDMCONFIGURATIONREG7 TAS256X_REG(0x0, 0x0, 0x0d)
#define DRV2634_TDMCONFIGURATIONREG10 TAS256X_REG(0x0, 0x0, 0x10)
#define DRV2634_NEW_1C TAS256X_REG(0x64, 0x08, 0x08)
#define DRV2634_IVHPFC_CFG5 TAS256X_REG(0x0, 0x02, 0x74)
#define DRV2634_IVHPFC_CFG9 TAS256X_REG(0x0, 0x02, 0x78)
#define DRV2634_1C TAS256X_REG(0x64, 0x08, 0x1c)

static char DRV2634_HPF_reverse_path1[] = {0x7f, 0xff, 0xff, 0xff};
static char DRV2634_HPF_reverse_path2[] = {0x00, 0x00, 0x00, 0x00};
static char DRV2634_HPF_reverse_path3[] = {0x00, 0x00, 0x00, 0x00};
static char DRV2634_HPF_reverse_path4[] = {0x20, 0x01, 0x01, 0xc0};

#endif

