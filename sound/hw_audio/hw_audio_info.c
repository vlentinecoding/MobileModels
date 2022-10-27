/*
 * hw_audio_info.c
 *
 * hw audio priv driver
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#include "hw_audio_info.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/string.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/version.h>
#include <linux/regulator/consumer.h>
#include <linux/pinctrl/consumer.h>
#include <linux/power/huawei_battery.h>
#include <sound/hw_audio/hw_audio_interface.h>

const char *smartpa_name[SMARTPA_TYPE_MAX] = { "none", "cs35lxx", "aw882xx", "tfa98xx", "tas256x", "cs35lxxa" };
static struct hw_audio_info g_hw_audio_priv;

static int kcontrol_value_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol, int value)
{
	if (kcontrol == NULL || ucontrol == NULL) {
		pr_err("%s: input pointer is null\n", __func__);
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] = value;
	return 0;
}

static int kcontrol_value_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol, int *value)
{
	if (kcontrol == NULL || ucontrol == NULL || value == NULL) {
		pr_err("%s: input pointer is null\n", __func__);
		return -EINVAL;
	}

	*value = ucontrol->value.integer.value[0];
	return 0;
}

static int hac_gpio_switch(int gpio, int value)
{
	if (!gpio_is_valid(gpio)) {
		pr_err("%s: invalid gpio:%d\n", __func__, gpio);
		return -EINVAL;
	}

	gpio_set_value(gpio, value);
	pr_info("%s:set gpio(%d):%d\n", __func__, gpio, value);
	return 0;
}

static struct hw_audio_info *get_audio_info_priv(void)
{
    return &g_hw_audio_priv;
}

int hac_switch_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	return kcontrol_value_get(kcontrol, ucontrol, priv->hac_switch);
}
EXPORT_SYMBOL_GPL(hac_switch_get);

int hac_switch_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	int value;
	struct hw_audio_info *priv = get_audio_info_priv();

	ret = kcontrol_value_put(kcontrol, ucontrol, &value);
	if (ret != 0)
		return ret;

	priv->hac_switch = value;
	return hac_gpio_switch(priv->hac_gpio, value);
}
EXPORT_SYMBOL_GPL(hac_switch_put);

int simple_pa_mode_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	return kcontrol_value_get(kcontrol, ucontrol, priv->pa_mode);
}
EXPORT_SYMBOL_GPL(simple_pa_mode_get);

int simple_pa_mode_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	int value;
	struct hw_audio_info *priv = get_audio_info_priv();

	ret = kcontrol_value_put(kcontrol, ucontrol, &value);
	if (ret != 0)
		return ret;

	if (value < 0 || value >= ARRAY_SIZE(g_simple_pa_mode_text)) {
		pr_err("%s: pa mode is invalid: %d\n", __func__, value);
		return -EINVAL;
	}

	priv->pa_mode = (priv->pa_type == SIMPLE_PA_TI) ?
		SIMPLE_PA_DEFAULT_MODE : value;
	pr_info("%s: pa mode %d\n", __func__, priv->pa_mode);
	return 0;
}
EXPORT_SYMBOL_GPL(simple_pa_mode_put);

int hw_simple_pa_power_set(int gpio, int value)
{
	int i;
	struct hw_audio_info *priv = get_audio_info_priv();

	if (!gpio_is_valid(gpio)) {
		pr_err("%s: Invalid gpio: %d\n", __func__, gpio);
		return -EINVAL;
	}

	if (value == GPIO_PULL_UP) {
		gpio_set_value(gpio, GPIO_PULL_DOWN);
		msleep(1);
		for (i = 0; i < priv->pa_mode; i++) {
			gpio_set_value(gpio, GPIO_PULL_DOWN);
			udelay(2);
			gpio_set_value(gpio, GPIO_PULL_UP);
			udelay(2);
		}
	} else {
		gpio_set_value(gpio, GPIO_PULL_DOWN);
	}
	return 0;
}

int hphr_simple_pa_enable(bool enable)
{
	int ret = 0;
	struct hw_audio_info *priv = get_audio_info_priv();

	if (enable)
		ret = hw_simple_pa_power_set(priv->pa_gpio, GPIO_PULL_UP);
	else
		ret = hw_simple_pa_power_set(priv->pa_gpio, GPIO_PULL_DOWN);

	if (ret == 0)
		mdelay(1);

	return ret;
}
EXPORT_SYMBOL(hphr_simple_pa_enable);

int simple_pa_switch_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	return kcontrol_value_get(kcontrol, ucontrol, priv->pa_switch);
}
EXPORT_SYMBOL_GPL(simple_pa_switch_get);

int simple_pa_switch_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	int value;
	struct hw_audio_info *priv = get_audio_info_priv();

	ret = kcontrol_value_put(kcontrol, ucontrol, &value);
	if (ret != 0)
		return ret;

	priv->pa_switch = value;
	pr_info("%s: pa switch %d\n", __func__, value);
	return hw_simple_pa_power_set(priv->pa_gpio, value);
}
EXPORT_SYMBOL_GPL(simple_pa_switch_put);

int audio_get_battery_info(enum battery_prop_type type, int *value)
{
	struct power_supply *psy = NULL;
	union power_supply_propval val = {0, };
	int rc;

	if (value == NULL)
		return -EINVAL;

	psy = power_supply_get_by_name("battery");
	if (!psy) {
		pr_err("%s: power supply battery not exist\n", __func__);
		return -EINVAL;
	}

	switch (type) {
	case BATT_TEMP:
		rc = get_prop_from_psy(psy, POWER_SUPPLY_PROP_TEMP, &val);
		break;
	case BATT_CAP:
		rc = get_prop_from_psy(psy, POWER_SUPPLY_PROP_CAPACITY, &val);
		break;
	case BATT_VOL:
		rc = get_prop_from_psy(psy, POWER_SUPPLY_PROP_VOLTAGE_NOW, &val);
		break;
	default:
		pr_err("%s: invalid param\n", __func__);
		return -EINVAL;
	}

	if (rc < 0) {
		pr_err("%s: get cat prop from psy fail\n", __func__);
		return -EINVAL;
	}

	*value = val.intval;
	return 0;
}
EXPORT_SYMBOL_GPL(audio_get_battery_info);

bool is_mic_differential_mode(void)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	return priv->mic_differential_mode;
}
EXPORT_SYMBOL_GPL(is_mic_differential_mode);

static int gpio_output_init(int gpio, const char *label, int value)
{
	int ret;

	if (!gpio_is_valid(gpio)) {
		pr_err("%s: invalid gpio %d\n", __func__, gpio);
		return -EINVAL;
	}

	ret = gpio_request(gpio, label);
	if (ret != 0) {
		pr_err("%s: request gpio %d failed %d\n", __func__, gpio, ret);
		return -EINVAL;
	}

	gpio_direction_output(gpio, value);
	return 0;
}

static void hw_hac_init(struct hw_audio_info *priv, struct device_node *np)
{
	int gpio;

	pr_info("%s: enter\n", __func__);
	gpio = of_get_named_gpio_flags(np, "hac-gpio", 0, NULL);
	if (gpio < 0)
		return;

	if (gpio_output_init(gpio, "hac_gpio", GPIO_PULL_DOWN) != 0)
		return;

	priv->hac_gpio = gpio;
	pr_info("%s: init hac gpio(%d) success\n", __func__, gpio);
}

static void get_simple_pa_type(struct hw_audio_info *priv,
	struct platform_device *pdev, int gpio)
{
	int aw_pa_value;
	struct device_node *np = pdev->dev.of_node;
	struct pinctrl *pinctrl = NULL;
	struct pinctrl_state *pin_state = NULL;

	if (!of_property_read_bool(np, "need_identify_pa_vendor")) {
		pr_info("%s: not need identify pa type\n", __func__);
		goto exit;
	}

	if (of_property_read_u32(np, "simple_pa_type_aw", &aw_pa_value) != 0) {
		pr_err("%s: simple_pa_type_aw not config\n", __func__);
		goto exit;
	}

	pinctrl = devm_pinctrl_get(&pdev->dev);
	if (pinctrl == NULL) {
		pr_err("%s: get pinctrl fail\n", __func__);
		goto exit;
	}
	pin_state = pinctrl_lookup_state(pinctrl, "ext_pa_default");
	if (IS_ERR(pin_state)) {
		pr_err("%s: pinctrl_lookup_state fail %d\n",
			__func__, PTR_ERR(pin_state));
		goto exit;
	}
	if (pinctrl_select_state(pinctrl, pin_state) != 0) {
		pr_err("%s: pinctrl_select_state fail\n", __func__);
		goto exit;
	}

	if (gpio_direction_input(gpio) != 0) {
		pr_err("%s: set gpio to input direction fail\n", __func__);
		goto exit;
	}
	msleep(1);
	if (gpio_get_value(gpio) == aw_pa_value)
		priv->pa_type = SIMPLE_PA_AWINIC;
	else
		priv->pa_type = SIMPLE_PA_TI;

exit:
	pr_info("%s: pa_type is %s\n", __func__,
		priv->pa_type == SIMPLE_PA_AWINIC ? "aw" : "ti");
}

static void simple_pa_init(struct hw_audio_info *priv,
	struct platform_device *pdev)
{
	int gpio;
	struct device_node *np = pdev->dev.of_node;

	gpio = of_get_named_gpio(np, "spk-ext-simple-pa", 0);
	if (gpio < 0) {
		pr_info("%s: missing spk-ext-simple-pa in dt node\n", __func__);
		return;
	}

	if (!gpio_is_valid(gpio)) {
		pr_err("%s: Invalid ext spk gpio: %d", __func__, gpio);
		return;
	}

	if (gpio_request(gpio, "simple_pa_gpio") != 0) {
		pr_err("%s: request gpio %d fail\n", __func__, gpio);
		return;
	}

	get_simple_pa_type(priv, pdev, gpio);
	gpio_direction_output(gpio, GPIO_PULL_DOWN);
	priv->pa_gpio = gpio;
	pr_info("%s: init simple pa gpio(%d) success\n", __func__, gpio);
}

static void hw_audio_info_priv(struct hw_audio_info *priv)
{
	priv->audio_prop = 0;
	priv->pa_type = SIMPLE_PA_TI;
	priv->pa_mode = SIMPLE_PA_DEFAULT_MODE;
	priv->pa_gpio = INVALID_GPIO;
	priv->pa_switch = SWITCH_OFF;
	priv->hac_gpio = INVALID_GPIO;
	priv->hac_switch = SWITCH_OFF;
	priv->mic_differential_mode = false;
	priv->smartpa_info.is_init_smartpa_type = false;
	priv->smartpa_info.smartpa_codecs_count = 0;
	priv->smartpa_info.is_load_firmware_by_product_name = false;
	mutex_init(&priv->smartpa_info.pa_probe_lock);
	priv->is_extpa_to_hphr = false;
}

static void hw_audio_property_init(struct hw_audio_info *priv,
	struct device_node *of_node)
{
	int i;

	if (of_node == NULL) {
		pr_err("hw_audio: %s failed,of_node is NULL\n", __func__);
		return;
	}

	for (i = 0; i < ARRAY_SIZE(audio_prop_table); i++) {
		if (of_property_read_bool(of_node, audio_prop_table[i].key))
			priv->audio_prop |= audio_prop_table[i].value;
	}
	if ((priv->audio_prop & audio_prop_table[0].value) == 0)
		pr_err("hw_audio: check mic config, no master mic found\n");
}

static void set_product_identifier(struct device_node *np, char *buf, int len)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	if (!of_property_read_bool(np, "separate_param_for_different_pa")) {
		pr_info("%s: not need to separate audio param for different pa\n", __func__);
		return;
	}

	if (of_property_read_bool(np, "need_identify_pa_vendor")) {
		if (len <= strlen(buf)) {
			pr_err("%s: buffer len not match\n", __func__);
			return;
		}
		switch (priv->pa_type) {
		case SIMPLE_PA_AWINIC:
			snprintf(buf + strlen(buf), len - strlen(buf), "_%s", "aw");
			break;
		case SIMPLE_PA_TI:
			snprintf(buf + strlen(buf), len - strlen(buf), "_%s", "ti");
			break;
		default:
			pr_err("%s: invalid simple pa type\n", __func__);
			break;
		}
	}
}

static void product_identifier_init(struct device_node *np, char *buf, int len)
{
	const char *string = NULL;

	memset(buf, 0, len);

	if (of_property_read_string(np, "product_identifier", &string) != 0)
		strncpy(buf, "default", strlen("default"));
	else
		strncpy(buf, string, strlen(string));
	set_product_identifier(np, buf, len);
	pr_info("%s:product_identifier %s", __func__, buf);
}

static void smartpa_info_init(struct device_node *np)
{
	const char *string = NULL;
	struct hw_audio_info *audio_priv = get_audio_info_priv();
	struct hw_smartpa_info *priv = &audio_priv->smartpa_info;

	of_property_read_u32(np, "smartpa_num", &priv->smartpa_num);

	if (!priv->is_init_smartpa_type) {
		pr_info("%s:get smartpa type from dtsi node", __func__);
		memset(priv->smartpa_type, 0, sizeof(priv->smartpa_type));
		if (of_property_read_string(np, "smartpa_type", &string) != 0)
			strncpy(priv->smartpa_type, "none", strlen("none"));
		else
			strncpy(priv->smartpa_type, string, strlen(string));
	}
	pr_info("%s:smartpa type %s", __func__, priv->smartpa_type);

	if (of_property_read_string(np, "smartpa_mi2s_rx", &string) != 0)
		strncpy(priv->i2s_rx_name, "none", strlen("none"));
	else
		strncpy(priv->i2s_rx_name, string, strlen(string));
	pr_info("%s:smartpa i2s_rx_name %s", __func__, priv->i2s_rx_name);

	if (of_property_read_string(np, "smartpa_mi2s_tx", &string) != 0)
		strncpy(priv->i2s_tx_name, "none", strlen("none"));
	else
		strncpy(priv->i2s_tx_name, string, strlen(string));
	pr_info("%s:smartpa i2s_tx_name %s", __func__, priv->i2s_tx_name);

	if (of_property_read_bool(np, "load_firmware_by_product_name")) {
		priv->is_load_firmware_by_product_name = true;
		pr_info("%s:load firmware by product identify", __func__);
	}
}

void hw_set_smartpa_type(const char *buf, int len)
{
	struct hw_audio_info *audio_priv = get_audio_info_priv();
	struct hw_smartpa_info *priv = &audio_priv->smartpa_info;

	if (buf == NULL) {
		pr_err("hw_audio: get smartpa type failed, ptr is NULL.\n");
		return;
	}

	memset(priv->smartpa_type, 0, sizeof(priv->smartpa_type));
	memcpy(priv->smartpa_type, buf, len);
	priv->is_init_smartpa_type = true;

	pr_info("%s: smartpa_type:%s", __func__, priv->smartpa_type);
	return;
}
EXPORT_SYMBOL(hw_set_smartpa_type);

enum smartpa_type hw_get_smartpa_type(void)
{
	int i;
	struct hw_audio_info *priv = get_audio_info_priv();

	for (i = 0; i < SMARTPA_TYPE_MAX; i++) {
		if (!strncmp(priv->smartpa_info.smartpa_type, smartpa_name[i],
			strlen(smartpa_name[i]) + 1)) {
			pr_info("%s: pa is %s", __func__, priv->smartpa_info.smartpa_type);
			return (enum smartpa_type)i;
		}
	}
	return INVALID;
}
EXPORT_SYMBOL(hw_get_smartpa_type);

bool hw_check_smartpa_type(char *pa_name, char *default_name)
{
	enum smartpa_type i = hw_get_smartpa_type();
	if (pa_name == NULL || default_name == NULL)
		return false;
	if (strcmp(pa_name, smartpa_name[i]) == 0
		|| strcmp(default_name, smartpa_name[i]) == 0) {
			pr_info("%s: smartpa_name = %s", __func__, smartpa_name[i]);
		return true;
	}
	return false;
}
EXPORT_SYMBOL(hw_check_smartpa_type);

int hw_lock_pa_probe_state()
{
	struct hw_audio_info *priv = get_audio_info_priv();
	int ret = mutex_trylock(&priv->smartpa_info.pa_probe_lock);
	pr_info("%s: try pa_probe_lock, ret = %d\n", __func__, ret);
	return ret;
}
EXPORT_SYMBOL(hw_lock_pa_probe_state);

void hw_unlock_pa_probe_state()
{
	struct hw_audio_info *priv = get_audio_info_priv();
	mutex_unlock(&priv->smartpa_info.pa_probe_lock);
	pr_info("%s: pa_probe unlock\n", __func__);
}
EXPORT_SYMBOL(hw_unlock_pa_probe_state);

/*
 * keep same way with fmt_single_name in sound/soc/soc-core.c
 */
