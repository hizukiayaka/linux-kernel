// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Fuzhou Rockchip Electronics Co., Ltd
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

#ifndef _RKVDEC_HAL_H
#define _RKVDEC_HAL_H

/* The maximum registers number of all the version */
#define ROCKCHIP_RKVDEC_REG_NUM			(109)

#define RKVDEC_REG_DEC_INT_EN			0x004
#define RKVDEC_REG_DEC_INT_EN_INDEX		(1)
#define		RKVDEC_WR_DDR_ALIGN_EN		BIT(23)
#define		RKVDEC_FORCE_SOFT_RESET_VALID	BIT(21)
#define		RKVDEC_SOFTWARE_RESET_EN	BIT(20)
#define		RKVDEC_INT_COLMV_REF_ERROR	BIT(17)
#define		RKVDEC_INT_BUF_EMPTY		BIT(16)
#define		RKVDEC_INT_TIMEOUT		BIT(15)
#define		RKVDEC_INT_STRM_ERROR		BIT(14)
#define		RKVDEC_INT_BUS_ERROR		BIT(13)
#define		RKVDEC_DEC_INT_RAW		BIT(9)
#define		RKVDEC_DEC_INT			BIT(8)
#define		RKVDEC_DEC_TIMEOUT_EN		BIT(5)
#define		RKVDEC_DEC_IRQ_DIS		BIT(4)
#define		RKVDEC_CLOCK_GATE_EN		BIT(1)
#define		RKVDEC_DEC_START		BIT(0)

#define RKVDEC_REG_SYS_CTRL			0x008
#define RKVDEC_REG_SYS_CTRL_INDEX		(2)
#define		RKVDEC_GET_FORMAT(x)		(((x) >> 20) & 0x3)
#define		RKVDEC_FMT_H265D		(0)
#define		RKVDEC_FMT_H264D		(1)
#define		RKVDEC_FMT_VP9D			(2)

#define RKVDEC_REG_STREAM_RLC_BASE		0x010
#define RKVDEC_REG_STREAM_RLC_BASE_INDEX	(4)

#define RKVDEC_REG_PPS_BASE			0x0a0
#define RKVDEC_REG_PPS_BASE_INDEX		(42)

#define RKVDEC_REG_VP9_REFCOLMV_BASE		0x0d0
#define RKVDEC_REG_VP9_REFCOLMV_BASE_INDEX	(52)

#define RKVDEC_REG_CACHE_ENABLE(i)		(0x41c + ((i) * 0x40))
#define		RKVDEC_CACHE_PERMIT_CACHEABLE_ACCESS	BIT(0)
#define		RKVDEC_CACHE_PERMIT_READ_ALLOCATE	BIT(1)
#define		RKVDEC_CACHE_LINE_SIZE_64_BYTES		BIT(4)

int rkvdec_avc_gen_reg(struct mpp_session *session, void *regs,
			struct vb2_v4l2_buffer *src_buf);

int rkvdec_hevc_gen_reg(struct mpp_session *session, void *regs,
			struct vb2_v4l2_buffer *src_buf);

#endif
