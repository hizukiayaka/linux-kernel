/*
 * linux/arch/arm/mach-exynos4/mach-smdk4x12.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/io.h>
#include <linux/lcd.h>
#include <linux/mfd/max8997.h>
#include <linux/mfd/max77686.h>
#include <linux/mmc/host.h>
#include <linux/platform_device.h>
#include <linux/pwm_backlight.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/fixed.h>
#include <linux/mfd/wm8994/pdata.h>
#include <linux/serial_core.h>
#include <linux/lcd.h>
#include <linux/delay.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_gpio.h>
#include <linux/input/pixcir_ts.h>
#include <linux/gpio_event.h>
#include <linux/platform_data/s3c-hsotg.h>
#include <linux/platform_data/exynos_thermal.h>
#include <linux/mfd/s5m87xx/s5m-pmic.h>
#include <linux/mfd/s5m87xx/s5m-core.h>

#include <asm/mach/arch.h>
#include <asm/hardware/gic.h>
#include <asm/mach-types.h>

#include <plat/backlight.h>
#include <plat/clock.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/gpio-cfg.h>
#include <plat/iic.h>
#include <plat/keypad.h>
#include <plat/mfc.h>
#include <plat/regs-serial.h>
#include <plat/sdhci.h>
#include <plat/regs-fb-v4.h>
#include <plat/fb.h>
#include <plat/pm.h>
#include <plat/hdmi.h>
#include <plat/ehci.h>
#include <plat/camport.h>
#include <plat/s3c64xx-spi.h>
#include <plat/fimg2d.h>

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
#include <mach/dwmci.h>
#endif

#include <mach/map.h>
#include <mach/exynos-ion.h>
#include <mach/regs-pmu.h>
#include <mach/ohci.h>
#include <mach/ppmu.h>
#include <mach/dev.h>

#include <media/v4l2-mediabus.h>
#include <media/s5p_fimc.h>
#include <media/m5mols.h>
#include <plat/mipi_csis.h>
#include <plat/camport.h>
#include <media/exynos_fimc_is.h>
#include "common.h"
#include <media/exynos_flite.h>

/* Following are default values for UCON, ULCON and UFCON UART registers */
#define SMDK4X12_UCON_DEFAULT	(S3C2410_UCON_TXILEVEL |	\
				 S3C2410_UCON_RXILEVEL |	\
				 S3C2410_UCON_TXIRQMODE |	\
				 S3C2410_UCON_RXIRQMODE |	\
				 S3C2410_UCON_RXFIFO_TOI |	\
				 S3C2443_UCON_RXERR_IRQEN)

#define SMDK4X12_ULCON_DEFAULT	S3C2410_LCON_CS8

#define SMDK4X12_UFCON_DEFAULT	(S3C2410_UFCON_FIFOMODE |	\
				 S5PV210_UFCON_TXTRIG4 |	\
				 S5PV210_UFCON_RXTRIG4)


static struct max77686_regulator_data max77686_regulators[MAX77686_REG_MAX];