static void smartpa_fmt_snd_component_name(char *comp_name, size_t name_len,
	const char* driver_name, const char* device_name)
{
	char *found;
	int id1, id2;

	pr_info("%s: comp_name:%s, %d, driver_name:%s, device_name:%s", __func__,
		comp_name, name_len, driver_name, device_name);
	strlcpy(comp_name, device_name, name_len);

	/* are we a "%s.%d" name (platform and SPI components) */
	found = strstr(comp_name, driver_name);
	if (found) {
		/* get ID */
		if (sscanf(&found[strlen(driver_name)], ".%d", &id1) == 1) {
			/* discard ID from name if ID == -1 */
			if (id1 == -1)
				found[strlen(driver_name)] = '\0';
		}
	} else {
		/* I2C component devices are named "bus-addr" */
		if (sscanf(comp_name, "%x-%x", &id1, &id2) == 2) {
			/* sanitize component name for DAI link creation */
			snprintf(comp_name, name_len, "%s.%s", driver_name,
				 device_name);
		}
	}
}

void hw_add_smartpa_codec(const char* driver_name, const char* device_name,
	const char* dai_name)
{
	struct hw_audio_info *audio_info = get_audio_info_priv();
	struct hw_smartpa_info *priv = &audio_info->smartpa_info;

	if ((driver_name == NULL) || (device_name == NULL) ||
		(dai_name == NULL)) {
		pr_err("%s: input illegal", __func__);
		return;
	}
	pr_info("%s: driver_name:%s, device_name:%s, dai_name:%s", __func__,
		driver_name, device_name, dai_name);

	if (priv->smartpa_codecs_count >= MAX_SMARTPA_NUM) {
		pr_err("%s: no more space for new smartpa codecs", __func__);
		return;
	}
	smartpa_fmt_snd_component_name(
		priv->smartpa_names[priv->smartpa_codecs_count].comp_name,
		MAX_DAI_NAME_LENGTH, driver_name, device_name);
	strlcpy(priv->smartpa_names[priv->smartpa_codecs_count].dai_name,
		dai_name, MAX_DAI_NAME_LENGTH);
	priv->smartpa_codecs[priv->smartpa_codecs_count].name =
		priv->smartpa_names[priv->smartpa_codecs_count].comp_name;
	priv->smartpa_codecs[priv->smartpa_codecs_count].dai_name =
		priv->smartpa_names[priv->smartpa_codecs_count].dai_name;
	priv->smartpa_codecs_count++;
}
EXPORT_SYMBOL(hw_add_smartpa_codec);

