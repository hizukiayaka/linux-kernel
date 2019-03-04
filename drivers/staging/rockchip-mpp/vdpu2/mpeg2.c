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

#define DEC_LITTLE_ENDIAN	(1)

static const u8 zigzag[64] = {
	0, 1, 8, 16, 9, 2, 3, 10,
	17, 24, 32, 25, 18, 11, 4, 5,
	12, 19, 26, 33, 40, 48, 41, 34,
	27, 20, 13, 6, 7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36,
	29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46,
	53, 60, 61, 54, 47, 55, 62, 63
};

static const u8 intra_default_q_matrix[64] = {
	8, 16, 19, 22, 26, 27, 29, 34,
	16, 16, 22, 24, 27, 29, 34, 37,
	19, 22, 26, 27, 29, 34, 34, 38,
	22, 22, 26, 27, 29, 34, 37, 40,
	22, 26, 27, 29, 32, 35, 40, 48,
	26, 27, 29, 32, 35, 40, 48, 58,
	26, 27, 29, 34, 38, 46, 56, 69,
	27, 29, 35, 38, 46, 56, 69, 83
};

static void mpeg2_dec_copy_qtable(u8 * qtable, const struct v4l2_ctrl_mpeg2_quantization
				  *ctrl)
{
	int i;

	if (!qtable || !ctrl)
		return;

	if (ctrl->load_intra_quantiser_matrix) {
		for (i = 0; i < 64; i++)
			qtable[zigzag[i]] = ctrl->intra_quantiser_matrix[i];
	} else {
		for (i = 0; i < 64; i++)
			qtable[zigzag[i]] = intra_default_q_matrix[i];

	}

	if (ctrl->load_non_intra_quantiser_matrix) {
		for (i = 0; i < 64; i++)
			qtable[zigzag[i] + 64] =
			    ctrl->non_intra_quantiser_matrix[i];
	} else {
		for (i = 0; i < 64; i++)
			qtable[zigzag[i] + 64] = 16;
	}
}

static void init_hw_cfg(struct vdpu2_regs *p_regs)
{
	p_regs->sw54.dec_strm_wordsp = 1;
	p_regs->sw54.dec_strendian_e = DEC_LITTLE_ENDIAN;
	p_regs->sw54.dec_in_wordsp = 1;
	p_regs->sw54.dec_out_wordsp = 1;
	p_regs->sw54.dec_in_endian = DEC_LITTLE_ENDIAN;	//change
	p_regs->sw54.dec_out_endian = DEC_LITTLE_ENDIAN;
	p_regs->sw57.dec_timeout = 1;
	p_regs->sw57.dec_timeout_e = 1;

	p_regs->sw57.dec_clk_gate_e = 1;

	p_regs->sw50.tiled_mode_msb = 0;
	p_regs->sw56.dec_max_burst = 16;
	p_regs->sw50.dec_scmd_dis = 0;
	p_regs->sw50.dec_adv_pre_dis = 0;
	p_regs->sw52.apf_threshold = 8;

	p_regs->sw50.dec_latency = 0;
	p_regs->sw56.dec_data_disc_e = 0;

	p_regs->sw55.dec_irq = 0;
	p_regs->sw56.dec_axi_rd_id = 0;
	p_regs->sw56.dec_axi_wr_id = 0;

	/* default for MPEG-2 */
	p_regs->sw136.mv_accuracy_fwd = 1;
	p_regs->sw136.mv_accuracy_bwd = 1;
}

int rkvdpu_mpeg2_gen_reg(struct mpp_session *session, void *regs,
			 struct vb2_v4l2_buffer *src_buf)
{
	const struct v4l2_ctrl_mpeg2_slice_params *params;
	const struct v4l2_mpeg2_sequence *sequence;
	const struct v4l2_mpeg2_picture *picture;
	const struct v4l2_ctrl_mpeg2_quantization *quantization;
	struct vdpu2_regs *p_regs = regs;
	void *qtable = NULL;
	size_t stream_len = 0;

	params = rockchip_mpp_get_cur_ctrl(session,
					   V4L2_CID_MPEG_VIDEO_MPEG2_SLICE_PARAMS);
	if (!params)
		return -EINVAL;

	sequence = &params->sequence;
	picture = &params->picture;
	quantization = rockchip_mpp_get_cur_ctrl(session,
			V4L2_CID_MPEG_VIDEO_MPEG2_QUANTIZATION);

	init_hw_cfg(p_regs);

	p_regs->sw120.pic_mb_width = DIV_ROUND_UP(sequence->horizontal_size,
						  16);
	p_regs->sw120.pic_mb_height_p = DIV_ROUND_UP(sequence->vertical_size,
						     16);

	/* PICT_FRAME */
	if (picture->picture_structure == 3) {
		p_regs->sw57.pic_fieldmode_e = 0;
	} else {
		p_regs->sw57.pic_fieldmode_e = 1;
		/* PICT_TOP_FIEL */
		if (picture->picture_structure == 1)
			p_regs->sw57.pic_topfield_e = 1;
	}

	switch (picture->picture_coding_type) {
	case V4L2_MPEG2_PICTURE_CODING_TYPE_B:
		p_regs->sw57.pic_b_e = 1;
	case V4L2_MPEG2_PICTURE_CODING_TYPE_P:
		p_regs->sw57.pic_inter_e = 1;
		break;
	case V4L2_MPEG2_PICTURE_CODING_TYPE_I:
	default:
		p_regs->sw57.pic_inter_e = 0;
		p_regs->sw57.pic_b_e = 0;
		break;
	}

	if (picture->top_field_first)
		p_regs->sw120.mpeg.topfieldfirst_e = 1;

