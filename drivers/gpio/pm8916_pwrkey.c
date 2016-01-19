/*
 * Qualcomm pm8916 pmic gpio driver - part of Qualcomm PM8916 PMIC
 *
 * (C) Copyright 2015 Mateusz Kulikowski <mateusz.kulikowski@gmail.com>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <dm.h>
#include <power/pmic.h>
#include <spmi/spmi.h>
#include <asm/io.h>
#include <asm/gpio.h>
#include <linux/bitops.h>

DECLARE_GLOBAL_DATA_PTR;

#define REG_TYPE               0x4
#define REG_SUBTYPE            0x5

/* Add pmic buttons as GPIO - there is no generic way for now */
#define PON_INT_RT_STS                        0x10
#define KPDPWR_ON_INT_BIT                     0
#define RESIN_ON_INT_BIT                      1

struct pm8916_pwrkey_priv {
	uint16_t pid; /* Peripheral ID on SPMI bus */
};

static int pm8941_pwrkey_get_function(struct udevice *dev, unsigned offset)
{
	return GPIOF_INPUT;
}

static int pm8941_pwrkey_get_value(struct udevice *dev, unsigned offset)
{
	struct pm8916_pwrkey_priv *priv = dev_get_priv(dev);

	int reg = pmic_reg_read(dev->parent, priv->pid + PON_INT_RT_STS);

	if (reg < 0)
		return 0;

	switch (offset) {
	case 0: /* Power button */
		return (reg & BIT(KPDPWR_ON_INT_BIT)) != 0;
		break;
	case 1: /* Reset button */
	default:
		return (reg & BIT(RESIN_ON_INT_BIT)) != 0;
		break;
	}
}

static const struct dm_gpio_ops pm8941_pwrkey_ops = {
	.get_value		= pm8941_pwrkey_get_value,
	.get_function		= pm8941_pwrkey_get_function,
};

static int pm8941_pwrkey_probe(struct udevice *dev)
{
	struct pm8916_pwrkey_priv *priv = dev_get_priv(dev);
	int reg;

	priv->pid = dev_get_addr(dev);
	if (priv->pid == FDT_ADDR_T_NONE)
		return -EINVAL;

	/* Do a sanity check */
	reg = pmic_reg_read(dev->parent, priv->pid + REG_TYPE);
	if (reg != 0x1)
		return -ENODEV;

	reg = pmic_reg_read(dev->parent, priv->pid + REG_SUBTYPE);
	if (reg != 0x1)
		return -ENODEV;

	return 0;
}

static int pm8941_pwrkey_ofdata_to_platdata(struct udevice *dev)
{
	struct gpio_dev_priv *uc_priv = dev_get_uclass_priv(dev);

	uc_priv->gpio_count = 2;
	if (uc_priv->bank_name == NULL)
		uc_priv->bank_name = "pm8916_key";

	return 0;
}

static const struct udevice_id pm8941_pwrkey_ids[] = {
	{ .compatible = "qcom,pm8916-pwrkey" },
	{ }
};

U_BOOT_DRIVER(pwrkey_pm8941) = {
	.name	= "pwrkey_pm8916",
	.id	= UCLASS_GPIO,
	.of_match = pm8941_pwrkey_ids,
	.ofdata_to_platdata = pm8941_pwrkey_ofdata_to_platdata,
	.probe	= pm8941_pwrkey_probe,
	.ops	= &pm8941_pwrkey_ops,
	.priv_auto_alloc_size = sizeof(struct pm8916_pwrkey_priv),
};
