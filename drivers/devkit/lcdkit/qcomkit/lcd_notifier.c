#include <linux/input/qpnp-power-on.h>
#include <linux/export.h>
#include <linux/notifier.h>
#include "lcd_defs.h"
#include "lcd_panel.h"
#include "lcd_notifier.h"

static int pwrkey_press_event_notifier(struct notifier_block *pwrkey_event_nb,
	unsigned long event, void *data)
{
	struct notifier_block *p = NULL;
	struct panel_info *pinfo = get_panel_info(0);
	void *pd = NULL;
	unsigned int time;
	p = pwrkey_event_nb;
	pd = data;

	if (!pinfo)
		return LCD_FAIL;
	time = pinfo->pwrkey_press.timer_val;
	switch (event) {
	case PON_PRESS_KEY_S1:
		pinfo->pwrkey_press.long_press_flag = true;
		LCD_ERR("PON_PRESS_KEY_S1 time:%d\n", time);
		schedule_delayed_work(&pinfo->pwrkey_press.pf_work,
			msecs_to_jiffies(time));
		break;
	case PON_PRESS_KEY_UP:
		if (pinfo->pwrkey_press.long_press_flag == false) {
			LCD_INFO("pwrkey_press_event_notifier power_key up\n");
			break;
		}
		pinfo->pwrkey_press.long_press_flag = false;
		cancel_delayed_work_sync(&pinfo->pwrkey_press.pf_work);
		break;
	default:
		break;
	}
	return LCD_OK;
}

static void power_off_work(struct work_struct *work)
{
	int ret;
	struct platform_ops *plat_ops = NULL;
	struct panel_info *pinfo = get_panel_info(0);

	if (!pinfo)
		return;
	plat_ops = pinfo->panel->pdata->plat_ops;
	if(plat_ops->force_power_off) {
		ret = plat_ops->force_power_off(pinfo->panel);
	}
}

void lcd_register_power_key_notify(void)
{
	int ret;
	struct delayed_work *p_work = NULL;
	struct panel_info *pinfo = get_panel_info(0);

	if (!pinfo)
		return;
	p_work = &pinfo->pwrkey_press.pf_work;
	INIT_DELAYED_WORK(p_work, power_off_work);
	pinfo->pwrkey_press.nb.notifier_call =
		pwrkey_press_event_notifier;
	ret = lcd_powerkey_register_notifier(
		&pinfo->pwrkey_press.nb);
	if (ret < 0)
		LCD_ERR("register power_key notifier failed!\n");
}


static ATOMIC_NOTIFIER_HEAD(lcd_powerkey_notifier_list);

int lcd_powerkey_register_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_register(&lcd_powerkey_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(lcd_powerkey_register_notifier);

int lcd_powerkey_unregister_notifier(struct notifier_block *nb)
{
	return atomic_notifier_chain_unregister(
		&lcd_powerkey_notifier_list, nb);
}
EXPORT_SYMBOL_GPL(lcd_powerkey_unregister_notifier);

int lcd_call_powerkey_notifiers(unsigned long val, void *v)
{
	return atomic_notifier_call_chain(&lcd_powerkey_notifier_list,
		val, v);
}

EXPORT_SYMBOL_GPL(lcd_call_powerkey_notifiers);

