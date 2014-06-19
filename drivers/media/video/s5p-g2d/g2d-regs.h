/*
 * Samsung S5P G2D - 2D Graphics Accelerator Driver
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Kamil Debski, <k.debski@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version
 */

/* General Registers */
#define SOFT_RESET_REG		0x0000	/* Software reset reg */
#define INTEN_REG		0x0004	/* Interrupt Enable reg */
#define INTC_PEND_REG		0x000C	/* Interrupt Control Pending reg */
#define FIFO_STAT_REG		0x0010	/* Command FIFO Status reg */
#define AXI_ID_MODE_REG		0x0014	/* AXI Read ID Mode reg */
#define CACHECTL_REG		0x0018	/* Cache & Buffer clear reg */
#define AXI_MODE_REG		0x001C	/* AXI Mode reg */

/* Command Registers */
#define BITBLT_START_REG	0x0100	/* BitBLT Start reg */
#define BITBLT_COMMAND_REG	0x0104	/* Command reg for BitBLT */
#define BLEND_FUNCTION_REG	0x0108  /* Blend function reg */
#define ROUND_MODE_REG		0x010C  /* Round mode reg */

/* Parameter Setting Registers (Rotate & Direction) */
#define ROTATE_REG		0x0200	/* Rotation reg */
#define SRC_MSK_DIRECT_REG	0x0204	/* Src and Mask Direction reg */
#define DST_PAT_DIRECT_REG	0x0208	/* Dest and Pattern Direction reg */

/* Parameter Setting Registers (Src) */
#define SRC_SELECT_REG		0x0300	/* Src Image Selection reg */
#define SRC_BASE_ADDR_REG	0x0304	/* Src Image Base Address reg */
#define SRC_STRIDE_REG		0x0308	/* Src Stride reg */
#define SRC_COLOR_MODE_REG	0x030C	/* Src Image Color Mode reg */
#define SRC_LEFT_TOP_REG	0x0310	/* Src Left Top Coordinate reg */
#define SRC_RIGHT_BOTTOM_REG	0x0314	/* Src Right Bottom Coordinate reg */
#define SRC_SCALE_CTRL_REG	0x0328  /* Src scaling mode select reg */
#define SRC_XSCALE_REG		0x032C  /* Src X scale ratio reg */
#define SRC_YSCALE_REG		0x0330  /* Src Y scale ratio reg */

/* Parameter Setting Registers (Dest) */
#define DST_SELECT_REG		0x0400	/* Dest Image Selection reg */
#define DST_BASE_ADDR_REG	0x0404	/* Dest Image Base Address reg */
#define DST_STRIDE_REG		0x0408	/* Dest Stride reg */
#define DST_COLOR_MODE_REG	0x040C	/* Dest Image Color Mode reg */
#define DST_LEFT_TOP_REG	0x0410	/* Dest Left Top Coordinate reg */
#define DST_RIGHT_BOTTOM_REG	0x0414	/* Dest Right Bottom Coordinate reg */

/* Parameter Setting Registers (Pattern) */
#define PAT_BASE_ADDR_REG	0x0500	/* Pattern Image Base Address reg */
#define PAT_SIZE_REG		0x0504	/* Pattern Image Size reg */
#define PAT_COLOR_MODE_REG	0x0508	/* Pattern Image Color Mode reg */
#define PAT_OFFSET_REG		0x050C	/* Pattern Left Top Coordinate reg */
#define PAT_STRIDE_REG		0x0510	/* Pattern Stride reg */

/* Parameter Setting Registers (Mask) */
#define MASK_BASE_ADDR_REG	0x0520	/* Mask Base Address reg */
#define MASK_STRIDE_REG		0x0524	/* Mask Stride reg */
#define MASK_LEFT_TOP_REG	0x0528	/* Mask Left Top Coordinate reg */
#define MASK_RIGHT_BOTTOM_REG	0x052C	/* Mask Right Bottom Coordinate reg */
#define MASK_MODE_REG		0x0530  /* Mask color and bit mode reg */

/* Parameter Setting Registers (Clipping Window) */
#define CW_LT_REG		0x0600	/* LeftTop coordinates of Clip Window */
#define CW_RB_REG		0x0604	/* RightBottom coordinates of Clip
								Window */

