// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Randy Li, <ayaka@soulik.info>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef _VDPU2_HAL_H_
#define _VDPU2_HAL_H_

#include <linux/types.h>

/* The maximum registers number of all the version */
#define ROCKCHIP_VDPU2_REG_NUM		159

/* The first register of the decoder is Reg50(0x000c8) */
#define RKVDPU2_REG_DEC_CTRL			0x0c8
#define RKVDPU2_REG_DEC_CTRL_INDEX		(50)

#define RKVDPU2_REG_DEC_INT_EN			0x0dc
#define RKVDPU2_REG_DEC_INT_EN_INDEX		(55)
#define		RKVDPU2_INT_TIMEOUT		BIT(13)
#define		RKVDPU2_INT_STRM_ERROR		BIT(12)
#define		RKVDPU2_INT_SLICE		BIT(9)
#define		RKVDPU2_INT_ASO_ERROR		BIT(8)
#define		RKVDPU2_INT_BUF_EMPTY		BIT(6)
#define		RKVDPU2_INT_BUS_ERROR		BIT(5)
#define		RKVDPU2_DEC_INT			BIT(4)
#define		RKVDPU2_DEC_IRQ_DIS		BIT(1)
#define		RKVDPU2_DEC_INT_RAW		BIT(0)

#define RKVDPU2_REG_DEC_DEV_CTRL		0x0e4
#define RKVDPU2_REG_DEC_DEV_CTRL_INDEX		(57)
#define		RKVDPU2_DEC_CLOCK_GATE_EN	BIT(4)
#define		RKVDPU2_DEC_START		BIT(0)

#define RKVDPU2_REG59				0x0ec
#define RKVDPU2_REG59_INDEX			(59)

int rkvdpu_mpeg2_gen_reg(struct mpp_session *session, void *regs,
			 struct vb2_v4l2_buffer *src_buf);
int rkvdpu_mpeg2_prepare_buf(struct mpp_session *session, void *regs);

#endif
