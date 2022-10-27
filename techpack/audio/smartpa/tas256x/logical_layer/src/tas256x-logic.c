#include "logical_layer/inc/tas256x-logic.h"
#include "physical_layer/inc/tas256x-device.h"
#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
#include "algo/inc/tas_smart_amp_v2.h"
#include "algo/inc/tas25xx-calib.h"
#if IS_ENABLED(CONFIG_TISA_SYSFS_INTF)
#include "algo/src/tas25xx-sysfs-debugfs-utils.h"
#endif /* CONFIG_TISA_SYSFS_INTF */
#endif /* CONFIG_TAS25XX_ALGO*/
#include "os_layer/inc/tas256x-regmap.h"

/* 128 Register Map to be used during Register Dump*/
#define REG_CAP_MAX	128
static struct tas256x_reg regtable[REG_CAP_MAX] = {
	{0,	0},	{1,	0},	{2,	0},	{3,	0},	{4,	0},
	{5,	0},	{6,	0},	{7,	0},	{8,	0},	{9,	0},
	{10,	0},	{11,	0},	{12,	0},	{13,	0},	{14,	0},
	{15,	0},	{16,	0},	{17,	0},	{18,	0},	{19,	0},
	{20,	0},	{21,	0},	{22,	0},	{23,	0},	{24,	0},
	{25,	0},	{26,	0},	{27,	0},	{28,	0},	{29,	0},
	{30,	0},	{31,	0},	{32,	0},	{33,	0},	{34,	0},
	{35,	0},	{36,	0},	{37,	0},	{38,	0},	{39,	0},
	{40,	0},	{41,	0},	{42,	0},	{43,	0},	{44,	0},
	{45,	0},	{46,	0},	{47,	0},	{48,	0},	{49,	0},
	{50,	0},	{51,	0},	{52,	0},	{53,	0},	{54,	0},
	{55,	0},	{56,	0},	{57,	0},	{58,	0},	{59,	0},
	{60,	0},	{61,	0},	{62,	0},	{63,	0},	{64,	0},
	{65,	0},	{66,	0},	{67,	0},	{68,	0},	{69,	0},
	{70,	0},	{71,	0},	{72,	0},	{73,	0},	{74,	0},
	{75,	0},	{76,	0},	{77,	0},	{78,	0},	{79,	0},
	{80,	0},	{81,	0},	{82,	0},	{83,	0},	{84,	0},
	{85,	0},	{86,	0},	{87,	0},	{88,	0},	{89,	0},
	{90,	0},	{91,	0},	{92,	0},	{93,	0},	{94,	0},
	{95,	0},	{96,	0},	{97,	0},	{98,	0},	{99,	0},
	{100,	0},	{101,	0},	{102,	0},	{103,	0},	{104,	0},
	{105,	0},	{106,	0},	{107,	0},	{108,	0},	{109,	0},
	{110,	0},	{111,	0},	{112,	0},	{113,	0},	{114,	0},
	{115,	0},	{116,	0},	{117,	0},	{118,	0},	{119,	0},
	{120,	0},	{121,	0},	{122,	0},	{123,	0},	{124,	0},
	{125,	0},	{126,	0},	{127,	0},
};

static int tas256x_change_book_page(struct tas256x_priv *p_tas256x,
	enum channel chn,
	int book, int page)
{
	int ret = -1, rc = 0;
	int i = 0;

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (((chn&channel_left) && (i == 0))
			|| ((chn&channel_right) && (i == 1))) {
			if (p_tas256x->devs[i]->mn_current_book != book) {
				ret = p_tas256x->plat_write(
						p_tas256x->platform_data,
						p_tas256x->devs[i]->mn_addr,
						TAS256X_BOOKCTL_PAGE, 0);
				if (ret < 0) {
					pr_err(
						"%s, ERROR, L=%d, E=%d\n",
						__func__, __LINE__, ret);
					rc |= ret;
					continue;
				}
				p_tas256x->devs[i]->mn_current_page = 0;
				ret = p_tas256x->plat_write(
						p_tas256x->platform_data,
						p_tas256x->devs[i]->mn_addr,
						TAS256X_BOOKCTL_REG, book);
				if (ret < 0) {
					pr_err(
						"%s, ERROR, L=%d, E=%d\n",
						__func__, __LINE__, ret);
					rc |= ret;
					continue;
				}
				p_tas256x->devs[i]->mn_current_book = book;
			}

			if (p_tas256x->devs[i]->mn_current_page != page) {
				ret = p_tas256x->plat_write(
						p_tas256x->platform_data,
						p_tas256x->devs[i]->mn_addr,
						TAS256X_BOOKCTL_PAGE, page);
				if (ret < 0) {
					pr_err(
						"%s, ERROR, L=%d, E=%d\n",
						__func__, __LINE__, ret);
					rc |= ret;
					continue;
				}
				p_tas256x->devs[i]->mn_current_page = page;
			}
		}
	}

	if (rc < 0) {
		if (chn&channel_left)
			p_tas256x->mn_err_code |= ERROR_DEVA_I2C_COMM;
		if (chn&channel_right)
			p_tas256x->mn_err_code |= ERROR_DEVB_I2C_COMM;
	} else {
		if (chn&channel_left)
			p_tas256x->mn_err_code &= ~ERROR_DEVA_I2C_COMM;
		if (chn&channel_right)
			p_tas256x->mn_err_code &= ~ERROR_DEVB_I2C_COMM;
	}
	return rc;
}

static int tas256x_dev_read(struct tas256x_priv *p_tas256x,
	enum channel chn,
	unsigned int reg, unsigned int *pValue)
{
	int ret = -1;
	int i = 0, chnTemp = 0;

	mutex_lock(&p_tas256x->dev_lock);

	if (chn == channel_both) {
		for (i = 0; i < p_tas256x->mn_channels; i++)
			chnTemp |= 1<<i;
		chn = (chnTemp == 0)?chn:(enum channel)chnTemp;
	}

	ret = tas256x_change_book_page(p_tas256x, chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (ret < 0)
		goto end;

	/*Force left incase of mono*/
	if ((chn == channel_right) && (p_tas256x->mn_channels == 1))
		chn = channel_left;

	ret = p_tas256x->plat_read(p_tas256x->platform_data,
			p_tas256x->devs[chn>>1]->mn_addr,
			TAS256X_PAGE_REG(reg), pValue);
	if (ret < 0) {
		pr_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, ret);
		if (chn&channel_left)
			p_tas256x->mn_err_code |= ERROR_DEVA_I2C_COMM;
		if (chn&channel_right)
			p_tas256x->mn_err_code |= ERROR_DEVB_I2C_COMM;
	} else {
		pr_debug(
			"%s: chn:%x:BOOK:PAGE:REG 0x%02x:0x%02x:0x%02x,0x%02x\n",
			__func__,
			p_tas256x->devs[chn>>1]->mn_addr, TAS256X_BOOK_ID(reg),
			TAS256X_PAGE_ID(reg),
			TAS256X_PAGE_REG(reg), *pValue);
		if (chn&channel_left)
			p_tas256x->mn_err_code &= ~ERROR_DEVA_I2C_COMM;
		if (chn&channel_right)
			p_tas256x->mn_err_code &= ~ERROR_DEVB_I2C_COMM;
	}
end:
	mutex_unlock(&p_tas256x->dev_lock);
	return ret;
}