void hw_get_smartpa_codecs(struct snd_soc_dai_link *dai_link, int size)
{
	struct hw_audio_info *audio_info = get_audio_info_priv();
	struct hw_smartpa_info *priv = &audio_info->smartpa_info;

	int i;

	if (dai_link == NULL) {
		pr_err("%s: input illegal", __func__);
		return;
	}
	if (priv->smartpa_codecs_count == 0) {
		pr_info("%s: no smartpa probed", __func__);
		return;
	}

	for (i = 0; i < size; i++) {
		if (strcmp(dai_link[i].name, priv->i2s_rx_name) == 0 ||
			(strcmp(dai_link[i].name, priv->i2s_tx_name) == 0 &&
			(hw_get_smartpa_type() == TAS256X))) {
			dai_link[i].codecs = priv->smartpa_codecs;
			dai_link[i].num_codecs = priv->smartpa_codecs_count;
			pr_info("%s: dai_link[%d].name = %s", __func__, i, dai_link[i].name);
		}
	}

	// TODO: Add dmd when smartpa_codecs_count != smartpa_num
	pr_info("%s exit", __func__);
}
EXPORT_SYMBOL(hw_get_smartpa_codecs);

void hw_reset_smartpa_firmware(char *str, char *suffix, char *buf, int len)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	if (str == NULL || suffix == NULL || buf == NULL) {
		pr_err("%s: input param is null", __func__);
		return;
	}

	if (priv->smartpa_info.is_load_firmware_by_product_name) {
		memset(buf, 0, len);
		snprintf(buf, len, "%s_%s.%s", str, priv->product_identifier,
			suffix);
		pr_info("%s to %s", __func__, buf);
		return;
	}
}
EXPORT_SYMBOL(hw_reset_smartpa_firmware);

