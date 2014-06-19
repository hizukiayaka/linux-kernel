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

#include <linux/io.h>

#include "g2d.h"
#include "g2d-regs.h"

#define w(x, a)	writel((x), d->regs + (a))
#define r(a)	readl(d->regs + (a))

/**
 * @COEFF_ONE: 1
 * @COEFF_ZERO: 0
 * @COEFF_SA: src alpha
 * @COEFF_SC: src color
 * @COEFF_DA: dst alpha
 * @COEFF_DC: dst color
 * @COEFF_GA: global(constant) alpha
 * @COEFF_GC: global(constant) color
 * @COEFF_DISJ_S:
 * @COEFF_DISJ_D:
 * @COEFF_CONJ_S:
 * @COEFF_CONJ_D:
 *
 * DO NOT CHANGE THIS ORDER
 */
enum fimg2d_coeff {
	COEFF_ONE = 0,
	COEFF_ZERO,
	COEFF_SA,
	COEFF_SC,
	COEFF_DA,
	COEFF_DC,
	COEFF_GA,
	COEFF_GC,
	COEFF_DISJ_S,
	COEFF_DISJ_D,
	COEFF_CONJ_S,
	COEFF_CONJ_D,
};

struct fimg2d_blend_coeff {
	bool s_coeff_inv;
	enum fimg2d_coeff s_coeff;
	bool d_coeff_inv;
	enum fimg2d_coeff d_coeff;
};

/**
 * Four channels of the image are computed with:
 *	R = [ coeff(S)*Sc  + coeff(D)*Dc ]
 *	where
 *	Rc is result color or alpha
 *	Sc is source color or alpha
 *	Dc is destination color or alpha
 *
 * Caution: supposed that Sc and Dc are perpixel-alpha-premultiplied value
 *
 * MODE:             Formula
 * ----------------------------------------------------------------------------
 * FILL:
 * CLEAR:	     R = 0
 * SRC:		     R = Sc
 * DST:		     R = Dc
 * SRC_OVER:         R = Sc + (1-Sa)*Dc
 * DST_OVER:         R = (1-Da)*Sc + Dc
 * SRC_IN:	     R = Da*Sc
 * DST_IN:           R = Sa*Dc
 * SRC_OUT:          R = (1-Da)*Sc
 * DST_OUT:          R = (1-Sa)*Dc
 * SRC_ATOP:         R = Da*Sc + (1-Sa)*Dc
 * DST_ATOP:         R = (1-Da)*Sc + Sa*Dc
 * XOR:              R = (1-Da)*Sc + (1-Sa)*Dc
 * ADD:              R = Sc + Dc
 * MULTIPLY:         R = Sc*Dc
 * SCREEN:           R = Sc + (1-Sc)*Dc
 * DARKEN:           R = (Da*Sc<Sa*Dc)? Sc+(1-Sa)*Dc : (1-Da)*Sc+Dc
 * LIGHTEN:          R = (Da*Sc>Sa*Dc)? Sc+(1-Sa)*Dc : (1-Da)*Sc+Dc
 * DISJ_SRC_OVER:    R = Sc + (min(1,(1-Sa)/Da))*Dc
 * DISJ_DST_OVER:    R = (min(1,(1-Da)/Sa))*Sc + Dc
 * DISJ_SRC_IN:      R = (max(1-(1-Da)/Sa,0))*Sc
 * DISJ_DST_IN:      R = (max(1-(1-Sa)/Da,0))*Dc
 * DISJ_SRC_OUT:     R = (min(1,(1-Da)/Sa))*Sc
 * DISJ_DST_OUT:     R = (min(1,(1-Sa)/Da))*Dc
 * DISJ_SRC_ATOP:    R = (max(1-(1-Da)/Sa,0))*Sc + (min(1,(1-Sa)/Da))*Dc
 * DISJ_DST_ATOP:    R = (min(1,(1-Da)/Sa))*Sc + (max(1-(1-Sa)/Da,0))*Dc
 * DISJ_XOR:         R = (min(1,(1-Da)/Sa))*Sc + (min(1,(1-Sa)/Da))*Dc
 * CONJ_SRC_OVER:    R = Sc + (max(1-Sa/Da,0))*Dc
 * CONJ_DST_OVER:    R = (max(1-Da/Sa,0))*Sc + Dc
 * CONJ_SRC_IN:      R = (min(1,Da/Sa))*Sc
 * CONJ_DST_IN:      R = (min(1,Sa/Da))*Dc
 * CONJ_SRC_OUT:     R = (max(1-Da/Sa,0)*Sc
 * CONJ_DST_OUT:     R = (max(1-Sa/Da,0))*Dc
 * CONJ_SRC_ATOP:    R = (min(1,Da/Sa))*Sc + (max(1-Sa/Da,0))*Dc
 * CONJ_DST_ATOP:    R = (max(1-Da/Sa,0))*Sc + (min(1,Sa/Da))*Dc
 * CONJ_XOR:         R = (max(1-Da/Sa,0))*Sc + (max(1-Sa/Da,0))*Dc
 */
