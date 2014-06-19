/* linux/arch/arm/mach-exynos4/setup-hdmi.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * GPIO configuration for Exynos4 HDMI device
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/gpio.h>
#include <plat/gpio-cfg.h>

void hdmi_hpd_cfg_gpio(bool hpd_to_hdmi)
{
	WARN_ON(gpio_request_one(EXYNOS4_GPX3(7), GPIOF_IN, "hpd-plug"));
	
	if(hpd_to_hdmi) {
		/* direct HPD to HDMI chip */
		s3c_gpio_cfgpin(EXYNOS4_GPX3(7), S3C_GPIO_SFN(0x3));
		s3c_gpio_setpull(EXYNOS4_GPX3(7), S3C_GPIO_PULL_NONE);
	} else {
		/* direct HPD as external interrupt */
		s3c_gpio_cfgpin(EXYNOS4_GPX3(7), S3C_GPIO_SFN(0xf));
		s3c_gpio_setpull(EXYNOS4_GPX3(7), S3C_GPIO_PULL_DOWN);
	}
	gpio_free(EXYNOS4_GPX3(7));
}

int hdmi_hpd_read_gpio(void)
{
	return gpio_get_value(EXYNOS4_GPX3(7));
}
