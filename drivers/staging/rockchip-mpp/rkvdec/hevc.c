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

#include "hevc-data.h"

static void init_hw_cfg(struct rkvdec_regs *p_regs)
{
	p_regs->sw_interrupt.dec_e = 1;
	p_regs->sw_interrupt.dec_timeout_e = 1;
	/* TODO: support HEVC tiles */
	p_regs->sw_interrupt.wr_ddr_align_en = 1;
	/* HEVC */
	p_regs->sw_sysctrl.dec_mode = RKVDEC_FMT_H265D;

	p_regs->sw_ref_valid = 0;
	/* cabac_error_en */
	p_regs->sw_strmd_error_en = 0xfdfffffd;
	/* p_regs->extern_error_en = 0x30000000; */
	p_regs->extern_error_en.error_en_highbits = 0x3000000;
}

static void ctb_calc(struct rkvdec_regs *p_regs,
		     const struct v4l2_ctrl_hevc_sps *sps)
{
	u32 min_cb_log2_size_y, ctb_log2_size_y, min_cb_size_y, ctb_size_y;
	u32 pic_width_in_min_cbs_y, pic_height_in_min_bbs_y;
	u32 stride_y, stride_uv, virstride_y, virstride_yuv;
	u32 width, height;

	min_cb_log2_size_y = sps->log2_min_luma_coding_block_size_minus3 + 3;
	ctb_log2_size_y = min_cb_log2_size_y +
	    sps->log2_diff_max_min_luma_coding_block_size;

	min_cb_size_y = 1 << min_cb_log2_size_y;
	/* uiMaxCUWidth */
	ctb_size_y = 1 << ctb_log2_size_y;

	/* PicWidthInCtbsY (7-15) */
	pic_width_in_min_cbs_y = sps->pic_width_in_luma_samples / min_cb_size_y;
	pic_height_in_min_bbs_y = sps->pic_height_in_luma_samples /
	    min_cb_size_y;

	width = pic_width_in_min_cbs_y << min_cb_log2_size_y;
	height = pic_height_in_min_bbs_y << min_cb_log2_size_y;

	stride_y = (roundup(width, ctb_size_y) *
		    ALIGN(sps->bit_depth_luma_minus8, 8)) >> 3;
	stride_uv = (roundup(height, ctb_size_y) *
		     ALIGN(sps->bit_depth_chroma_minus8, 8)) >> 3;
	stride_y = ALIGN(stride_y, 256) | 256;
	stride_uv = ALIGN(stride_uv, 256) | 256;

	virstride_y = stride_y * ALIGN(height, 8);
	/* TODO: only NV12 is supported by device now */
	virstride_yuv = virstride_y + ((stride_uv * ALIGN(height, 8)) >> 1);

	p_regs->sw_picparameter.y_hor_virstride = stride_y >> 4;
	p_regs->sw_picparameter.uv_hor_virstride = stride_uv >> 4;
	p_regs->sw_y_virstride = virstride_y >> 4;
	p_regs->sw_yuv_virstride = virstride_yuv >> 4;
}

static int rkvdec_hevc_gen_ref(struct rkvdec_regs *p_regs,
			       struct vb2_v4l2_buffer *dst_buf,
			       const struct v4l2_ctrl_hevc_slice_params *slice_params)
{
	struct vb2_queue *cap_q = dst_buf->vb2_buf.vb2_queue;
	dma_addr_t cur_addr;
	u16 i = 0;

	cur_addr = vb2_dma_contig_plane_dma_addr(&dst_buf->vb2_buf, 0);
	p_regs->sw_decout_base = cur_addr;

	/* FIXME: use a const value */
	for (i = 0; i < 15; i++)
		p_regs->sw_refer_base[i].ref_base = cur_addr >> 4;

	for (i = 0; i < slice_params->num_active_dpb_entries; i++) {
		dma_addr_t ref_addr;
		/* FIXME: why two pic_order_cnt */
		p_regs->sw_refer_poc[i] = slice_params->dpb[i].pic_order_cnt[0];

		ref_addr = rockchip_mpp_find_addr(cap_q, &dst_buf->vb2_buf,
						  slice_params->dpb[i].
						  timestamp);
		if (!ref_addr)
			ref_addr = cur_addr;

		p_regs->sw_refer_base[i].ref_base = ref_addr >> 4;
		cur_addr = ref_addr;

		p_regs->sw_ref_valid |= (1 << i);
	}

	/* Enable flag for reference picture */
	p_regs->sw_refer_base[0].ref_valid_flag =
	    (p_regs->sw_ref_valid >> 0) & 0xf;
	p_regs->sw_refer_base[1].ref_valid_flag =
	    (p_regs->sw_ref_valid >> 4) & 0xf;
	p_regs->sw_refer_base[2].ref_valid_flag =
	    (p_regs->sw_ref_valid >> 8) & 0xf;
	p_regs->sw_refer_base[3].ref_valid_flag =
	    (p_regs->sw_ref_valid >> 12) & 0x7;

	return 0;
}

int rkvdec_hevc_gen_reg(struct mpp_session *session, void *regs,
			struct vb2_v4l2_buffer *src_buf)
{
	const struct v4l2_ctrl_hevc_sps *sps;
	const struct v4l2_ctrl_hevc_pps *pps;
	const struct v4l2_ctrl_hevc_slice_params *slice_params;
	struct vb2_v4l2_buffer *dst_buf;
	struct rkvdec_regs *p_regs = regs;
	size_t stream_len = 0;

	sps = rockchip_mpp_get_cur_ctrl(session, V4L2_CID_MPEG_VIDEO_HEVC_SPS);
	pps = rockchip_mpp_get_cur_ctrl(session, V4L2_CID_MPEG_VIDEO_HEVC_PPS);
	slice_params = rockchip_mpp_get_cur_ctrl(session,
						 V4L2_CID_MPEG_VIDEO_HEVC_SLICE_PARAMS);
	if (!sps || !pps || !slice_params)
		return -EINVAL;

	init_hw_cfg(p_regs);

	ctb_calc(p_regs, sps);

	/* FIXME: support more than 1 slice */
	p_regs->sw_picparameter.slice_num_lowbits = 0;
	p_regs->sw_strm_rlc_base =
	    vb2_dma_contig_plane_dma_addr(&src_buf->vb2_buf, 0);
	/* The bitstream must be 128bit align ? */
	p_regs->sw_sysctrl.strm_start_bit = slice_params->data_bit_offset;

	/* hardware wants a zerod memory at the stream end */
	stream_len = DIV_ROUND_UP(slice_params->bit_size, 8);
	stream_len = DIV_ROUND_UP(stream_len, 16) + 64;
	p_regs->sw_stream_len = stream_len;

	dst_buf = v4l2_m2m_next_dst_buf(session->fh.m2m_ctx);
	rkvdec_hevc_gen_ref(p_regs, dst_buf, slice_params);

	return 0;
}