static struct fimg2d_blend_coeff const coeff_table[MAX_FIMG2D_BLIT_OP] = {
	{ 0, 0, 0, 0 },		/* FILL */
	{ 0, COEFF_ZERO,	0, COEFF_ZERO },	/* CLEAR */
	{ 0, COEFF_ONE,		0, COEFF_ZERO },	/* SRC */
	{ 0, COEFF_ZERO,	0, COEFF_ONE },		/* DST */
	{ 0, COEFF_ONE,		1, COEFF_SA },		/* SRC_OVER */
	{ 1, COEFF_DA,		0, COEFF_ONE },		/* DST_OVER */
	{ 0, COEFF_DA,		0, COEFF_ZERO },	/* SRC_IN */
	{ 0, COEFF_ZERO,	0, COEFF_SA },		/* DST_IN */
	{ 1, COEFF_DA,		0, COEFF_ZERO },	/* SRC_OUT */
	{ 0, COEFF_ZERO,	1, COEFF_SA },		/* DST_OUT */
	{ 0, COEFF_DA,		1, COEFF_SA },		/* SRC_ATOP */
	{ 1, COEFF_DA,		0, COEFF_SA },		/* DST_ATOP */
	{ 1, COEFF_DA,		1, COEFF_SA },		/* XOR */
	{ 0, COEFF_ONE,		0, COEFF_ONE },		/* ADD */
	{ 0, COEFF_DC,		0, COEFF_ZERO },	/* MULTIPLY */
	{ 0, COEFF_ONE,		1, COEFF_SC },		/* SCREEN */
	{ 0, 0, 0, 0 },		/* DARKEN */
	{ 0, 0, 0, 0 },		/* LIGHTEN */
	{ 0, COEFF_ONE,		0, COEFF_DISJ_S },	/* DISJ_SRC_OVER */
	{ 0, COEFF_DISJ_D,	0, COEFF_ONE },		/* DISJ_DST_OVER */
	{ 1, COEFF_DISJ_D,	0, COEFF_ZERO },	/* DISJ_SRC_IN */
	{ 0, COEFF_ZERO,	1, COEFF_DISJ_S },	/* DISJ_DST_IN */
	{ 0, COEFF_DISJ_D,	0, COEFF_ONE },		/* DISJ_SRC_OUT */
	{ 0, COEFF_ZERO,	0, COEFF_DISJ_S },	/* DISJ_DST_OUT */
	{ 1, COEFF_DISJ_D,	0, COEFF_DISJ_S },	/* DISJ_SRC_ATOP */
	{ 0, COEFF_DISJ_D,	1, COEFF_DISJ_S },	/* DISJ_DST_ATOP */
	{ 0, COEFF_DISJ_D,	0, COEFF_DISJ_S },	/* DISJ_XOR */
	{ 0, COEFF_ONE,		1, COEFF_DISJ_S },	/* CONJ_SRC_OVER */
	{ 1, COEFF_DISJ_D,	0, COEFF_ONE },		/* CONJ_DST_OVER */
	{ 0, COEFF_CONJ_D,	0, COEFF_ONE },		/* CONJ_SRC_IN */
	{ 0, COEFF_ZERO,	0, COEFF_CONJ_S },	/* CONJ_DST_IN */
	{ 1, COEFF_CONJ_D,	0, COEFF_ZERO },	/* CONJ_SRC_OUT */
	{ 0, COEFF_ZERO,	1, COEFF_CONJ_S },	/* CONJ_DST_OUT */
	{ 0, COEFF_CONJ_D,	1, COEFF_CONJ_S },	/* CONJ_SRC_ATOP */
	{ 1, COEFF_CONJ_D,	0, COEFF_CONJ_D },	/* CONJ_DST_ATOP */
	{ 1, COEFF_CONJ_D,	1, COEFF_CONJ_S },	/* CONJ_XOR */
	{ 0, 0, 0, 0 },		/* USER */
	{ 1, COEFF_GA,		1, COEFF_ZERO },	/* USER_SRC_GA */
};