static bool is_extpa_to_hphr(struct device_node *np)
{
	const char *extpa_to_hphr_swh = NULL;
	int ret = of_property_read_string(np, "audio_extpa_to_hphr", &extpa_to_hphr_swh);

	if (ret == 0 && strcmp(extpa_to_hphr_swh, "true") == 0) {
		pr_info("%s: audio_extpa_to_hphr = %s", __func__, extpa_to_hphr_swh);
		return true;
	}

	return false;
}

static int audio_info_probe(struct platform_device *pdev)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	if (pdev == NULL || pdev->dev.of_node == NULL) {
		pr_err("hw_audio: %s: prop failed, input is NULL\n", __func__);
		return -EINVAL;
	}

	hw_audio_info_priv(priv);
	hw_audio_property_init(priv, pdev->dev.of_node);
	hw_hac_init(priv, pdev->dev.of_node);
	simple_pa_init(priv, pdev);
	product_identifier_init(pdev->dev.of_node, priv->product_identifier,
		sizeof(priv->product_identifier));
	smartpa_info_init(pdev->dev.of_node);
	if (of_property_read_bool(pdev->dev.of_node, "mic_differential_mode"))
		priv->mic_differential_mode = true;
	priv->is_extpa_to_hphr = is_extpa_to_hphr(pdev->dev.of_node);
	return 0;
}

