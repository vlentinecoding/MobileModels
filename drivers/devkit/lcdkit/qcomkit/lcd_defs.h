#ifndef LCD_DEFS_H
#define LCD_DEFS_H
#include <linux/types.h>
#include <drm/drm_print.h>
#include "dsi_panel.h"

#define LCD_WARN(fmt, ...)	DRM_WARN("[LCD /W]: "fmt, ##__VA_ARGS__)
#define LCD_ERR(fmt, ...)	DRM_DEV_ERROR(NULL, "[LCD /E]: " fmt, \
								##__VA_ARGS__)
#define LCD_INFO(fmt, ...)	DRM_DEV_INFO(NULL, "[LCD /I]: "fmt, \
								##__VA_ARGS__)
#define LCD_DEV_DEBUG(fmt, ...)	DRM_DEV_DEBUG(NULL, "[LCD /D]: "fmt, \
								##__VA_ARGS__)
/* fb max number */
#define FB_MAX 32

#define HBM_SET_MAX_LEVEL 5000

#define LCD_FAIL (-1)
#define LCD_OK   0
#define NOT_SUPPORT 0

/* Register/unregister for screen events */
#define FB_EVENT_PANEL 0x20

enum HBM_CFG_TYPE {
	HBM_FOR_FP = 0,
	HBM_FOR_MMI = 1,
	HBM_FOR_LIGHT = 2,
	LOCAL_HBM_FOR_MMI = 3,
	LOCAL_HBM_FOR_IDENTIFY = 4,
	TEIRQ_ENABLE = 5,
	TEIRQ_DISABLE = 6
};

enum lcd_panel_state {
	LCD_POWER_OFF,
	LCD_POWER_ON,
	LCD_LP_ON,
	LCD_HS_ON,
	LCD_POWER_SUSPEND,
};

struct panel_info *get_panel_info(int idx);
int get_fb_index(struct device *dev);

#ifdef CONFIG_LCD_FACTORY
int factory_init(struct dsi_panel *panel, struct panel_info *pinfo);
#endif
int panel_get_status_by_type(int type, int *status);
#if defined(LCD_DEBUG_ENABLE)
int debugfs_init(void);
int dbg_set_cmd(struct dsi_panel *panel, enum dsi_cmd_set_type type);
int get_dbg_level(void);

#define LCD_DEBUG(fmt, ...) do { \
	if (get_dbg_level() >= 4) \
		DRM_DEV_INFO(NULL, "[LCD /D]: " fmt, ##__VA_ARGS__); \
} while (0)
#endif
#endif
