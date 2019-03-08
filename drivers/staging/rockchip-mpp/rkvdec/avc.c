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

#include <linux/types.h>

#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-dma-contig.h>

#include "mpp_dev_common.h"
#include "hal.h"
#include "regs.h"

static void init_hw_cfg(struct rkvdec_regs *p_regs)
{
	p_regs->sw_interrupt.dec_e = 1;
	p_regs->sw_interrupt.dec_timeout_e = 1;
	/* AVC */
	p_regs->sw_sysctrl.dec_mode = RKVDEC_FMT_H264D;
	p_regs->sw_sysctrl.rlc_mode = 0;

	p_regs->sw_picparameter.slice_num_lowbits = 0x7ff;
	p_regs->sw_picparameter.slice_num_highbit = 1;
}

/* FIXME: TODO: support field coding */
static void stride_calc(struct rkvdec_regs *p_regs,
			const struct v4l2_ctrl_h264_sps *sps)
{
	u32 stride_y, stride_uv, virstride_y, virstride_yuv;
	u32 width, height;

	width = (sps->pic_width_in_mbs_minus1 + 1) << 4;
	/* TODO: frame_mbs_only_flag (7-18) */
	height = (sps->pic_height_in_map_units_minus1 + 1) << 4;

	stride_y = (width * ALIGN(sps->bit_depth_luma_minus8, 8)) >> 3;
	stride_uv = (height * ALIGN(sps->bit_depth_chroma_minus8, 8)) >> 3;
	/* TODO: align with 16 bytes while the resolution is under HD */
	stride_y = ALIGN(stride_y, 256) | 256;
	stride_uv = ALIGN(stride_uv, 256) | 256;

	virstride_y = stride_y * ALIGN(height, 16);

	switch (sps->chroma_format_idc) {
	default:
	case 0:
		virstride_yuv = virstride_y;
		break;
	case 1:
		virstride_yuv = virstride_y * 3 / 2;
		break;
	case 2:
		virstride_yuv = virstride_y;
		break;
	}

	p_regs->sw_picparameter.y_hor_virstride = stride_y >> 4;
	p_regs->sw_picparameter.uv_hor_virstride = stride_uv >> 4;
	p_regs->sw_y_virstride = virstride_y >> 4;
	p_regs->sw_yuv_virstride = virstride_yuv >> 4;
}

static int rkvdec_avc_gen_ref(struct rkvdec_regs *p_regs,
			      struct vb2_v4l2_buffer *dst_buf,
			      const struct v4l2_ctrl_h264_decode_param
			      *decode_param)
{
	struct vb2_queue *cap_q = dst_buf->vb2_buf.vb2_queue;
	dma_addr_t cur_addr;
	u16 i = 0;

	cur_addr = vb2_dma_contig_plane_dma_addr(&dst_buf->vb2_buf, 0);
	p_regs->sw_decout_base = cur_addr;

	p_regs->sw_cur_poc = decode_param->top_field_order_cnt;
	p_regs->sw_cur_poc_b = decode_param->bottom_field_order_cnt;

	/* 8.2.1 Decoding process for picture order count */
	for (i = 0; i < 8; i++) {
		/* TopFieldOrderCnt */
		p_regs->sw_refer_poc[i * 2] =
		    decode_param->dpb[i].top_field_order_cnt;
		p_regs->sw_refer_poc[i * 2 + 1] =
		    decode_param->dpb[i].bottom_field_order_cnt;
	}

	for (i = 8; i < 15; i++) {
		u16 j = i - 8;

		p_regs->sw_refer15_29_poc[j * 2] =
		    decode_param->dpb[i].top_field_order_cnt;
		p_regs->sw_refer15_29_poc[j * 2 + 1] =
		    decode_param->dpb[i].bottom_field_order_cnt;
	}

	p_regs->sw72_h264_refer30_poc =
	    decode_param->dpb[15].top_field_order_cnt;
	p_regs->sw73_h264_refer31_poc =
	    decode_param->dpb[15].bottom_field_order_cnt;

	for (i = 0; i < 16; i++) {
		dma_addr_t ref_addr;

		ref_addr = rockchip_mpp_find_addr(cap_q, &dst_buf->vb2_buf,
						  decode_param->
						  dpb[i].timestamp);
		if (!ref_addr)
			ref_addr = cur_addr;

		/* TODO: support filed codec */
		if (15 == i) {
			p_regs->sw48_h264.ref_base = ref_addr >> 4;
			p_regs->sw48_h264.ref_topfield_used = 1;
			p_regs->sw48_h264.ref_botfield_used = 1;
			p_regs->sw48_h264.ref_colmv_use_flag = 1;
			break;
		}

		p_regs->sw_refer_base[i].ref_base = ref_addr >> 4;
		p_regs->sw_refer_base[i].ref_topfield_used = 1;
		p_regs->sw_refer_base[i].ref_botfield_used = 1;
		p_regs->sw_refer_base[i].ref_colmv_use_flag = 1;

		cur_addr = ref_addr;
	}

	return 0;
}

int rkvdec_avc_gen_reg(struct mpp_session *session, void *regs,
		       struct vb2_v4l2_buffer *src_buf)
{
	const struct v4l2_ctrl_h264_sps *sps;
	const struct v4l2_ctrl_h264_pps *pps;
	const struct v4l2_ctrl_h264_slice_param *slice_param;
	const struct v4l2_ctrl_h264_decode_param *decode_param;
	struct vb2_v4l2_buffer *dst_buf;
	struct rkvdec_regs *p_regs = regs;
	size_t stream_len = 0;

	sps = rockchip_mpp_get_cur_ctrl(session, V4L2_CID_MPEG_VIDEO_H264_SPS);
	pps = rockchip_mpp_get_cur_ctrl(session, V4L2_CID_MPEG_VIDEO_H264_PPS);
	slice_param = rockchip_mpp_get_cur_ctrl(session,
						V4L2_CID_MPEG_VIDEO_H264_SLICE_PARAMS);
	decode_param = rockchip_mpp_get_cur_ctrl(session,
						 V4L2_CID_MPEG_VIDEO_H264_DECODE_PARAMS);

	if (!sps || !pps || !slice_param || !decode_param)
		return -EINVAL;

	init_hw_cfg(p_regs);

	stride_calc(p_regs, sps);

	p_regs->sw_strm_rlc_base =
	    vb2_dma_contig_plane_dma_addr(&src_buf->vb2_buf, 0);
	/* The bitstream must be 128bit align ? */
	p_regs->sw_sysctrl.strm_start_bit = slice_param->header_bit_size;

	/* hardware wants a zerod memory at the stream end */
	stream_len = DIV_ROUND_UP(slice_param->size, 16) + 64;
	p_regs->sw_stream_len = stream_len;

	dst_buf = v4l2_m2m_next_dst_buf(session->fh.m2m_ctx);
	rkvdec_avc_gen_ref(p_regs, dst_buf, decode_param);

	return 0;
}