static struct s3c2410_uartcfg smdk4x12_uartcfgs[] __initdata = {
	[0] = {
		.hwport		= 0,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
	[1] = {
		.hwport		= 1,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
	[2] = {
		.hwport		= 2,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
	[3] = {
		.hwport		= 3,
		.flags		= 0,
		.ucon		= SMDK4X12_UCON_DEFAULT,
		.ulcon		= SMDK4X12_ULCON_DEFAULT,
		.ufcon		= SMDK4X12_UFCON_DEFAULT,
	},
};

static struct s3c_sdhci_platdata smdk4x12_hsmmc2_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_GPIO,
	.ext_cd_gpio		= EXYNOS4_GPK2(2),
	.ext_cd_gpio_invert	= 1,
#ifdef CONFIG_EXYNOS4_SDHCI_CH2_8BIT
	.max_width		= 8,
	.host_caps		= MMC_CAP_8_BIT_DATA,
#endif
};

static struct s3c_sdhci_platdata smdk4x12_hsmmc3_pdata __initdata = {
	.cd_type		= S3C_SDHCI_CD_INTERNAL,
};

static struct regulator_consumer_supply max8997_buck1 =
	REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply max8997_buck2 =
	REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_consumer_supply max8997_buck3 =
	REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_init_data max8997_buck1_data = {
	.constraints	= {
		.name		= "VDD_ARM_SMDK4X12",
		.min_uV		= 925000,
		.max_uV		= 1350000,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max8997_buck1,
};

static struct regulator_init_data max8997_buck2_data = {
	.constraints	= {
		.name		= "VDD_INT_SMDK4X12",
		.min_uV		= 950000,
		.max_uV		= 1150000,
		.always_on	= 1,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max8997_buck2,
};

static struct regulator_init_data max8997_buck3_data = {
	.constraints	= {
		.name		= "VDD_G3D_SMDK4X12",
		.min_uV		= 950000,
		.max_uV		= 1150000,
		.valid_ops_mask	= REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max8997_buck3,
};

static struct max8997_regulator_data smdk4x12_max8997_regulators[] = {
	{ MAX8997_BUCK1, &max8997_buck1_data },
	{ MAX8997_BUCK2, &max8997_buck2_data },
	{ MAX8997_BUCK3, &max8997_buck3_data },
};

static struct max8997_platform_data smdk4x12_max8997_pdata = {
	.num_regulators	= ARRAY_SIZE(smdk4x12_max8997_regulators),
	.regulators	= smdk4x12_max8997_regulators,

	.buck1_voltage[0] = 1100000,	/* 1.1V */
	.buck1_voltage[1] = 1100000,	/* 1.1V */
	.buck1_voltage[2] = 1100000,	/* 1.1V */
	.buck1_voltage[3] = 1100000,	/* 1.1V */
	.buck1_voltage[4] = 1100000,	/* 1.1V */
	.buck1_voltage[5] = 1100000,	/* 1.1V */
	.buck1_voltage[6] = 1000000,	/* 1.0V */
	.buck1_voltage[7] = 950000,	/* 0.95V */

	.buck2_voltage[0] = 1100000,	/* 1.1V */
	.buck2_voltage[1] = 1000000,	/* 1.0V */
	.buck2_voltage[2] = 950000,	/* 0.95V */
	.buck2_voltage[3] = 900000,	/* 0.9V */
	.buck2_voltage[4] = 1100000,	/* 1.1V */
	.buck2_voltage[5] = 1000000,	/* 1.0V */
	.buck2_voltage[6] = 950000,	/* 0.95V */
	.buck2_voltage[7] = 900000,	/* 0.9V */

	.buck5_voltage[0] = 1100000,	/* 1.1V */
	.buck5_voltage[1] = 1100000,	/* 1.1V */
	.buck5_voltage[2] = 1100000,	/* 1.1V */
	.buck5_voltage[3] = 1100000,	/* 1.1V */
	.buck5_voltage[4] = 1100000,	/* 1.1V */
	.buck5_voltage[5] = 1100000,	/* 1.1V */
	.buck5_voltage[6] = 1100000,	/* 1.1V */
	.buck5_voltage[7] = 1100000,	/* 1.1V */
};
#ifdef CONFIG_REGULATOR_MAX77686
static struct regulator_consumer_supply max77686_buck1 =
REGULATOR_SUPPLY("vdd_mif", NULL);

static struct regulator_consumer_supply max77686_buck2 =
REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply max77686_buck3 =
REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_consumer_supply max77686_buck4 =
REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_consumer_supply max77686_ldo11_consumer =
REGULATOR_SUPPLY("vdd_ldo11", NULL);

static struct regulator_consumer_supply max77686_ldo14_consumer =
REGULATOR_SUPPLY("vdd_ldo14", NULL);

static struct regulator_consumer_supply max77686_ldo12_consumer =
REGULATOR_SUPPLY("vusb_a", NULL);

static struct regulator_consumer_supply max77686_ldo15_consumer =
REGULATOR_SUPPLY("vusb_d", NULL);

static struct regulator_consumer_supply max77686_ldo16_consumer =
REGULATOR_SUPPLY("vdd_hsic", NULL);

static struct regulator_consumer_supply max77686_ldo8_consumer[] = {
REGULATOR_SUPPLY("vdd", "exynos4-hdmi"),
REGULATOR_SUPPLY("vdd_pll", "exynos4-hdmi"),
REGULATOR_SUPPLY("vdd8_mipi", NULL),
};

static struct regulator_consumer_supply max77686_ldo10_consumer[] = {
REGULATOR_SUPPLY("vdd_osc", "exynos4-hdmi"),
REGULATOR_SUPPLY("vdd10_mipi", NULL),
REGULATOR_SUPPLY("vdd_tmu", NULL),
};

static struct regulator_init_data max77686_ldo8_data = {
	.constraints = {
		.name		= "vdd_ldo8 range",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies	= ARRAY_SIZE(max77686_ldo8_consumer),
	.consumer_supplies	= max77686_ldo8_consumer,
};

static struct regulator_init_data max77686_ldo10_data = {
	.constraints = {
		.name		= "vdd_ldo10 range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies	= ARRAY_SIZE(max77686_ldo10_consumer),
	.consumer_supplies	= max77686_ldo10_consumer,
};

static struct regulator_init_data max77686_ldo12_data = {
	.constraints = {
		.name		= "vdd_ldo12 range",
		.min_uV		= 3000000,
		.max_uV		= 3300000,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo12_consumer,
};

static struct regulator_init_data max77686_ldo15_data = {
	.constraints = {
		.name		= "vdd_ldo15 range",
		.min_uV		= 1000000,
		.max_uV		= 1000000,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo15_consumer,
};

static struct regulator_init_data max77686_ldo16_data = {
	.constraints = {
		.name		= "vdd_ldo16 range",
		.min_uV		= 1800000,
		.max_uV		= 1800000,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo16_consumer,
};

static struct regulator_init_data max77686_buck1_data = {
	.constraints = {
		.name		= "vdd_mif range",
		.min_uV		= 800000,
		.max_uV		= 1300000,
		.always_on	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck1,
};

static struct regulator_init_data max77686_buck2_data = {
	.constraints = {
		.name		= "vdd_arm range",
		.min_uV		= 800000,
		.max_uV		= 1350000,
		.always_on	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies 	= 1,
	.consumer_supplies	= &max77686_buck2,
};

static struct regulator_init_data max77686_buck3_data = {
	.constraints = {
		.name		= "vdd_int range",
		.min_uV		= 800000,
		.max_uV		= 1150000,
		.always_on	= 1,
		.boot_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck3,
};

static struct regulator_init_data max77686_buck4_data = {
	.constraints = {
		.name		= "vdd_g3d range",
		.min_uV		= 850000,
		.max_uV		= 1200000,
		.boot_on	= 1,
		.always_on	= 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				  REGULATOR_CHANGE_STATUS,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM, 
	},
	.num_consumer_supplies = 1,
	.consumer_supplies = &max77686_buck4,
};

static struct regulator_init_data __initdata max77686_ldo11_data = {
	.constraints	= {
		.name		= "vdd_ldo11 range",
		.min_uV		= 1900000,
		.max_uV		= 1900000,
		.apply_uV	= 1,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo11_consumer,
};

static struct regulator_init_data __initdata max77686_ldo14_data = {
	.constraints	= {
		.name		= "vdd_ldo14 range",
		.min_uV		= 1900000,
		.max_uV		= 1900000,
		.apply_uV	= 1,
		.always_on	= 1,
		.state_mem	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
	},
	.num_consumer_supplies	= 1,
	.consumer_supplies	= &max77686_ldo14_consumer,
};

static struct regulator_init_data __initdata max77686_always_on = {
	.constraints = {
		.always_on 	= 1,
		.state_mem 	= {
			.disabled	= 1,
			.mode		= REGULATOR_MODE_STANDBY,
		},
	},
};

static void max77686_populate_pdata (void)
{
	int i;

	/* LDOs[0-7] and BUCKs[5-7] are not initialized yet but required to
	 * be always enabled for stability */
	for (i = 0; i <= MAX77686_LDO7; i++)
		max77686_regulators[i].initdata = &max77686_always_on;
	max77686_regulators[MAX77686_BUCK5].initdata = &max77686_always_on;
	max77686_regulators[MAX77686_BUCK6].initdata = &max77686_always_on;
	max77686_regulators[MAX77686_BUCK7].initdata = &max77686_always_on;

	max77686_regulators[MAX77686_BUCK1].initdata = &max77686_buck1_data;
	max77686_regulators[MAX77686_BUCK2].initdata = &max77686_buck2_data;
	max77686_regulators[MAX77686_BUCK3].initdata = &max77686_buck3_data;
	max77686_regulators[MAX77686_BUCK4].initdata = &max77686_buck4_data;
	max77686_regulators[MAX77686_LDO8].initdata  = &max77686_ldo8_data;
	max77686_regulators[MAX77686_LDO10].initdata = &max77686_ldo10_data;
	max77686_regulators[MAX77686_LDO11].initdata = &max77686_ldo11_data;
	max77686_regulators[MAX77686_LDO12].initdata = &max77686_ldo12_data;
	max77686_regulators[MAX77686_LDO14].initdata = &max77686_ldo14_data;
	max77686_regulators[MAX77686_LDO15].initdata = &max77686_ldo15_data;
	max77686_regulators[MAX77686_LDO16].initdata = &max77686_ldo16_data;

	regulator_has_full_constraints();
}

static struct max77686_platform_data smdk4412_max77686_pdata = {
	.num_regulators = ARRAY_SIZE(max77686_regulators),
	.regulators = max77686_regulators,
};
#endif
#ifdef CONFIG_REGULATOR_S5M8767
/* S5M8767 Regulator */
static int s5m_cfg_irq(void)
{
        /* AP_PMIC_IRQ: EINT26 */
	s3c_gpio_cfgpin(EXYNOS4_GPX3(2), S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(EXYNOS4_GPX3(2), S3C_GPIO_PULL_UP);
        return 0;
}
static struct regulator_consumer_supply s5m8767_buck1_consumer =
REGULATOR_SUPPLY("vdd_mif", NULL);

static struct regulator_consumer_supply s5m8767_buck2_consumer =
REGULATOR_SUPPLY("vdd_arm", NULL);

static struct regulator_consumer_supply s5m8767_buck3_consumer =
REGULATOR_SUPPLY("vdd_int", NULL);

static struct regulator_consumer_supply s5m8767_buck4_consumer =
REGULATOR_SUPPLY("vdd_g3d", NULL);

static struct regulator_consumer_supply s5m8767_ldo11_consumer =
REGULATOR_SUPPLY("vdd_ldo11", NULL);

static struct regulator_consumer_supply s5m8767_ldo14_consumer =
REGULATOR_SUPPLY("vdd_ldo14", NULL);

static struct regulator_consumer_supply s5m8767_ldo12_consumer =
REGULATOR_SUPPLY("vusb_a", NULL);

static struct regulator_consumer_supply s5m8767_ldo15_consumer =
REGULATOR_SUPPLY("vusb_d", NULL);

static struct regulator_consumer_supply s5m8767_ldo16_consumer =
REGULATOR_SUPPLY("vdd_hsic", NULL);

static struct regulator_consumer_supply s5m8767_ldo8_consumer[] = {
REGULATOR_SUPPLY("vdd", "exynos4-hdmi"),
REGULATOR_SUPPLY("vdd_pll", "exynos4-hdmi"),
REGULATOR_SUPPLY("vdd8_mipi", NULL),
};
static struct regulator_consumer_supply s5m8767_ldo10_consumer[] = {
REGULATOR_SUPPLY("vdd_osc", "exynos4-hdmi"),
REGULATOR_SUPPLY("vdd10_mipi", NULL),
REGULATOR_SUPPLY("vdd_tmu", NULL),
};

static struct regulator_init_data s5m8767_ldo8_data = {
	.constraints	={
		.name		= "vdd_ldo8 range",
		.min_uV		= 1000000,
		.max_uV         = 1000000,
		.boot_on        = 1,
		.always_on      = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem      = {
			.disabled       = 1,
			.mode  	        = REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM,
	},
	.num_consumer_supplies  = ARRAY_SIZE(s5m8767_ldo8_consumer),
	.consumer_supplies      = s5m8767_ldo8_consumer,
};

static struct regulator_init_data s5m8767_ldo10_data = {
	.constraints = {
		.name           = "vdd_ldo10 range",
		.min_uV         = 1800000,
		.max_uV         = 1800000,
		.boot_on        = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.state_mem      = {
			.disabled       = 1,
			.mode           = REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM,
	},
	.num_consumer_supplies  = ARRAY_SIZE(s5m8767_ldo10_consumer),
	.consumer_supplies      = s5m8767_ldo10_consumer,
};

static struct regulator_init_data s5m8767_ldo12_data = {
	.constraints = {
		.name           = "vdd_ldo12 range",
		.min_uV         = 3000000,
		.max_uV         = 3000000,
		.boot_on        = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS | REGULATOR_CHANGE_VOLTAGE,
		.always_on      = 1,
		.state_mem      = {
			.disabled       = 1,
			.mode           = REGULATOR_MODE_STANDBY,
		},
	.initial_state = PM_SUSPEND_MEM,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_ldo12_consumer,
};

static struct regulator_init_data s5m8767_ldo15_data = {
	.constraints = {
		.name           = "vdd_ldo15 range",
		.min_uV         = 1000000,
		.max_uV         = 1000000,
		.boot_on        = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on      = 1,
		.state_mem      = {
			.disabled       = 1,
			.mode           = REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_ldo15_consumer,
};

static struct regulator_init_data s5m8767_ldo16_data = {
	.constraints = {
		.name           = "vdd_ldo16 range",
		.min_uV         = 1800000,
		.max_uV         = 1800000,
		.boot_on        = 1,
		.valid_ops_mask = REGULATOR_CHANGE_STATUS,
		.always_on      = 1,
		.state_mem      = {
			.disabled       = 1,
			.mode           = REGULATOR_MODE_STANDBY,
		},
		.initial_state = PM_SUSPEND_MEM,
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_ldo16_consumer,
};

static struct regulator_init_data __initdata s5m8767_ldo11_data = {
	.constraints    = {
		.name           = "vdd_ldo11 range",
		.min_uV         = 1800000,
		.max_uV         = 1800000,
		.always_on      = 1,
		.apply_uV       = 1,
		.state_mem      = {
			.disabled       = 1,
			.mode           = REGULATOR_MODE_STANDBY,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_ldo11_consumer,
};

static struct regulator_init_data __initdata s5m8767_ldo14_data = {
	.constraints    = {
		.name           = "vdd_ldo14 range",
		.min_uV         = 1800000,
		.max_uV         = 1800000,
		.apply_uV       = 1,
		.always_on      = 1,
		.state_mem      = {
			.disabled       = 1,
			.mode           = REGULATOR_MODE_STANDBY,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_ldo14_consumer,
};

static struct regulator_init_data s5m8767_buck1_data = {
	.constraints    = {
		.name           = "vdd_mif range",
		.min_uV         = 800000,
		.max_uV         = 1100000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem      = {
			.disabled       = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_buck1_consumer,
};

static struct regulator_init_data s5m8767_buck2_data = {
	.constraints    = {
		.name           = "vdd_arm range",
		.min_uV         =  800000,
		.max_uV         = 1350000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
			REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem      = {
				.disabled       = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_buck2_consumer,
};

static struct regulator_init_data s5m8767_buck3_data = {
	.constraints    = {
		.name           = "vdd_int range",
		.min_uV         =  800000,
		.max_uV         = 1150000,
		.apply_uV       = 1,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem      = {
			.uV             = 1100000,
			.mode           = REGULATOR_MODE_NORMAL,
			.disabled       = 1,
		},
	},
	.num_consumer_supplies  = 1,
	.consumer_supplies      = &s5m8767_buck3_consumer,
};
static struct regulator_init_data s5m8767_buck4_data = {
	.constraints    = {
		.name           = "vdd_g3d range",
		.min_uV         = 850000,
		.max_uV         = 1200000,
		.valid_ops_mask = REGULATOR_CHANGE_VOLTAGE |
				REGULATOR_CHANGE_STATUS,
		.always_on = 1,
		.boot_on = 1,
		.state_mem      = {
			.disabled       = 1,
			},
		},
	.num_consumer_supplies = 1,
	.consumer_supplies = &s5m8767_buck4_consumer,
};

static struct s5m_regulator_data pegasus_regulators[] = {
	{ S5M8767_LDO8,  &s5m8767_ldo8_data},
	{ S5M8767_LDO10, &s5m8767_ldo10_data},
	{ S5M8767_LDO11, &s5m8767_ldo11_data},
	{ S5M8767_LDO12, &s5m8767_ldo12_data},
	{ S5M8767_LDO14, &s5m8767_ldo14_data},
	{ S5M8767_LDO15, &s5m8767_ldo15_data},
	{ S5M8767_LDO16, &s5m8767_ldo16_data},
	{ S5M8767_BUCK1, &s5m8767_buck1_data },
	{ S5M8767_BUCK2, &s5m8767_buck2_data },
	{ S5M8767_BUCK3, &s5m8767_buck3_data },
	{ S5M8767_BUCK4, &s5m8767_buck4_data },
};

struct s5m_opmode_data s5m8767_opmode_data[S5M8767_REG_MAX] = {
	[S5M8767_BUCK1] = {S5M8767_BUCK1, S5M_OPMODE_SUSPEND},
	[S5M8767_BUCK2] = {S5M8767_BUCK2, S5M_OPMODE_SUSPEND},
	[S5M8767_BUCK3] = {S5M8767_BUCK3, S5M_OPMODE_SUSPEND},
	[S5M8767_BUCK4] = {S5M8767_BUCK4, S5M_OPMODE_SUSPEND},
	[S5M8767_LDO8]  = {S5M8767_LDO8,  S5M_OPMODE_SUSPEND},
	[S5M8767_LDO10] = {S5M8767_LDO10, S5M_OPMODE_SUSPEND},
	[S5M8767_LDO11] = {S5M8767_LDO11, S5M_OPMODE_SUSPEND},
	[S5M8767_LDO12] = {S5M8767_LDO12, S5M_OPMODE_SUSPEND},
	[S5M8767_LDO14] = {S5M8767_LDO14, S5M_OPMODE_SUSPEND},
	[S5M8767_LDO15] = {S5M8767_LDO15, S5M_OPMODE_SUSPEND},
	[S5M8767_LDO16] = {S5M8767_LDO16, S5M_OPMODE_SUSPEND},
};

static struct s5m_platform_data exynos4_s5m8767_pdata = {
	.device_type            = S5M8767X,
	.irq_base               = IRQ_BOARD_START,
	.num_regulators         = ARRAY_SIZE(pegasus_regulators),
	.regulators             = pegasus_regulators,
	.cfg_pmic_irq           = s5m_cfg_irq,
	.wakeup                 = 1,
	.opmode            = s5m8767_opmode_data,
	//        .wtsr_smpl              = 1,

	.buck_default_idx       = 1,
	.buck_gpios[0]          = EXYNOS4_GPL0(3),
	.buck_gpios[1]          = EXYNOS4_GPL0(4),
	.buck_gpios[2]          = EXYNOS4_GPL0(6),

	.buck_ramp_delay        = 25,
	.buck2_ramp_enable      = true,
	.buck3_ramp_enable      = true,
	.buck4_ramp_enable      = true,
#if 0
	.buck_ds[0]             = EXYNOS4_GPL0(0),
	.buck_ds[1]             = EXYNOS4_GPL0(1),
	.buck_ds[2]             = EXYNOS4_GPL0(2),
	.buck2_init             = 1100000,
	.buck3_init             = 1000000,
	.buck4_init             = 1000000,
#endif
};
/* End of S5M8767 */
#endif

static struct regulator_consumer_supply wm8994_fixed_voltage0_supplies[] = {
        REGULATOR_SUPPLY("AVDD2", "1-001a"),
        REGULATOR_SUPPLY("CPVDD", "1-001a"),
};

static struct regulator_consumer_supply wm8994_fixed_voltage1_supplies[] = {
        REGULATOR_SUPPLY("SPKVDD1", "1-001a"),
        REGULATOR_SUPPLY("SPKVDD2", "1-001a"),
};

static struct regulator_consumer_supply wm8994_fixed_voltage2_supplies =
        REGULATOR_SUPPLY("DBVDD", "1-001a");

static struct regulator_init_data wm8994_fixed_voltage0_init_data = {
        .constraints = {
                .always_on = 1,
        },
        .num_consumer_supplies  = ARRAY_SIZE(wm8994_fixed_voltage0_supplies),
        .consumer_supplies      = wm8994_fixed_voltage0_supplies,
};

static struct regulator_init_data wm8994_fixed_voltage1_init_data = {
        .constraints = {
                .always_on = 1,
        },
        .num_consumer_supplies  = ARRAY_SIZE(wm8994_fixed_voltage1_supplies),
        .consumer_supplies      = wm8994_fixed_voltage1_supplies,
};

static struct regulator_init_data wm8994_fixed_voltage2_init_data = {
        .constraints = {
                .always_on = 1,
        },
        .num_consumer_supplies  = 1,
        .consumer_supplies      = &wm8994_fixed_voltage2_supplies,
};

static struct fixed_voltage_config wm8994_fixed_voltage0_config = {
        .supply_name    = "VDD_1.8V",
        .microvolts     = 1800000,
        .gpio           = -EINVAL,
        .init_data      = &wm8994_fixed_voltage0_init_data,
};

static struct fixed_voltage_config wm8994_fixed_voltage1_config = {
        .supply_name    = "DC_5V",
        .microvolts     = 5000000,
        .gpio           = -EINVAL,
        .init_data      = &wm8994_fixed_voltage1_init_data,
};

static struct fixed_voltage_config wm8994_fixed_voltage2_config = {
        .supply_name    = "VDD_3.3V",
        .microvolts     = 3300000,
        .gpio           = -EINVAL,
        .init_data      = &wm8994_fixed_voltage2_init_data,
};

static struct platform_device wm8994_fixed_voltage0 = {
        .name           = "reg-fixed-voltage",
        .id             = 0,
        .dev            = {
                .platform_data  = &wm8994_fixed_voltage0_config,
        },
};

static struct platform_device wm8994_fixed_voltage1 = {
        .name           = "reg-fixed-voltage",
        .id             = 1,
        .dev            = {
                .platform_data  = &wm8994_fixed_voltage1_config,
        },
};

static struct platform_device wm8994_fixed_voltage2 = {
        .name           = "reg-fixed-voltage",
        .id             = 2,
        .dev            = {
                .platform_data  = &wm8994_fixed_voltage2_config,
        },
};

static struct regulator_consumer_supply wm8994_avdd1_supply =
        REGULATOR_SUPPLY("AVDD1", "1-001a");

static struct regulator_consumer_supply wm8994_dcvdd_supply =
        REGULATOR_SUPPLY("DCVDD", "1-001a");

static struct regulator_init_data wm8994_ldo1_data = {
        .constraints    = {
                .name           = "AVDD1",
        },
        .num_consumer_supplies  = 1,
        .consumer_supplies      = &wm8994_avdd1_supply,
};

static struct regulator_init_data wm8994_ldo2_data = {
        .constraints    = {
                .name           = "DCVDD",
        },
        .num_consumer_supplies  = 1,
        .consumer_supplies      = &wm8994_dcvdd_supply,
};

static struct wm8994_pdata wm8994_platform_data = {
        /* configure gpio1 function: 0x0001(Logic level input/output) */
        .gpio_defaults[0] = 0x0001,
        /* If the i2s0 and i2s2 is enabled simultaneously */
        .gpio_defaults[7] = 0x8100, /* GPIO8  DACDAT3 in */
        .gpio_defaults[8] = 0x0100, /* GPIO9  ADCDAT3 out */
        .gpio_defaults[9] = 0x0100, /* GPIO10 LRCLK3  out */
        .gpio_defaults[10] = 0x0100,/* GPIO11 BCLK3   out */
        .ldo[0] = { 0, &wm8994_ldo1_data },
        .ldo[1] = { 0, &wm8994_ldo2_data },
};
static struct i2c_board_info smdk4x12_i2c_devs0_s5m8767[] __initdata = {
#ifdef CONFIG_REGULATOR_S5M8767
	{
		I2C_BOARD_INFO("s5m87xx", 0xCC >> 1),
		.platform_data	= &exynos4_s5m8767_pdata,
		.irq		= IRQ_EINT(26),
	},
#endif
};
static struct i2c_board_info smdk4x12_i2c_devs0_max77686[] __initdata = {
#ifdef CONFIG_REGULATOR_MAX77686
	{
		I2C_BOARD_INFO("max77686", (0x12 >> 1)),
		.platform_data	= &smdk4412_max77686_pdata,
		.irq		= IRQ_EINT(26),
	},
#endif
};

static struct i2c_board_info smdk4x12_i2c_devs1[] __initdata = {
	{ 
		I2C_BOARD_INFO("wm8994", 0x1a), 
	  	.platform_data  = &wm8994_platform_data,
	}
};

static struct i2c_board_info smdk4x12_i2c_devs2[] __initdata = {
        {
                I2C_BOARD_INFO("s5p_ddc", (0x74 >> 1)),
        },
};


static struct s5p_platform_mipi_csis mipi_csis_platdata = {
#ifdef CONFIG_VIDEO_S5K6A3
	.clk_rate	= 160000000UL,
	.lanes		= 1,
	.alignment	= 24,
	.hs_settle	= 12,
	.phy_enable	= s5p_csis_phy_enable,
#endif
#ifdef CONFIG_VIDEO_M5MOLS
	.clk_rate	= 166000000UL,
	.lanes		= 2,
	.alignment	= 32,
	.hs_settle	= 12,
	.phy_enable	= s5p_csis_phy_enable,
#endif
};
#define GPIO_CAM_LEVEL_EN(n)	EXYNOS4_GPX1(2)
#define GPIO_CAM_8M_ISP_INT	EXYNOS4_GPX3(3)	/* XEINT_27 */
#define GPIO_CAM_MEGA_nRST	EXYNOS4_GPX1(2) 
static int m5mols_set_power(struct device *dev, int on)
{
	gpio_set_value(EXYNOS4_GPX1(2), !on);
	gpio_set_value(EXYNOS4_GPX1(2), !!on);
	return 0;
}
static struct m5mols_platform_data m5mols_platdata = {
	.gpio_reset	= GPIO_CAM_MEGA_nRST,
	.reset_polarity	= 0,
	.set_power	= m5mols_set_power,
};
static struct i2c_board_info m5mols_board_info = {
	I2C_BOARD_INFO("M5MOLS", 0x1F),
	.platform_data = &m5mols_platdata,
};

#ifdef CONFIG_VIDEO_S5K6A3
static struct i2c_board_info s5k6a3_sensor_info = {
        .type = "S5K6A3",
};
#endif

#ifdef CONFIG_VIDEO_S5K6A3
static int smdk4x12_cam1_reset(int dummy) 
{
        int err;

        /* Camera B */

        err = gpio_request(EXYNOS4_GPX1(0), "GPX1");
        if (err)
                printk(KERN_ERR "#### failed to request GPX1_0 ####\n");

        s3c_gpio_setpull(EXYNOS4_GPX1(0), S3C_GPIO_PULL_NONE);
        gpio_direction_output(EXYNOS4_GPX1(0), 0);
        gpio_direction_output(EXYNOS4_GPX1(0), 1);
        gpio_free(EXYNOS4_GPX1(0));

        return 0;
}

static struct s3c_platform_camera s5k6a3 = {
        .id             = CAMERA_CSI_D,
        .clk_name       = "sclk_cam1",
        .cam_power      = smdk4x12_cam1_reset,
        .type           = CAM_TYPE_MIPI,
        .fmt            = MIPI_CSI_RAW10,
        .order422       = CAM_ORDER422_8BIT_YCBYCR,
        .pixelformat    = V4L2_PIX_FMT_UYVY,
        .line_length    = 1920,
        .width          = 1920,
        .height         = 1080,
        .window         = {
                .left   = 0,
                .top    = 0,
                .width  = 1920,
                .height = 1080,
        },
        .srclk_name     = "xusbxti",
        .clk_rate       = 24000000,
        .mipi_lanes     = 1,
        .mipi_settle    = 12,
        .mipi_align     = 24,

        .initialized    = 0,
        .flite_id       = FLITE_IDX_B,
        .use_isp        = true,
        .sensor_index   = 102,
	.type  		= CAM_TYPE_MIPI,
        .use_isp 	= true,
        .inv_pclk 	= 0,
        .inv_vsync 	= 0,
        .inv_href 	= 0,
        .inv_hsync 	= 0,
};


static struct s3c_platform_fimc fimc_plat = {
	.default_cam    = CAMERA_CSI_D,
	.camera         = {
			&s5k6a3,
	},
};
#endif
static struct s5p_fimc_isp_info smdk4x12_camera_sensors[] = {
#ifdef CONFIG_VIDEO_S5K6A3
	{
                .board_info     = &s5k6a3_sensor_info,
                .clk_frequency  = 24000000UL,
                .bus_type       = FIMC_MIPI_CSI2,
		.i2c_bus_num    = 1,
                .mux_id         = 1, /* A-Port : 0, B-Port : 1 */
                .flite_id       = FLITE_IDX_B,
                .cam_power      = smdk4x12_cam1_reset,
		.flags          = 0,
                .csi_data_align = 24,
                .use_isp        = true,
        },
#endif
#ifdef CONFIG_VIDEO_M5MOLS
	{
		.mux_id		= 0,
		.flags		= V4L2_MBUS_PCLK_SAMPLE_FALLING |
				  V4L2_MBUS_VSYNC_ACTIVE_LOW,
		.bus_type	= FIMC_MIPI_CSI2,
		.board_info	= &m5mols_board_info,
		.i2c_bus_num	= 4,
		.clk_frequency	= 24000000UL,
		.csi_data_align	= 32,
	},
#endif
};
static struct s5p_platform_fimc fimc_md_platdata = {
	.isp_info	= smdk4x12_camera_sensors,
	.num_clients	= ARRAY_SIZE(smdk4x12_camera_sensors),
#ifdef CONFIG_VIDEO_S5K6A3
	.fimc_plat	= &fimc_plat,
#endif
};

static struct gpio smdk4x12_camera_gpios[] = {
	{ GPIO_CAM_8M_ISP_INT,	GPIOF_IN,            "8M_ISP_INT"  },
	{ GPIO_CAM_MEGA_nRST,	GPIOF_OUT_INIT_LOW,  "CAM_8M_NRST" },
};
static void __init smdk4x12_camera_init(void)
{
	s3c_set_platdata(&mipi_csis_platdata, sizeof(mipi_csis_platdata),
			 &s5p_device_mipi_csis0);
	s3c_set_platdata(&mipi_csis_platdata, sizeof(mipi_csis_platdata),
                         &s5p_device_mipi_csis1);
	s3c_set_platdata(&fimc_md_platdata,  sizeof(fimc_md_platdata),
			 &s5p_device_fimc_md);
	

	if (gpio_request_array(smdk4x12_camera_gpios,
			       ARRAY_SIZE(smdk4x12_camera_gpios))) {
		pr_err("%s: GPIO request failed\n", __func__);
		return;
	}

	if (!s3c_gpio_cfgpin(GPIO_CAM_8M_ISP_INT, S3C_GPIO_SFN(0xf)))
	{
        	s3c_gpio_setpull(GPIO_CAM_8M_ISP_INT, S3C_GPIO_PULL_NONE);
		m5mols_board_info.irq = gpio_to_irq(GPIO_CAM_8M_ISP_INT);
	}
	else
		pr_err("Failed to configure 8M_ISP_INT GPIO\n");

	/* Free GPIOs controlled directly by the sensor drivers. */
	gpio_free(GPIO_CAM_MEGA_nRST);
	gpio_free(GPIO_CAM_8M_ISP_INT);

	if (exynos4_fimc_setup_gpio(S5P_CAMPORT_A))
		pr_err("Camera port A setup failed\n");
}

static struct i2c_board_info smdk4x12_i2c_devs3[] __initdata = {
	/* nothing here yet */
};

/* I2C module and id for HDMIPHY */
static struct i2c_board_info smdk4x12_i2c_hdmiphy[] __initdata = {
        { I2C_BOARD_INFO("hdmiphy-exynos4412", 0x38), }
};

static void s5p_tv_setup(void)
{
        /* direct HPD to External Interrupt */
        WARN_ON(gpio_request_one(EXYNOS4_GPX3(7), GPIOF_IN, "hpd-plug"));
        s3c_gpio_cfgpin(EXYNOS4_GPX3(7), S3C_GPIO_SFN(0xf));
        s3c_gpio_setpull(EXYNOS4_GPX3(7), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4_GPX3(7));
}

static int smdk4x12_touch_attb_read_val (void)
{
	return gpio_get_value(EXYNOS4_GPX2(6));
}

static struct pixcir_ts_platform_data smdk4x12_touch_pdata = {	
	.attb_read_val = smdk4x12_touch_attb_read_val,
        .x_max = 480,
        .y_max = 800,   
};

static struct i2c_board_info smdk4x12_i2c_devs7[] __initdata = {
	{
                I2C_BOARD_INFO("pixcir_ts", 0x5C),
                .platform_data  = &smdk4x12_touch_pdata,
                .irq            = IRQ_EINT(22),
        },
};

static void __init smdk4x12_touch_init(void)
{
	int err;
	err = gpio_request_one(EXYNOS4X12_GPM3(4),GPIOF_OUT_INIT_HIGH,"GPM3");
        if (err) {
                pr_err("failed to request GPM3 for touch reset control\n");
        }
        s3c_gpio_cfgpin(EXYNOS4X12_GPM3(4), S3C_GPIO_OUTPUT);
	gpio_direction_output(EXYNOS4X12_GPM3(4),0);
	mdelay(20);
	gpio_direction_output(EXYNOS4X12_GPM3(4),1);
	s3c_gpio_setpull(EXYNOS4_GPX2(6),S3C_GPIO_PULL_NONE);
}

static struct samsung_bl_gpio_info smdk4x12_bl_gpio_info = {
	.no = EXYNOS4_GPD0(1),
	.func = S3C_GPIO_SFN(2),
};

static struct platform_pwm_backlight_data smdk4x12_bl_data = {
	.pwm_id = 1,
	.pwm_period_ns  = 1000,
};

static struct s3c_fb_pd_win smdk4x12_fb_win0 = {
	.xres			= 480,
	.yres			= 800,
        .virtual_x              = 480,
        .virtual_y              = 800 * CONFIG_FB_S3C_NR_BUFFERS,
        .max_bpp                = 32,
        .default_bpp            = 24,
	.width			= 66,
	.height 		= 109,
};

static struct s3c_fb_pd_win smdk4x12_fb_win1 = {
	.xres			= 480,
	.yres			= 800,
        .virtual_x              = 480,
        .virtual_y              = 800 * CONFIG_FB_S3C_NR_BUFFERS,
        .max_bpp                = 32,
        .default_bpp            = 24,
	.width			= 66,
	.height 		= 109,
};

static struct s3c_fb_pd_win smdk4x12_fb_win2 = {
	.xres			= 480,
	.yres			= 800,
        .virtual_x              = 480,
        .virtual_y              = 800 * CONFIG_FB_S3C_NR_BUFFERS,
        .max_bpp                = 32,
        .default_bpp            = 24,
	.width			= 66,
	.height 		= 109,
};

static struct s3c_fb_pd_win smdk4x12_fb_win3 = {
	.xres			= 480,
	.yres			= 800,
        .virtual_x              = 480,
        .virtual_y              = 800 * CONFIG_FB_S3C_NR_BUFFERS,
        .max_bpp                = 32,
        .default_bpp            = 24,
	.width			= 66,
	.height 		= 109,
};

static struct s3c_fb_pd_win smdk4x12_fb_win4 = {
	.xres			= 480,
	.yres			= 800,
        .virtual_x              = 480,
        .virtual_y              = 800 * CONFIG_FB_S3C_NR_BUFFERS,
        .max_bpp                = 32,
        .default_bpp            = 24,
	.width			= 66,
	.height 		= 109,
};

static struct fb_videomode smdk4x12_lcd_timing = {
	.left_margin	= 9,
	.right_margin	= 9,
	.upper_margin	= 5,
	.lower_margin	= 5,
	.hsync_len	= 2,
	.vsync_len	= 2,
	.xres		= 480,
	.yres		= 800,
};

static struct s3c_fb_platdata smdk4x12_lcd0_pdata __initdata = {
        .win[0]         = &smdk4x12_fb_win0,
        .win[1]         = &smdk4x12_fb_win1,
        .win[2]         = &smdk4x12_fb_win2,
        .win[3]         = &smdk4x12_fb_win3,
        .win[4]         = &smdk4x12_fb_win4,
	.vtiming	= &smdk4x12_lcd_timing,
        .vidcon0        = VIDCON0_VIDOUT_RGB | VIDCON0_PNRMODE_RGB,
        .vidcon1        = VIDCON1_INV_HSYNC | VIDCON1_INV_VSYNC,
        .setup_gpio     = exynos4_fimd0_gpio_setup_24bpp,
};

#ifdef CONFIG_S3C64XX_DEV_SPI0
static struct s3c64xx_spi_csinfo spi0_csi[] = {
	[0] = {
		.line = EXYNOS4_GPB(1),
		.fb_delay = 0x0,
	},
};

static struct spi_board_info spi0_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.platform_data = NULL,
		.max_speed_hz = 10*1000*1000,
		.bus_num = 0,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi0_csi[0],
	}
};
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI1
static struct s3c64xx_spi_csinfo spi1_csi[] = {
	[0] = {
		.line = EXYNOS4_GPB(5),
		.fb_delay = 0x0,
	},
};

static struct spi_board_info spi1_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.platform_data = NULL,
		.max_speed_hz = 10*1000*1000,
		.bus_num = 1,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi1_csi[0],
	}
};
#endif

#ifdef CONFIG_S3C64XX_DEV_SPI2
static struct s3c64xx_spi_csinfo spi2_csi[] = {
	[0] = {
		.line = EXYNOS4_GPC1(2),
		.fb_delay = 0x0,
	},
};

static struct spi_board_info spi2_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.platform_data = NULL,
		.max_speed_hz = 10*1000*1000,
		.bus_num = 2,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &spi2_csi[0],
	}
};
#endif

#ifdef CONFIG_LCD_LMS501KF03
static int lcd_power_on(struct lcd_device *ld, int enable)
{
        return 1;
}

static int reset_lcd(struct lcd_device *ld)
{
        int err = 0;
        err = gpio_request_one(EXYNOS4X12_GPM3(6),
                               GPIOF_OUT_INIT_HIGH, "GPM3");
	if (err) {
		pr_err("failed to request GPM3 for lcd reset control\n");
                return err;
	}
        gpio_set_value(EXYNOS4X12_GPM3(6), 0);
        mdelay(1);
        gpio_set_value(EXYNOS4X12_GPM3(6), 1);
	gpio_free(EXYNOS4X12_GPM3(6));

        return 1;
}
static struct lcd_platform_data lms501kf03_platform_data = {
        .reset                  = reset_lcd,
        .power_on               = lcd_power_on,
        .lcd_enabled            = 0,
        .reset_delay            = 100,  /* 100ms */
};
#define         LCD_BUS_NUM     3
static struct spi_board_info spi_board_info[] __initdata = {
        {
                .modalias               = "lms501kf03",
                .platform_data          = (void *)&lms501kf03_platform_data,
                .max_speed_hz           = 1200000,
                .bus_num                = LCD_BUS_NUM,
                .chip_select            = 0,
                .mode                   = SPI_MODE_3,
                .controller_data        = (void *)EXYNOS4_GPB(5),
        }
};

static struct spi_gpio_platform_data lms501kf03_spi_gpio_data = {
        .sck    = EXYNOS4_GPB(4), /* DISPLAY_CLK */
        .mosi   = EXYNOS4_GPB(7), /* DISPLAY_SI */
        .miso   = SPI_GPIO_NO_MISO,
        .num_chipselect = 1,
};

static struct platform_device s3c_device_spi_gpio = {
        .name   = "spi_gpio",
        .id     = LCD_BUS_NUM,
        .dev    = {
                .parent         = &s5p_device_fimd0.dev,
                .platform_data  = &lms501kf03_spi_gpio_data,
        },
};
#endif

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
static void exynos_dwmci_cfg_gpio(int width)
{
	unsigned int gpio;

	for (gpio = EXYNOS4_GPK0(0); gpio < EXYNOS4_GPK0(2); gpio++) {
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_NONE);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
	}

	switch (width) {
	case 8:
		for (gpio = EXYNOS4_GPK1(3); gpio <= EXYNOS4_GPK1(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(4));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
		}
	case 4:
		for (gpio = EXYNOS4_GPK0(3); gpio <= EXYNOS4_GPK0(6); gpio++) {
			s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
			s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
			s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
		}
		break;
	case 1:
		gpio = EXYNOS4_GPK0(3);
		s3c_gpio_cfgpin(gpio, S3C_GPIO_SFN(3));
		s3c_gpio_setpull(gpio, S3C_GPIO_PULL_UP);
		s5p_gpio_set_drvstr(gpio, S5P_GPIO_DRVSTR_LV4);
	default:
		break;
	}

	gpio = EXYNOS4_GPK0(2);
	s3c_gpio_cfgpin(gpio, S3C_GPIO_INPUT);
}

static struct dw_mci_board exynos_dwmci_pdata __initdata = {
	.num_slots		= 1,
	.quirks			= DW_MCI_QUIRK_BROKEN_CARD_DETECTION | DW_MCI_QUIRK_HIGHSPEED,
	.bus_hz			= 100 * 1000 * 1000,
	.caps			= MMC_CAP_UHS_DDR50 | MMC_CAP_1_8V_DDR |
				MMC_CAP_8_BIT_DATA | MMC_CAP_CMD23,
	.fifo_depth		= 0x80,
	.detect_delay_ms	= 200,
	.hclk_name		= "dwmci",
	.cclk_name		= "sclk_dwmci",
	.cfg_gpio		= exynos_dwmci_cfg_gpio,
};
#endif

static struct gpio_event_direct_entry smdk4x12_keypad_key_map[] = {
	{
		.gpio   = EXYNOS4_GPX0(0),
		.code   = KEY_POWER,
	}
};

static struct gpio_event_input_info smdk4x12_keypad_key_info = {
	.info.func              = gpio_event_input_func,
	.info.no_suspend        = true,
	.debounce_time.tv64     = 20 * NSEC_PER_MSEC,
	.type                   = EV_KEY,
	.keymap                 = smdk4x12_keypad_key_map,
	.keymap_size            = ARRAY_SIZE(smdk4x12_keypad_key_map)
};

static struct gpio_event_info *smdk4x12_input_info[] = {
	&smdk4x12_keypad_key_info.info,
};

static struct gpio_event_platform_data smdk4x12_input_data = {
	.names  = {
		"smdk4x12-keypad",
		NULL,
	},
	.info           = smdk4x12_input_info,
	.info_count     = ARRAY_SIZE(smdk4x12_input_info),
};

static struct platform_device smdk4x12_input_device = {
	.name   = GPIO_EVENT_DEV_NAME,
	.id     = 0,
	.dev    = {
		.platform_data = &smdk4x12_input_data,
	},
};

static uint32_t smdk4x12_keymap[] __initdata = {
	/* KEY(row, col, keycode) */
	KEY(1, 3, KEY_1), KEY(1, 4, KEY_2), KEY(1, 5, KEY_3),
	KEY(1, 6, KEY_4), KEY(1, 7, KEY_5),
	KEY(2, 5, KEY_D), KEY(2, 6, KEY_A), KEY(2, 7, KEY_B),
	KEY(0, 7, KEY_E), KEY(0, 5, KEY_C),
	KEY(0, 6, KEY_DOWN), KEY(0, 3, KEY_MENU), KEY(0, 4, KEY_RIGHT)
};

static struct matrix_keymap_data smdk4x12_keymap_data __initdata = {
        .keymap         = smdk4x12_keymap,
        .keymap_size    = ARRAY_SIZE(smdk4x12_keymap),
};

static struct samsung_keypad_platdata smdk4x12_keypad_data __initdata = {
        .keymap_data    = &smdk4x12_keymap_data,
        .rows           = 3,
        .cols           = 8,
};

/* USB OTG */
static struct s3c_hsotg_plat smdk4x12_hsotg_pdata;

static struct platform_device exynos4_bus_devfreq = {
	.name			= "exynos4412-busfreq",
	.id			= 1,
};

static struct platform_device samsung_audio = {
	.name   = "SOC-AUDIO-SAMSUNG",
	.id     = -1,
};

/* USB EHCI */
static struct s5p_ehci_platdata smdk4x12_ehci_pdata;

static void __init smdk4x12_ehci_init(void)
{
	struct s5p_ehci_platdata *pdata = &smdk4x12_ehci_pdata;

	s5p_ehci_set_platdata(pdata);
}

/* USB OHCI */
static struct exynos4_ohci_platdata smdk4x12_ohci_pdata;

static void __init smdk4x12_ohci_init(void)
{
	struct exynos4_ohci_platdata *pdata = &smdk4x12_ohci_pdata;

	exynos4_ohci_set_platdata(pdata);
}

#ifdef CONFIG_BUSFREQ_OPP
/* BUSFREQ to control memory/bus*/
static struct device_domain busfreq;
#endif
static struct platform_device exynos4_busfreq = {
	.id = -1,
	.name = "exynos-busfreq",
};

#ifdef CONFIG_BATTERY_SAMSUNG
static struct platform_device samsung_device_battery = {
	.name	= "samsung-fake-battery",
	.id	= -1,
};
#endif

static struct platform_device *smdk4x12_devices[] __initdata = {
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	&exynos_device_dwmci,
#endif
	&s3c_device_hsmmc2,
	&s3c_device_hsmmc3,
	&wm8994_fixed_voltage0,
        &wm8994_fixed_voltage1,
        &wm8994_fixed_voltage2,
	&s3c_device_i2c0,
	&s3c_device_i2c1,
	&s3c_device_i2c2,
	&s3c_device_i2c3,
#ifdef CONFIG_VIDEO_M5MOLS
	&s3c_device_i2c4,
#endif
	&s3c_device_i2c7,
	&s3c_device_rtc,
	&s3c_device_usb_hsotg,
	&s3c_device_wdt,
	&s5p_device_ehci,
	&exynos4_device_ohci,
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
	&exynos_device_flite0,
	&exynos_device_flite1,
#endif
	&s5p_device_mipi_csis0,
	&s5p_device_mipi_csis1,
	&s5p_device_fimc0,
	&s5p_device_fimc1,
	&s5p_device_fimc2,
	&s5p_device_fimc3,
	&s5p_device_fimc_md,
	&s5p_device_fimd0,
	&mali_gpu_device,
	&s5p_device_mfc,
	&s5p_device_mfc_l,
	&s5p_device_mfc_r,
	&s5p_device_jpeg,
	&samsung_device_keypad,
	&smdk4x12_input_device,
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
	&exynos4_device_fimc_is,
#endif
#ifdef CONFIG_LCD_LMS501KF03
        &s3c_device_spi_gpio,
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI0
	&s3c64xx_device_spi0,
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI1
	&s3c64xx_device_spi1,
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI2
	&s3c64xx_device_spi2,
#endif
#ifdef CONFIG_ION_EXYNOS
        &exynos_device_ion,
#endif
	&s5p_device_i2c_hdmiphy,
	&s5p_device_hdmi,
	&s5p_device_mixer,
	&exynos4_bus_devfreq,
	&samsung_asoc_dma,
	&samsung_asoc_idma,
#ifdef CONFIG_SND_SAMSUNG_I2S
	&exynos4_device_i2s0,
#endif
#ifdef CONFIG_SND_SAMSUNG_PCM
	&exynos4_device_pcm0,
#endif
#ifdef CONFIG_SND_SAMSUNG_SPDIF
        &exynos4_device_spdif,
#endif
	&samsung_audio,
#ifdef CONFIG_VIDEO_EXYNOS_FIMG2D
	&s5p_device_fimg2d,
#endif
#ifdef CONFIG_EXYNOS_THERMAL
	&exynos_device_tmu,
#endif
	&s5p_device_usbswitch,
#if defined CONFIG_SND_SAMSUNG_ALP
        &exynos_device_srp,
#endif
#ifdef CONFIG_BUSFREQ_OPP
	&exynos4_busfreq,
#endif
#ifdef CONFIG_BATTERY_SAMSUNG
	&samsung_device_battery,
#endif
};

static void __init smdk4x12_map_io(void)
{
	clk_xusbxti.rate = 24000000;

	exynos_init_io(NULL, 0);
	s3c24xx_init_clocks(clk_xusbxti.rate);
	s3c24xx_init_uarts(smdk4x12_uartcfgs, ARRAY_SIZE(smdk4x12_uartcfgs));
}

static void __init smdk4x12_reserve(void)
{
	// HACK: This reserved memory will be used for FIMC-IS
	s5p_mfc_reserve_mem(0x58000000, 32<< 20, 0x43000000, 0 << 20);
}

static void smdk4x12_pmu_wdt_init(void)
{
	unsigned int value;

	if (soc_is_exynos4212() || soc_is_exynos4412()) {
		value = __raw_readl(S5P_AUTOMATIC_WDT_RESET_DISABLE);
		value &= ~S5P_SYS_WDTRESET;
		__raw_writel(value, S5P_AUTOMATIC_WDT_RESET_DISABLE);
		value = __raw_readl(S5P_MASK_WDT_RESET_REQUEST);
		value &= ~S5P_SYS_WDTRESET;
		__raw_writel(value, S5P_MASK_WDT_RESET_REQUEST);
	}
}

static void smdk4x12_rtc_wake_init(void)
{
#ifdef CONFIG_PM
	gic_arch_extn.irq_set_wake = s3c_irq_wake;
#endif
}

static struct s3c2410_platform_i2c universal_i2c4_platdata __initdata = {
	.frequency	= 300 * 1000,
	.sda_delay	= 200,
};
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
static void __set_flite_camera_config(struct exynos_platform_flite *data,
                                        u32 active_index, u32 max_cam)
{       
        data->active_cam_index = active_index;
        data->num_clients = max_cam;
}

static void __init smdk4x12_set_camera_flite_platdata(void)
{
        int flite0_cam_index = 0;
        int flite1_cam_index = 0;
#ifdef CONFIG_VIDEO_S5K6A3
#ifdef CONFIG_S5K6A3_CSI_C
        exynos_flite0_default_data.cam[flite0_cam_index++] = &s5k6a3;
#endif
#ifdef CONFIG_S5K6A3_CSI_D
        exynos_flite1_default_data.cam[flite1_cam_index++] = &s5k6a3;
#endif
#endif
        __set_flite_camera_config(&exynos_flite0_default_data, 0, flite0_cam_index);
        __set_flite_camera_config(&exynos_flite1_default_data, 0, flite1_cam_index);
}
#endif

#ifdef CONFIG_USB_EXYNOS_SWITCH
static struct s5p_usbswitch_platdata smdk4x12_usbswitch_pdata;

static void __init smdk4x12_usbswitch_init(void)
{
	struct s5p_usbswitch_platdata *pdata = &smdk4x12_usbswitch_pdata;
	int err;

	pdata->gpio_host_detect = EXYNOS4_GPX3(5); /* low active */
	err = gpio_request_one(pdata->gpio_host_detect, GPIOF_IN,
							"HOST_DETECT");
	if (err) {
		pr_err("failed to request gpio_host_detect\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_host_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_host_detect, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_host_detect);

	pdata->gpio_device_detect = EXYNOS4_GPX3(4); /* high active */
	err = gpio_request_one(pdata->gpio_device_detect, GPIOF_IN,
							"DEVICE_DETECT");
	if (err) {
		pr_err("failed to request gpio_host_detect for\n");
		return;
	}

	s3c_gpio_cfgpin(pdata->gpio_device_detect, S3C_GPIO_SFN(0xF));
	s3c_gpio_setpull(pdata->gpio_device_detect, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_device_detect);

	pdata->gpio_host_vbus = EXYNOS4_GPL2(0);
	err = gpio_request_one(pdata->gpio_host_vbus, GPIOF_OUT_INIT_LOW,
							"HOST_VBUS_CONTROL");
	if (err) {
		pr_err("failed to request gpio_host_vbus\n");
		return;
	}

	s3c_gpio_setpull(pdata->gpio_host_vbus, S3C_GPIO_PULL_NONE);
	gpio_free(pdata->gpio_host_vbus);

	s5p_usbswitch_set_platdata(pdata);
}
#endif

#ifdef CONFIG_VIDEO_EXYNOS_FIMG2D
static struct fimg2d_platdata fimg2d_data __initdata = {
	.ip_ver         = IP_VER_G2D_4P,
	.hw_ver         = 0x41,
	.parent_clkname = "mout_g2d0",
	.clkname        = "sclk_fimg2d",
	.gate_clkname   = "fimg2d",
	.clkrate        = 200 * MHZ,
};
#endif

static int __init exynos4_setup_clock(struct device *dev,
						const char *clock,
						const char *parent,
						unsigned long clk_rate)
{
	struct clk *clk_parent;
	struct clk *sclk;

	sclk = clk_get(dev, clock);
	if (IS_ERR(sclk)) {
		pr_err("Unable to get clock:%s.\n", clock);
		return PTR_ERR(sclk);
	}

	clk_parent = clk_get(NULL, parent);
	if (IS_ERR(clk_parent)) {
		clk_put(sclk);
		pr_err("Unable to get parent clock:%s of clock:%s.\n",
				parent, sclk->name);
		return PTR_ERR(clk_parent);
	}

	if (clk_set_parent(sclk, clk_parent)) {
		pr_err("Unable to set parent %s of clock %s.\n", parent, clock);
		clk_put(sclk);
		clk_put(clk_parent);
		return PTR_ERR(sclk);
	}

	if (clk_rate)
		if (clk_set_rate(sclk, clk_rate)) {
			pr_err("%s rate change failed: %lu\n", sclk->name,
				clk_rate);
			clk_put(sclk);
			clk_put(clk_parent);
			return PTR_ERR(sclk);
		}

	clk_put(sclk);
	clk_put(clk_parent);

	return 0;
}

static void initialize_prime_clocks(void)
{
	exynos4_setup_clock(&s5p_device_fimd0.dev, "sclk_fimd",
                                        "mout_mpll_user", 176 * MHZ);

	exynos4_setup_clock(&s5p_device_fimc0.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc1.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc2.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc3.dev, "sclk_fimc",
					"mout_mpll_user", 176 * MHZ);

	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 176 * MHZ);
	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 176 * MHZ);

	exynos4_setup_clock(NULL, "mout_mfc0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_mfc",
					"mout_mfc0", 220 * MHZ);

	exynos4_setup_clock(NULL, "mout_jpeg0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_jpeg",
					"mout_jpeg0", 176 * MHZ);

	exynos4_setup_clock(&s3c_device_hsmmc2.dev, "dout_mmc2",
					"mout_mpll_user", 100 * MHZ);
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
#ifdef CONFIG_SND_SAMSUNG_I2S_MASTER
	exynos4_setup_clock(&exynos_device_dwmci.dev, "dout_mmc4",
					"mout_epll", 400 * MHZ);
#else
	exynos4_setup_clock(&exynos_device_dwmci.dev, "dout_mmc4",
					"mout_mpll_user", 440 * MHZ);
#endif
#endif
}

static void initialize_non_prime_clocks(void)
{
	exynos4_setup_clock(&s5p_device_fimd0.dev, "sclk_fimd",
                                        "mout_mpll_user", 160 * MHZ);

	exynos4_setup_clock(&s5p_device_fimc0.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc1.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc2.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_fimc3.dev, "sclk_fimc",
					"mout_mpll_user", 160 * MHZ);

	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 160 * MHZ);
	exynos4_setup_clock(&s5p_device_mipi_csis0.dev, "sclk_csis",
					"mout_mpll_user", 160 * MHZ);

	exynos4_setup_clock(NULL, "mout_mfc0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_mfc",
					"mout_mfc0", 200 * MHZ);

	exynos4_setup_clock(NULL, "mout_jpeg0", "mout_mpll", 0);
	exynos4_setup_clock(&s5p_device_mfc.dev, "sclk_jpeg",
					"mout_jpeg0", 160 * MHZ);

	exynos4_setup_clock(&s3c_device_hsmmc2.dev, "dout_mmc2",
					"mout_mpll_user", 100 * MHZ);
#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	exynos4_setup_clock(&exynos_device_dwmci.dev, "dout_mmc4",
					"mout_mpll_user", 400 * MHZ);
#endif
}

static void __init smdk4x12_machine_init(void)
{

#ifdef CONFIG_S3C64XX_DEV_SPI0
	spi_register_board_info(spi0_board_info, ARRAY_SIZE(spi0_board_info));
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI1
	spi_register_board_info(spi1_board_info, ARRAY_SIZE(spi1_board_info));
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI2
	spi_register_board_info(spi2_board_info, ARRAY_SIZE(spi2_board_info));
#endif

	s3c_i2c0_set_platdata(NULL);

	if (samsung_pack() == EXYNOS4412_PACK_SCP)
#ifdef CONFIG_REGULATOR_S5M8767
		i2c_register_board_info(0, smdk4x12_i2c_devs0_s5m8767,
				ARRAY_SIZE(smdk4x12_i2c_devs0_s5m8767));
#endif
	else {
#ifdef CONFIG_REGULATOR_MAX77686
		max77686_populate_pdata();
		i2c_register_board_info(0, smdk4x12_i2c_devs0_max77686,
				ARRAY_SIZE(smdk4x12_i2c_devs0_max77686));
#endif
	}

	s3c_i2c1_set_platdata(NULL);
	i2c_register_board_info(1, smdk4x12_i2c_devs1,
				ARRAY_SIZE(smdk4x12_i2c_devs1));

	s3c_i2c2_set_platdata(NULL);
        i2c_register_board_info(2, smdk4x12_i2c_devs2, ARRAY_SIZE(smdk4x12_i2c_devs2));

	s3c_i2c3_set_platdata(NULL);
	i2c_register_board_info(3, smdk4x12_i2c_devs3,
				ARRAY_SIZE(smdk4x12_i2c_devs3));
	s3c_i2c4_set_platdata(NULL);

	smdk4x12_rtc_wake_init();
	smdk4x12_pmu_wdt_init();
	smdk4x12_touch_init();
	s3c_i2c7_set_platdata(NULL);
	i2c_register_board_info(7, smdk4x12_i2c_devs7,
				ARRAY_SIZE(smdk4x12_i2c_devs7));

	s3c_hsotg_set_platdata(&smdk4x12_hsotg_pdata);
#ifdef CONFIG_USB_EXYNOS_SWITCH
	smdk4x12_usbswitch_init();
#endif
	samsung_bl_set(&smdk4x12_bl_gpio_info, &smdk4x12_bl_data);
	s5p_fimd0_set_platdata(&smdk4x12_lcd0_pdata);
#ifdef CONFIG_LCD_LMS501KF03
        spi_register_board_info(spi_board_info, ARRAY_SIZE(spi_board_info));
#endif
	samsung_keypad_set_platdata(&smdk4x12_keypad_data);

#ifdef CONFIG_EXYNOS4_DEV_DWMCI
	exynos_dwmci_set_platdata(&exynos_dwmci_pdata);
#endif

	s3c_sdhci2_set_platdata(&smdk4x12_hsmmc2_pdata);
	s3c_sdhci3_set_platdata(&smdk4x12_hsmmc3_pdata);

#ifdef CONFIG_ION_EXYNOS
        exynos_ion_set_platdata();
#endif
	s5p_tv_setup();
	s5p_i2c_hdmiphy_set_platdata(NULL);
	s5p_hdmi_set_platdata(smdk4x12_i2c_hdmiphy, NULL, 0);

#ifdef CONFIG_VIDEO_EXYNOS_FIMG2D
	s5p_fimg2d_set_platdata(&fimg2d_data);
#endif
#if defined(CONFIG_VIDEO_M5MOLS) || defined(CONFIG_VIDEO_S5K6A3)
	smdk4x12_camera_init();
#endif
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_LITE
        smdk4x12_set_camera_flite_platdata();
        s3c_set_platdata(&exynos_flite0_default_data,
                        sizeof(exynos_flite0_default_data), &exynos_device_flite0);
        s3c_set_platdata(&exynos_flite1_default_data,
                        sizeof(exynos_flite1_default_data), &exynos_device_flite1);
#endif
	smdk4x12_ehci_init();

#ifdef CONFIG_S3C64XX_DEV_SPI0
	s3c64xx_spi0_set_platdata(NULL, 0, 1);
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI1
	s3c64xx_spi1_set_platdata(NULL, 0, 1);
#endif
#ifdef CONFIG_S3C64XX_DEV_SPI2
	s3c64xx_spi2_set_platdata(NULL, 0, 1);
#endif

	smdk4x12_ohci_init();
	platform_add_devices(smdk4x12_devices, ARRAY_SIZE(smdk4x12_devices));
#ifdef CONFIG_VIDEO_EXYNOS_FIMC_IS
        exynos4_fimc_is_set_platdata(NULL);
#endif

	if (soc_is_exynos4412()) {
		if ((samsung_rev() >= EXYNOS4412_REV_2_0))
			initialize_prime_clocks();
		else
			initialize_non_prime_clocks();
	}
#ifdef CONFIG_BUSFREQ_OPP
        dev_add(&busfreq, &exynos4_busfreq.dev);
        ppmu_init(&exynos_ppmu[PPMU_DMC0], &exynos4_busfreq.dev);
        ppmu_init(&exynos_ppmu[PPMU_DMC1], &exynos4_busfreq.dev);
        ppmu_init(&exynos_ppmu[PPMU_CPU], &exynos4_busfreq.dev);
#endif
	set_tmu_platdata();
}

MACHINE_START(SMDK4212, "SMDK4212")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	.atag_offset	= 0x100,
	.init_irq	= exynos4_init_irq,
	.map_io		= smdk4x12_map_io,
	.handle_irq	= gic_handle_irq,
	.init_machine	= smdk4x12_machine_init,
	.timer		= &exynos4_timer,
	.restart	= exynos4_restart,
	.reserve	= &smdk4x12_reserve,
MACHINE_END

MACHINE_START(SMDK4412, "SMDK4412")
	/* Maintainer: Kukjin Kim <kgene.kim@samsung.com> */
	/* Maintainer: Changhwan Youn <chaos.youn@samsung.com> */
	.atag_offset	= 0x100,
	.init_irq	= exynos4_init_irq,
	.map_io		= smdk4x12_map_io,
	.handle_irq	= gic_handle_irq,
	.init_machine	= smdk4x12_machine_init,
	.init_late	= exynos_init_late,
	.timer		= &exynos4_timer,
	.restart	= exynos4_restart,
	.reserve	= &smdk4x12_reserve,
MACHINE_END
