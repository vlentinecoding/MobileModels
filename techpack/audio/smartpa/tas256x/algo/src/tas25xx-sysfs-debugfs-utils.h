#ifndef __TAS25XX_ALGO_SYSFS_INTF_H__
#define __TAS25XX_ALGO_SYSFS_INTF_H__

#include <linux/version.h>
#include <sound/soc.h>
#include <linux/i2c.h>
#include <linux/debugfs.h>

int tas25xx_parse_algo_dt_sysfs(struct device_node *np);
void tas25xx_algo_bump_oc_count(int channel, int reset);
#if KERNEL_VERSION(4, 19, 0) <= LINUX_VERSION_CODE
int tas_smartamp_add_algo_controls_debugfs(struct snd_soc_component *c,
	int number_of_channels);
void tas_smartamp_remove_algo_controls_debugfs(struct snd_soc_component *c);
#else
int tas_smartamp_add_algo_controls_debugfs(struct snd_soc_codec *c,
	int number_of_channels);
void tas_smartamp_remove_algo_controls_debugfs(struct snd_soc_codec *c);
#endif

#endif /* __TAS25XX_ALGO_SYSFS_INTF_H__ */