/*
 * coefficient table with global (constant) alpha
 * replace COEFF_ONE with COEFF_GA
 *
 * MODE:             Formula with Global Alpha (Ga is multiplied to both Sc and Sa)
 * ----------------------------------------------------------------------------
 * FILL:
 * CLEAR:	     R = 0
 * SRC:		     R = Ga*Sc
 * DST:		     R = Dc
 * SRC_OVER:         R = Ga*Sc + (1-Sa*Ga)*Dc
 * DST_OVER:         R = (1-Da)*Ga*Sc + Dc --> (W/A) 1st:Ga*Sc, 2nd:DST_OVER
 * SRC_IN:	     R = Da*Ga*Sc
 * DST_IN:           R = Sa*Ga*Dc
 * SRC_OUT:          R = (1-Da)*Ga*Sc --> (W/A) 1st: Ga*Sc, 2nd:SRC_OUT
 * DST_OUT:          R = (1-Sa*Ga)*Dc
 * SRC_ATOP:         R = Da*Ga*Sc + (1-Sa*Ga)*Dc
 * DST_ATOP:         R = (1-Da)*Ga*Sc + Sa*Ga*Dc --> (W/A) 1st: Ga*Sc, 2nd:DST_ATOP
 * XOR:              R = (1-Da)*Ga*Sc + (1-Sa*Ga)*Dc --> (W/A) 1st: Ga*Sc, 2nd:XOR
 * ADD:              R = Ga*Sc + Dc
 * MULTIPLY:         R = Ga*Sc*Dc --> (W/A) 1st: Ga*Sc, 2nd: MULTIPLY
 * SCREEN:           R = Ga*Sc + (1-Ga*Sc)*Dc --> (W/A) 1st: Ga*Sc, 2nd: SCREEN
 * DARKEN:           R = (W/A) 1st: Ga*Sc, 2nd: OP
 * LIGHTEN:          R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_SRC_OVER:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_DST_OVER:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_SRC_IN:      R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_DST_IN:      R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_SRC_OUT:     R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_DST_OUT:     R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_SRC_ATOP:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_DST_ATOP:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * DISJ_XOR:         R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_SRC_OVER:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_DST_OVER:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_SRC_IN:      R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_DST_IN:      R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_SRC_OUT:     R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_DST_OUT:     R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_SRC_ATOP:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_DST_ATOP:    R = (W/A) 1st: Ga*Sc, 2nd: OP
 * CONJ_XOR:         R = (W/A) 1st: Ga*Sc, 2nd: OP
 */
