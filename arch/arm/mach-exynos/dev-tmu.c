/* linux/arch/arm/mach-exynos/dev-tmu.c
 *
 * Copyright 2012 by SAMSUNG
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/platform_device.h>
#include <linux/slab.h>

#include <asm/irq.h>

#include <mach/irqs.h>
#include <mach/map.h>

#include <plat/devs.h>
#include <linux/platform_data/exynos_thermal.h>

#define IRQ_TMU EXYNOS4_IRQ_TMU_CPU0

static struct resource tmu_resource[] = {
	[0] = {
		.start	= EXYNOS4_PA_TMU,
		.end	= EXYNOS4_PA_TMU + 0xFFFF - 1,
		.flags	= IORESOURCE_MEM,
	},
	[1] = {
		.start	= IRQ_TMU,
		.end	= IRQ_TMU,
		.flags	= IORESOURCE_IRQ,
	},
};

struct platform_device exynos_device_tmu = {
	.name	= "exynos4210-tmu",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(tmu_resource),
	.resource	= tmu_resource,
};

static struct exynos_tmu_platform_data const exynos_default_tmu_data = {
	.trigger_levels[0] = 85,
	.trigger_levels[1] = 103,
	.trigger_levels[2] = 110,
	.trigger_level0_en = 1,
	.trigger_level1_en = 1,
	.trigger_level2_en = 1,
	.trigger_level3_en = 0,
	.gain = 8,
	.reference_voltage = 16,
	.noise_cancel_mode = 4,
	.cal_type = TYPE_ONE_POINT_TRIMMING,
	.efuse_value = 55,
	.freq_tab[0] = {
		.freq_clip_max = 800 * 1000,
	},
	.freq_tab[1] = {
		.freq_clip_max = 200 * 1000,
	},
	.freq_tab_count = 2,
	.type = SOC_ARCH_EXYNOS,
};

void __init set_tmu_platdata(void)
{
	s3c_set_platdata(&exynos_default_tmu_data,
			 sizeof(struct exynos_tmu_platform_data),
			 &exynos_device_tmu);
}
