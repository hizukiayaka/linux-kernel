/*
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * S5P Power Manager (Suspend-To-RAM) support
 *
 * Based on arch/arm/plat-s3c24xx/pm.c
 * Copyright (c) 2004,2006 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/suspend.h>
#include <plat/pm.h>

#include <mach/regs-clock.h>

#define PFX "s5p pm: "

/* s3c_pm_configure_extint
 *
 * configure all external interrupt pins
*/

static struct sleep_save exynos4_peril0_save[] = {
	SAVE_ITEM(EXYNOS4_CLKSRC_PERIL0),
	SAVE_ITEM(EXYNOS4_CLKDIV_PERIL0),
};

void s3c_pm_configure_extint(void)
{
	/* nothing here yet */
}

void s3c_pm_restore_core(void)
{
	s3c_pm_do_restore_core(exynos4_peril0_save, ARRAY_SIZE(exynos4_peril0_save));
}

void s3c_pm_save_core(void)
{
	s3c_pm_do_save(exynos4_peril0_save, ARRAY_SIZE(exynos4_peril0_save));
}