bool is_need_close_hph_pa(void)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	if (!priv->is_extpa_to_hphr)
		return true;
	if (gpio_get_value(priv->pa_gpio) == GPIO_PULL_DOWN)
		return true;
	return false;
}
EXPORT_SYMBOL(is_need_close_hph_pa);

static ssize_t audio_property_show(struct device_driver *driver, char *buf)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	UNUSED(driver);
	if (buf == NULL) {
		pr_err("%s: buf is null", __func__);
		return 0;
	}
	return scnprintf(buf, PAGE_SIZE, "%08X\n", priv->audio_prop);
}
static DRIVER_ATTR_RO(audio_property);

static ssize_t codec_dump_show(struct device_driver *driver, char *buf)
{
	UNUSED(driver);
	UNUSED(buf);
	return 0;
}
static DRIVER_ATTR_RO(codec_dump);

static ssize_t product_identifier_show(struct device_driver *driver, char *buf)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	UNUSED(driver);
	if (buf == NULL) {
		pr_err("%s: buf is null", __func__);
		return 0;
	}

	return scnprintf(buf, PAGE_SIZE, "%s", priv->product_identifier);
}
static DRIVER_ATTR_RO(product_identifier);

static ssize_t smartpa_type_show(struct device_driver *driver, char *buf)
{
	struct hw_audio_info *priv = get_audio_info_priv();

	UNUSED(driver);
	if (buf == NULL) {
		pr_err("%s: buf is null", __func__);
		return 0;
	}

	return scnprintf(buf, PAGE_SIZE, "%s", priv->smartpa_info.smartpa_type);
}
static DRIVER_ATTR_RO(smartpa_type);