static struct fimg2d_blend_coeff const ga_coeff_table[MAX_FIMG2D_BLIT_OP] = {
	{ 0, 0, 0, 0 },		/* FILL */
	{ 0, COEFF_ZERO,	0, COEFF_ZERO },	/* CLEAR */
	{ 0, COEFF_GA,		0, COEFF_ZERO },	/* SRC */
	{ 0, COEFF_ZERO,	0, COEFF_ONE },		/* DST */
	{ 0, COEFF_GA,		1, COEFF_SA },		/* SRC_OVER */
	{ 1, COEFF_DA,		0, COEFF_ONE },		/* DST_OVER (use W/A) */
	{ 0, COEFF_DA,		0, COEFF_ZERO },	/* SRC_IN */
	{ 0, COEFF_ZERO,	0, COEFF_SA },		/* DST_IN */
	{ 1, COEFF_DA,		0, COEFF_ZERO },	/* SRC_OUT (use W/A) */
	{ 0, COEFF_ZERO,	1, COEFF_SA },		/* DST_OUT */
	{ 0, COEFF_DA,		1, COEFF_SA },		/* SRC_ATOP */
	{ 1, COEFF_DA,		0, COEFF_SA },		/* DST_ATOP (use W/A) */
	{ 1, COEFF_DA,		1, COEFF_SA },		/* XOR (use W/A) */
	{ 0, COEFF_GA,		0, COEFF_ONE },		/* ADD */
	{ 0, COEFF_DC,		0, COEFF_ZERO },	/* MULTIPLY (use W/A) */
	{ 0, COEFF_ONE,		1, COEFF_SC },		/* SCREEN (use W/A) */
	{ 0, 0, 0, 0 },		/* DARKEN (use W/A) */
	{ 0, 0, 0, 0 },		/* LIGHTEN (use W/A) */
	{ 0, COEFF_ONE,		0, COEFF_DISJ_S },	/* DISJ_SRC_OVER (use W/A) */
	{ 0, COEFF_DISJ_D,	0, COEFF_ONE },		/* DISJ_DST_OVER (use W/A) */
	{ 1, COEFF_DISJ_D,	0, COEFF_ZERO },	/* DISJ_SRC_IN (use W/A) */
	{ 0, COEFF_ZERO,	1, COEFF_DISJ_S },	/* DISJ_DST_IN (use W/A) */
	{ 0, COEFF_DISJ_D,	0, COEFF_ONE },		/* DISJ_SRC_OUT (use W/A) */
	{ 0, COEFF_ZERO,	0, COEFF_DISJ_S },	/* DISJ_DST_OUT (use W/A) */
	{ 1, COEFF_DISJ_D,	0, COEFF_DISJ_S },	/* DISJ_SRC_ATOP (use W/A) */
	{ 0, COEFF_DISJ_D,	1, COEFF_DISJ_S },	/* DISJ_DST_ATOP (use W/A) */
	{ 0, COEFF_DISJ_D,	0, COEFF_DISJ_S },	/* DISJ_XOR (use W/A) */
	{ 0, COEFF_ONE,		1, COEFF_DISJ_S },	/* CONJ_SRC_OVER (use W/A) */
	{ 1, COEFF_DISJ_D,	0, COEFF_ONE },		/* CONJ_DST_OVER (use W/A) */
	{ 0, COEFF_CONJ_D,	0, COEFF_ONE },		/* CONJ_SRC_IN (use W/A) */
	{ 0, COEFF_ZERO,	0, COEFF_CONJ_S },	/* CONJ_DST_IN (use W/A) */
	{ 1, COEFF_CONJ_D,	0, COEFF_ZERO },	/* CONJ_SRC_OUT (use W/A) */
	{ 0, COEFF_ZERO,	1, COEFF_CONJ_S },	/* CONJ_DST_OUT (use W/A) */
	{ 0, COEFF_CONJ_D,	1, COEFF_CONJ_S },	/* CONJ_SRC_ATOP (use W/A) */
	{ 1, COEFF_CONJ_D,	0, COEFF_CONJ_D },	/* CONJ_DST_ATOP (use W/A) */
	{ 1, COEFF_CONJ_D,	1, COEFF_CONJ_S },	/* CONJ_XOR (use W/A) */
	{ 0, 0, 0, 0 },		/* USER */
	{ 1, COEFF_GA,		1, COEFF_ZERO },	/* USER_SRC_GA */
};



/* g2d_reset clears all g2d registers */
void g2d_reset(struct g2d_dev *d)
{
	w(1, SOFT_RESET_REG);
}

