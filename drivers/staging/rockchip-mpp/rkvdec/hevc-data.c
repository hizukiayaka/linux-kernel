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

#include "hevc-data.h"

/* 7.3.2.2.1 General sequence parameter set RBSP syntax */
int rkvdec_hevc_write_sps(struct rbsp *rbsp,
			  const struct v4l2_ctrl_hevc_sps *sps)
{
	/* TODO: sps_video_parameter_set_id */
	rbsp_write_bits(rbsp, 4, 0);
	/* TODO: sps_seq_parameter_set_id */
	rbsp_write_bits(rbsp, 4, 0);
	/* chroma_format_idc */
	rbsp_write_bits(rbsp, 2, sps->chroma_format_idc);
	rbsp_write_bits(rbsp, 13, sps->pic_width_in_luma_samples);
	rbsp_write_bits(rbsp, 13, sps->pic_height_in_luma_samples);
	/* bit_depth_luma */
	rbsp_write_bits(rbsp, 4, sps->bit_depth_luma_minus8 + 8);
	rbsp_write_bits(rbsp, 4, sps->bit_depth_chroma_minus8 + 8);
	/* log2_max_pic_order_cnt_lsb */
	rbsp_write_bits(rbsp, 5, sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
	rbsp_write_bits(rbsp, 3, sps->log2_diff_max_min_luma_coding_block_size);
	/* log2_min_luma_coding_block_size */
	rbsp_write_bits(rbsp, 3,
			sps->log2_min_luma_coding_block_size_minus3 + 3);
	/* log2_min_transform_block_size */
	rbsp_write_bits(rbsp, 3,
			sps->log2_min_luma_transform_block_size_minus2 + 2);
	rbsp_write_bits(rbsp, 2,
			sps->log2_diff_max_min_luma_transform_block_size);
	rbsp_write_bits(rbsp, 3, sps->max_transform_hierarchy_depth_inter);
	rbsp_write_bits(rbsp, 3, sps->max_transform_hierarchy_depth_intra);

	rbsp_write_flag(rbsp, sps->scaling_list_enabled_flag);
	rbsp_write_flag(rbsp, sps->amp_enabled_flag);
	rbsp_write_flag(rbsp, sps->sample_adaptive_offset_enabled_flag);
	rbsp_write_flag(rbsp, sps->pcm_enabled_flag);

	/* pcm_sample_bit_depth_luma */
	rbsp_write_bits(rbsp, 4, sps->pcm_sample_bit_depth_luma_minus1 + 1);
	/* pcm_sample_bit_depth_chroma */
	rbsp_write_bits(rbsp, 4, sps->pcm_sample_bit_depth_chroma_minus1 + 1);
	rbsp_write_flag(rbsp, sps->pcm_loop_filter_disabled_flag);

	rbsp_write_bits(rbsp, 3,
			sps->log2_diff_max_min_pcm_luma_coding_block_size);
	/* log2_min_pcm_luma_coding_block_size */
	rbsp_write_bits(rbsp, 3,
			sps->log2_min_pcm_luma_coding_block_size_minus3 + 3);
	rbsp_write_bits(rbsp, 7, sps->num_short_term_ref_pic_sets);
	rbsp_write_flag(rbsp, sps->long_term_ref_pics_present_flag);
	rbsp_write_bits(rbsp, 6, sps->num_long_term_ref_pics_sps);
	rbsp_write_flag(rbsp, sps->sps_temporal_mvp_enabled_flag);
	rbsp_write_flag(rbsp, sps->strong_intra_smoothing_enabled_flag);
#if 0
	/* transform_skip_rotation_enabled_flag to intra_smoothing_disabled_flag */
	rbsp_write_bits(rbsp, 7, 0);
	/* sps_max_dec_pic_buffering_minus1 */
	rbsp_write_bits(rbsp, 4, sps->sps_max_dec_pic_buffering_minus1);
	rbsp_write_flag(rbsp, sps->separate_colour_plane_flag);
	/* TODO: high_precision_offsets_enabled */
	rbsp_write_flag(rbsp, 0);
	/* TODO: persistent_rice_adaptation_enabled_flag */
	rbsp_write_flag(rbsp, 0);
	/* reserved */
	rbsp_write_bits(rbsp, 32, 0);
#else
	rbsp_write_bits(rbsp, 7, 0);
	rbsp_write_bits(rbsp, 7, 0x7f);
	/* FIXME align */
	rbsp_write_bits(rbsp, 32, 0xffffffff);
#endif

	return 0;
}

int rkvdec_hevc_write_pps(struct rbsp *rbsp,
			  const struct v4l2_ctrl_hevc_pps *pps)
{
	const struct v4l2_ctrl_hevc_slice_params *slice_params = NULL;
	/* pps_pic_parameter_set_id */
	rbsp_write_bits(rbsp, 6, 0);
	/* pps_seq_parameter_set_id */
	rbsp_write_bits(rbsp, 4, 0);
	/* dependent_slice_segments_enabled_flag */
	rbsp_write_flag(rbsp, pps->dependent_slice_segment_flag);
	rbsp_write_flag(rbsp, pps->output_flag_present_flag);
	rbsp_write_bits(rbsp, 3, pps->num_extra_slice_header_bits);
	/* sign_data_hiding_flag */
	rbsp_write_flag(rbsp, pps->sign_data_hiding_enabled_flag);
	rbsp_write_flag(rbsp, pps->cabac_init_present_flag);
	/* FIXME: from slice params ? */
	rbsp_write_bits(rbsp, 4, slice_params->num_ref_idx_l0_active_minus1);
	rbsp_write_bits(rbsp, 4, slice_params->num_ref_idx_l1_active_minus1);
	/* init_qp_minus26 */
	rbsp_write_bits(rbsp, 6, pps->init_qp_minus26);
	rbsp_write_flag(rbsp, pps->constrained_intra_pred_flag);
	rbsp_write_flag(rbsp, pps->transform_skip_enabled_flag);
	rbsp_write_flag(rbsp, pps->cu_qp_delta_enabled_flag);
	/* TODO: FIXME: Log2MinCuQpDeltaSize */
	//rbsp_write_bits(rbsp, 3, log2_min_cu_qp_delta_size);
	rbsp_write_bits(rbsp, 5, pps->pps_cb_qp_offset);
	rbsp_write_bits(rbsp, 5, pps->pps_cr_qp_offset);
	rbsp_write_flag(rbsp, pps->pps_slice_chroma_qp_offsets_present_flag);
	rbsp_write_flag(rbsp, pps->weighted_pred_flag);
	rbsp_write_flag(rbsp, pps->weighted_bipred_flag);
	rbsp_write_flag(rbsp, pps->transquant_bypass_enabled_flag);
	rbsp_write_flag(rbsp, pps->tiles_enabled_flag);
	rbsp_write_flag(rbsp, pps->entropy_coding_sync_enabled_flag);
	rbsp_write_flag(rbsp, pps->pps_loop_filter_across_slices_enabled_flag);
	rbsp_write_flag(rbsp, pps->loop_filter_across_tiles_enabled_flag);
	rbsp_write_flag(rbsp, pps->deblocking_filter_override_enabled_flag);
	/* pps_deblocking_filter_disabled_flag */
	rbsp_write_flag(rbsp, pps->pps_disable_deblocking_filter_flag);
	rbsp_write_bits(rbsp, 4, pps->pps_beta_offset_div2);
	rbsp_write_bits(rbsp, 4, pps->pps_tc_offset_div2);
	rbsp_write_flag(rbsp, pps->lists_modification_present_flag);
	rbsp_write_bits(rbsp, 3, pps->log2_parallel_merge_level_minus2);
	rbsp_write_flag(rbsp, pps->slice_segment_header_extension_present_flag);
	/* reserved, log2_transform_skip_max_size_minus2 */
	rbsp_write_bits(rbsp, 3, 0);
	/* num_tile_columns */
	rbsp_write_bits(rbsp, 5, pps->num_tile_columns_minus1 + 1);
	/* num_tile_rows */
	rbsp_write_bits(rbsp, 5, pps->num_tile_rows_minus1 + 1);
	/* FIXME */
	rbsp_write_bits(rbsp, 2, 3);
	/* TODO: align 30 */
	/* TODO: column_width[20] */
	/* TODO: column_height[20] */

	/* TODO: scaleing_address */

	return 0;
}

/* 7.3.7 Short-term reference picture set syntax */
int rkvdec_hevc_write_rps(struct rbsp *rbsp,
			  const struct v4l2_ctrl_hevc_pps *pps)
{
}