static int tas256x_dev_write(struct tas256x_priv *p_tas256x, enum channel chn,
	unsigned int reg, unsigned int value)
{
	int ret = -1, rc = 0;
	int i = 0, chnTemp = 0;

	mutex_lock(&p_tas256x->dev_lock);

	if (chn == channel_both) {
		for (i = 0; i < p_tas256x->mn_channels; i++)
			chnTemp |= 1<<i;
		chn = (chnTemp == 0)?chn:(enum channel)chnTemp;
	}

	ret = tas256x_change_book_page(p_tas256x, chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (ret < 0)
		goto end;

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (((chn&channel_left) && (i == 0))
			|| ((chn&channel_right) && (i == 1))) {
			ret = p_tas256x->plat_write(
					p_tas256x->platform_data,
					p_tas256x->devs[i]->mn_addr,
					TAS256X_PAGE_REG(reg), value);
			if (ret < 0) {
				pr_err(
					"%s, ERROR, L=%u, chn=0x%02x, E=%d\n",
					__func__, __LINE__,
					p_tas256x->devs[i]->mn_addr, ret);
				rc |= ret;
				if (chn&channel_left)
					p_tas256x->mn_err_code |= ERROR_DEVA_I2C_COMM;
				if (chn&channel_right)
					p_tas256x->mn_err_code |= ERROR_DEVB_I2C_COMM;
			} else {
				pr_debug(
					"%s: %u: chn:0x%02x:BOOK:PAGE:REG 0x%02x:0x%02x:0x%02x, VAL: 0x%02x\n",
					__func__, __LINE__,
					p_tas256x->devs[i]->mn_addr,
					TAS256X_BOOK_ID(reg),
					TAS256X_PAGE_ID(reg),
					TAS256X_PAGE_REG(reg), value);
				if (chn&channel_left)
					p_tas256x->mn_err_code &= ~ERROR_DEVA_I2C_COMM;
				if (chn&channel_right)
					p_tas256x->mn_err_code &= ~ERROR_DEVB_I2C_COMM;
			}
		}
	}
end:
	mutex_unlock(&p_tas256x->dev_lock);
	return rc;
}

static int tas256x_dev_bulk_write(struct tas256x_priv *p_tas256x,
	enum channel chn,
	unsigned int reg, unsigned char *p_data, unsigned int n_length)
{
	int ret = -1, rc = 0;
	int i = 0, chnTemp = 0;

	mutex_lock(&p_tas256x->dev_lock);

	if (chn == channel_both) {
		for (i = 0; i < p_tas256x->mn_channels; i++)
			chnTemp |= 1<<i;
		chn = (chnTemp == 0)?chn:(enum channel)chnTemp;
	}

	ret = tas256x_change_book_page(p_tas256x, chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (ret < 0)
		goto end;

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (((chn&channel_left) && (i == 0))
			|| ((chn&channel_right) && (i == 1))) {
			ret = p_tas256x->plat_bulk_write(
					p_tas256x->platform_data,
					p_tas256x->devs[i]->mn_addr,
					TAS256X_PAGE_REG(reg),
					p_data, n_length);
			if (ret < 0) {
				pr_err(
					"%s, ERROR, L=%u, chn=0x%02x: E=%d\n",
					__func__, __LINE__,
					p_tas256x->devs[i]->mn_addr, ret);
				rc |= ret;
				if (chn&channel_left)
					p_tas256x->mn_err_code |= ERROR_DEVA_I2C_COMM;
				if (chn&channel_right)
					p_tas256x->mn_err_code |= ERROR_DEVB_I2C_COMM;
			} else {
				pr_debug(
					"%s: chn%x:BOOK:PAGE:REG 0x%02x:0x%02x:0x%02x, len: %u\n",
					__func__, p_tas256x->devs[i]->mn_addr,
					TAS256X_BOOK_ID(reg),
					TAS256X_PAGE_ID(reg),
					TAS256X_PAGE_REG(reg), n_length);
				if (chn&channel_left)
					p_tas256x->mn_err_code &= ~ERROR_DEVA_I2C_COMM;
				if (chn&channel_right)
					p_tas256x->mn_err_code &= ~ERROR_DEVB_I2C_COMM;
			}
		}
	}

end:
	mutex_unlock(&p_tas256x->dev_lock);
	return rc;
}

static int tas256x_dev_bulk_read(struct tas256x_priv *p_tas256x,
	enum channel chn,
	unsigned int reg, unsigned char *p_data, unsigned int n_length)
{
	int ret = -1;
	int i = 0, chnTemp = 0;

	mutex_lock(&p_tas256x->dev_lock);

	if (chn == channel_both) {
		for (i = 0; i < p_tas256x->mn_channels; i++) {
				chnTemp |= 1<<i;
		}
		chn = (chnTemp == 0)?chn:(enum channel)chnTemp;
	}

	/*Force left incase of mono*/
	if ((chn == channel_right) && (p_tas256x->mn_channels == 1))
		chn = channel_left;

	ret = tas256x_change_book_page(p_tas256x, chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (ret < 0)
		goto end;

	ret = p_tas256x->plat_bulk_read(p_tas256x->platform_data,
		p_tas256x->devs[chn>>1]->mn_addr, TAS256X_PAGE_REG(reg),
		p_data, n_length);
	if (ret < 0) {
		pr_err("%s, ERROR, L=%d, E=%d\n",
			__func__, __LINE__, ret);
		if (chn&channel_left)
			p_tas256x->mn_err_code |= ERROR_DEVA_I2C_COMM;
		if (chn&channel_right)
			p_tas256x->mn_err_code |= ERROR_DEVB_I2C_COMM;
	} else {
		pr_debug(
			"%s: chn%x:BOOK:PAGE:REG %u:%u:%u, len: 0x%02x\n",
			__func__, p_tas256x->devs[chn>>1]->mn_addr,
			TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg),
			TAS256X_PAGE_REG(reg), n_length);
		if (chn&channel_left)
			p_tas256x->mn_err_code &= ~ERROR_DEVA_I2C_COMM;
		if (chn&channel_right)
			p_tas256x->mn_err_code &= ~ERROR_DEVB_I2C_COMM;
	}
end:
	mutex_unlock(&p_tas256x->dev_lock);
	return ret;
}

static int tas256x_dev_update_bits(struct tas256x_priv *p_tas256x,
	enum channel chn,
	unsigned int reg, unsigned int mask, unsigned int value)
{
	int ret = -1, rc = 0;
	int i = 0, chnTemp = 0;

	mutex_lock(&p_tas256x->dev_lock);

	if (chn == channel_both) {
		for (i = 0; i < p_tas256x->mn_channels; i++)
				chnTemp |= 1<<i;
		chn = (chnTemp == 0)?chn:(enum channel)chnTemp;
	}

	ret = tas256x_change_book_page(p_tas256x, chn,
		TAS256X_BOOK_ID(reg), TAS256X_PAGE_ID(reg));
	if (ret < 0) {
		rc = ret;
		goto end;
	}

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (((chn&channel_left) && (i == 0))
			|| ((chn&channel_right) && (i == 1))) {
			ret = p_tas256x->plat_update_bits(
					p_tas256x->platform_data,
					p_tas256x->devs[i]->mn_addr,
					TAS256X_PAGE_REG(reg),
					mask, value);
			if (ret < 0) {
				pr_err(
					"%s, ERROR, L=%u, chn=0x%02x: E=%d\n",
					__func__, __LINE__,
					p_tas256x->devs[i]->mn_addr, ret);
				rc |= ret;
				p_tas256x->mn_err_code |=
					(chn == channel_left) ? ERROR_DEVA_I2C_COMM : ERROR_DEVB_I2C_COMM;
			} else {
				pr_debug(
					"%s: chn%x:BOOK:PAGE:REG 0x%02x:0x%02x:0x%02x, mask: 0x%02x, val: 0x%02x\n",
					__func__, p_tas256x->devs[i]->mn_addr,
					TAS256X_BOOK_ID(reg),
					TAS256X_PAGE_ID(reg),
					TAS256X_PAGE_REG(reg), mask, value);
				p_tas256x->mn_err_code &=
					(chn == channel_left) ? ~ERROR_DEVA_I2C_COMM : ~ERROR_DEVB_I2C_COMM;
			}
		}
	}

end:
	mutex_unlock(&p_tas256x->dev_lock);
	return rc;
}