void g2d_set_src_size(struct g2d_dev *d, struct g2d_frame *f)
{
	u32 n;

	w(f->stride & 0xFFFF, SRC_STRIDE_REG);

	n = f->o_height & 0xFFF;
	n <<= 16;
	n |= f->o_width & 0xFFF;
	w(n, SRC_LEFT_TOP_REG);

	n = f->bottom & 0xFFF;
	n <<= 16;
	n |= f->right & 0xFFF;
	w(n, SRC_RIGHT_BOTTOM_REG);

	w(f->fmt->hw, SRC_COLOR_MODE_REG);
	w(f->type, SRC_SELECT_REG);
}

void g2d_set_src_addr(struct g2d_dev *d, dma_addr_t a)
{
	w(a, SRC_BASE_ADDR_REG);
}

void g2d_set_dst_size(struct g2d_dev *d, struct g2d_frame *f)
{
	u32 n;

	w(f->stride & 0xFFFF, DST_STRIDE_REG);

	n = f->o_height & 0xFFF;
	n <<= 16;
	n |= f->o_width & 0xFFF;
	w(n, DST_LEFT_TOP_REG);

	n = f->bottom & 0xFFF;
	n <<= 16;
	n |= f->right & 0xFFF;
	w(n, DST_RIGHT_BOTTOM_REG);

	w(f->fmt->hw, DST_COLOR_MODE_REG);
	w(f->type, DST_SELECT_REG);
}

void g2d_set_dst_addr(struct g2d_dev *d, dma_addr_t a)
{
	w(a, DST_BASE_ADDR_REG);
}

static unsigned long scale_factor_to_fixed16(int n, int d)
{
	int i;
	u32 fixed16;

	fixed16 = (n/d) << 16;
	n %= d;

	for (i = 0; i < 16; i++) {
		if (!n)
			break;
		n <<= 1;
		if (n/d)
			fixed16 |= 1 << (15-i);
		n %= d;
	}

	return fixed16;
}


void g2d_set_src_scaling(struct g2d_dev *d, struct g2d_frame *in,
						struct g2d_frame *out)
{
	unsigned long wcfg, hcfg;

	w(d->curr->smode, SRC_SCALE_CTRL_REG);

	w(0x780 & 0xFFFF, DST_STRIDE_REG);

	/* inversed scaling factor: src is numerator */
	wcfg = scale_factor_to_fixed16(in->width, out->width);
	hcfg = scale_factor_to_fixed16(in->height, out->height);

	w(wcfg, SRC_XSCALE_REG);
	w(hcfg, SRC_YSCALE_REG);
}

void g2d_set_mask_size(struct g2d_dev *d, struct g2d_frame *f)
{
	u32 n;

	w(f->stride & 0xFFFF, MASK_STRIDE_REG);

	n = f->o_height & 0xFFF;
	n <<= 16;
	n |= f->o_width & 0xFFF;
	w(n, MASK_LEFT_TOP_REG);

	n = f->bottom & 0xFFF;
	n <<= 16;
	n |= f->right & 0xFFF;
	w(n, MASK_RIGHT_BOTTOM_REG);

	n = f->fmt->hw & 0x30;
	w(n , MASK_MODE_REG);
}

void g2d_set_mask_addr(struct g2d_dev *d, dma_addr_t a)
{
	w(a, MASK_BASE_ADDR_REG);
}

void g2d_set_clip_size(struct g2d_dev *d, struct g2d_frame *f)
{
	u32 n;

	n = f->o_height & 0xFFF;
	n <<= 16;
	n |= f->o_width & 0xFFF;
	w(n, CW_LT_REG);

	n = f->bottom & 0xFFF;
	n <<= 16;
	n |= f->right & 0xFFF;
	w(n, CW_RB_REG);
}

void g2d_set_bluescreen(struct g2d_dev *d, struct bs_info *b)
{
	/* In bluescreen mode BS_COLOR pixels are replaced by
	 * BG_COLOR.
	 */
	if (b->bs_mode == 2)
		w(b->bg_color, BG_COLOR_REG);

	/* In transparant mode BS_COLOR pixels are ignored by g2d
	 * and that region is mode transparant.
	 */
	w(b->bs_color, BS_COLOR_REG);
}

