#ifndef __AWINIC_DSP_INTERFACE_H__
#define __AWINIC_DSP_INTERFACE_H__
#ifdef CONFIG_SND_SOC_AW882XX
void aw_get_afe_callback(int (*afe_get_topology)(int port_id),
	int (*send_afe_cal_apr)(uint32_t param_id, void *buf, int cmd_size, bool write),
	int (*send_afe_rx_module_enable)(void *buf, int size),
	int (*send_afe_tx_module_enable)(void *buf, int size));
#else
static inline void aw_get_afe_callback(int (*afe_get_topology)(int port_id),
	int (*send_afe_cal_apr)(uint32_t param_id, void *buf, int cmd_size, bool write),
	int (*send_afe_rx_module_enable)(void *buf, int size),
	int (*send_afe_tx_module_enable)(void *buf, int size)) {};
#endif
#endif