/* Function to Dump Registers */
void tas256x_dump_regs(struct tas256x_priv  *p_tas256x, int chn)
{
	int i = 0;

	pr_info("TAS256X %s\n", __func__);
	pr_info("----------- TAS256X Channel-%d RegDump ------------\n",
		chn-1);
	for (i = 0; i < REG_CAP_MAX; i++)
		p_tas256x->read(p_tas256x, chn, regtable[i].reg_index, &(regtable[i].reg_val));
	for (i = 0; i < REG_CAP_MAX/4; i++) {
		pr_info(
			"%s: 0x%02x=0x%02x, 0x%02x=0x%02x, 0x%02x=0x%02x, 0x%02x=0x%02x\n",
			__func__,
			regtable[4 * i].reg_index, regtable[4 * i].reg_val,
			regtable[4 * i + 1].reg_index, regtable[4 * i + 1].reg_val,
			regtable[4 * i + 2].reg_index, regtable[4 * i + 2].reg_val,
			regtable[4 * i + 3].reg_index, regtable[4 * i + 3].reg_val);
	}
	if (REG_CAP_MAX % 4) {
		for (i = 4 * (REG_CAP_MAX / 4); i < REG_CAP_MAX; i++)
			pr_info("%s: 0x%02x=0x%02x\n",
				__func__, regtable[i].reg_index, regtable[i].reg_val);
	}
	pr_info("%s: ------------------------------------------\n",
		__func__);
}

/*TODO: Revisit the function as its not usually used*/
static void tas256x_hard_reset(struct tas256x_priv  *p_tas256x)
{
	int i = 0;

	p_tas256x->hw_reset(p_tas256x);

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		p_tas256x->devs[i]->mn_current_book = -1;
		p_tas256x->devs[i]->mn_current_page = -1;
	}

	if (p_tas256x->mn_err_code)
		pr_info("%s: before reset, ErrCode=0x%x\n", __func__,
			p_tas256x->mn_err_code);
	p_tas256x->mn_err_code = 0;
}

/*TODO: Important: Revisit this function as
 *hardware reset is actually needed here
 */
void tas256x_failsafe(struct tas256x_priv  *p_tas256x, int chn)
{
	int ret = -1;

	pr_info("tas256x %s\n", __func__);
	p_tas256x->mn_err_code |= ERROR_FAILSAFE;

	if (p_tas256x->devs[chn-1]->mn_restart < RESTART_MAX) {
		p_tas256x->devs[chn-1]->mn_restart++;
		msleep(100);
		pr_err("I2C COMM error, restart SmartAmp.\n");
		tas256x_load_config(p_tas256x, chn);
		return;
	}

	ret = tas256x_set_power_shutdown(p_tas256x, chn);
	msleep(20);
	/*Mask interrupt for TDM*/
	ret = tas256x_interrupt_enable(p_tas256x, 0/*Disable*/,
		chn);
	p_tas256x->write(p_tas256x, chn, TAS256X_SOFTWARERESET,
		TAS256X_SOFTWARERESET_SOFTWARERESET_RESET);
	udelay(1000);
}

int tas256x_load_i2s_tdm_interface_settings(struct tas256x_priv *p_tas256x,
	int chn)
{
	int ret = -1;

	/*Frame_Start Settings*/
	ret = tas256x_rx_set_frame_start(p_tas256x,
		p_tas256x->mn_frame_start, chn);
	/*RX Edge Settings*/
	ret |= tas256x_rx_set_edge(p_tas256x,
		p_tas256x->mn_rx_edge, chn);
	/*RX Offset Settings*/
	ret |= tas256x_rx_set_start_slot(p_tas256x,
		p_tas256x->mn_rx_offset, chn);
	/*TX Edge Settings*/
	ret |= tas256x_tx_set_edge(p_tas256x,
		p_tas256x->mn_tx_edge, chn);
	/*TX Offset Settings*/
	ret |= tas256x_tx_set_start_slot(p_tas256x,
		p_tas256x->mn_tx_offset, chn);

	return ret;
}