static ssize_t smartpa_dai_link_show(struct device_driver *driver, char *buf)
{
	unsigned int i;
	struct hw_audio_info *audio_info = get_audio_info_priv();
	struct hw_smartpa_info *priv = &audio_info->smartpa_info;

	UNUSED(driver);
	if (buf == NULL) {
		pr_err("%s: buf is null", __func__);
		return 0;
	}

	scnprintf(buf, PAGE_SIZE, "SmartPA DAIs (%d)\n", priv->smartpa_codecs_count);
	for (i = 0; i < priv->smartpa_codecs_count; ++i) {
		strlcat(buf, priv->smartpa_codecs[i].name, PAGE_SIZE);
		strlcat(buf, "\t", PAGE_SIZE);
		strlcat(buf, priv->smartpa_codecs[i].dai_name, PAGE_SIZE);
		strlcat(buf, "\n", PAGE_SIZE);
	}
	return strlen(buf) + 1;
}
static DRIVER_ATTR_RO(smartpa_dai_link);

static struct attribute *audio_attrs[] = {
	&driver_attr_audio_property.attr,
	&driver_attr_codec_dump.attr,
	&driver_attr_product_identifier.attr,
	&driver_attr_smartpa_type.attr,
	&driver_attr_smartpa_dai_link.attr,
	NULL,
};

static struct attribute_group audio_group = {
	.name = "hw_audio_info",
	.attrs = audio_attrs,
};

static const struct attribute_group *groups[] = {
	&audio_group,
	NULL,
};

static const struct of_device_id audio_info_match_table[] = {
	{ .compatible = "hw,hw_audio_info", },
	{ },
};

static struct platform_driver audio_info_driver = {
	.driver = {
		.name = "hw_audio_info",
		.owner = THIS_MODULE,
		.groups = groups,
		.of_match_table = audio_info_match_table,
	},

	.probe = audio_info_probe,
	.remove = NULL,
};

static int __init audio_info_init(void)
{
	return platform_driver_register(&audio_info_driver);
}

static void __exit audio_info_exit(void)
{
	platform_driver_unregister(&audio_info_driver);
}

module_init(audio_info_init);
module_exit(audio_info_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Huawei audio driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
