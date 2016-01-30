/*
 * Samsung HDMI interface driver
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 *
 * Tomasz Stanislawski, <t.stanislaws@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published
 * by the Free Software Foundiation. either version 2 of the License,
 * or (at your option) any later version
 */

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_DEBUG
#define DEBUG
#endif

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>

#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/delay.h>
#include <linux/bug.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/regulator/consumer.h>


#include <media/s5p_hdmi.h>


#include <mach/gpio.h>
#include <mach/cpufreq.h>
#include "regs-hdmi.h"
#include "hdmi.h"

MODULE_AUTHOR("Tomasz Stanislawski, <t.stanislaws@samsung.com>");
MODULE_DESCRIPTION("Samsung HDMI");
MODULE_LICENSE("GPL");

/* default preset configured on probe */
#define HDMI_DEFAULT_PRESET V4L2_DV_1080P60
#define BUSFREQ_400MHZ        400000

extern int exynos4_busfreq_lock(bool);

static const struct hdmi_preset_conf hdmi_conf_480p60 = {
	.core = {
		.h_blank = {0x8a, 0x00},
		.v2_blank = {0x0d, 0x02},
		.v1_blank = {0x2d, 0x00},
		.v_line = {0x0d, 0x02},
		.h_line = {0x5a, 0x03},
		.hsync_pol = {0x01},
		.vsync_pol = {0x01},
		.int_pro_mode = {0x00},
		.v_blank_f0 = {0xff, 0xff},
		.v_blank_f1 = {0xff, 0xff},
		.h_sync_start = {0x0e, 0x00},
		.h_sync_end = {0x4c, 0x00},
		.v_sync_line_bef_2 = {0x0f, 0x00},
		.v_sync_line_bef_1 = {0x09, 0x00},
		.v_sync_line_aft_2 = {0xff, 0xff},
		.v_sync_line_aft_1 = {0xff, 0xff},
		.v_sync_line_aft_pxl_2 = {0xff, 0xff},
		.v_sync_line_aft_pxl_1 = {0xff, 0xff},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
		.vact_space_2 = {0xff, 0xff},
		.vact_space_3 = {0xff, 0xff},
		.vact_space_4 = {0xff, 0xff},
		.vact_space_5 = {0xff, 0xff},
		.vact_space_6 = {0xff, 0xff},
		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0x5a, 0x03, /* h_fsz */
		0x8a, 0x00, 0xd0, 0x02, /* hact */
		0x0d, 0x02, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x2d, 0x00, 0xe0, 0x01, /* vact */
		0x33, 0x02, /* field_chg */
		0x48, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x01, 0x00, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const struct hdmi_preset_conf hdmi_conf_720p50 = {
	.core = {
		.h_blank = {0xbc, 0x02},
		.v2_blank = {0xee, 0x02},
		.v1_blank = {0x1e, 0x00},
		.v_line = {0xee, 0x02},
		.h_line = {0xbc, 0x07},
		.hsync_pol = {0x00},
		.vsync_pol = {0x00},
		.int_pro_mode = {0x00},
		.v_blank_f0 = {0xff, 0xff},
		.v_blank_f1 = {0xff, 0xff},
		.h_sync_start = {0xb6, 0x01},
		.h_sync_end = {0xde, 0x01},
		.v_sync_line_bef_2 = {0x0a, 0x00},
		.v_sync_line_bef_1 = {0x05, 0x00},
		.v_sync_line_aft_2 = {0xff, 0xff},
		.v_sync_line_aft_1 = {0xff, 0xff},
		.v_sync_line_aft_pxl_2 = {0xff, 0xff},
		.v_sync_line_aft_pxl_1 = {0xff, 0xff},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
		.vact_space_2 = {0xff, 0xff},
		.vact_space_3 = {0xff, 0xff},
		.vact_space_4 = {0xff, 0xff},
		.vact_space_5 = {0xff, 0xff},
		.vact_space_6 = {0xff, 0xff},
		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0xbc, 0x07, /* h_fsz */
		0xbc, 0x02, 0x00, 0x05, /* hact */
		0xee, 0x02, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x1e, 0x00, 0xd0, 0x02, /* vact */
		0x33, 0x02, /* field_chg */
		0x48, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x01, 0x00, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const struct hdmi_preset_conf hdmi_conf_720p60 = {
	.core = {
		.h_blank = {0x72, 0x01},
		.v2_blank = {0xee, 0x02},
		.v1_blank = {0x1e, 0x00},
		.v_line = {0xee, 0x02},
		.h_line = {0x72, 0x06},
		.hsync_pol = {0x00},
		.vsync_pol = {0x00},
		.int_pro_mode = {0x00},
		.v_blank_f0 = {0xff, 0xff},
		.v_blank_f1 = {0xff, 0xff},
		.h_sync_start = {0x6c, 0x00},
		.h_sync_end = {0x94, 0x00},
		.v_sync_line_bef_2 = {0x0a, 0x00},
		.v_sync_line_bef_1 = {0x05, 0x00},
		.v_sync_line_aft_2 = {0xff, 0xff},
		.v_sync_line_aft_1 = {0xff, 0xff},
		.v_sync_line_aft_pxl_2 = {0xff, 0xff},
		.v_sync_line_aft_pxl_1 = {0xff, 0xff},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
		.vact_space_2 = {0xff, 0xff},
		.vact_space_3 = {0xff, 0xff},
		.vact_space_4 = {0xff, 0xff},
		.vact_space_5 = {0xff, 0xff},
		.vact_space_6 = {0xff, 0xff},
		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0x72, 0x06, /* h_fsz */
		0x72, 0x01, 0x00, 0x05, /* hact */
		0xee, 0x02, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x1e, 0x00, 0xd0, 0x02, /* vact */
		0x33, 0x02, /* field_chg */
		0x48, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x01, 0x00, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const struct hdmi_preset_conf hdmi_conf_1080i50 = {
	.core = {
		.h_blank = {0xd0, 0x02},
		.v2_blank = {0x32, 0x02},
		.v1_blank = {0x16, 0x00},
		.v_line = {0x65, 0x04},
		.h_line = {0x50, 0x0a},
		.hsync_pol = {0x00},
		.vsync_pol = {0x00},
		.int_pro_mode = {0x01},
		.v_blank_f0 = {0x49, 0x02},
		.v_blank_f1 = {0x65, 0x04},
		.h_sync_start = {0x0e, 0x02},
		.h_sync_end = {0x3a, 0x02},
		.v_sync_line_bef_2 = {0x07, 0x00},
		.v_sync_line_bef_1 = {0x02, 0x00},
		.v_sync_line_aft_2 = {0x39, 0x02},
		.v_sync_line_aft_1 = {0x34, 0x02},
		.v_sync_line_aft_pxl_2 = {0x38, 0x07},
		.v_sync_line_aft_pxl_1 = {0x38, 0x07},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
		.vact_space_2 = {0xff, 0xff},
		.vact_space_3 = {0xff, 0xff},
		.vact_space_4 = {0xff, 0xff},
		.vact_space_5 = {0xff, 0xff},
		.vact_space_6 = {0xff, 0xff},
		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0x50, 0x0a, /* h_fsz */
		0xd0, 0x02, 0x80, 0x07, /* hact */
		0x65, 0x04, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x16, 0x00, 0x1c, 0x02, /* vact */
		0x33, 0x02, /* field_chg */
		0x49, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x33, 0x02, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const struct hdmi_preset_conf hdmi_conf_1080i60 = {
	.core = {
		.h_blank = {0x18, 0x01},
		.v2_blank = {0x32, 0x02},
		.v1_blank = {0x16, 0x00},
		.v_line = {0x65, 0x04},
		.h_line = {0x98, 0x08},
		.hsync_pol = {0x00},
		.vsync_pol = {0x00},
		.int_pro_mode = {0x01},
		.v_blank_f0 = {0x49, 0x02},
		.v_blank_f1 = {0x65, 0x04},
		.h_sync_start = {0x56, 0x00},
		.h_sync_end = {0x82, 0x00},
		.v_sync_line_bef_2 = {0x07, 0x00},
		.v_sync_line_bef_1 = {0x02, 0x00},
		.v_sync_line_aft_2 = {0x39, 0x02},
		.v_sync_line_aft_1 = {0x34, 0x02},
		.v_sync_line_aft_pxl_2 = {0xa4, 0x04},
		.v_sync_line_aft_pxl_1 = {0xa4, 0x04},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
		.vact_space_2 = {0xff, 0xff},
		.vact_space_3 = {0xff, 0xff},
		.vact_space_4 = {0xff, 0xff},
		.vact_space_5 = {0xff, 0xff},
		.vact_space_6 = {0xff, 0xff},
		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0x98, 0x08, /* h_fsz */
		0x18, 0x01, 0x80, 0x07, /* hact */
		0x65, 0x04, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x16, 0x00, 0x1c, 0x02, /* vact */
		0x33, 0x02, /* field_chg */
		0x49, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x33, 0x02, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const struct hdmi_preset_conf hdmi_conf_1080p30 = {
	.core = {
		.h_blank = {0x18, 0x01},
		.v2_blank = {0x65, 0x04},
		.v1_blank = {0x2d, 0x00},
		.v_line = {0x65, 0x04},
		.h_line = {0x98, 0x08},
		.hsync_pol = {0x00},
		.vsync_pol = {0x00},
		.int_pro_mode = {0x00},
		.v_blank_f0 = {0xff, 0xff},
		.v_blank_f1 = {0xff, 0xff},
		.h_sync_start = {0x56, 0x00},
		.h_sync_end = {0x82, 0x00},
		.v_sync_line_bef_2 = {0x09, 0x00},
		.v_sync_line_bef_1 = {0x04, 0x00},
		.v_sync_line_aft_2 = {0xff, 0xff},
		.v_sync_line_aft_1 = {0xff, 0xff},
		.v_sync_line_aft_pxl_2 = {0xff, 0xff},
		.v_sync_line_aft_pxl_1 = {0xff, 0xff},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
		.vact_space_2 = {0xff, 0xff},
		.vact_space_3 = {0xff, 0xff},
		.vact_space_4 = {0xff, 0xff},
		.vact_space_5 = {0xff, 0xff},
		.vact_space_6 = {0xff, 0xff},
		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0x98, 0x08, /* h_fsz */
		0x18, 0x01, 0x80, 0x07, /* hact */
		0x65, 0x04, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x2d, 0x00, 0x38, 0x04, /* vact */
		0x33, 0x02, /* field_chg */
		0x48, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x01, 0x00, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const struct hdmi_preset_conf hdmi_conf_1080p50 = {
	.core = {
		.h_blank = {0xd0, 0x02},
		.v2_blank = {0x65, 0x04},
		.v1_blank = {0x2d, 0x00},
		.v_line = {0x65, 0x04},
		.h_line = {0x50, 0x0a},
		.hsync_pol = {0x00},
		.vsync_pol = {0x00},
		.int_pro_mode = {0x00},
		.v_blank_f0 = {0xff, 0xff},
		.v_blank_f1 = {0xff, 0xff},
		.h_sync_start = {0x0e, 0x02},
		.h_sync_end = {0x3a, 0x02},
		.v_sync_line_bef_2 = {0x09, 0x00},
		.v_sync_line_bef_1 = {0x04, 0x00},
		.v_sync_line_aft_2 = {0xff, 0xff},
		.v_sync_line_aft_1 = {0xff, 0xff},
		.v_sync_line_aft_pxl_2 = {0xff, 0xff},
		.v_sync_line_aft_pxl_1 = {0xff, 0xff},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
		.vact_space_2 = {0xff, 0xff},
		.vact_space_3 = {0xff, 0xff},
		.vact_space_4 = {0xff, 0xff},
		.vact_space_5 = {0xff, 0xff},
		.vact_space_6 = {0xff, 0xff},
		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0x50, 0x0a, /* h_fsz */
		0xd0, 0x02, 0x80, 0x07, /* hact */
		0x65, 0x04, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x2d, 0x00, 0x38, 0x04, /* vact */
		0x33, 0x02, /* field_chg */
		0x48, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x01, 0x00, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const struct hdmi_preset_conf hdmi_conf_1080p60 = {
	.core = {
		.h_blank = {0x18, 0x01},
		.v2_blank = {0x65, 0x04},
		.v1_blank = {0x2d, 0x00},
		.v_line = {0x65, 0x04},
		.h_line = {0x98, 0x08},
		.hsync_pol = {0x00},
		.vsync_pol = {0x00},
		.int_pro_mode = {0x00},
		.v_blank_f0 = {0xff, 0xff},
		.v_blank_f1 = {0xff, 0xff},
		.h_sync_start = {0x56, 0x00},
		.h_sync_end = {0x82, 0x00},
		.v_sync_line_bef_2 = {0x09, 0x00},
		.v_sync_line_bef_1 = {0x04, 0x00},
		.v_sync_line_aft_2 = {0xff, 0xff},
		.v_sync_line_aft_1 = {0xff, 0xff},
		.v_sync_line_aft_pxl_2 = {0xff, 0xff},
		.v_sync_line_aft_pxl_1 = {0xff, 0xff},
		.v_blank_f2 = {0xff, 0xff},
		.v_blank_f3 = {0xff, 0xff},
		.v_blank_f4 = {0xff, 0xff},
		.v_blank_f5 = {0xff, 0xff},
		.v_sync_line_aft_3 = {0xff, 0xff},
		.v_sync_line_aft_4 = {0xff, 0xff},
		.v_sync_line_aft_5 = {0xff, 0xff},
		.v_sync_line_aft_6 = {0xff, 0xff},
		.v_sync_line_aft_pxl_3 = {0xff, 0xff},
		.v_sync_line_aft_pxl_4 = {0xff, 0xff},
		.v_sync_line_aft_pxl_5 = {0xff, 0xff},
		.v_sync_line_aft_pxl_6 = {0xff, 0xff},
		.vact_space_1 = {0xff, 0xff},
                .vact_space_2 = {0xff, 0xff},
                .vact_space_3 = {0xff, 0xff},
                .vact_space_4 = {0xff, 0xff},
                .vact_space_5 = {0xff, 0xff},
                .vact_space_6 = {0xff, 0xff},

		/* other don't care */
	},
	.tg = {
		0x00, /* cmd */
		0x98, 0x08, /* h_fsz */
		0x18, 0x01, 0x80, 0x07, /* hact */
		0x65, 0x04, /* v_fsz */
		0x01, 0x00, 0x33, 0x02, /* vsync */
		0x2d, 0x00, 0x38, 0x04, /* vact */
		0x33, 0x02, /* field_chg */
		0x48, 0x02, /* vact_st2 */
		0x00, 0x00, /* vact_st3 */
		0x00, 0x00, /* vact_st4 */
		0x01, 0x00, 0x01, 0x00, /* vsync top/bot */
		0x01, 0x00, 0x33, 0x02, /* field top/bot */
		0x00, /* 3d FP */
	},
};

static const u8 hdmiphy_conf27_027[32] = {
	0x01, 0xd1, 0x2d, 0x72, 0x40, 0x64, 0x12, 0x08,
	0x43, 0xa0, 0x0e, 0xd9, 0x45, 0xa0, 0xac, 0x80,
	0x08, 0x80, 0x11, 0x04, 0x02, 0x22, 0x44, 0x86,
	0x54, 0xe3, 0x24, 0x00, 0x00, 0x00, 0x01, 0x00,
};

static const u8 hdmiphy_conf74_176[32] = {
	0x01, 0xd1, 0x1f, 0x10, 0x40, 0x5b, 0xef, 0x08,
	0x81, 0xa0, 0xb9, 0xd8, 0x45, 0xa0, 0xac, 0x80,
	0x5a, 0x80, 0x11, 0x04, 0x02, 0x22, 0x44, 0x86,
	0x54, 0xa6, 0x24, 0x01, 0x00, 0x00, 0x01, 0x00,
};

static const u8 hdmiphy_conf74_25[32] = {
	0x01, 0xd1, 0x1f, 0x10, 0x40, 0x40, 0xf8, 0x08,
	0x81, 0xa0, 0xba, 0xd8, 0x45, 0xa0, 0xac, 0x80,
	0x3c, 0x80, 0x11, 0x04, 0x02, 0x22, 0x44, 0x86,
	0x54, 0xa5, 0x24, 0x01, 0x00, 0x00, 0x01, 0x00,
};

static const u8 hdmiphy_conf148_5[32] = {
	0x01, 0xd1, 0x1f, 0x00, 0x40, 0x40, 0xf8, 0x08,
	0x81, 0xa0, 0xba, 0xd8, 0x45, 0xa0, 0xac, 0x80,
	0x3c, 0x80, 0x11, 0x04, 0x02, 0x22, 0x44, 0x86,
	0x54, 0x4b, 0x25, 0x03, 0x00, 0x00, 0x01, 0x00,
};

const struct {
        u32 width;
	u32 height;
	int vrefresh;
	bool interlace;
	const u8 *hdmiphy_data;
        u32 preset;
        const struct hdmi_preset_conf *timings;
}hdmi_timings[]={
        { 720, 480, 60, false, hdmiphy_conf27_027,V4L2_DV_480P59_94, &hdmi_conf_480p60 },
	{ 1280, 720, 50, false, hdmiphy_conf74_25,V4L2_DV_720P50, &hdmi_conf_720p50 },
	{ 1280, 720, 60, false, hdmiphy_conf74_25,V4L2_DV_720P60, &hdmi_conf_720p60 },
	{ 1920, 1080, 50, true, hdmiphy_conf74_25,V4L2_DV_1080I50, &hdmi_conf_1080i50 },
	{ 1920, 1080, 60, true, hdmiphy_conf74_25,V4L2_DV_1080I60, &hdmi_conf_1080i60 },
	{ 1920, 1080, 30, false, hdmiphy_conf74_176,V4L2_DV_1080P30, &hdmi_conf_1080p30 },
	{ 1920, 1080, 50, false, hdmiphy_conf148_5,V4L2_DV_1080P50, &hdmi_conf_1080p50 },
	{ 1920, 1080, 60, false, hdmiphy_conf148_5,V4L2_DV_1080P60, &hdmi_conf_1080p60 },
};

static struct platform_device_id hdmi_driver_types[] = {
	{
		.name		= "s5pv210-hdmi",
	}, {
		.name		= "exynos4-hdmi",
	}, {
		/* end node */
	}
};

static const struct v4l2_subdev_ops hdmi_sd_ops;

static struct hdmi_device *sd_to_hdmi_dev(struct v4l2_subdev *sd)
{
	return container_of(sd, struct hdmi_device, sd);
}

static inline
void hdmi_write(struct hdmi_device *hdev, u32 reg_id, u32 value)
{
	writel(value, hdev->regs + reg_id);
}

static inline
void hdmi_write_mask(struct hdmi_device *hdev, u32 reg_id, u32 value, u32 mask)
{
	u32 old = readl(hdev->regs + reg_id);
	value = (value & mask) | (old & ~mask);
	writel(value, hdev->regs + reg_id);
}

static inline
void hdmi_writeb(struct hdmi_device *hdev, u32 reg_id, u8 value)
{
	writeb(value, hdev->regs + reg_id);
}

static inline
void hdmi_writebn(struct hdmi_device *hdev, u32 reg_id, int n, u32 value)
{
	switch (n) {
	default:
		writeb(value >> 24, hdev->regs + reg_id + 12);
	case 3:
		writeb(value >> 16, hdev->regs + reg_id + 8);
	case 2:
		writeb(value >>  8, hdev->regs + reg_id + 4);
	case 1:
		writeb(value >>  0, hdev->regs + reg_id + 0);
	}
}

static inline u32 hdmi_read(struct hdmi_device *hdev, u32 reg_id)
{
	return readl(hdev->regs + reg_id);
}

static irqreturn_t hdmi_ext_irq_handler(int irq, void *dev_data)
{
	struct hdmi_device *hdmi_dev = dev_data;
	struct s5p_hdmi_platform_data *pdata = hdmi_dev->dev->platform_data;
	(void) irq;
	if(pdata->read_gpio())	{
		irq_set_irq_type(hdmi_dev->ext_irq, IRQ_TYPE_LEVEL_LOW);
		hdmi_dev->connected = true;
		schedule_work(&hdmi_dev->work);
	} else {
		irq_set_irq_type(hdmi_dev->ext_irq, IRQ_TYPE_LEVEL_HIGH);
		hdmi_dev->connected = false;
		schedule_work(&hdmi_dev->work);
	}
	return IRQ_HANDLED;
}

static irqreturn_t hdmi_irq_handler(int irq, void *dev_data)
{
	struct hdmi_device *hdev = dev_data;
	u32 intc_flag;

	(void)irq;
	intc_flag = hdmi_read(hdev, HDMI_INTC_FLAG);

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_HDCP
	if (intc_flag & S5P_HDMI_INTC_FLAG_HDCP) {
		return s5p_hdcp_irq_handler(irq, dev_data);
	}
#endif
	/* clearing flags for HPD plug/unplug */
	if (intc_flag & HDMI_INTC_FLAG_HPD_UNPLUG) {
		hdmi_write_mask(hdev, HDMI_INTC_FLAG, ~0,
			HDMI_INTC_FLAG_HPD_UNPLUG);

		hdev->connected = false;
		schedule_work(&hdev->work);
	}
	if (intc_flag & HDMI_INTC_FLAG_HPD_PLUG) {
		hdmi_write_mask(hdev, HDMI_INTC_FLAG, ~0,
			HDMI_INTC_FLAG_HPD_PLUG);

		hdev->connected = true;
		schedule_work(&hdev->work);
	}

	return IRQ_HANDLED;
}

static void hdmi_hpd_work(struct work_struct *data)
{
	struct hdmi_device *hdmi_dev = container_of(data, struct hdmi_device, work);
	int connected;
	if (hdmi_dev->connected != hdmi_dev->sw_connected.state) {
		connected = hdmi_dev->connected;
		switch_set_state(&hdmi_dev->sw_connected, connected);
	}
	if (hdmi_dev->connected)
		printk(KERN_INFO "HDMI plugged\n");
	else
		printk(KERN_INFO "HDMI unplugged\n");
}

static void hdmi_reg_init(struct hdmi_device *hdev)
{
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_HDCP
	hdmi_write_mask(hdev, HDMI_INTC_CON, ~0, HDMI_INTC_EN_GLOBAL |
		HDMI_INTC_EN_HPD_PLUG | HDMI_INTC_EN_HPD_UNPLUG);
#else
	hdmi_write_mask(hdev, HDMI_INTC_CON, ~0, HDMI_INTC_EN_GLOBAL);
#endif
	/* choose HDMI mode */
	hdmi_write_mask(hdev, HDMI_MODE_SEL,HDMI_MODE_HDMI_EN, HDMI_MODE_MASK);
	/* disable bluescreen */
	hdmi_write_mask(hdev, HDMI_CON_0, 0, HDMI_BLUE_SCR_EN);

	hdmi_writeb(hdev, HDMI_AVI_CON, 0x02);
	hdmi_writeb(hdev, HDMI_AVI_BYTE(1), 2 << 5);

}


static void hdmi_timing_apply(struct hdmi_device *hdev,
	const struct hdmi_preset_conf *conf)
{
	/* setting core registers */
	const struct hdmi_core_regs *core = &conf->core;
        const struct hdmi_tg_regs *tg = &conf->tg;
	hdmi_writeb(hdev, HDMI_H_BLANK_0, core->h_blank[0]);
	hdmi_writeb(hdev, HDMI_H_BLANK_1, core->h_blank[1]);
	hdmi_writeb(hdev, HDMI_V2_BLANK_0, core->v2_blank[0]);
	hdmi_writeb(hdev, HDMI_V2_BLANK_1, core->v2_blank[1]);
	hdmi_writeb(hdev, HDMI_V1_BLANK_0, core->v1_blank[0]);
	hdmi_writeb(hdev, HDMI_V1_BLANK_1, core->v1_blank[1]);
	hdmi_writeb(hdev, HDMI_V_LINE_0, core->v_line[0]);
	hdmi_writeb(hdev, HDMI_V_LINE_1, core->v_line[1]);
	hdmi_writeb(hdev, HDMI_H_LINE_0, core->h_line[0]);
	hdmi_writeb(hdev, HDMI_H_LINE_1, core->h_line[1]);
	hdmi_writeb(hdev, HDMI_HSYNC_POL, core->hsync_pol[0]);
	hdmi_writeb(hdev, HDMI_VSYNC_POL, core->vsync_pol[0]);
	hdmi_writeb(hdev, HDMI_INT_PRO_MODE, core->int_pro_mode[0]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F0_0, core->v_blank_f0[0]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F0_1, core->v_blank_f0[1]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F1_0, core->v_blank_f1[0]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F1_1, core->v_blank_f1[1]);
	hdmi_writeb(hdev, HDMI_H_SYNC_START_0, core->h_sync_start[0]);
	hdmi_writeb(hdev, HDMI_H_SYNC_START_1, core->h_sync_start[1]);
	hdmi_writeb(hdev, HDMI_H_SYNC_END_0, core->h_sync_end[0]);
	hdmi_writeb(hdev, HDMI_H_SYNC_END_1, core->h_sync_end[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_BEF_2_0,
			core->v_sync_line_bef_2[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_BEF_2_1,
			core->v_sync_line_bef_2[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_BEF_1_0,
			core->v_sync_line_bef_1[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_BEF_1_1,
			core->v_sync_line_bef_1[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_2_0,
			core->v_sync_line_aft_2[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_2_1,
			core->v_sync_line_aft_2[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_1_0,
			core->v_sync_line_aft_1[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_1_1,
			core->v_sync_line_aft_1[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_2_0,
			core->v_sync_line_aft_pxl_2[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_2_1,
			core->v_sync_line_aft_pxl_2[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_1_0,
			core->v_sync_line_aft_pxl_1[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_1_1,
			core->v_sync_line_aft_pxl_1[1]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F2_0, core->v_blank_f2[0]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F2_1, core->v_blank_f2[1]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F3_0, core->v_blank_f3[0]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F3_1, core->v_blank_f3[1]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F4_0, core->v_blank_f4[0]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F4_1, core->v_blank_f4[1]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F5_0, core->v_blank_f5[0]);
	hdmi_writeb(hdev, HDMI_V_BLANK_F5_1, core->v_blank_f5[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_3_0,
			core->v_sync_line_aft_3[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_3_1,
			core->v_sync_line_aft_3[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_4_0,
			core->v_sync_line_aft_4[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_4_1,
			core->v_sync_line_aft_4[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_5_0,
			core->v_sync_line_aft_5[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_5_1,
			core->v_sync_line_aft_5[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_6_0,
			core->v_sync_line_aft_6[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_6_1,
			core->v_sync_line_aft_6[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_3_0,
			core->v_sync_line_aft_pxl_3[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_3_1,
			core->v_sync_line_aft_pxl_3[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_4_0,
			core->v_sync_line_aft_pxl_4[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_4_1,
			core->v_sync_line_aft_pxl_4[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_5_0,
			core->v_sync_line_aft_pxl_5[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_5_1,
			core->v_sync_line_aft_pxl_5[1]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_6_0,
			core->v_sync_line_aft_pxl_6[0]);
	hdmi_writeb(hdev, HDMI_V_SYNC_LINE_AFT_PXL_6_1,
			core->v_sync_line_aft_pxl_6[1]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_1_0, core->vact_space_1[0]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_1_1, core->vact_space_1[1]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_2_0, core->vact_space_2[0]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_2_1, core->vact_space_2[1]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_3_0, core->vact_space_3[0]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_3_1, core->vact_space_3[1]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_4_0, core->vact_space_4[0]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_4_1, core->vact_space_4[1]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_5_0, core->vact_space_5[0]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_5_1, core->vact_space_5[1]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_6_0, core->vact_space_6[0]);
	hdmi_writeb(hdev, HDMI_VACT_SPACE_6_1, core->vact_space_6[1]);

	/* Timing generator registers */
	hdmi_writeb(hdev, HDMI_TG_H_FSZ_L, tg->h_fsz_l);
	hdmi_writeb(hdev, HDMI_TG_H_FSZ_H, tg->h_fsz_h);
	hdmi_writeb(hdev, HDMI_TG_HACT_ST_L, tg->hact_st_l);
	hdmi_writeb(hdev, HDMI_TG_HACT_ST_H, tg->hact_st_h);
	hdmi_writeb(hdev, HDMI_TG_HACT_SZ_L, tg->hact_sz_l);
	hdmi_writeb(hdev, HDMI_TG_HACT_SZ_H, tg->hact_sz_h);
	hdmi_writeb(hdev, HDMI_TG_V_FSZ_L, tg->v_fsz_l);
	hdmi_writeb(hdev, HDMI_TG_V_FSZ_H, tg->v_fsz_h);
	hdmi_writeb(hdev, HDMI_TG_VSYNC_L, tg->vsync_l);
	hdmi_writeb(hdev, HDMI_TG_VSYNC_H, tg->vsync_h);
	hdmi_writeb(hdev, HDMI_TG_VSYNC2_L, tg->vsync2_l);
	hdmi_writeb(hdev, HDMI_TG_VSYNC2_H, tg->vsync2_h);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST_L, tg->vact_st_l);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST_H, tg->vact_st_h);
	hdmi_writeb(hdev, HDMI_TG_VACT_SZ_L, tg->vact_sz_l);
	hdmi_writeb(hdev, HDMI_TG_VACT_SZ_H, tg->vact_sz_h);
	hdmi_writeb(hdev, HDMI_TG_FIELD_CHG_L, tg->field_chg_l);
	hdmi_writeb(hdev, HDMI_TG_FIELD_CHG_H, tg->field_chg_h);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST2_L, tg->vact_st2_l);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST2_H, tg->vact_st2_h);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST3_L, tg->vact_st3_l);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST3_H, tg->vact_st3_h);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST4_L, tg->vact_st4_l);
	hdmi_writeb(hdev, HDMI_TG_VACT_ST4_H, tg->vact_st4_h);
	hdmi_writeb(hdev, HDMI_TG_VSYNC_TOP_HDMI_L, tg->vsync_top_hdmi_l);
	hdmi_writeb(hdev, HDMI_TG_VSYNC_TOP_HDMI_H, tg->vsync_top_hdmi_h);
	hdmi_writeb(hdev, HDMI_TG_VSYNC_BOT_HDMI_L, tg->vsync_bot_hdmi_l);
	hdmi_writeb(hdev, HDMI_TG_VSYNC_BOT_HDMI_H, tg->vsync_bot_hdmi_h);
	hdmi_writeb(hdev, HDMI_TG_FIELD_TOP_HDMI_L, tg->field_top_hdmi_l);
	hdmi_writeb(hdev, HDMI_TG_FIELD_TOP_HDMI_H, tg->field_top_hdmi_h);
	hdmi_writeb(hdev, HDMI_TG_FIELD_BOT_HDMI_L, tg->field_bot_hdmi_l);
	hdmi_writeb(hdev, HDMI_TG_FIELD_BOT_HDMI_H, tg->field_bot_hdmi_h);
	hdmi_writeb(hdev, HDMI_TG_3D, tg->tg_3d);

	if (core->int_pro_mode[0])
		hdmi_write_mask(hdev, HDMI_TG_CMD, ~0, HDMI_FIELD_EN);
	else
		hdmi_write_mask(hdev, HDMI_TG_CMD, 0, HDMI_FIELD_EN);

}

static void hdmi_set_acr(u32 freq, u8 *acr)
{
        u32 n, cts;

        switch (freq) {
        case 32000:
                n = 4096;
                cts = 27000;
                break;
        case 44100:
                n = 6272;
                cts = 30000;
                break;
        case 88200:
                n = 12544;
                cts = 30000;
                break;
        case 176400:
                n = 25088;
                cts = 30000;
                break;
        case 48000:
                n = 6144;
                cts = 27000;
                break;
        case 96000:
                n = 12288;
                cts = 27000;
                break;
        case 192000:
                n = 24576;
                cts = 27000;
                break;
        default:
                n = 0;
                cts = 0;
                break;
        }

        acr[1] = cts >> 16;
        acr[2] = cts >> 8 & 0xff;
        acr[3] = cts & 0xff;

	acr[4] = n >> 16;
        acr[5] = n >> 8 & 0xff;
        acr[6] = n & 0xff;
}

static void hdmi_reg_acr(struct hdmi_device *hdev, u8 *acr)
{
	hdmi_writeb(hdev, HDMI_ACR_N0, acr[6]);
	hdmi_writeb(hdev, HDMI_ACR_N1, acr[5]);
	hdmi_writeb(hdev, HDMI_ACR_N2, acr[4]);
	hdmi_writeb(hdev, HDMI_ACR_MCTS0, acr[3]);
	hdmi_writeb(hdev, HDMI_ACR_MCTS1, acr[2]);
	hdmi_writeb(hdev, HDMI_ACR_MCTS2, acr[1]);
	hdmi_writeb(hdev, HDMI_ACR_CTS0, acr[3]);
	hdmi_writeb(hdev, HDMI_ACR_CTS1, acr[2]);
	hdmi_writeb(hdev, HDMI_ACR_CTS2, acr[1]);
	hdmi_writeb(hdev, HDMI_ACR_CON, 4);
}

void hdmi_reg_asp(struct hdmi_device *hdev, u32 channel)
{
        if (channel == 2)
                hdmi_writeb(hdev, HDMI_ASP_CON, HDMI_AUD_NO_DST_DOUBLE | HDMI_AUD_TYPE_SAMPLE |
                        HDMI_AUD_MODE_TWO_CH | HDMI_AUD_SP_ALL_DIS);
        else            
                hdmi_writeb(hdev, HDMI_ASP_CON, HDMI_AUD_MODE_MULTI_CH | HDMI_AUD_SP_AUD2_EN |
                        HDMI_AUD_SP_AUD1_EN | HDMI_AUD_SP_AUD0_EN);
        
        hdmi_writeb(hdev, HDMI_ASP_SP_FLAT, HDMI_ASP_SP_FLAT_AUD_SAMPLE);

        if (channel == 2) {
                hdmi_writeb(hdev, HDMI_ASP_CHCFG0, HDMI_SPK0R_SEL_I_PCM0R | HDMI_SPK0L_SEL_I_PCM0L);
                hdmi_writeb(hdev, HDMI_ASP_CHCFG1, HDMI_SPK0R_SEL_I_PCM0R | HDMI_SPK0L_SEL_I_PCM0L);
                hdmi_writeb(hdev, HDMI_ASP_CHCFG2, HDMI_SPK0R_SEL_I_PCM0R | HDMI_SPK0L_SEL_I_PCM0L);
                hdmi_writeb(hdev, HDMI_ASP_CHCFG3, HDMI_SPK0R_SEL_I_PCM0R | HDMI_SPK0L_SEL_I_PCM0L);
        } else {
                hdmi_writeb(hdev, HDMI_ASP_CHCFG0, HDMI_SPK0R_SEL_I_PCM0R | HDMI_SPK0L_SEL_I_PCM0L);
                hdmi_writeb(hdev, HDMI_ASP_CHCFG1, HDMI_SPK0R_SEL_I_PCM1L | HDMI_SPK0L_SEL_I_PCM1R);
                hdmi_writeb(hdev, HDMI_ASP_CHCFG2, HDMI_SPK0R_SEL_I_PCM2R | HDMI_SPK0L_SEL_I_PCM2L);
                hdmi_writeb(hdev, HDMI_ASP_CHCFG3, HDMI_SPK0R_SEL_I_PCM3R | HDMI_SPK0L_SEL_I_PCM3L);
        }
}

static void hdmi_audio_init(struct hdmi_device *hdev)
{
	u32 sample_rate, bits_per_sample, frame_size_code;
	u32 data_num, bit_ch, val;
	u8 acr[7];
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_SPDIF
	int rep_time;
#endif
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_I2S
	u32 sample_frq;
#endif
	sample_rate = 44100;
	bits_per_sample = 16;
	frame_size_code = 0;

	switch (bits_per_sample) {
	case 20:
		data_num = 2;
		bit_ch  = 1;
		break;
	case 24:
		data_num = 3;
		bit_ch  = 1;
		break;
	default:
		data_num = 1;
		bit_ch  = 0;
		break;
	}

	hdmi_set_acr(sample_rate, acr);
	hdmi_reg_acr(hdev, acr);
	hdmi_reg_asp(hdev,2);

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_I2S
	hdmi_writeb(hdev, HDMI_I2S_MUX_CON, HDMI_I2S_IN_DISABLE
				| HDMI_I2S_AUD_I2S | HDMI_I2S_CUV_I2S_ENABLE
				| HDMI_I2S_MUX_ENABLE);

	hdmi_writeb(hdev, HDMI_I2S_MUX_CH, HDMI_I2S_CH0_EN
			| HDMI_I2S_CH1_EN | HDMI_I2S_CH2_EN);

	hdmi_writeb(hdev, HDMI_I2S_MUX_CUV, HDMI_I2S_CUV_RL_EN);

	sample_frq = (sample_rate == 44100) ? 0 :
			(sample_rate == 48000) ? 2 :
			(sample_rate == 32000) ? 3 :
			(sample_rate == 96000) ? 0xa : 0x0;

	hdmi_writeb(hdev, HDMI_I2S_CLK_CON, HDMI_I2S_CLK_DIS);
	hdmi_writeb(hdev, HDMI_I2S_CLK_CON, HDMI_I2S_CLK_EN);

	val = hdmi_read(hdev, HDMI_I2S_DSD_CON) | 0x01;
	hdmi_writeb(hdev, HDMI_I2S_DSD_CON, val);

	/* Configuration I2S input ports. Configure I2S_PIN_SEL_0~4 */
	hdmi_writeb(hdev, HDMI_I2S_PIN_SEL_0, HDMI_I2S_SEL_SCLK(5)
			| HDMI_I2S_SEL_LRCK(6));
	hdmi_writeb(hdev, HDMI_I2S_PIN_SEL_1, HDMI_I2S_SEL_SDATA1(1)
			| HDMI_I2S_SEL_SDATA2(4));
	hdmi_writeb(hdev, HDMI_I2S_PIN_SEL_2, HDMI_I2S_SEL_SDATA3(1)
			| HDMI_I2S_SEL_SDATA2(2));
	hdmi_writeb(hdev, HDMI_I2S_PIN_SEL_3, HDMI_I2S_SEL_DSD(0));

	/* I2S_CON_1 & 2 */
	hdmi_writeb(hdev, HDMI_I2S_CON_1, HDMI_I2S_SCLK_FALLING_EDGE
			| HDMI_I2S_L_CH_LOW_POL);
	hdmi_writeb(hdev, HDMI_I2S_CON_2, HDMI_I2S_MSB_FIRST_MODE
			| HDMI_I2S_SET_BIT_CH(bit_ch)
			| HDMI_I2S_SET_SDATA_BIT(data_num)
			| HDMI_I2S_BASIC_FORMAT);

	/* Configure register related to CUV information */
	hdmi_writeb(hdev, HDMI_I2S_CH_ST_0, HDMI_I2S_CH_STATUS_MODE_0
			| HDMI_I2S_2AUD_CH_WITHOUT_PREEMPH
			| HDMI_I2S_COPYRIGHT
			| HDMI_I2S_LINEAR_PCM
			| HDMI_I2S_CONSUMER_FORMAT);
	hdmi_writeb(hdev, HDMI_I2S_CH_ST_1, HDMI_I2S_CD_PLAYER);
	hdmi_writeb(hdev, HDMI_I2S_CH_ST_2, HDMI_I2S_SET_SOURCE_NUM(0));
	hdmi_writeb(hdev, HDMI_I2S_CH_ST_3, HDMI_I2S_CLK_ACCUR_LEVEL_2
			| HDMI_I2S_SET_SMP_FREQ(sample_frq));
	hdmi_writeb(hdev, HDMI_I2S_CH_ST_4,
			HDMI_I2S_ORG_SMP_FREQ_44_1
			| HDMI_I2S_WORD_LEN_MAX24_24BITS
			| HDMI_I2S_WORD_LEN_MAX_24BITS);

	hdmi_writeb(hdev, HDMI_I2S_CH_ST_CON, HDMI_I2S_CH_STATUS_RELOAD);
#endif

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_SPDIF
	hdmi_write(hdev, HDMI_I2S_CLK_CON, HDMI_I2S_CLK_EN);

        val = HDMI_SPDIFIN_CFG_NOISE_FILTER_2_SAMPLE |
                HDMI_SPDIFIN_CFG_PCPD_MANUAL |
                HDMI_SPDIFIN_CFG_WORD_LENGTH_MANUAL |
                HDMI_SPDIFIN_CFG_UVCP_REPORT |
                HDMI_SPDIFIN_CFG_HDMI_2_BURST |
                HDMI_SPDIFIN_CFG_DATA_ALIGN_32;
        hdmi_write(hdev, HDMI_SPDIFIN_CONFIG_1, val);
        hdmi_write(hdev, HDMI_SPDIFIN_CONFIG_2, 0);

        rep_time = 0;
        val = HDMI_SPDIFIN_USER_VAL_REPETITION_TIME_LOW(rep_time) |
                HDMI_SPDIFIN_USER_VAL_WORD_LENGTH_24;
        hdmi_write(hdev, HDMI_SPDIFIN_USER_VALUE_1, val);
        val = HDMI_SPDIFIN_USER_VAL_REPETITION_TIME_HIGH(rep_time);
        hdmi_write(hdev, HDMI_SPDIFIN_USER_VALUE_2, val);
        hdmi_write(hdev, HDMI_SPDIFIN_USER_VALUE_3, 0);
        hdmi_write(hdev, HDMI_SPDIFIN_USER_VALUE_4, 0);

        val = HDMI_I2S_IN_ENABLE | HDMI_I2S_AUD_SPDIF | HDMI_I2S_MUX_ENABLE;
        hdmi_write(hdev, HDMI_I2S_MUX_CON, val);

        hdmi_write(hdev, HDMI_I2S_MUX_CH, HDMI_I2S_CH_ALL_EN);
        hdmi_write(hdev, HDMI_I2S_MUX_CUV, HDMI_I2S_CUV_RL_EN);

        hdmi_write_mask(hdev, HDMI_SPDIFIN_CLK_CTRL, 0, HDMI_SPDIFIN_CLK_ON);
        hdmi_write_mask(hdev, HDMI_SPDIFIN_CLK_CTRL, ~0, HDMI_SPDIFIN_CLK_ON);

        hdmi_write(hdev, HDMI_SPDIFIN_OP_CTRL, HDMI_SPDIFIN_STATUS_CHECK_MODE);
        hdmi_write(hdev, HDMI_SPDIFIN_OP_CTRL,
                        HDMI_SPDIFIN_STATUS_CHECK_MODE_HDMI);

#endif
}

static void hdmi_audio_control(struct hdmi_device *hdev, bool onoff)
{
	hdmi_writeb(hdev, HDMI_AUI_CON, onoff ? 2 : 0);
	hdmi_write_mask(hdev, HDMI_CON_0, onoff ?
			HDMI_ASP_EN : HDMI_ASP_DIS, HDMI_ASP_MASK);
}

static int hdmiphy_conf_apply(struct hdmi_device *hdmi_dev,u32 preset)
{
	const u8 *data;
	u8 buffer[32];
	int ret;
	int i;
	u8 operation[2];
	u8 read_buffer[32] = {0, };
        
	for (i = 0; i < ARRAY_SIZE(hdmi_timings); ++i)
	{
		if (hdmi_timings[i].preset == preset)
		{
		  data = hdmi_timings[i].hdmiphy_data;
		  break;
		}
	}	
	
	if (!data) {
		dev_err(hdmi_dev->dev, "format not supported\n");
		return -EINVAL;
	}
	
	/* storing configuration to the device */
	memcpy(buffer, data, 32);
	ret = i2c_master_send(hdmi_dev->hdmiphy_port, buffer, 32);
	if (ret != 32) {
		dev_err(hdmi_dev->dev, "failed to configure HDMIPHY via I2C\n");
		return -EIO;
	}

        mdelay(10);

	/* operation mode */
	operation[0] = 0x1f;
	operation[1] = 0x80;

	ret = i2c_master_send(hdmi_dev->hdmiphy_port, operation, 2);
	if (ret != 2) {
		dev_err(hdmi_dev->dev,"failed to enable hdmiphy\n");
		return -EIO;
	}

	ret = i2c_master_recv(hdmi_dev->hdmiphy_port, read_buffer, 32);
	if (ret < 0) {
		dev_err(hdmi_dev->dev,"failed to read hdmiphy config\n");
		return -EIO;
	}

	return 0;
}


static int hdmi_conf_apply(struct hdmi_device *hdmi_dev)
{
	struct device *dev = hdmi_dev->dev;
	const struct hdmi_preset_conf *conf = hdmi_dev->cur_conf;
	struct hdmi_resources *res = &hdmi_dev->res;
	struct v4l2_dv_preset preset;
        u8 buffer[2];
	dev_dbg(dev, "%s\n", __func__);

        clk_disable(res->sclk_hdmi);
	clk_set_parent(res->sclk_hdmi, res->sclk_pixel);
	clk_enable(res->sclk_hdmi);

	/* operation mode */
	buffer[0] = 0x1f;
	buffer[1] = 0x00;

	i2c_master_send(hdmi_dev->hdmiphy_port, buffer, 32);
	/* skip if conf is already synchronized with HW */
	if (!hdmi_dev->cur_conf_dirty)
		return 0;

	/* reset hdmiphy */
	hdmi_write_mask(hdmi_dev, HDMI_PHY_RSTOUT, ~0, HDMI_PHY_SW_RSTOUT);
	mdelay(10);
	hdmi_write_mask(hdmi_dev, HDMI_PHY_RSTOUT,  0, HDMI_PHY_SW_RSTOUT);
	//mdelay(10);

	/* configure presets */
	preset.preset = hdmi_dev->cur_preset;
	hdmiphy_conf_apply(hdmi_dev,preset.preset);
	
	/* resetting HDMI core */
	hdmi_write_mask(hdmi_dev, HDMI_CORE_RSTOUT,  0, HDMI_CORE_SW_RSTOUT);
	mdelay(10);
	hdmi_write_mask(hdmi_dev, HDMI_CORE_RSTOUT, ~0, HDMI_CORE_SW_RSTOUT);
	//mdelay(10);

	hdmi_reg_init(hdmi_dev);
        hdmi_audio_init(hdmi_dev);
	/* setting core registers */
	hdmi_timing_apply(hdmi_dev, conf);

	hdmi_dev->cur_conf_dirty = 0;

	return 0;
}

static void hdmi_dumpregs(struct hdmi_device *hdev, char *prefix)
{
	int i;
#define DUMPREG(reg_id) \
	dev_dbg(hdev->dev, "%s:" #reg_id " = %08x\n", prefix, \
		readl(hdev->regs + reg_id))
	dev_dbg(hdev->dev, "%s: ---- CONTROL REGISTERS ----\n", prefix);
	DUMPREG(HDMI_INTC_CON);
	DUMPREG(HDMI_INTC_FLAG);
	DUMPREG(HDMI_HPD_STATUS);
	DUMPREG(HDMI_INTC_CON_1);
	DUMPREG(HDMI_INTC_FLAG_1);
	DUMPREG(HDMI_PHY_STATUS_0);
	DUMPREG(HDMI_PHY_STATUS_PLL);
	DUMPREG(HDMI_PHY_CON_0);
	DUMPREG(HDMI_PHY_RSTOUT);
	DUMPREG(HDMI_PHY_VPLL);
	DUMPREG(HDMI_PHY_CMU);
	DUMPREG(HDMI_CORE_RSTOUT);

	dev_dbg(hdev->dev, "%s: ---- CORE REGISTERS ----\n", prefix);
	DUMPREG(HDMI_CON_0);
	DUMPREG(HDMI_CON_1);
	DUMPREG(HDMI_CON_2);
	DUMPREG(HDMI_SYS_STATUS);
	DUMPREG(HDMI_PHY_STATUS_0);
	DUMPREG(HDMI_STATUS_EN);
	DUMPREG(HDMI_HPD);
	DUMPREG(HDMI_MODE_SEL);
	DUMPREG(HDMI_ENC_EN);
	DUMPREG(HDMI_DC_CONTROL);
	DUMPREG(HDMI_VIDEO_PATTERN_GEN);
        DUMPREG(HDMI_YMAX);
        DUMPREG(HDMI_YMIN);
        DUMPREG(HDMI_CMAX);
        DUMPREG(HDMI_CMIN);
        DUMPREG(HDMI_ACR_N0);
        DUMPREG(HDMI_ACR_MCTS0);
        DUMPREG(HDMI_ACR_CTS0);
        DUMPREG(HDMI_ACR_CON);
        DUMPREG(HDMI_ASP_CON);
        DUMPREG(HDMI_GCP_CON);
        DUMPREG(HDMI_ACP_TYPE);


	dev_dbg(hdev->dev, "%s: ---- CORE SYNC REGISTERS ----\n", prefix);
	DUMPREG(HDMI_H_BLANK_0);
	DUMPREG(HDMI_H_BLANK_1);
	DUMPREG(HDMI_V2_BLANK_0);
	DUMPREG(HDMI_V2_BLANK_1);
	DUMPREG(HDMI_V1_BLANK_0);
	DUMPREG(HDMI_V1_BLANK_1);
	DUMPREG(HDMI_V_LINE_0);
	DUMPREG(HDMI_V_LINE_1);
	DUMPREG(HDMI_H_LINE_0);
	DUMPREG(HDMI_H_LINE_1);
	DUMPREG(HDMI_HSYNC_POL);

	DUMPREG(HDMI_VSYNC_POL);
	DUMPREG(HDMI_INT_PRO_MODE);
	DUMPREG(HDMI_V_BLANK_F0_0);
	DUMPREG(HDMI_V_BLANK_F0_1);
	DUMPREG(HDMI_V_BLANK_F1_0);
	DUMPREG(HDMI_V_BLANK_F1_1);

	DUMPREG(HDMI_H_SYNC_START_0);
	DUMPREG(HDMI_H_SYNC_START_1);
	DUMPREG(HDMI_H_SYNC_END_0);
	DUMPREG(HDMI_H_SYNC_END_1);

	DUMPREG(HDMI_V_SYNC_LINE_BEF_2_0);
	DUMPREG(HDMI_V_SYNC_LINE_BEF_2_1);
	DUMPREG(HDMI_V_SYNC_LINE_BEF_1_0);
	DUMPREG(HDMI_V_SYNC_LINE_BEF_1_1);

	DUMPREG(HDMI_V_SYNC_LINE_AFT_2_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_2_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_1_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_1_1);

	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_2_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_2_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_1_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_1_1);

	DUMPREG(HDMI_V_BLANK_F2_0);
	DUMPREG(HDMI_V_BLANK_F2_1);
	DUMPREG(HDMI_V_BLANK_F3_0);
	DUMPREG(HDMI_V_BLANK_F3_1);
	DUMPREG(HDMI_V_BLANK_F4_0);
	DUMPREG(HDMI_V_BLANK_F4_1);
	DUMPREG(HDMI_V_BLANK_F5_0);
	DUMPREG(HDMI_V_BLANK_F5_1);

	DUMPREG(HDMI_V_SYNC_LINE_AFT_3_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_3_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_4_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_4_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_5_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_5_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_6_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_6_1);

	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_3_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_3_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_4_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_4_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_5_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_5_1);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_6_0);
	DUMPREG(HDMI_V_SYNC_LINE_AFT_PXL_6_1);

	DUMPREG(HDMI_VACT_SPACE_1_0);
	DUMPREG(HDMI_VACT_SPACE_1_1);
	DUMPREG(HDMI_VACT_SPACE_2_0);
	DUMPREG(HDMI_VACT_SPACE_2_1);
	DUMPREG(HDMI_VACT_SPACE_3_0);
	DUMPREG(HDMI_VACT_SPACE_3_1);
	DUMPREG(HDMI_VACT_SPACE_4_0);
	DUMPREG(HDMI_VACT_SPACE_4_1);
	DUMPREG(HDMI_VACT_SPACE_5_0);
	DUMPREG(HDMI_VACT_SPACE_5_1);
	DUMPREG(HDMI_VACT_SPACE_6_0);
	DUMPREG(HDMI_VACT_SPACE_6_1);

	dev_dbg(hdev->dev, "%s: ---- TG REGISTERS ----\n", prefix);
	DUMPREG(HDMI_TG_CMD);
	DUMPREG(HDMI_TG_H_FSZ_L);
	DUMPREG(HDMI_TG_H_FSZ_H);
	DUMPREG(HDMI_TG_HACT_ST_L);
	DUMPREG(HDMI_TG_HACT_ST_H);
	DUMPREG(HDMI_TG_HACT_SZ_L);
	DUMPREG(HDMI_TG_HACT_SZ_H);
	DUMPREG(HDMI_TG_V_FSZ_L);
	DUMPREG(HDMI_TG_V_FSZ_H);
	DUMPREG(HDMI_TG_VSYNC_L);
	DUMPREG(HDMI_TG_VSYNC_H);
	DUMPREG(HDMI_TG_VSYNC2_L);
	DUMPREG(HDMI_TG_VSYNC2_H);
	DUMPREG(HDMI_TG_VACT_ST_L);
	DUMPREG(HDMI_TG_VACT_ST_H);
	DUMPREG(HDMI_TG_VACT_SZ_L);
	DUMPREG(HDMI_TG_VACT_SZ_H);
	DUMPREG(HDMI_TG_FIELD_CHG_L);
	DUMPREG(HDMI_TG_FIELD_CHG_H);
	DUMPREG(HDMI_TG_VACT_ST2_L);
	DUMPREG(HDMI_TG_VACT_ST2_H);
	DUMPREG(HDMI_TG_VACT_ST3_L);
	DUMPREG(HDMI_TG_VACT_ST3_H);
	DUMPREG(HDMI_TG_VACT_ST4_L);
	DUMPREG(HDMI_TG_VACT_ST4_H);
	DUMPREG(HDMI_TG_VSYNC_TOP_HDMI_L);
	DUMPREG(HDMI_TG_VSYNC_TOP_HDMI_H);
	DUMPREG(HDMI_TG_VSYNC_BOT_HDMI_L);
	DUMPREG(HDMI_TG_VSYNC_BOT_HDMI_H);
	DUMPREG(HDMI_TG_FIELD_TOP_HDMI_L);
	DUMPREG(HDMI_TG_FIELD_TOP_HDMI_H);
	DUMPREG(HDMI_TG_FIELD_BOT_HDMI_L);
	DUMPREG(HDMI_TG_FIELD_BOT_HDMI_H);
	DUMPREG(HDMI_TG_3D);

	dev_dbg(hdev->dev, "%s: ---- PACKET REGISTERS ----\n", prefix);
	DUMPREG(HDMI_AVI_CON);
	DUMPREG(HDMI_AVI_HEADER0);
	DUMPREG(HDMI_AVI_HEADER1);
	DUMPREG(HDMI_AVI_HEADER2);
	DUMPREG(HDMI_AVI_CHECK_SUM);
	DUMPREG(HDMI_VSI_CON);
	DUMPREG(HDMI_VSI_HEADER0);
	DUMPREG(HDMI_VSI_HEADER1);
	DUMPREG(HDMI_VSI_HEADER2);
	for (i = 0; i < 7; ++i)
		DUMPREG(HDMI_VSI_DATA(i));

#undef DUMPREG
}

static const struct hdmi_preset_conf *hdmi_preset2timings(u32 preset)
{
	int i;

	for (i = 0; i < ARRAY_SIZE(hdmi_timings); ++i)
		if (hdmi_timings[i].preset == preset)
			return  hdmi_timings[i].timings;
	return NULL;
}

static int hdmi_streamon(struct hdmi_device *hdev)
{
	struct device *dev = hdev->dev;
	struct hdmi_resources *res = &hdev->res;
	int ret, tries;

	dev_dbg(dev, "%s\n", __func__);

	ret = hdmi_conf_apply(hdev);
	if (ret)
		return ret;

	ret = v4l2_subdev_call(hdev->phy_sd, video, s_stream, 1);
	if (ret)
		return ret;

	/* waiting for HDMIPHY's PLL to get to steady state */
	for (tries = 100; tries; --tries) {
		u32 val = hdmi_read(hdev, HDMI_PHY_STATUS_0);
		if (val & HDMI_PHY_STATUS_READY)
			break;
		mdelay(1);
	}
	/* steady state not achieved */
	if (tries == 0) {
		dev_err(dev, "hdmiphy's pll could not reach steady state.\n");
		v4l2_subdev_call(hdev->phy_sd, video, s_stream, 0);
		hdmi_dumpregs(hdev, "hdmiphy - s_stream");
		return -EIO;
	}

	/* hdmiphy clock is used for HDMI in streaming mode */
	clk_disable(res->sclk_hdmi);
	clk_set_parent(res->sclk_hdmi, res->sclk_hdmiphy);
	clk_enable(res->sclk_hdmi);

	/* enable HDMI and timing generator */
	hdmi_write_mask(hdev, HDMI_CON_0, ~0, HDMI_EN);
	hdmi_write_mask(hdev, HDMI_TG_CMD, ~0, HDMI_TG_EN);
	hdmi_audio_control(hdev,true);
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_HDCP
	on_stop_process = false;
	if (hdev->hdcp_enabled)
		s5p_hdcp_start(hdev);
#endif
	hdmi_dumpregs(hdev,"For_Audio");
	return 0;
}

static int hdmi_streamoff(struct hdmi_device *hdev)
{
	struct device *dev = hdev->dev;
	struct hdmi_resources *res = &hdev->res;

	dev_dbg(dev, "%s\n", __func__);
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_HDCP
	on_stop_process = true;
	if (hdev->hdcp_enabled)
		s5p_hdcp_stop();
#endif
	hdmi_write_mask(hdev, HDMI_CON_0, 0, HDMI_EN);
	hdmi_write_mask(hdev, HDMI_TG_CMD, 0, HDMI_TG_EN);

	/* pixel(vpll) clock is used for HDMI in config mode */
	clk_disable(res->sclk_hdmi);
	clk_set_parent(res->sclk_hdmi, res->sclk_pixel);
	clk_enable(res->sclk_hdmi);

	v4l2_subdev_call(hdev->phy_sd, video, s_stream, 0);

	hdmi_dumpregs(hdev, "streamoff");
	return 0;
}

static int hdmi_s_stream(struct v4l2_subdev *sd, int enable)
{
	struct hdmi_device *hdev = sd_to_hdmi_dev(sd);
	struct device *dev = hdev->dev;
        int ret = 0;
	dev_dbg(dev, "%s(%d)\n", __func__, enable);
	if (enable) {
        exynos_cpufreq_lock_freq(enable, MAX_CPU_FREQ);
#ifdef CONFIG_BUSFREQ_OPP
        dev_lock(hdev->bus_dev, dev, BUSFREQ_400MHZ);
#endif
    	ret = hdmi_streamon(hdev);
        } else {
	ret = hdmi_streamoff(hdev);
#ifdef CONFIG_BUSFREQ_OPP
        dev_unlock(hdev->bus_dev, dev);
#endif
        exynos_cpufreq_lock_freq(enable, MAX_CPU_FREQ);
        }
    return ret;
}

static void hdmi_resource_poweron(struct hdmi_resources *res)
{
	/* turn HDMI power on */
	regulator_bulk_enable(res->regul_count, res->regul_bulk);
	/* power-on hdmi physical interface */
	clk_enable(res->hdmiphy);
	/* use VPP as parent clock; HDMIPHY is not working yet */
	clk_set_parent(res->sclk_hdmi, res->sclk_pixel);
	clk_enable(res->hdmi);
	/* turn clocks on */
	clk_enable(res->sclk_hdmi);
}

static void hdmi_resource_poweroff(struct hdmi_resources *res)
{
	/* turn clocks off */
	clk_disable(res->sclk_hdmi);
	clk_disable(res->hdmi);
	/* power-off hdmiphy */
	clk_disable(res->hdmiphy);
	/* turn HDMI power off */
	regulator_bulk_disable(res->regul_count, res->regul_bulk);
}

static int hdmi_s_power(struct v4l2_subdev *sd, int on)
{
	struct hdmi_device *hdev = sd_to_hdmi_dev(sd);
	struct s5p_hdmi_platform_data *pdata = hdev->dev->platform_data;
	int ret;

#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_HDCP
	if (on) {
		ret = pm_runtime_get_sync(hdev->dev);
		disable_irq(hdev->ext_irq);
		enable_irq(hdev->irq);
	} else {
		disable_irq(hdev->irq);
		enable_irq(hdev->ext_irq);
		ret = pm_runtime_put_sync(hdev->dev);
	}
	pdata->cfg_gpio(on);
#else
	if (on)
		ret = pm_runtime_get_sync(hdev->dev);
	else
		ret = pm_runtime_put_sync(hdev->dev);
#endif
	/* only values < 0 indicate errors */
	return IS_ERR_VALUE(ret) ? ret : 0;
}

static int hdmi_s_ctrl(struct v4l2_subdev *sd, struct v4l2_control *ctrl)
{
	struct hdmi_device *hdev = sd_to_hdmi_dev(sd);
	switch (ctrl->id) {
	case V4L2_CID_TV_HDCP_ENABLE:
		hdev->hdcp_enabled = ctrl->value;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}
static int hdmi_s_dv_preset(struct v4l2_subdev *sd,
	struct v4l2_dv_preset *preset)
{
	struct hdmi_device *hdev = sd_to_hdmi_dev(sd);
	struct device *dev = hdev->dev;
	const struct hdmi_preset_conf *conf;
	conf = hdmi_preset2timings(preset->preset);
	if (conf == NULL) {
		dev_err(dev, "preset (%u) not supported\n", preset->preset);
		return -EINVAL;
	}
	hdev->cur_conf = conf;
	hdev->cur_conf_dirty = 1;
	hdev->cur_preset = preset->preset;
	return 0;
}

static int hdmi_g_dv_preset(struct v4l2_subdev *sd,
	struct v4l2_dv_preset *preset)
{
	memset(preset, 0, sizeof(*preset));
	preset->preset = sd_to_hdmi_dev(sd)->cur_preset;
	return 0;
}

static int hdmi_g_mbus_fmt(struct v4l2_subdev *sd,
	  struct v4l2_mbus_framefmt *fmt)
{
	struct hdmi_device *hdev = sd_to_hdmi_dev(sd);
	int i;
	dev_dbg(hdev->dev, "%s\n", __func__);
	if (!hdev->cur_conf)
		return -EINVAL;
	memset(fmt, 0, sizeof *fmt);
	
	for (i = 0; i < ARRAY_SIZE(hdmi_timings); ++i)
	{
		if (hdmi_timings[i].preset == hdev->cur_preset)
		{
		  fmt->width=hdmi_timings[i].width;
		  fmt->height=hdmi_timings[i].height;
		  break;
		}
	}	
	fmt->code = V4L2_MBUS_FMT_FIXED; /* means RGB888 */
	fmt->colorspace = V4L2_COLORSPACE_SRGB;
	if (hdmi_timings[i].interlace) 
		fmt->field = V4L2_FIELD_INTERLACED;
	else 
		fmt->field = V4L2_FIELD_NONE;

	return 0;
}

static int hdmi_enum_dv_presets(struct v4l2_subdev *sd,
	struct v4l2_dv_enum_preset *preset)
{
	if (preset->index >= ARRAY_SIZE(hdmi_timings))
		return -EINVAL;
	return v4l_fill_dv_preset_info(hdmi_timings[preset->index].preset,
		preset);
}

static const struct v4l2_subdev_core_ops hdmi_sd_core_ops = {
	.s_power = hdmi_s_power,
	.s_ctrl  = hdmi_s_ctrl,
};

static const struct v4l2_subdev_video_ops hdmi_sd_video_ops = {
	.s_dv_preset = hdmi_s_dv_preset,
	.g_dv_preset = hdmi_g_dv_preset,
	.enum_dv_presets = hdmi_enum_dv_presets,
	.g_mbus_fmt = hdmi_g_mbus_fmt,
	.s_stream = hdmi_s_stream,
};

static const struct v4l2_subdev_ops hdmi_sd_ops = {
	.core = &hdmi_sd_core_ops,
	.video = &hdmi_sd_video_ops,
};

static int hdmi_runtime_suspend(struct device *dev)
{
	struct v4l2_subdev *sd = dev_get_drvdata(dev);
	struct hdmi_device *hdev = sd_to_hdmi_dev(sd);

	dev_dbg(dev, "%s\n", __func__);
	hdmi_resource_poweroff(&hdev->res);
	/* flag that device context is lost */
	hdev->cur_conf_dirty = 1;
	return 0;
}

static int hdmi_runtime_resume(struct device *dev)
{
	struct v4l2_subdev *sd = dev_get_drvdata(dev);
	struct hdmi_device *hdev = sd_to_hdmi_dev(sd);
	dev_dbg(dev, "%s\n", __func__);

	hdmi_resource_poweron(&hdev->res);

	dev_dbg(dev, "poweron succeed\n");

	return 0;

}

static const struct dev_pm_ops hdmi_pm_ops = {
	.runtime_suspend = hdmi_runtime_suspend,
	.runtime_resume	 = hdmi_runtime_resume,
};

static void hdmi_resources_cleanup(struct hdmi_device *hdev)
{
	struct hdmi_resources *res = &hdev->res;

	dev_dbg(hdev->dev, "HDMI resource cleanup\n");
	/* put clocks, power */
	if (res->regul_count)
		regulator_bulk_free(res->regul_count, res->regul_bulk);
	/* kfree is NULL-safe */
	kfree(res->regul_bulk);
	if (!IS_ERR_OR_NULL(res->hdmiphy))
		clk_put(res->hdmiphy);
	if (!IS_ERR_OR_NULL(res->sclk_hdmiphy))
		clk_put(res->sclk_hdmiphy);
	if (!IS_ERR_OR_NULL(res->sclk_pixel))
		clk_put(res->sclk_pixel);
	if (!IS_ERR_OR_NULL(res->sclk_hdmi))
		clk_put(res->sclk_hdmi);
	if (!IS_ERR_OR_NULL(res->hdmi))
		clk_put(res->hdmi);
	memset(res, 0, sizeof *res);
}

static int hdmi_resources_init(struct hdmi_device *hdev)
{
	struct device *dev = hdev->dev;
	struct hdmi_resources *res = &hdev->res;
	/* ToDo: Removing hdmi-en from regulators supply list as 
	*  it is not clear from which LDO it has to come from
	*/
	static char *supply[] = {
	/*	"hdmi-en", */
		"vdd",
		"vdd_osc",
		"vdd_pll",
	};
	int i, ret;

	dev_dbg(dev, "HDMI resource init\n");

	memset(res, 0, sizeof *res);
	/* get clocks, power */

	res->hdmi = clk_get(dev, "hdmi");
	if (IS_ERR_OR_NULL(res->hdmi)) {
		dev_err(dev, "failed to get clock 'hdmi'\n");
		goto fail;
	}
	res->sclk_hdmi = clk_get(dev, "sclk_hdmi");
	if (IS_ERR_OR_NULL(res->sclk_hdmi)) {
		dev_err(dev, "failed to get clock 'sclk_hdmi'\n");
		goto fail;
	}
	res->sclk_pixel = clk_get(dev, "sclk_pixel");
	if (IS_ERR_OR_NULL(res->sclk_pixel)) {
		dev_err(dev, "failed to get clock 'sclk_pixel'\n");
		goto fail;
	}
	res->sclk_hdmiphy = clk_get(dev, "sclk_hdmiphy");
	if (IS_ERR_OR_NULL(res->sclk_hdmiphy)) {
		dev_err(dev, "failed to get clock 'sclk_hdmiphy'\n");
		goto fail;
	}
	res->hdmiphy = clk_get(dev, "hdmiphy");
	if (IS_ERR_OR_NULL(res->hdmiphy)) {
		dev_err(dev, "failed to get clock 'hdmiphy'\n");
		goto fail;
	}
	res->regul_bulk = kcalloc(ARRAY_SIZE(supply),
				  sizeof(res->regul_bulk[0]), GFP_KERNEL);
	if (!res->regul_bulk) {
		dev_err(dev, "failed to get memory for regulators\n");
		goto fail;
	}
	for (i = 0; i < ARRAY_SIZE(supply); ++i) {
		res->regul_bulk[i].supply = supply[i];
		res->regul_bulk[i].consumer = NULL;
	}

	ret = regulator_bulk_get(dev, ARRAY_SIZE(supply), res->regul_bulk);
	if (ret) {
		dev_err(dev, "failed to get regulators\n");
		goto fail;
	}
	res->regul_count = ARRAY_SIZE(supply);

	return 0;
fail:
	dev_err(dev, "HDMI resource init - failed\n");
	hdmi_resources_cleanup(hdev);
	return -ENODEV;
}

static int __devinit hdmi_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct resource *res;
	struct i2c_adapter *adapter;
	struct v4l2_subdev *sd;
	struct hdmi_device *hdmi_dev = NULL;
	struct s5p_hdmi_platform_data *pdata = dev->platform_data;
	int ret;

	dev_dbg(dev, "probe start\n");

	if (!pdata) {
		dev_err(dev, "platform data is missing\n");
		ret = -ENODEV;
		goto fail;
	}

	hdmi_dev = devm_kzalloc(&pdev->dev, sizeof(*hdmi_dev), GFP_KERNEL);
	if (!hdmi_dev) {
		dev_err(dev, "out of memory\n");
		ret = -ENOMEM;
		goto fail;
	}

	hdmi_dev->dev = dev;
	hdmi_dev->connected = false;
	INIT_WORK(&hdmi_dev->work, hdmi_hpd_work);

	ret = hdmi_resources_init(hdmi_dev);
	if (ret)
		goto fail;

	/* mapping HDMI registers */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(dev, "get memory resource failed.\n");
		ret = -ENXIO;
		goto fail_init;
	}

	hdmi_dev->regs = devm_ioremap(&pdev->dev, res->start,
				      resource_size(res));
	if (hdmi_dev->regs == NULL) {
		dev_err(dev, "register mapping failed.\n");
		ret = -ENXIO;
		goto fail_init;
	}

	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		dev_err(dev, "get interrupt resource failed.\n");
		ret = -ENXIO;
		goto fail_init;
	}

	ret = devm_request_irq(&pdev->dev, res->start, hdmi_irq_handler, 0,
			       "hdmi", hdmi_dev);
	if (ret) {
		dev_err(dev, "request interrupt failed.\n");
		goto fail_init;
	}
	hdmi_dev->irq = res->start;
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_HDCP
	disable_irq(hdmi_dev->irq);
#endif
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 1);
	if (res == NULL) {
		dev_err(dev, "get interrupt (HDMI_HPD) resource failed.\n");
		ret = -ENXIO;
		goto fail_init;
	}
	hdmi_dev->sw_connected.name = "hdmi";
        ret = switch_dev_register(&hdmi_dev->sw_connected);
        if (ret < 0)
                goto fail_init;

	hdmi_dev->ext_irq = res->start;

	ret = devm_request_irq(&pdev->dev, res->start, hdmi_ext_irq_handler, 
		IRQF_TRIGGER_HIGH, "hdmi_hpd", hdmi_dev);
	if (ret) {
		dev_err(dev, "hdmi_hpd request interrupt failed.\n");
		goto fail_init;
	}

	/* setting v4l2 name to prevent WARN_ON in v4l2_device_register */
	strlcpy(hdmi_dev->v4l2_dev.name, dev_name(dev),
		sizeof(hdmi_dev->v4l2_dev.name));
	/* passing NULL owner prevents driver from erasing drvdata */
	ret = v4l2_device_register(NULL, &hdmi_dev->v4l2_dev);
	if (ret) {
		dev_err(dev, "could not register v4l2 device.\n");
		goto fail_init;
	}

	/* testing if hdmiphy info is present */
	if (!pdata->hdmiphy_info) {
		dev_err(dev, "hdmiphy info is missing in platform data\n");
		ret = -ENXIO;
		goto fail_vdev;
	}

	adapter = i2c_get_adapter(pdata->hdmiphy_bus);
	if (adapter == NULL) {
		dev_err(dev, "hdmiphy adapter request failed\n");
		ret = -ENXIO;
		goto fail_vdev;
	}

	hdmi_dev->phy_sd = v4l2_i2c_new_subdev_board(&hdmi_dev->v4l2_dev,
		adapter, pdata->hdmiphy_info, NULL);
	/* on failure or not adapter is no longer useful */
	i2c_put_adapter(adapter);
	if (hdmi_dev->phy_sd == NULL) {
		dev_err(dev, "missing subdev for hdmiphy\n");
		ret = -ENODEV;
		goto fail_vdev;
	}

	clk_enable(hdmi_dev->res.hdmi);
#ifdef CONFIG_VIDEO_SAMSUNG_S5P_HDMI_HDCP
	s5p_hdcp_init();
#endif
#ifdef CONFIG_BUSFREQ_OPP
        /* To lock bus frequency in OPP mode */
        hdmi_dev->bus_dev = dev_get("exynos-busfreq");
#endif

	pm_runtime_enable(dev);

	sd = &hdmi_dev->sd;
	v4l2_subdev_init(sd, &hdmi_sd_ops);
	sd->owner = THIS_MODULE;

	strlcpy(sd->name, "s5p-hdmi", sizeof sd->name);
	hdmi_dev->cur_preset = HDMI_DEFAULT_PRESET;
	/* FIXME: missing fail preset is not supported */
	hdmi_dev->cur_conf = hdmi_preset2timings(hdmi_dev->cur_preset);
	hdmi_dev->cur_conf_dirty = 1;

	/* storing subdev for call that have only access to struct device */
	dev_set_drvdata(dev, sd);
	hdmi_dev->hdmiphy_port = v4l2_get_subdevdata(hdmi_dev->phy_sd);
	clk_disable(hdmi_dev->res.hdmi);

	dev_info(dev, "probe successful\n");

	return 0;

fail_vdev:
	v4l2_device_unregister(&hdmi_dev->v4l2_dev);

fail_init:
	hdmi_resources_cleanup(hdmi_dev);

fail:
	dev_err(dev, "probe failed\n");
	return ret;
}

static int __devexit hdmi_remove(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct v4l2_subdev *sd = dev_get_drvdata(dev);
	struct hdmi_device *hdmi_dev = sd_to_hdmi_dev(sd);

	pm_runtime_disable(dev);
	clk_disable(hdmi_dev->res.hdmi);
	v4l2_device_unregister(&hdmi_dev->v4l2_dev);
	disable_irq(hdmi_dev->irq);
	hdmi_resources_cleanup(hdmi_dev);
	switch_dev_unregister(&hdmi_dev->sw_connected);
	dev_info(dev, "remove successful\n");

	return 0;
}

static struct platform_driver hdmi_driver __refdata = {
	.probe = hdmi_probe,
	.remove = __devexit_p(hdmi_remove),
	.id_table = hdmi_driver_types,
	.driver = {
		.name = "s5p-hdmi",
		.owner = THIS_MODULE,
		.pm = &hdmi_pm_ops,
	}
};

module_platform_driver(hdmi_driver);