int tas256x_load_init(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	pr_info("%s:\n", __func__);

	if (p_tas256x->devs[chn-1]->dev_ops.tas_init) {
		ret = (p_tas256x->devs[chn-1]->dev_ops.tas_init)(p_tas256x,
			chn);
		if (ret < 0)
			goto end;
	}

	ret = tas256x_set_misc_config(p_tas256x, 0/*Ignored*/, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_load_i2s_tdm_interface_settings(p_tas256x, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_set_clock_config(p_tas256x, 0/*Ignored*/, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_update_clk_halt_timer(p_tas256x, 6/*419ms*/, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_update_icn_hysterisis(p_tas256x,
		p_tas256x->devs[chn-1]->icn_hyst,
		p_tas256x->mn_sampling_rate, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_update_icn_threshold(p_tas256x,
		p_tas256x->devs[chn-1]->icn_thr, chn);
	if (ret < 0)
		goto end;
	
	/*ICN Improve Performance*/
	ret |= tas256x_icn_config(p_tas256x, 0/*Ignored*/, chn);
	if (ret < 0)
		goto end;

#if IS_ENABLED(HPF_BYPASS)
	/*Disable the HPF in Forward Path*/
	ret |= tas256x_HPF_FF_Bypass(p_tas256x, 0/*Ignored*/, chn);
	if (ret < 0)
		goto end;
	/*Disable the HPF in Reverse Path*/
	ret |= tas256x_HPF_FB_Bypass(p_tas256x, 0/*Ignored*/, chn);
	if (ret < 0)
		goto end;
#endif
	ret |= tas256x_set_classH_config(p_tas256x, 0/*Ignored*/, chn);
end:
	return ret;
}

int tas256x_load_ctrl_values(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = 0;

	ret = tas256x_update_playback_volume(p_tas256x,
		p_tas256x->devs[chn-1]->dvc_pcm, chn);

	ret |= tas256x_update_lim_max_attenuation(p_tas256x,
		p_tas256x->devs[chn-1]->lim_max_attn, chn);

	ret |= tas256x_update_lim_max_thr(p_tas256x,
		p_tas256x->devs[chn-1]->lim_thr_max, chn);

	ret |= tas256x_update_lim_min_thr(p_tas256x,
		p_tas256x->devs[chn-1]->lim_thr_min, chn);

	ret |= tas256x_update_lim_inflection_point(p_tas256x,
		p_tas256x->devs[chn-1]->lim_infl_pt, chn);

	ret |= tas256x_update_lim_slope(p_tas256x,
		p_tas256x->devs[chn-1]->lim_trk_slp, chn);

	ret |= tas256x_update_bop_thr(p_tas256x,
		p_tas256x->devs[chn-1]->bop_thd, chn);

	ret |= tas256x_update_bosd_thr(p_tas256x,
		p_tas256x->devs[chn-1]->bosd_thd, chn);

	ret |= tas256x_update_boost_voltage(p_tas256x,
		p_tas256x->devs[chn-1]->bst_vltg, chn);

	ret |= tas256x_update_current_limit(p_tas256x,
		p_tas256x->devs[chn-1]->bst_ilm, chn);

	ret |= tas256x_update_ampoutput_level(p_tas256x,
		p_tas256x->devs[chn-1]->ampoutput_lvl, chn);

	ret |= tas256x_update_limiter_enable(p_tas256x,
		p_tas256x->devs[chn-1]->lim_switch, chn);

	ret |= tas256x_update_limiter_attack_rate(p_tas256x,
		p_tas256x->devs[chn-1]->lim_att_rate, chn);

	ret |= tas256x_update_limiter_attack_step_size(p_tas256x,
		p_tas256x->devs[chn-1]->lim_att_stp_size, chn);

	ret |= tas256x_update_limiter_release_rate(p_tas256x,
		p_tas256x->devs[chn-1]->lim_rel_rate, chn);

	ret |= tas256x_update_limiter_release_step_size(p_tas256x,
		p_tas256x->devs[chn-1]->lim_rel_stp_size, chn);

	ret |= tas256x_update_bop_enable(p_tas256x,
		p_tas256x->devs[chn-1]->bop_enable, chn);

	ret |= tas256x_update_bop_mute(p_tas256x,
		p_tas256x->devs[chn-1]->bop_mute, chn);

	ret |= tas256x_update_bop_shutdown_enable(p_tas256x,
		p_tas256x->devs[chn-1]->bosd_enable, chn);

	ret |= tas256x_update_bop_attack_rate(p_tas256x,
		p_tas256x->devs[chn-1]->bop_att_rate, chn);

	ret |= tas256x_update_bop_attack_step_size(p_tas256x,
		p_tas256x->devs[chn-1]->bop_att_stp_size, chn);

	ret |= tas256x_update_bop_hold_time(p_tas256x,
		p_tas256x->devs[chn-1]->bop_hld_time, chn);

	ret |= tas256x_update_vbat_lpf(p_tas256x,
		p_tas256x->devs[chn-1]->vbat_lpf, chn);

	ret |= tas256x_update_rx_cfg(p_tas256x,
		p_tas256x->devs[chn-1]->rx_cfg, chn);

	ret |= tas256x_update_classh_timer(p_tas256x,
		p_tas256x->devs[chn-1]->classh_timer, chn);

	ret |= tas256x_enable_reciever_mode(p_tas256x,
		p_tas256x->devs[chn-1]->reciever_enable, chn);

	ret |= tas256x_icn_disable(p_tas256x,
		p_tas256x->devs[chn-1]->icn_sw, chn);

	ret |= tas256x_rx_set_slot(p_tas256x,
		p_tas256x->mn_rx_slot_map[chn-1], chn);

	ret |= tas256x_update_icn_hysterisis(p_tas256x,
		p_tas256x->devs[chn-1]->icn_hyst,
		p_tas256x->mn_sampling_rate, chn);

	ret |= tas256x_update_icn_threshold(p_tas256x,
		p_tas256x->devs[chn-1]->icn_thr,
		chn);	

	return ret;
}

void tas256x_irq_reload(struct tas256x_priv *p_tas256x, int chn)
{
	int nDevInt1Status = 0;

	pr_info("%s:\n", __func__);

	tas256x_power_check(p_tas256x, &nDevInt1Status, chn);
	if (!nDevInt1Status)
		tas256x_set_power_state(p_tas256x,
			p_tas256x->mn_power_state, chn);
	else
		tas256x_interrupt_enable(p_tas256x, 1/*Enable*/,
				chn);
}

/* Init Work Routine or Failsafe reload */
/*TODO: Failsafe is needed or not ??*/
void tas256x_load_config(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	pr_info("%s:\n", __func__);

	ret = tas56x_software_reset(p_tas256x, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_load_ctrl_values(p_tas256x, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_load_init(p_tas256x, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_update_rx_cfg(p_tas256x,
		p_tas256x->devs[chn-1]->rx_cfg,
		chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_iv_sense_enable_set(p_tas256x, 1,
		chn);
	if (ret < 0)
		goto end;
	/* Since Bitwidth and IV Width from DT file may different and still needs to work
	 * To support above mn_rx_width(same as tx) is used instead of mn_rx/tx_slot_width
	 */
	if (p_tas256x->mn_fmt_mode == 2) {
		ret |= tas256x_set_tdm_rx_slot(p_tas256x, p_tas256x->mn_slots,
			p_tas256x->mn_rx_width);
		if (ret < 0)
			goto end;
		ret |= tas256x_set_tdm_tx_slot(p_tas256x, p_tas256x->mn_slots,
			p_tas256x->mn_rx_width);
		if (ret < 0)
			goto end;
	} else { /*I2S Mode*/
		ret |= tas256x_set_bitwidth(p_tas256x,
			p_tas256x->mn_rx_width, TAS256X_STREAM_PLAYBACK);
		if (ret < 0)
			goto end;
		ret |= tas256x_set_bitwidth(p_tas256x,
			p_tas256x->mn_rx_width, TAS256X_STREAM_CAPTURE);
		if (ret < 0)
			goto end;
	}

	ret |= tas256x_set_samplerate(p_tas256x, p_tas256x->mn_sampling_rate,
		channel_both);
	if (ret < 0)
		goto end;
	if (p_tas256x->devs[chn-1]->dac_power)
		ret |= tas256x_set_power_state(p_tas256x,
			p_tas256x->mn_power_state, chn);
end:
	return;
}

/* DC Detect Reload */
/*TODO: Can it use load_config instead if another function??*/
/*TOD: Failsafe is needed or not ??*/
void tas256x_reload(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;
	/*To be used later*/
	(void)chn;

	pr_info("%s: chn %d\n", __func__, chn);
	p_tas256x->enable_irq(p_tas256x, false);

	ret = tas56x_software_reset(p_tas256x, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_load_init(p_tas256x, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_load_ctrl_values(p_tas256x, chn);
	if (ret < 0)
		goto end;
	ret |= tas256x_iv_sense_enable_set(p_tas256x, 1,
		chn);
	if (ret < 0)
		goto end;
	/* Since TDM & I2S Mode can have different width and slot settings
	 * It needs to be differentiated here
	 */
	if (p_tas256x->mn_fmt_mode == 2) {
		ret |= tas256x_set_tdm_rx_slot(p_tas256x, p_tas256x->mn_slots,
			p_tas256x->mn_rx_width);
		if (ret < 0)
			goto end;
		ret |= tas256x_set_tdm_tx_slot(p_tas256x, p_tas256x->mn_slots,
			p_tas256x->mn_rx_width);
		if (ret < 0)
			goto end;
	} else { /*I2S Mode*/
		ret |= tas256x_set_bitwidth(p_tas256x,
			p_tas256x->mn_rx_width, TAS256X_STREAM_PLAYBACK);
		if (ret < 0)
			goto end;
		ret |= tas256x_set_bitwidth(p_tas256x,
			p_tas256x->mn_rx_width, TAS256X_STREAM_CAPTURE);
		if (ret < 0)
			goto end;
	}
	ret |= tas256x_set_samplerate(p_tas256x, p_tas256x->mn_sampling_rate,
		chn);
	if (ret < 0)
		goto end;
	if (p_tas256x->devs[chn-1]->dac_power)
		ret |= tas256x_set_power_state(p_tas256x,
			p_tas256x->mn_power_state, chn);
	if (ret < 0)
		goto end;
end:
	p_tas256x->enable_irq(p_tas256x, true);
}

static int tas2558_specific(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	pr_info("%s: chn %d\n", __func__, chn);
	ret = tas256x_boost_volt_update(p_tas256x, DEVICE_TAS2558, chn);

	return ret;
}

static int tas2564_specific(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	pr_info("%s: chn %d\n", __func__, chn);
	ret = tas256x_boost_volt_update(p_tas256x, DEVICE_TAS2564, chn);

	return ret;
}

int tas256x_irq_work_func(struct tas256x_priv *p_tas256x)
{
	unsigned int nDevInt1Status = 0, nDevInt2Status = 0,
		nDevInt3Status = 0, nDevInt4Status = 0;
	int ret = -1;
	int i;
	int error_code_l = 0, error_code_r = 0; 
	enum channel chn = 0;

	pr_info("%s:\n", __func__);

	p_tas256x->enable_irq(p_tas256x, false);

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		if (p_tas256x->devs[i]->dac_power == 1)
			chn |= 1<<i;
	}

	if (chn & channel_left) {
		ret = tas256x_interrupt_read(p_tas256x,
			&nDevInt1Status, &nDevInt2Status, channel_left);
		error_code_l = tas256x_interrupt_determine(p_tas256x, channel_left,
			nDevInt1Status, nDevInt2Status);
		pr_info("%s: Ch-0 IRQ status : 0x%x 0x%x error_code 0x%0x\n",
			__func__, nDevInt1Status, nDevInt2Status, error_code_l);
	}

	if (chn & channel_right) {
		ret = tas256x_interrupt_read(p_tas256x,
			&nDevInt3Status, &nDevInt4Status, channel_right);
		error_code_r = tas256x_interrupt_determine(p_tas256x, channel_right,
			nDevInt3Status, nDevInt4Status);
		pr_info("%s: Ch-1 IRQ status : 0x%x 0x%x error_code 0x%0x\n",
			__func__, nDevInt3Status, nDevInt4Status, error_code_r);
	}

	if (error_code_l) {
		if (p_tas256x->mb_power_up && p_tas256x->devs[0]->dac_power) {
#if IS_ENABLED(CONFIG_TAS25XX_ALGO) && IS_ENABLED(CONFIG_TISA_SYSFS_INTF)
				tas25xx_algo_bump_oc_count(0, 0);
#endif
			ret = tas256x_interrupt_enable(p_tas256x, 0/*Disable*/,
				channel_left);
			if (p_tas256x->devs[0]->irq_count != 0) {
				if (time_after(jiffies, p_tas256x->devs[0]->jiffies +
					msecs_to_jiffies(TAS256X_IRQ_DET_TIMEOUT))) {
						p_tas256x->devs[0]->jiffies = jiffies;
						p_tas256x->devs[0]->irq_count = 0;
				} else {
					p_tas256x->devs[0]->irq_count++;
					if (p_tas256x->devs[0]->irq_count > TAS256X_IRQ_DET_CNT_LIMIT) {
						pr_info("%s: Channel-0 continuous interrupt detected %d, No Retry ...\n",
							__func__, p_tas256x->devs[0]->irq_count);
						tas256x_dump_regs(p_tas256x, channel_left);
						goto end;
					}
				}
			} else {
					p_tas256x->devs[0]->jiffies = jiffies;
					p_tas256x->devs[0]->irq_count = 1;
			}
			ret = tas256x_interrupt_clear(p_tas256x, channel_left);
			tas256x_irq_reload(p_tas256x, channel_left);
		}
	}

	if (error_code_r) {
		if (p_tas256x->mb_power_up && p_tas256x->devs[1]->dac_power) {
			ret = tas256x_interrupt_enable(p_tas256x, 0/*Disable*/,
				channel_right);
#if IS_ENABLED(CONFIG_TAS25XX_ALGO) && IS_ENABLED(CONFIG_TISA_SYSFS_INTF)
				tas25xx_algo_bump_oc_count(1, 0);
#endif
			if (p_tas256x->devs[1]->irq_count != 0) {
				if (time_after(jiffies, p_tas256x->devs[1]->jiffies +
					msecs_to_jiffies(TAS256X_IRQ_DET_TIMEOUT))) {
						p_tas256x->devs[1]->jiffies = jiffies;
						p_tas256x->devs[1]->irq_count = 0;
				} else {
					p_tas256x->devs[1]->irq_count++;
					if (p_tas256x->devs[1]->irq_count > TAS256X_IRQ_DET_CNT_LIMIT) {
						pr_info("%s: Channel-1 continuous interrupt detected %d, No Retry ...\n",
							__func__, p_tas256x->devs[1]->irq_count);
						tas256x_dump_regs(p_tas256x, channel_right);
						goto end;
					}
				}
			} else {
					p_tas256x->devs[1]->jiffies = jiffies;
					p_tas256x->devs[1]->irq_count = 1;
			}
			ret = tas256x_interrupt_clear(p_tas256x, channel_right);
			tas256x_irq_reload(p_tas256x, channel_right);
		}
	}

	/* False Interrupt Case */
	if((error_code_l == 0) && (error_code_r == 0)) {
		if (chn & channel_left)
			tas256x_dump_regs(p_tas256x, channel_left);
		if (chn & channel_right)
			tas256x_dump_regs(p_tas256x, channel_right);
	}
end:
	p_tas256x->enable_irq(p_tas256x, true);

	return ret;
}

int tas256x_init_work_func(struct tas256x_priv *p_tas256x, struct tas_device *dev_tas256x)
{		
	int chn = 0;
	int ret = 0;
	unsigned int nDevInt1Status = 0;

	/* Find the channel no from device id */
	chn = dev_tas256x->channel_no + 1;

	pr_err("%s: Channel-%d", __func__, chn-1);

	/* Increament OC Counter */
#if IS_ENABLED(CONFIG_TAS25XX_ALGO) && IS_ENABLED(CONFIG_TISA_SYSFS_INTF)
	tas25xx_algo_bump_oc_count(chn-1, 0);
#endif

	ret = tas256x_power_check(p_tas256x,
		&nDevInt1Status,
		chn);

	if (!nDevInt1Status) {
		if (p_tas256x->devs[chn-1]->counter == 20) {
			pr_err("%s: Critical Error; Device Shutdown, No Retry ...",
				__func__);
			tas256x_dump_regs(p_tas256x, chn);
			ret = tas256x_interrupt_clear(p_tas256x, chn);
			p_tas256x->devs[chn-1]->counter = 0;
			goto end;
		}
		pr_err("%s: Device(Ch-%d) not powered up, Trying Again",
			__func__, chn);
		/* To fix rcv silence issue */
		ret = tas256x_set_power_shutdown(p_tas256x, chn);
		/* Time to shutdown for the device */
		msleep(5);
		ret |= tas256x_set_power_state(p_tas256x,
			TAS256X_POWER_ACTIVE, chn);
		p_tas256x->devs[chn-1]->counter++;
	} else {
		p_tas256x->devs[chn-1]->counter = 0;
		/* Clear latched IRQ before enabling IRQ */
		ret = tas256x_interrupt_clear(p_tas256x, chn);
		/* Enable Interrupt */
		ret |= tas256x_interrupt_enable(p_tas256x, 1/* Enable */,
				chn);
		/* Set IRQ COunt to 0 */
		p_tas256x->devs[chn-1]->irq_count = 0;
	}

	if (p_tas256x->mn_err_code) {
		if (chn & channel_right) {
			if (p_tas256x->mn_err_code &
				(ERROR_DEVB_I2C_COMM))
					tas256x_failsafe(p_tas256x, chn);
		} else { /*Assumed Left*/
			if (p_tas256x->mn_err_code &
				(ERROR_DEVA_I2C_COMM))
					tas256x_failsafe(p_tas256x, chn);			
		}
	}

end:
	return ret;
}

int tas256x_dc_work_func(struct tas256x_priv *p_tas256x, int chn)
{
	pr_info("%s: ch %d\n", __func__, chn);
	tas256x_reload(p_tas256x, chn);

	return 0;
}

int tas256x_pwrup_work_func(struct tas256x_priv *p_tas256x, struct tas_device *dev_tas256x)
{
	int ret = -1;
	int chn = 0;

	/* Find the channel no from device id */
	chn = dev_tas256x->channel_no + 1;

	pr_info("%s: Channel-%d\n", __func__, chn-1);
	ret = tas256x_update_playback_volume(p_tas256x,
		p_tas256x->devs[chn-1]->dvc_pcm_bk, chn);

	return ret;
}

int tas256x_register_device(struct tas256x_priv  *p_tas256x)
{
	int ret = -1;
	int i = 0;

	pr_info("%s:\n", __func__);
	p_tas256x->read = tas256x_dev_read;
	p_tas256x->write = tas256x_dev_write;
	p_tas256x->bulk_read = tas256x_dev_bulk_read;
	p_tas256x->bulk_write = tas256x_dev_bulk_write;
	p_tas256x->update_bits = tas256x_dev_update_bits;

	tas256x_hard_reset(p_tas256x);

	pr_debug("Before SW reset\n");
	/* Reset the chip */
	ret = tas56x_software_reset(p_tas256x, channel_both);
	if (ret < 0) {
		pr_err("I2c fail, %d\n", ret);
		goto err;
	}

	pr_debug("After SW reset\n");

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		ret = tas56x_get_chipid(p_tas256x,
			&(p_tas256x->devs[i]->mn_chip_id),
			(i == 0) ? channel_left : channel_right);
		if (ret < 0)
			goto err;
		switch (p_tas256x->devs[i]->mn_chip_id) {
		case 0x10:
		case 0x20:
			pr_info("TAS2562 chip");
			p_tas256x->devs[i]->device_id = DEVICE_TAS2562;
			p_tas256x->devs[i]->dev_ops.tas_init = NULL;
			break;
		case 0x00:
			pr_info("TAS2564 chip");
			p_tas256x->devs[i]->device_id = DEVICE_TAS2564;
			p_tas256x->devs[i]->dev_ops.tas_init =
				tas2564_specific;
			break;
		default:
			pr_info("TAS2558 chip");
			p_tas256x->devs[i]->device_id = DEVICE_TAS2558;
			p_tas256x->devs[i]->dev_ops.tas_init =
				tas2558_specific;
			break;
		}
		p_tas256x->devs[i]->channel_no = i;
		ret |= tas256x_set_misc_config(p_tas256x, 0,
				(i == 0) ? channel_left : channel_right);
	}
err:
	return ret;
}

int tas256x_probe(struct tas256x_priv *p_tas256x)
{
	int ret = -1, i = 0;
#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
	struct linux_platform *plat_data =
			(struct linux_platform *) p_tas256x->platform_data;
#endif

	pr_info("%s:\n", __func__);

	for (i = 0; i < p_tas256x->mn_channels; i++) {
		ret = tas256x_load_init(p_tas256x, i+1);
		if (ret < 0)
			goto end;
	}
#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
	if (plat_data) {
		tas_smartamp_add_algo_controls(plat_data->codec, plat_data->dev,
			p_tas256x->mn_channels);
		/*Send IV Vbat format but don't update to algo yet*/
		tas25xx_set_iv_bit_fomat(p_tas256x->mn_iv_width,
			p_tas256x->mn_vbat, 0);
	}
#endif
	/* REGBIN related */
	/* Ignore the return */
	tas256x_load_container(p_tas256x);
	pr_info("%s Bin file loading requested\n", __func__);
end:
	return ret;
}

void tas256x_remove(struct tas256x_priv *p_tas256x)
{
#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
	struct linux_platform *plat_data =
			(struct linux_platform *) p_tas256x->platform_data;
	if (plat_data)
		tas_smartamp_remove_algo_controls(plat_data->codec);
#else
	/*Ignore argument*/
	(void)p_tas256x;
#endif
	/* REGBIN related */
	tas256x_config_info_remove(p_tas256x);
}

int tas256x_pre_powerup_codec_update(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	/* To Support different TX config between Stereo Left/Right Only
	 * and full stereo its moved from tas256x_load_init to here
	 * as former function is called only at boot time.
	 */
	if ((p_tas256x->mn_channels == 2) &&
		((p_tas256x->devs[0]->dac_power == 1) ||
		(p_tas256x->devs[1]->dac_power == 1))) {
		ret = tas256x_set_tx_config(p_tas256x,
			0/*Ignored*/, channel_both);
	} else {
		ret = tas256x_set_tx_config(p_tas256x,
			0/*Ignored*/, chn);
	}
	if (p_tas256x->curr_mn_iv_width == 8)
		ret |= tas256x_enable_emphasis_filter(p_tas256x,
			chn,
			p_tas256x->mn_sampling_rate);
	ret |= tas256x_iv_sense_enable_set(p_tas256x, 1, chn);
	
	/* Move to Physical Layer */
#if IS_ENABLED(CONFIG_TAS26XX_HAPTICS)
	if ((p_tas256x->mn_haptics) && (p_tas256x->mn_channels == 2))
			&& (chn & channel_right)) {
		ret |= p_tas256x->update_bits(p_tas256x, channel_right,
				TAS256X_INTERRUPTCONFIGURATION, 0xff, 0xd9);
		ret = |= p_tas256x->update_bits(p_tas256x, channel_right,
				DRV2634_BOOST_CFG1, 0xff, 0xb4);
		ret |= p_tas256x->update_bits(p_tas256x, channel_right,
				DRV2634_NEW_04, 0xff, 0xa0);
	}
#endif
	/* REGBIN related */
	/*set p_tas256x->profile_cfg_id by tinymix*/
	tas256x_select_cfg_blk(p_tas256x,
		p_tas256x->profile_cfg_id,
		TAS256X_BIN_BLK_PRE_POWER_UP);

	return ret;
}

int tas256x_pre_powerup_algo_update(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = 0;

	/* Channel No not used */
	(void)chn;

	if (p_tas256x->mb_power_up == false) {
#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
		if (p_tas256x->algo_bypass == 0) {
			tas25xx_algo_enable_common_controls(1);
#if IS_ENABLED(CONFIG_TAS25XX_CALIB_VAL_BIG) || IS_ENABLED(CONFIG_TISA_DEBUGFS_INTF)
			tas25xx_send_channel_mapping();
			tas25xx_send_algo_calibration();
			tas25xx_set_iv_bit_fomat(
				p_tas256x->curr_mn_iv_width,
				p_tas256x->curr_mn_vbat, 1);
#endif
		}
#endif /* CONFIG_TAS25XX_ALGO */
	}

	return ret;
}

int tas256x_post_powerup_codec_update(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	ret = tas256x_update_icn_hysterisis(p_tas256x,
		p_tas256x->devs[chn-1]->icn_hyst,
		p_tas256x->mn_sampling_rate, chn);
	ret |= tas256x_update_icn_threshold(p_tas256x,
		p_tas256x->devs[chn-1]->icn_thr,
		chn);
	/* REGBIN related */
	/*set p_tas256x->profile_cfg_id by tinymix*/
	tas256x_select_cfg_blk(p_tas256x,
		p_tas256x->profile_cfg_id,
		TAS256X_BIN_BLK_POST_POWER_UP);

	return ret;
}

int tas256x_post_shutdown_algo_update(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = 0;
	
	/* Channel No not used */
	(void)chn;

#if IS_ENABLED(CONFIG_TAS25XX_ALGO)
	if (p_tas256x->algo_bypass == 0) {
		tas25xx_algo_enable_common_controls(0);
#if IS_ENABLED(CONFIG_TAS25XX_CALIB_VAL_BIG)
		tas25xx_update_big_data();
#endif /* CONFIG_TAS25XX_CALIB_VAL_BIG */
#if IS_ENABLED(CONFIG_TISA_KBIN_INTF)
		tas25xx_algo_set_inactive();
#endif /* CONFIG_TISA_KBIN_INTF */
	}
#if IS_ENABLED(CONFIG_TISA_SYSFS_INTF)
	tas25xx_algo_bump_oc_count(chn-1, 1);
#endif /*CONFIG_TISA_SYSFS_INTF*/
#endif /*CONFIG_TAS25XX_ALGO*/

	return ret;
}

int tas256x_pre_shutdown_codec_update(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	/*Mask interrupt for TDM*/
	ret = tas256x_interrupt_enable(p_tas256x, 0/*Disable*/, chn);
	/* Cancel IRQ work routine */
	p_tas256x->cancel_irq_work(p_tas256x);
	/* REGBIN related */
	/*set p_tas256x->profile_cfg_id by tinymix*/
	tas256x_select_cfg_blk(p_tas256x, p_tas256x->profile_cfg_id,
		TAS256X_BIN_BLK_PRE_SHUTDOWN);

	return ret;
}

int tas256x_post_shutdown_codec_update(struct tas256x_priv *p_tas256x, int chn)
{
	int ret = -1;

	ret = tas256x_iv_sense_enable_set(p_tas256x, 0, chn);
	ret |= tas256x_ivvbat_slot_disable(p_tas256x, p_tas256x->curr_mn_vbat,
			chn);
	/* REGBIN related */
	/*set p_tas256x->profile_cfg_id by tinymix*/
	tas256x_select_cfg_blk(p_tas256x, p_tas256x->profile_cfg_id,
		TAS256X_BIN_BLK_POST_SHUTDOWN);

	return ret;
}

int tas256x_set_power_state(struct tas256x_priv *p_tas256x,
			int state, int chn)
{
	int ret = -1, i = 0;

	pr_info("%s: state %d\n", __func__, state);

	if ((p_tas256x->mb_mute) && (state == TAS256X_POWER_ACTIVE))
		state = TAS256X_POWER_MUTE;

	switch (state) {
	case TAS256X_POWER_ACTIVE:
		pr_info("%s:%u:chn=%d,dac_power=%d\n", __func__, __LINE__, chn-1, p_tas256x->devs[chn-1]->dac_power);
		if(p_tas256x->devs[chn-1]->dac_power) {
			/* All Register Updates Before Power Up */
			ret = tas256x_pre_powerup_codec_update(p_tas256x, chn);

			/* All Algo Set Parameters Before Power Up */
			ret |= tas256x_pre_powerup_algo_update(p_tas256x, chn);

			/* Power Up Device Finally -
			* No Need to check if already Powered Up */
			/* Added for "handset" -> "voip-speaker-lowpower" usecase fix */
			if (p_tas256x->devs[chn-1]->pwrup_delay) {
				p_tas256x->devs[chn-1]->dvc_pcm_bk =
					p_tas256x->devs[chn-1]->dvc_pcm;
				/* Set -110dB */
				ret |= tas256x_update_playback_volume(p_tas256x,
					0, chn);
				p_tas256x->schedule_pwrup_work(p_tas256x, chn);
			}
			ret |= tas256x_set_power_up(p_tas256x, chn);

			ret |= tas256x_post_powerup_codec_update(p_tas256x, chn);

			p_tas256x->enable_irq(p_tas256x, true);

			p_tas256x->mb_power_up = true;
			p_tas256x->mn_power_state = TAS256X_POWER_ACTIVE;
			p_tas256x->schedule_init_work(p_tas256x, chn);
		}
		break;
	case TAS256X_POWER_MUTE:
		ret = tas256x_set_power_mute(p_tas256x, chn);
			p_tas256x->mb_power_up = true;
			p_tas256x->mn_power_state = TAS256X_POWER_MUTE;

		/*Mask interrupt for TDM*/
		ret |= tas256x_interrupt_enable(p_tas256x, 0/*Disable*/,
			chn);
		break;

	case TAS256X_POWER_SHUTDOWN:
		p_tas256x->mb_power_up = false;
		for (i = 0; i < p_tas256x->mn_channels; i++) {
			p_tas256x->cancel_init_work(p_tas256x, i+1);
			if (p_tas256x->devs[i]->device_id == DEVICE_TAS2564) {
				if (p_tas256x->devs[i]->dac_power == 1) {
					/* All Register Updates Before ShutDown */
					ret = tas256x_pre_shutdown_codec_update(
							p_tas256x, i+1);
					/* To Fix Leakage Current Issue of TAS2564 */
					ret |= tas256x_set_power_mute(
							p_tas256x, i+1);
					/* All Register Updates After ShutDown */
					ret |= tas256x_post_shutdown_codec_update(
							p_tas256x, i+1);
				}
			} else {
				if (p_tas256x->devs[i]->dac_power == 1) {
					/* All Register Updates Before ShutDown */
					ret = tas256x_pre_shutdown_codec_update(
							p_tas256x, i+1);
					/* Shutdown */
					ret |= tas256x_set_power_shutdown(
							p_tas256x, i+1);
					/* All Register Updates After ShutDown */
					ret |= tas256x_post_shutdown_codec_update(
							p_tas256x, i+1);
				}
			}
			if (p_tas256x->cancel_pwrup_work(p_tas256x, i+1))
				ret |= tas256x_update_playback_volume(p_tas256x,
					p_tas256x->devs[i]->dvc_pcm_bk, i+1);
		}
		p_tas256x->mn_power_state = TAS256X_POWER_SHUTDOWN;
		p_tas256x->enable_irq(p_tas256x, false);
		/*Device Shutdown need 16ms after shutdown writes are made*/
	        msleep(16);
		/* Update Algorithm; Channel No is ignored */
		ret |= tas256x_post_shutdown_algo_update(p_tas256x, chn);
		break;
	default:
		pr_err("wrong power state setting %d\n",
				state);
	}

	return ret;
}

/* In Order to fix the issue of "24bit Bitwidth and 8bit IV Width and no VBat"
 * Function is redesigned to support all the (1)Bitwidth (2) IVwidth and (3)VBat
 * Configurations
 */

int tas256x_iv_vbat_slot_config(struct tas256x_priv *p_tas256x,
	int mn_slot_width)
{
	int ret = -1;

	pr_info("%s: mn_slot_width %d\n", __func__, mn_slot_width);

	if (p_tas256x->mn_fmt_mode == 2) { /*TDM Mode*/
		if (p_tas256x->mn_channels == 2) {
			if (mn_slot_width == 16) {
				if (p_tas256x->mn_vbat == 1) {
					ret =
						tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT1,
							TX_SLOT0);
					ret |=
						tas256x_set_iv_slot(p_tas256x,
							channel_right, TX_SLOT5,
							TX_SLOT4);
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT3);
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_right,
							TX_SLOT7);
					p_tas256x->curr_mn_iv_width = 12;
					p_tas256x->curr_mn_vbat = 1;
				} else {
					ret =
						tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT2,
							TX_SLOT0);
					ret |=
						tas256x_set_iv_slot(p_tas256x,
							channel_right, TX_SLOT6,
							TX_SLOT4);
					p_tas256x->curr_mn_iv_width = 16;
					p_tas256x->curr_mn_vbat = 0;
				}

			} else if (mn_slot_width == 24) {
				ret =
					tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT4,
						TX_SLOT0);
				ret |= tas256x_set_iv_slot(p_tas256x,
						channel_right, TX_SLOTc,
						TX_SLOT8);
				p_tas256x->curr_mn_vbat = 0;
				if (p_tas256x->mn_vbat == 1) {
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT6);
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_right,
+							TX_SLOTe);
					p_tas256x->curr_mn_vbat = 1;
				}
				p_tas256x->curr_mn_iv_width = 16;

			} else { /*Assumed 32bit*/
				ret =
					tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT4,
						TX_SLOT0);
				ret |= tas256x_set_iv_slot(p_tas256x,
						channel_right, TX_SLOTc,
						TX_SLOT8);
				p_tas256x->curr_mn_vbat = 0;
				if (p_tas256x->mn_vbat == 1) {
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT6);
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_right,
							TX_SLOTe);
					p_tas256x->curr_mn_vbat = 1;
				}
				p_tas256x->curr_mn_iv_width = 16;
			}
		} else { /*Assumed Mono Channels*/
			if (mn_slot_width == 16) {
				if (p_tas256x->mn_vbat == 1) {
					ret =
						tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT1,
							TX_SLOT0);
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT3);
					p_tas256x->curr_mn_iv_width = 12;
					p_tas256x->curr_mn_vbat = 1;
				} else {
					ret =
						tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT2,
							TX_SLOT0);
					p_tas256x->curr_mn_iv_width = 16;
					p_tas256x->curr_mn_vbat = 1;
				}
			} else if (mn_slot_width == 24) {
				ret =
					tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT4,
						TX_SLOT0);
				p_tas256x->curr_mn_iv_width = 16;
				p_tas256x->curr_mn_vbat = 0;
				if (p_tas256x->mn_vbat == 1) {
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT6);
					p_tas256x->curr_mn_vbat = 1;
				}
			} else { /*Assumed 32bit*/
				ret =
					tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT4,
						TX_SLOT0);
				p_tas256x->curr_mn_iv_width = 16;
				p_tas256x->curr_mn_vbat = 0;
				if (p_tas256x->mn_vbat == 1) {
					ret |=
						tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT6);
					p_tas256x->curr_mn_vbat = 1;
				}
			}
		}
	} else { /*I2S Mode*/
		if (mn_slot_width == 16) {
			if (p_tas256x->mn_channels == 2) {
				switch (p_tas256x->mn_iv_width) {
				case 8:
				case 12:
				case 16:
				default:
					ret = tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT1, TX_SLOT0);
					ret |= tas256x_set_iv_slot(p_tas256x,
						channel_right, TX_SLOT3, TX_SLOT2);
					p_tas256x->curr_mn_iv_width = 8;
					p_tas256x->curr_mn_vbat = 0;
					break;
				}
			} else {
				switch (p_tas256x->mn_iv_width) {
				case 8:
					ret = tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT1, TX_SLOT0);
					p_tas256x->curr_mn_iv_width = 8;
					if (p_tas256x->mn_vbat == 1) {
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT2);
						p_tas256x->curr_mn_vbat = 1;
					} else {
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				case 12:
					p_tas256x->curr_mn_iv_width = 12;
					if (p_tas256x->mn_vbat == 1) {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT1, TX_SLOT0);
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT3);
						p_tas256x->curr_mn_vbat = 1;
					} else {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT2, TX_SLOT0);
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				case 16:
				default:
					if (p_tas256x->mn_vbat == 1) {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT1, TX_SLOT0);
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT3);
						p_tas256x->curr_mn_iv_width = 12;
						p_tas256x->curr_mn_vbat = 1;
					} else {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT2, TX_SLOT0);
						p_tas256x->curr_mn_iv_width = 16;
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				}
			}
		} else { /* mn_slot_width == 24 or mn_slot_width == 32 */
			if (p_tas256x->mn_channels == 2) {
				switch (p_tas256x->mn_iv_width) {
				case 8:
					ret = tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT1, TX_SLOT0);
					ret |= tas256x_set_iv_slot(p_tas256x,
						channel_right, TX_SLOT5, TX_SLOT4);
					p_tas256x->curr_mn_iv_width = 8;
					if (p_tas256x->mn_vbat == 1) {
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT2);
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_right, TX_SLOT6);
						p_tas256x->curr_mn_vbat = 1;
					} else {
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				case 12:
					if (p_tas256x->mn_vbat == 1) {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT1, TX_SLOT0);
						ret |= tas256x_set_iv_slot(p_tas256x,
							channel_right, TX_SLOT5, TX_SLOT4);
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT2);
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_right, TX_SLOT6);
						p_tas256x->curr_mn_iv_width = 8;
						p_tas256x->curr_mn_vbat = 1;
					} else {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT1, TX_SLOT0);
						ret |= tas256x_set_iv_slot(p_tas256x,
							channel_right, TX_SLOT5, TX_SLOT4);
						p_tas256x->curr_mn_iv_width = 12;
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				case 16:
				default:
					if (p_tas256x->mn_vbat == 1) {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT1, TX_SLOT0);
						ret |= tas256x_set_iv_slot(p_tas256x,
							channel_right, TX_SLOT5, TX_SLOT4);
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT2);
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_right, TX_SLOT6);
						p_tas256x->curr_mn_iv_width = 8;
						p_tas256x->curr_mn_vbat = 1;
					} else {
						ret = tas256x_set_iv_slot(p_tas256x,
							channel_left, TX_SLOT2, TX_SLOT0);
						ret |= tas256x_set_iv_slot(p_tas256x,
							channel_right, TX_SLOT6, TX_SLOT4);
						p_tas256x->curr_mn_iv_width = 16;
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				}
			} else { /* Assumed Mono */
				switch (p_tas256x->mn_iv_width) {
				case 8:
					ret = tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT1, TX_SLOT0);
					p_tas256x->curr_mn_iv_width = 8;
					if (p_tas256x->mn_vbat == 1) {
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT2);
						p_tas256x->curr_mn_vbat = 1;
					} else {
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				case 12:
					ret = tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT1, TX_SLOT0);
					p_tas256x->curr_mn_iv_width = 12;
					if (p_tas256x->mn_vbat == 1) {
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT4);
						p_tas256x->curr_mn_vbat = 1;
					} else {
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				case 16:
				default:
					ret = tas256x_set_iv_slot(p_tas256x,
						channel_left, TX_SLOT4, TX_SLOT0);
					p_tas256x->curr_mn_iv_width = 16;
					if (p_tas256x->mn_vbat == 1) {
						ret |= tas256x_set_vbat_slot(p_tas256x,
							channel_left, TX_SLOT6);
						p_tas256x->curr_mn_vbat = 1;
					} else {
						p_tas256x->curr_mn_vbat = 0;
					}
					break;
				}
			}
		}
	}

	if (ret == 0)
		p_tas256x->mn_tx_slot_width = mn_slot_width;

	return ret;
}

