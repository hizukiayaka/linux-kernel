/*
 * linux/arch/arm/mach-exynos/tiny4412-lcds.c
 *
 * Copyright (c) 2013 FriendlyARM (www.arm9.net)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/module.h>

#include <mach/s3cfb.h>


/* s3cfb configs for supported LCD */

static struct s3cfb_lcd wxga_hd700 = {
	.width = 800,
	.height = 1280,
	.p_width = 94,
	.p_height = 151,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 20,
		.h_bp = 20,
		.h_sw = 24,
		.v_fp =  4,
		.v_fpe = 1,
		.v_bp =  4,
		.v_bpe = 1,
		.v_sw =  8,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 0,
		.inv_vsync = 0,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_s70 = {
	.width = 800,
	.height = 480,
	.p_width = 155,
	.p_height = 93,
	.bpp = 32,
	.freq = 63,

	.timing = {
		.h_fp = 80,
		.h_bp = 36,
		.h_sw = 10,
		.v_fp = 22,
		.v_fpe = 1,
		.v_bp = 15,
		.v_bpe = 1,
		.v_sw = 8,
	},
	.polarity = {
		.rise_vclk = 1,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wvga_w50 = {
	.width= 800,
	.height = 480,
	.p_width = 108,
	.p_height = 64,
	.bpp = 32,
	.freq = 70,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 48,
		.v_fp = 20,
		.v_fpe = 1,
		.v_bp = 20,
		.v_bpe = 1,
		.v_sw = 12,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd wsvga_w101 = {
	.width= 1024,
	.height = 600,
	.p_width = 204,
	.p_height = 120,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 40,
		.h_bp = 40,
		.h_sw = 120,
		.v_fp = 10,
		.v_fpe = 1,
		.v_bp = 10,
		.v_bpe = 1,
		.v_sw = 12,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct s3cfb_lcd xga_a97 = {
	.width = 1024,
	.height = 768,
	.p_width = 200,
	.p_height = 150,
	.bpp = 32,
	.freq = 61,

	.timing = {
		.h_fp = 12,
		.h_bp = 12,
		.h_sw = 4,
		.v_fp = 8,
		.v_fpe = 1,
		.v_bp = 8,
		.v_bpe = 1,
		.v_sw =  4,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

/* HDMI */
static struct s3cfb_lcd hdmi_def = {
	.width = 1920,
	.height = 1080,
	.p_width = 480,
	.p_height = 320,
	.bpp = 32,
	.freq = 60,

	.timing = {
		.h_fp = 12,
		.h_bp = 12,
		.h_sw = 4,
		.v_fp = 8,
		.v_fpe = 1,
		.v_bp = 8,
		.v_bpe = 1,
		.v_sw =  4,
	},
	.polarity = {
		.rise_vclk = 0,
		.inv_hsync = 1,
		.inv_vsync = 1,
		.inv_vden = 0,
	},
};

static struct hdmi_config {
	char *name;
	int width;
	int height;
} tiny4412_hdmi_config[] = {
	{ "HDMI1080P60",	1920, 1080 },
	{ "HDMI1080I60",	1920, 1080 },
	{ "HDMI1080P30",	1920, 1080 },

	{ "HDMI1080P60D",	 960,  536 },
	{ "HDMI1080I60D",	 960,  536 },
	{ "HDMI1080P30D",	 960,  536 },

	{ "HDMI720P60",		1280,  720 },
	{ "HDMI720P60D",	 640,  360 },

	{ "HDMI576P16X9",	 720,  576 },
	{ "HDMI576P16X9D",	 720,  576 },
	{ "HDMI576P4X3",	 720,  576 },
	{ "HDMI576P4X3D",	 720,  576 },

	{ "HDMI480P16X9",	 720,  480 },
	{ "HDMI480P16X9D",	 720,  480 },
	{ "HDMI480P4X3",	 720,  480 },
	{ "HDMI480P4X3D",	 720,  480 },
};


/* Try to guess LCD panel by kernel command line, or
 * using *W50* as default */

static struct {
	char *name;
	struct s3cfb_lcd *lcd;
	int ctp;
} tiny4412_lcd_config[] = {
	{ "HD700",	&wxga_hd700, 1 },
	{ "S70",	&wvga_s70,   1 },
	{ "W50",	&wvga_w50,   0 },
	{ "W101",	&wsvga_w101, 0 },
	{ "A97",	&xga_a97,    0 },
	{ "HDM",	&hdmi_def,   0 },	/* Pls keep it at last */
};

static int lcd_idx = 0;

static int __init tiny4412_setup_lcd(char *str)
{
	int i;

	if (!strncasecmp("HDMI", str, 4)) {
		struct hdmi_config *cfg = &tiny4412_hdmi_config[0];
		struct s3cfb_lcd *lcd;

		lcd_idx = ARRAY_SIZE(tiny4412_lcd_config) - 1;
		lcd = tiny4412_lcd_config[lcd_idx].lcd;

		for (i = 0; i < ARRAY_SIZE(tiny4412_hdmi_config); i++, cfg++) {
			if (!strcasecmp(cfg->name, str)) {
				lcd->width = cfg->width;
				lcd->height = cfg->height;
				goto __ret;
			}
		}
	}

	for (i = 0; i < ARRAY_SIZE(tiny4412_lcd_config); i++) {
		if (!strcasecmp(tiny4412_lcd_config[i].name, str)) {
			lcd_idx = i;
			break;
		}
	}

__ret:
	printk("TINY4412: %s selected\n", tiny4412_lcd_config[lcd_idx].name);
	return 0;
}
early_param("lcd", tiny4412_setup_lcd);


struct s3cfb_lcd *tiny4412_get_lcd(void)
{
	return tiny4412_lcd_config[lcd_idx].lcd;
}

void tiny4412_get_lcd_res(int *w, int *h)
{
	struct s3cfb_lcd *lcd = tiny4412_lcd_config[lcd_idx].lcd;

	if (w)
		*w = lcd->width;
	if (h)
		*h = lcd->height;

	return;
}
EXPORT_SYMBOL(tiny4412_get_lcd_res);


#if defined(CONFIG_TOUCHSCREEN_GOODIX) || defined(CONFIG_TOUCHSCREEN_FT5X0X) || \
	defined(CONFIG_TOUCHSCREEN_1WIRE)
#include <plat/ctouch.h>

static unsigned int ctp_type = CTP_AUTO;

static int __init tiny4412_init_ctp(char *str)
{
	unsigned int val;
	char *p = str, *end;

	val = simple_strtoul(p, &end, 10);
	if (end <= p) {
		return 1;
	}

	if (val < CTP_MAX && tiny4412_lcd_config[lcd_idx].ctp) {
		ctp_type = val;
	}

	return 1;
}
__setup("ctp=", tiny4412_init_ctp);

unsigned int tiny4412_get_ctp(void)
{
	return ctp_type;
}
EXPORT_SYMBOL(tiny4412_get_ctp);

void tiny4412_set_ctp(int type)
{
	if (ctp_type == CTP_AUTO && type < CTP_MAX) {
		ctp_type = type;
	}
}
EXPORT_SYMBOL(tiny4412_set_ctp);
#endif