void g2d_set_rop4(struct g2d_dev *d, u32 r)
{
	w(r, ROP4_REG);
}

void g2d_set_flip(struct g2d_dev *d, u32 r)
{
	w(r, SRC_MSK_DIRECT_REG);
}

void g2d_set_rotation(struct g2d_dev *d, u32 r)
{
	w(r, ROTATE_REG);
}

void g2d_set_color_fill(struct g2d_dev *d, u32 r)
{
	w(r, SF_COLOR_REG);
}

void g2d_set_alpha(struct g2d_dev *d, u32 r)
{
	w(r, ALPHA_REG);
}

void g2d_set_mask(struct g2d_dev *d, u32 r)
{
	int n;

	n = r(MASK_MODE_REG);
	w(n | r, MASK_MODE_REG);
}

void g2d_set_alpha_composite(struct g2d_dev *d,
		enum blit_op op, unsigned char g_alpha)
{
	unsigned long cfg = 0;
	struct fimg2d_blend_coeff const *tbl;

	switch (op) {
	case BLIT_OP_SOLID_FILL:
	case BLIT_OP_CLR:
		/* nop */
		return;
	case BLIT_OP_DARKEN:
		cfg |= FIMG2D_DARKEN;
		break;
	case BLIT_OP_LIGHTEN:
		cfg |= FIMG2D_LIGHTEN;
		break;
	case BLIT_OP_USER_COEFF:
		/* TODO */
		return;
	default:
		if (g_alpha < 0xff)	/* with global alpha */
			tbl = &ga_coeff_table[op];
		else
			tbl = &coeff_table[op];

		/* src coefficient */
		cfg |= tbl->s_coeff << FIMG2D_SRC_COEFF_SHIFT;

		cfg |= 0x2 << FIMG2D_SRC_COEFF_SA_SHIFT;
		cfg |= 0x2 << FIMG2D_SRC_COEFF_DA_SHIFT;

		if (tbl->s_coeff_inv)
			cfg |= FIMG2D_INV_SRC_COEFF;

		/* dst coefficient */
		cfg |= tbl->d_coeff << FIMG2D_DST_COEFF_SHIFT;

		cfg |= 0x2 << FIMG2D_DST_COEFF_DA_SHIFT;
		cfg |= 0x2 << FIMG2D_DST_COEFF_SA_SHIFT;

		if (tbl->d_coeff_inv)
			cfg |= FIMG2D_INV_DST_COEFF;


		break;
	}

	w(cfg, BLEND_FUNCTION_REG);

	/* round mode: depremult round mode is not used */
	cfg = r(ROUND_MODE_REG);

	/* premult */
	cfg &= ~FIMG2D_PREMULT_ROUND_MASK;
	cfg |= 0x1 << FIMG2D_PREMULT_ROUND_SHIFT;

	/* blend */
	cfg &= ~FIMG2D_BLEND_ROUND_MASK;
	cfg |= 0x0 << FIMG2D_BLEND_ROUND_SHIFT;

	w(cfg, ROUND_MODE_REG);
	/* FIXME: Should come from application */
	w(0 , DST_SELECT_REG);
	w(1 , DST_COLOR_MODE_REG);
	w(1 , SRC_COLOR_MODE_REG);
}



u32 g2d_cmd_stretch(u32 e)
{
	e &= 1;
	return e << 4;
}

void g2d_set_cmd(struct g2d_dev *d, u32 c)
{
	w(c, BITBLT_COMMAND_REG);
}

void g2d_start(struct g2d_dev *d)
{
	/* Clear cache */
	w(0x7, CACHECTL_REG);
	/* Enable interrupt */
	w(1, INTEN_REG);
	/* Start G2D engine */
	w(1, BITBLT_START_REG);
}

void g2d_clear_int(struct g2d_dev *d)
{
	w(1, INTC_PEND_REG);
}