/* tas256x_set_bitwidth function is redesigned to accomodate change in
 * tas256x_iv_vbat_slot_config()
 */
int tas256x_set_bitwidth(struct tas256x_priv *p_tas256x,
	int bitwidth, int stream)
{
	int ret = -1;

	pr_info("%s: bitwidth %d stream %d\n", __func__, bitwidth, stream);

	if (stream == TAS256X_STREAM_PLAYBACK) {
		ret = tas256x_rx_set_bitwidth(p_tas256x, bitwidth,
			channel_both);
		ret |= tas256x_rx_set_slot_len(p_tas256x, bitwidth,
			channel_both);
	} else { /*stream == TAS256X_STREAM_CAPTURE*/
		ret = tas256x_iv_vbat_slot_config(p_tas256x,
				bitwidth);
		ret |= tas256x_iv_bitwidth_config(p_tas256x,
			p_tas256x->curr_mn_iv_width, channel_both);
	}

	return ret;
}

/* tas256x_set_tdm_rx_slot function is redesigned to accomodate change in
 * tas256x_iv_vbat_slot_config()
 */
int tas256x_set_tdm_rx_slot(struct tas256x_priv *p_tas256x,
	int slots, int slot_width)
{
	int ret = -1;

	if (((p_tas256x->mn_channels == 1) && (slots < 1)) ||
		((p_tas256x->mn_channels == 2) && (slots < 2))) {
		pr_err("Invalid Slots %d\n", slots);
		return ret;
	}
	p_tas256x->mn_slots = slots;

	if ((slot_width != 16) &&
		(slot_width != 24) &&
		(slot_width != 32)) {
		pr_err("Unsupported slot width %d\n", slot_width);
		return ret;
	}

	ret = tas256x_rx_set_slot_len(p_tas256x, slot_width, channel_both);

	ret |= tas256x_rx_set_bitwidth(p_tas256x,
			slot_width, channel_both);

	/*Enable Auto Detect of Sample Rate */
	ret |= tas256x_set_auto_detect_clock(p_tas256x,
			1, channel_both);

	/*Enable Clock Config*/
	ret |= tas256x_set_clock_config(p_tas256x, 0/*Ignored*/,
			channel_both);

	return ret;
}

/* tas256x_set_tdm_tx_slot function is redesigned to accomodate change in
 * tas256x_iv_vbat_slot_config()
 */
int tas256x_set_tdm_tx_slot(struct tas256x_priv *p_tas256x,
	int slots, int slot_width)
{
	int ret = -1;

	if ((slot_width != 16) &&
		(slot_width != 24) &&
		(slot_width != 32)) {
		pr_err("Unsupported slot width %d\n", slot_width);
		return ret;
	}

	if (((p_tas256x->mn_channels == 1) && (slots < 2)) ||
		((p_tas256x->mn_channels == 2) && (slots < 4))) {
		pr_err("Invalid Slots %d\n", slots);
		return ret;
	}
	p_tas256x->mn_slots = slots;

	ret = tas256x_iv_vbat_slot_config(p_tas256x, slot_width);

	ret |= tas256x_iv_bitwidth_config(p_tas256x,
		p_tas256x->curr_mn_iv_width, channel_both);

	return ret;
}


