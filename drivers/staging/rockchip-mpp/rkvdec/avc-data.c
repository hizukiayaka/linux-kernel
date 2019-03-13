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

#include "avc-data.h"

static const u32 zig_zag_4x4[16] = {
	0, 1, 4, 8, 5, 2, 3, 6, 9, 12, 13, 10, 7, 11, 14, 15
};

static const u32 zig_zag_8x8[64] = {
	0, 1, 8, 16, 9, 2, 3, 10, 17, 24, 32, 25, 18, 11, 4, 5,
	12, 19, 26, 33, 40, 48, 41, 34, 27, 20, 13, 6, 7, 14, 21, 28,
	35, 42, 49, 56, 57, 50, 43, 36, 29, 22, 15, 23, 30, 37, 44, 51,
	58, 59, 52, 45, 38, 31, 39, 46, 53, 60, 61, 54, 47, 55, 62, 63
};

static void fill_is_long_term(struct rbsp *rbsp, const struct v4l2_ctrl_h264_decode_param
			      *decode_param)
{
	u16 is_long_term = 0;
	u8 i;

	for (i = 0; i < 16; i++)
		if (decode_param->
		    dpb[i].flags & V4L2_H264_DPB_ENTRY_FLAG_LONG_TERM)
			is_long_term |= (1 << i);

	rbsp_write_bits(rbsp, 16, is_long_term);
}

/* in zig-zag order */
void rkvdec_avc_update_scaling_list(u8 * buf, const struct
				    v4l2_ctrl_h264_scaling_matrix
				    *scaling)
{
	u8 i, j;

	for (i = 0; i < 6; i++)
		for (j = 0; j < 16; j++)
			buf[zig_zag_4x4[j] + (i << 4)] =
			    scaling->scaling_list_4x4[i][j];

	for (i = 0; i < 2; i++)
		for (j = 0; j < 64; j++)
			buf[zig_zag_8x8[j] + (i << 6)] =
			    scaling->scaling_list_8x8[i][j];
}

int rkvdec_avc_write_sps(struct rbsp *rbsp,
			 const struct v4l2_ctrl_h264_sps *sps)
{
	/* TODO: seq_parameter_set_id */
	rbsp_write_bits(rbsp, 4, 0);
	rbsp_write_bits(rbsp, 8, sps->profile_idc);
	/* constraint_set3_flag */
	rbsp_write_flag(rbsp, sps->constraint_set_flags >> 3);
	rbsp_write_bits(rbsp, 2, sps->chroma_format_idc);
	/* bit_depth_luma */
	rbsp_write_bits(rbsp, 3, sps->bit_depth_luma_minus8 + 8);
	rbsp_write_bits(rbsp, 3, sps->bit_depth_chroma_minus8 + 8);
	/* TODO: qpprime_y_zero_transform_bypass_flag */
	rbsp_write_flag(rbsp, 0);
	rbsp_write_bits(rbsp, 4, sps->log2_max_frame_num_minus4);
	rbsp_write_bits(rbsp, 4, sps->max_num_ref_frames);
	rbsp_write_bits(rbsp, 2, sps->pic_order_cnt_type);
	rbsp_write_bits(rbsp, 4, sps->log2_max_pic_order_cnt_lsb_minus4);
	/* delta_pic_order_always_zero_flag */
	rbsp_write_flag(rbsp,
			sps->flags &
			V4L2_H264_SPS_FLAG_DELTA_PIC_ORDER_ALWAYS_ZERO);
	rbsp_write_bits(rbsp, 9, sps->pic_width_in_mbs_minus1 + 1);
	/* TODO: check whether it work for field coding */
	rbsp_write_bits(rbsp, 9, sps->pic_height_in_map_units_minus1 + 1);
	rbsp_write_flag(rbsp, sps->flags & V4L2_H264_SPS_FLAG_FRAME_MBS_ONLY);
	rbsp_write_flag(rbsp,
			sps->flags &
			V4L2_H264_SPS_FLAG_MB_ADAPTIVE_FRAME_FIELD);
	rbsp_write_flag(rbsp,
			sps->flags & V4L2_H264_SPS_FLAG_DIRECT_8X8_INFERENCE);

	/* TODO: mvc */
	rbsp_write_flag(rbsp, 0);
	rbsp_write_bits(rbsp, 2, 0);
	/* view_id[0] */
	rbsp_write_bits(rbsp, 10, 0);
	/* view_id[1] */
	rbsp_write_bits(rbsp, 10, 0);
	/* num_anchor_refs_l0 */
	rbsp_write_flag(rbsp, 0);
	/* anchor_ref_l0 */
	rbsp_write_bits(rbsp, 10, 0);
	rbsp_write_flag(rbsp, 0);
	rbsp_write_bits(rbsp, 10, 0);
	/* num_non_anchor_refs_l0 */
	rbsp_write_flag(rbsp, 0);
	rbsp_write_bits(rbsp, 10, 0);
	rbsp_write_flag(rbsp, 0);
	rbsp_write_bits(rbsp, 10, 0);
	/* Align with 128 bit */
	rbsp_write_bits(rbsp, 3, 0);

	return 0;
}