	p_regs->sw57.fwd_interlace_e = 0;
	p_regs->sw57.write_mvs_e = 0;

	p_regs->sw120.mpeg.alt_scan_e = picture->alternate_scan;
	p_regs->sw136.alt_scan_flag_e = picture->alternate_scan;

	p_regs->sw122.qscale_type = picture->q_scale_type;
	p_regs->sw122.intra_dc_prec = picture->intra_dc_precision;
	p_regs->sw122.con_mv_e = picture->concealment_motion_vectors;
	p_regs->sw122.intra_vlc_tab = picture->intra_vlc_format;
	p_regs->sw122.frame_pred_dct = picture->frame_pred_frame_dct;
	p_regs->sw51.qp_init = 1;

	/* MPEG-2 decoding mode */
	p_regs->sw53.dec_mode = RKVDPU2_FMT_MPEG2D;

	p_regs->sw136.fcode_fwd_hor = picture->f_code[0][0];
	p_regs->sw136.fcode_fwd_ver = picture->f_code[0][1];
	p_regs->sw136.fcode_bwd_hor = picture->f_code[1][0];
	p_regs->sw136.fcode_bwd_ver = picture->f_code[1][1];

	p_regs->sw57.pic_interlace_e = 1 - sequence->progressive_sequence;
#if 0
	/* MPEG-1 decoding mode */
	p_regs->sw53.sw_dec_mode = 6;
	p_regs->sw136.fcode_fwd_hor = picture->f_code[0][1];
	p_regs->sw136.fcode_fwd_ver = picture->f_code[0][1];
	p_regs->sw136.fcode_bwd_hor = picture->f_code[1][1];
	p_regs->sw136.fcode_bwd_ver = picture->f_code[1][1];
	if (picture->f_code[0][0])
		p_regs->sw136.mv_accuracy_fwd = 0;
	if (picture->f_code[1][0])
		p_regs->sw136.mv_accuracy_bwd = 0;
#endif
	p_regs->sw52.startmb_x = 0;
	p_regs->sw52.startmb_y = 0;
	p_regs->sw57.dec_out_dis = 0;
	p_regs->sw50.filtering_dis = 1;

	p_regs->sw64.rlc_vlc_base =
	    vb2_dma_contig_plane_dma_addr(&src_buf->vb2_buf, 0);
	p_regs->sw122.strm_start_bit = params->data_bit_offset;
	stream_len = vb2_get_plane_payload(&src_buf->vb2_buf, 0);
	p_regs->sw51.stream_len = stream_len;

	qtable = vb2_plane_vaddr(&src_buf->vb2_buf, 0) + ALIGN(stream_len, 8);
	mpeg2_dec_copy_qtable(qtable, quantization);
        p_regs->sw61.qtable_base = p_regs->sw64.rlc_vlc_base
		+ ALIGN(stream_len, 8);

	return 0;
}

int rkvdpu_mpeg2_prepare_buf(struct mpp_session *session, void *regs)
{
	const struct v4l2_ctrl_mpeg2_slice_params *params;
	const struct v4l2_mpeg2_sequence *sequence;
	const struct v4l2_mpeg2_picture *picture;
	struct vb2_v4l2_buffer *dst_buf;
	dma_addr_t cur_addr, fwd_addr, bwd_addr;
	struct vb2_queue *cap_q = &session->fh.m2m_ctx->cap_q_ctx.q;
	struct vdpu2_regs *p_regs = regs;

	params =
	    rockchip_mpp_get_cur_ctrl(session,
				      V4L2_CID_MPEG_VIDEO_MPEG2_SLICE_PARAMS);
	picture = &params->picture;
	sequence = &params->sequence;

	dst_buf = v4l2_m2m_next_dst_buf(session->fh.m2m_ctx);
	cur_addr = vb2_dma_contig_plane_dma_addr(&dst_buf->vb2_buf, 0);

	fwd_addr =
	    rockchip_mpp_find_addr(cap_q, &dst_buf->vb2_buf,
				   params->forward_ref_ts);
	bwd_addr =
	    rockchip_mpp_find_addr(cap_q, &dst_buf->vb2_buf,
				   params->backward_ref_ts);
	if (!bwd_addr)
		bwd_addr = cur_addr;
	if (!fwd_addr)
		fwd_addr = cur_addr;

	/* Starting from the second horizontal line */
	if (picture->picture_structure == 2)
		cur_addr += ALIGN(sequence->horizontal_size, 16);
	p_regs->sw63.dec_out_base = cur_addr;

	if (picture->picture_coding_type == V4L2_MPEG2_PICTURE_CODING_TYPE_B) {
		p_regs->sw131.refer0_base = fwd_addr >> 2;
		p_regs->sw148.refer1_base = fwd_addr >> 2;
		p_regs->sw134.refer2_base = bwd_addr >> 2;
		p_regs->sw135.refer3_base = bwd_addr >> 2;
	} else {
		if (picture->picture_structure == 3 ||
		    (picture->picture_structure == 1
		     && picture->top_field_first)
		    || (picture->picture_structure == 2
			&& !picture->top_field_first)) {
			p_regs->sw131.refer0_base = fwd_addr >> 2;
			p_regs->sw148.refer1_base = fwd_addr >> 2;
		} else if (picture->picture_structure == 1) {
			p_regs->sw131.refer0_base = fwd_addr >> 2;
			p_regs->sw148.refer1_base = cur_addr >> 2;
		} else if (picture->picture_structure == 2) {
			p_regs->sw131.refer0_base = cur_addr >> 2;
			p_regs->sw148.refer1_base = fwd_addr >> 2;
		}
		p_regs->sw134.refer2_base = cur_addr >> 2;
		p_regs->sw135.refer3_base = cur_addr >> 2;
	}

	return 0;
}