/* Parameter Setting Registers (ROP & Alpha Setting) */
#define THIRD_OPERAND_REG	0x0610	/* Third Operand Selection reg */
#define ROP4_REG		0x0614	/* Raster Operation reg */
#define ALPHA_REG		0x0618	/* Alpha value, Fading offset value */

/* Parameter Setting Registers (Color) */
#define FG_COLOR_REG		0x0700	/* Foreground Color reg */
#define BG_COLOR_REG		0x0704	/* Background Color reg */
#define BS_COLOR_REG		0x0708	/* Blue Screen Color reg */
#define SF_COLOR_REG		0x070C /* solid color fill */

/* Parameter Setting Registers (Color Key) */
#define SRC_COLORKEY_CTRL_REG	0x0710	/* Src Colorkey control reg */
#define SRC_COLORKEY_DR_MIN_REG	0x0714	/* Src Colorkey Decision Reference
								Min reg */
#define SRC_COLORKEY_DR_MAX_REG	0x0718	/* Src Colorkey Decision Reference
								Max reg */
#define DST_COLORKEY_CTRL_REG	0x071C	/* Dest Colorkey control reg */
#define DST_COLORKEY_DR_MIN_REG	0x0720	/* Dest Colorkey Decision Reference
								Min reg */
#define DST_COLORKEY_DR_MAX_REG	0x0724	/* Dest Colorkey Decision Reference
								Max reg */

/* Rotation and Flip */

#define ROTATE_SRC_90		(1 << 0)
#define ROTATE_MASK_90		(1 << 8)

#define SRC_X_DIR_NEGATIVE	(1 << 0)
#define SRC_Y_DIR_NEGATIVE	(1 << 1)
#define SRC_XY_DIR_NEGATIVE	(SRC_X_DIR_NEGATIVE | SRC_Y_DIR_NEGATIVE)

/* commands */
#define SOLID_FILL		(1 << 28)
#define ALPHA_BLEND		(1 << 20)
#define OPAQUE			(0 << 12)
#define TRANSPARENT		(1 << 12)
#define BLUESCREEN		(2 << 12)
#define CLIP			(1 << 8)
#define DITHER			(1 << 3)
#define NORMAL_MASK		(1 << 1)
#define ROP4_MSK		(1 << 0)

/* FIMG2D_BLEND_FUNCTION_REG */
#define FIMG2D_WINCE_SRC_OVER                   (1 << 22)
#define FIMG2D_DARKEN                           (1 << 21)
#define FIMG2D_LIGHTEN                          (1 << 20)
#define FIMG2D_INV_DST_COEFF                    (1 << 18)
#define FIMG2D_INV_SRC_COEFF                    (1 << 16)

#define FIMG2D_DST_COEFF_DA_SHIFT               (14)
#define FIMG2D_DST_COEFF_SA_SHIFT               (12)
#define FIMG2D_SRC_COEFF_DA_SHIFT               (6)
#define FIMG2D_SRC_COEFF_SA_SHIFT               (4)

#define FIMG2D_DST_COEFF_SHIFT                  (8)
#define FIMG2D_SRC_COEFF_SHIFT                  (0)

/* FIMG2D_ROUND_MODE_REG */
#define FIMG2D_PREMULT_ROUND_MASK               (3 << 4)
#define FIMG2D_PREMULT_ROUND_SHIFT              (4)

#define FIMG2D_BLEND_ROUND_MASK                 (3 << 0)
#define FIMG2D_BLEND_ROUND_SHIFT                (0)

/* Color mode values */

#define ORDER_XRGB		0
#define ORDER_RGBX		1
#define ORDER_XBGR		2
#define ORDER_BGRX		3

#define MODE_XRGB_8888		0
#define MODE_ARGB_8888		1
#define MODE_RGB_565		2
#define MODE_XRGB_1555		3
#define MODE_ARGB_1555		4
#define MODE_XRGB_4444		5
#define MODE_ARGB_4444		6
#define MODE_PACKED_RGB_888	7

#define COLOR_MODE(o, m)	(((o) << 4) | (m))

/* ROP4 operation values */
#define ROP4_COPY		0xCCCC
#define ROP4_INVERT		0x3333

/* Alpha blending values */
#define DEFAULT_ALPHA		0xff

/* Hardware limits */
#define MAX_WIDTH		8000
#define MAX_HEIGHT		8000

#define G2D_TIMEOUT		500

#define DEFAULT_WIDTH		100
#define DEFAULT_HEIGHT		100