int rkvdec_avc_write_pps(struct rbsp *rbsp,
			 const struct v4l2_ctrl_h264_pps *pps)
{
	/* TODO: pps_pic_parameter_set_id */
	rbsp_write_bits(rbsp, 8, 0);
	rbsp_write_bits(rbsp, 5, 0);
	rbsp_write_flag(rbsp,
			pps->flags & V4L2_H264_PPS_FLAG_ENTROPY_CODING_MODE);
	rbsp_write_flag(rbsp,
			pps->flags &
			V4L2_H264_PPS_FLAG_BOTTOM_FIELD_PIC_ORDER_IN_FRAME_PRESENT);
	rbsp_write_bits(rbsp, 5, pps->num_ref_idx_l0_default_active_minus1);
	rbsp_write_bits(rbsp, 5, pps->num_ref_idx_l1_default_active_minus1);
	rbsp_write_flag(rbsp,
			(pps->flags & V4L2_H264_PPS_FLAG_WEIGHTED_PRED) >> 2);
	rbsp_write_bits(rbsp, 2, pps->weighted_bipred_idc);
	rbsp_write_bits(rbsp, 7, pps->pic_init_qp_minus26);
	rbsp_write_bits(rbsp, 6, pps->pic_init_qs_minus26);
	rbsp_write_bits(rbsp, 5, pps->chroma_qp_index_offset);
	rbsp_write_flag(rbsp,
			pps->flags &
			V4L2_H264_PPS_FLAG_DEBLOCKING_FILTER_CONTROL_PRESENT);
	rbsp_write_flag(rbsp,
			pps->flags & V4L2_H264_PPS_FLAG_CONSTRAINED_INTRA_PRED);
	rbsp_write_flag(rbsp,
			pps->flags &
			V4L2_H264_PPS_FLAG_REDUNDANT_PIC_CNT_PRESENT);
	rbsp_write_flag(rbsp,
			pps->flags & V4L2_H264_PPS_FLAG_TRANSFORM_8X8_MODE);
	rbsp_write_bits(rbsp, 5, pps->second_chroma_qp_index_offset);
	rbsp_write_flag(rbsp,
			pps->flags &
			V4L2_H264_PPS_FLAG_PIC_SCALING_MATRIX_PRESENT);

	return 0;
}

int rkvdec_avc_write_pps_tail(struct rbsp *rbsp, dma_addr_t scaling_addr, const struct v4l2_ctrl_h264_decode_param
			      *decode_param)
{
	/* scaling list buffer */
	rbsp_write_bits(rbsp, 32, scaling_addr);

	/* DPB */
	fill_is_long_term(rbsp, decode_param);

	/* TODO: VOIdx, Layer id */
	rbsp_write_bits(rbsp, 16, 0);

	/* Align with 128 bit */
	rbsp_write_bits(rbsp, 8, 0);

	return 0;
}

static inline void fill_rps_list(struct rbsp *rbsp, const struct v4l2_ctrl_h264_decode_param
				 *decode_param, const u8 * list)
{
	u8 i;

	for (i = 0; i < 32; i++) {
		u8 idx, active_flag;

		idx = list[i];

		active_flag = decode_param->dpb[idx].flags &
		    V4L2_H264_DPB_ENTRY_FLAG_ACTIVE;
		if (!active_flag) {
			rbsp_write_bits(rbsp, 7, 0);
		} else {
			rbsp_write_bits(rbsp, 5, idx | BIT(5));
			/* TODO: bottom flag */
			rbsp_write_flag(rbsp, 0);
			/* TODO: view id */
			rbsp_write_flag(rbsp, 0);
		}
	}
}

int rkvdec_avc_write_rps(struct rbsp *rbsp,
			 const struct v4l2_ctrl_h264_sps *sps,
			 const struct v4l2_ctrl_h264_slice_param *slice_param,
			 const struct v4l2_ctrl_h264_decode_param *decode_param)
{
	int max_frame_num = sps->log2_max_frame_num_minus4 + 4;
	u8 i;

	for (i = 0; i < 16; i++) {
		u16 frame_num = decode_param->dpb[i].frame_num;

		rbsp_write_bits(rbsp, 16, frame_num > max_frame_num ?
				frame_num - max_frame_num : frame_num);
	}

	/* reserved */
	rbsp_write_bits(rbsp, 16, 0);
	/* TODO: VoidX */
	rbsp_write_bits(rbsp, 16, 0);

	switch (slice_param->slice_type) {
	case V4L2_H264_SLICE_TYPE_P:
		fill_rps_list(rbsp, decode_param, slice_param->ref_pic_list0);
		rbsp_write_bits(rbsp, 224, 0);
		rbsp_write_bits(rbsp, 224, 0);
		break;
	case V4L2_H264_SLICE_TYPE_B:
		rbsp_write_bits(rbsp, 224, 0);
		fill_rps_list(rbsp, decode_param, slice_param->ref_pic_list0);
		fill_rps_list(rbsp, decode_param, slice_param->ref_pic_list1);
		break;
	case V4L2_H264_SLICE_TYPE_I:
		/* TODO: SVC */
	default:
		rbsp_write_bits(rbsp, 224, 0);
		rbsp_write_bits(rbsp, 224, 0);
		rbsp_write_bits(rbsp, 224, 0);
		break;
	}
}
