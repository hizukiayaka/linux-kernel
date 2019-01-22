// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2017 Fuzhou Rockchip Electronics Co., Ltd
 *		Randy Li, <ayaka@soulik.info>
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

#ifndef _VDPU2_REGS_H_
#define _VDPU2_REGS_H_

#define RKVDPU2_REG_SYS_CTRL			0x0d4
#define RKVDPU2_REG_SYS_CTRL_INDEX		(53)
#define		RKVDPU2_GET_FORMAT(x)		((x) & 0xf)
#define		RKVDPU2_FMT_H264D		0
#define		RKVDPU2_FMT_MPEG4D		1
#define		RKVDPU2_FMT_H263D		2
#define		RKVDPU2_FMT_JPEGD		3
#define		RKVDPU2_FMT_VC1D		4
#define		RKVDPU2_FMT_MPEG2D		5
#define		RKVDPU2_FMT_MPEG1D		6
#define		RKVDPU2_FMT_VP6D		7
#define		RKVDPU2_FMT_RESERVED		8
#define		RKVDPU2_FMT_VP7D		9
#define		RKVDPU2_FMT_VP8D		10
#define		RKVDPU2_FMT_AVSD		11

#define RKVDPU2_REG_DIR_MV_BASE                 0x0f8
#define RKVDPU2_REG_DIR_MV_BASE_INDEX           (62)

#define RKVDPU2_REG_STREAM_RLC_BASE		0x100
#define RKVDPU2_REG_STREAM_RLC_BASE_INDEX	(64)

struct vdpu2_regs {
	/* Post processor */
	u32 sw00_49[50];

	struct {
		u32 tiled_mode_msb:1;
		u32 dec_latency:6;
		u32 dec_fixed_quant:1;
		u32 filtering_dis:1;
		u32 skip_sel:1;
		u32 dec_scmd_dis:1;
		u32 dec_adv_pre_dis:1;
		u32 tiled_mode_lsb:1;
		u32 refbuf_thrd:12;
		u32 refbuf_pid:5;
		u32 reserved0:2;
	} sw50;

	struct {
		u32 stream_len:24;
		u32 stream_len_ext:1;
		u32 qp_init:6;
		u32 reserved0:1;
	} sw51;

	struct {
		/* ydim_mbst */
		u32 startmb_y:8;
		/* xdim_mbst */
		u32 startmb_x:9;
		/* adv_pref_thrd */
		u32 apf_threshold:14;
		u32 reserved0:1;
	} sw52;

	struct {
		u32 dec_mode:4;
		u32 reserved0:28;
	} sw53;

	struct {
		u32 dec_in_endian:1;
		u32 dec_out_endian:1;
		u32 dec_in_wordsp:1;
		u32 dec_out_wordsp:1;
		u32 dec_strm_wordsp:1;
		u32 dec_strendian_e:1;
		u32 reserved0:26;
	} sw54;

	struct {
		u32 dec_irq:1;
		u32 dec_irq_dis:1;
		u32 reserved0:2;
		u32 dec_rdy_sts:1;
		u32 pp_bus_sts:1;
		u32 buf_emt_sts:1;
		u32 reserved1:1;
		u32 aso_det_sts:1;
		u32 slice_det_sts:1;
		u32 bslice_det_sts:1;
		u32 reserved2:1;
		u32 error_det_sts:1;
		u32 timeout_det_sts:1;
		u32 reserved3:18;
	} sw55;

	struct {
		u32 dec_axi_rd_id:8;
		u32 dec_axi_wr_id:8;
		u32 dec_max_burst:5;
		u32 bus_pos_sel:1;
		u32 dec_data_disc_e:1;
		u32 axi_sel:1;
		u32 reserved0:8;
	} sw56;

	struct {
		u32 dec_e:1;
		u32 refbuf2_buf_e:1;
		u32 dec_out_dis:1;
		u32 reserved2:1;
		u32 dec_clk_gate_e:1;
		u32 dec_timeout_e:1;
		/* rd_cnt_tab_en */
		u32 picord_count_e:1;
		u32 seq_mbaff_e:1;
		u32 reftopfirst_e:1;
		u32 ref_topfield_e:1;
		u32 write_mvs_e:1;
		u32 sorenson_e:1;
		u32 fwd_interlace_e:1;
		u32 pic_topfield_e:1;
		/* sw_pic_type_sel0 */
		u32 pic_inter_e:1;
		u32 pic_b_e:1;
		u32 pic_fieldmode_e:1;
		u32 pic_interlace_e:1;
		u32 pjpeg_e:1;
		u32 divx3_e:1;
		u32 rlc_mode_e:1;
		u32 ch_8pix_ileav_e:1;
		u32 start_code_e:1;
		u32 reserved1:2;
		/* sw_init_dc_match0 ? */
		u32 inter_dblspeed:1;
		u32 intra_dblspeed:1;
		u32 intra_dbl3t:1;
		u32 pref_sigchan:1;
		u32 cache_en:1;
		u32 reserved0:1;
		/* dec_timeout_mode */
		u32 dec_timeout:1;
	} sw57;

	struct {
		u32 soft_rst:1;
		u32 reserved0:31;
	} sw58;

	struct {
		u32 reserved0:2;
		/* sw_pflt_set0_tap2 */
		u32 pred_bc_tap_0_2:10;
		u32 pred_bc_tap_0_1:10;
		/* pflt_set0_tap0 */
		u32 pred_bc_tap_0_0:10;
	} sw59;

	struct {
		u32 addit_ch_st_adr:32;
	} sw60;

	struct {
		u32 qtable_base:32;
	} sw61;

	struct {
		u32 dir_mv_base:32;
	} sw62;

	struct {
		/* dec_out_st_adr */
		u32 dec_out_base:32;
	} sw63;

	struct {
		u32 rlc_vlc_base:32;
	} sw64;

	struct {
		u32 refbuf_y_offset:9;
		u32 reserve0:3;
		u32 refbuf_fildpar_mode_e:1;
		u32 refbuf_idcal_e:1;
		u32 refbuf_picid:5;
		u32 refbuf_thr_level:12;
		u32 refbuf_e:1;
	} sw65;

	u32 sw66;
	u32 sw67;

	struct {
		u32 refbuf_sum_bot:16;
		u32 refbuf_sum_top:16;
	} sw68;

	struct {
		u32 luma_sum_intra:16;
		u32 refbuf_sum_hit:16;
	} sw69;

	struct {
		u32 ycomp_mv_sum:22;
		u32 reserve0:10;
	} sw70;

	u32 sw71;
	u32 sw72;
	u32 sw73;

	struct {
		u32 init_reflist_pf4:5;
		u32 init_reflist_pf5:5;
		u32 init_reflist_pf6:5;
		u32 init_reflist_pf7:5;
		u32 init_reflist_pf8:5;
		u32 init_reflist_pf9:5;
		u32 reserved0:2;
	} sw74;

	struct {
		u32 init_reflist_pf10:5;
		u32 init_reflist_pf11:5;
		u32 init_reflist_pf12:5;
		u32 init_reflist_pf13:5;
		u32 init_reflist_pf14:5;
		u32 init_reflist_pf15:5;
		u32 reserved0:2;
	} sw75;

	struct {
		u32 num_ref_idx0:16;
		u32 num_ref_idx1:16;
	} sw76;

	struct {
		u32 num_ref_idx2:16;
		u32 num_ref_idx3:16;
	} sw77;

	struct {
		u32 num_ref_idx4:16;
		u32 num_ref_idx5:16;
	} sw78;

	struct {
		u32 num_ref_idx6:16;
		u32 num_ref_idx7:16;
	} sw79;

	struct {
		u32 num_ref_idx8:16;
		u32 num_ref_idx9:16;
	} sw80;

	struct {
		u32 num_ref_idx10:16;
		u32 num_ref_idx11:16;
	} sw81;

	struct {
		u32 num_ref_idx12:16;
		u32 num_ref_idx13:16;
	} sw82;

	struct {
		u32 num_ref_idx14:16;
		u32 num_ref_idx15:16;
	} sw83;

	/* Used by H.264 */
	union {
		u32 ref0_st_addr;
		struct {
			u32 ref0_closer_sel:1;
			u32 ref0_field_en:1;
			u32 reserved0:30;
		};
	} sw84;

	union {
		u32 ref1_st_addr;
		struct {
			u32 ref1_closer_sel:1;
			u32 ref1_field_en:1;
			u32 reserved0:30;
		};
	} sw85;

	union {
		u32 ref2_st_addr;
		struct {
			u32 ref2_closer_sel:1;
			u32 ref2_field_en:1;
			u32 reserved0:30;
		};
	} sw86;

	union {
		u32 ref3_st_addr;
		struct {
			u32 ref3_closer_sel:1;
			u32 ref3_field_en:1;
			u32 reserved0:30;
		};
	} sw87;

	union {
		u32 ref4_st_addr;
		struct {
			u32 ref4_closer_sel:1;
			u32 ref4_field_en:1;
			u32 reserved0:30;
		};
	} sw88;

	union {
		u32 ref5_st_addr;
		struct {
			u32 ref5_closer_sel:1;
			u32 ref5_field_en:1;
			u32 reserved0:30;
		};
	} sw89;

	union {
		u32 ref6_st_addr;
		struct {
			u32 ref6_closer_sel:1;
			u32 ref6_field_en:1;
			u32 reserved0:30;
		};
	} sw90;

	union {
		u32 ref7_st_addr;
		struct {
			u32 ref7_closer_sel:1;
			u32 ref7_field_en:1;
			u32 reserved0:30;
		};
	} sw91;

	union {
		u32 ref8_st_addr;
		struct {
			u32 ref8_closer_sel:1;
			u32 ref8_field_en:1;
			u32 reserved0:30;
		};
	} sw92;

	union {
		u32 ref9_st_addr;
		struct {
			u32 ref9_closer_sel:1;
			u32 ref9_field_en:1;
			u32 reserved0:30;
		};
	} sw93;

	union {
		u32 ref10_st_addr;
		struct {
			u32 ref10_closer_sel:1;
			u32 ref10_field_en:1;
			u32 reserved0:30;
		};
	} sw94;

	union {
		u32 ref11_st_addr;
		struct {
			u32 ref11_closer_sel:1;
			u32 ref11_field_en:1;
			u32 reserved0:30;
		};
	} sw95;

	union {
		u32 ref12_st_addr;
		struct {
			u32 ref12_closer_sel:1;
			u32 ref12_field_en:1;
			u32 reserved0:30;
		};
	} sw96;

	union {
		u32 ref13_st_addr;
		struct {
			u32 ref13_closer_sel:1;
			u32 ref13_field_en:1;
			u32 reserved0:30;
		};
	} sw97;

	union {
		u32 ref14_st_addr;
		struct {
			u32 ref14_closer_sel:1;
			u32 ref14_field_en:1;
			u32 reserved0:30;
		};
	} sw98;

	/* Used by H.264 */
	union {
		u32 ref15_st_addr;
		struct {
			u32 ref15_closer_sel:1;
			u32 ref15_field_en:1;
			u32 reserved0:30;
		};
	} sw99;

	struct {
		u32 init_reflist_df0:5;
		u32 init_reflist_df1:5;
		u32 init_reflist_df2:5;
		u32 init_reflist_df3:5;
		u32 init_reflist_df4:5;
		u32 init_reflist_df5:5;
		u32 reserved0:2;
	} sw100;

	struct {
		u32 init_reflist_df6:5;
		u32 init_reflist_df7:5;
		u32 init_reflist_df8:5;
		u32 init_reflist_df9:5;
		u32 init_reflist_df10:5;
		u32 init_reflist_df11:5;
		u32 reserved0:2;
	} sw101;

	struct {
		u32 init_reflist_df12:5;
		u32 init_reflist_df13:5;
		u32 init_reflist_df14:5;
		u32 init_reflist_df15:5;
		u32 reserved0:12;
	} sw102;

	struct {
		u32 init_reflist_db0:5;
		u32 init_reflist_db1:5;
		u32 init_reflist_db2:5;
		u32 init_reflist_db3:5;
		u32 init_reflist_db4:5;
		u32 init_reflist_db5:5;
		u32 reserved0:2;
	} sw103;

	struct {
		u32 init_reflist_db6:5;
		u32 init_reflist_db7:5;
		u32 init_reflist_db8:5;
		u32 init_reflist_db9:5;
		u32 init_reflist_db10:5;
		u32 init_reflist_db11:5;
		u32 reserved0:2;
	} sw104;

	struct {
		u32 init_reflist_db12:5;
		u32 init_reflist_db13:5;
		u32 init_reflist_db14:5;
		u32 init_reflist_db15:5;
		u32 reserved0:12;
	} sw105;

	struct {
		u32 init_reflist_pf0:5;
		u32 init_reflist_pf1:5;
		u32 init_reflist_pf2:5;
		u32 init_reflist_pf3:5;
		u32 reserved0:12;
	} sw106;

	struct {
		u32 refpic_term_flag:32;
	} sw107;

	struct {
		u32 refpic_valid_flag:32;
	} sw108;

	struct {
		u32 strm_start_bit:6;
		u32 reserved0:26;
	} sw109;

	struct {
		u32 pic_mb_w:9;
		u32 pic_mb_h:8;
		u32 flt_offset_cb_qp:5;
		u32 flt_offset_cr_qp:5;
		u32 reserved0:5;
	} sw110;

	struct {
		u32 max_refnum:5;
		u32 reserved0:11;
		u32 wp_bslice_sel:2;
		u32 reserved1:14;
	} sw111;

	struct {
		u32 curfrm_num:16;
		u32 cur_frm_len:5;
		u32 reserved0:9;
		u32 rpcp_flag:1;
		u32 dblk_ctrl_flag:1;
	} sw112;

	struct {
		u32 idr_pic_id:16;
		u32 refpic_mk_len:11;
		u32 reserved0:5;
	} sw113;

	struct {
		u32 poc_field_len:8;
		u32 reserved0:6;
		u32 max_refidx0:5;
		u32 max_refidx1:5;
		u32 pps_id:5;
	} sw114;

	struct {
		u32 fieldpic_flag_exist:1;
		u32 scl_matrix_en:1;
		u32 tranf_8x8_flag_en:1;
		u32 const_intra_en:1;
		u32 weight_pred_en:1;
		u32 cabac_en:1;
		u32 monochr_en:1;
		u32 dlmv_method_en:1;
		u32 idr_pic_flag:1;
		u32 reserved0:23;
	} sw115;

	/* Not used */
	u32 sw116_119[4];

	union {
		struct {
			u32 pic_refer_flag:1;
			u32 reserved0:10;
		} avs;

		struct {
			u32 pic_mb_w_ext:3;
			u32 pic_mb_h_ext:3;
			u32 reserved0:1;
			u32 mb_height_off:4;
		} vc1;

		struct {
			u32 ref_frames:5;
			u32 reserved0:6;
		} h264;

		struct {
			u32 ref_frames:5;
			u32 topfieldfirst_e:1;
			u32 alt_scan_e:1;
			u32 reserved0:4;
		} mpeg;

		struct {
			u32 pad:11;
			u32 pic_mb_height_p:8;
			/* this register is only used by VC-1 */
			u32 mb_width_off:4;
			u32 pic_mb_width:9;
		};
	} sw120 __attribute__ ((packed));

	u32 sw121;

	struct {
		u32 frame_pred_dct:1;
		u32 intra_vlc_tab:1;
		u32 intra_dc_prec:1;
		u32 con_mv_e:1;
		u32 reserved0:19;
		u32 qscale_type:1;
		u32 reserved1:1;
		u32 strm_start_bit:6;
	} sw122;

	u32 sw123;
	u32 sw124;
	u32 sw125;
	u32 sw126;
	u32 sw127;
	u32 sw128;
	u32 sw129;
	u32 sw130;

	struct {
		u32 refer0_topc_e:1;
		u32 refer0_field_e:1;
		u32 refer0_base:30;
	} sw131;

	u32 sw132;
	u32 sw133;

	struct {
		u32 refer2_topc_e:1;
		u32 refer2_field_e:1;
		u32 refer2_base:30;
	} sw134;

	struct {
		u32 refer3_topc_e:1;
		u32 refer3_field_e:1;
		u32 refer3_base:30;
	} sw135;

	struct {
		u32 reserved0:1;
		u32 mv_accuracy_bwd:1;
		u32 mv_accuracy_fwd:1;
		u32 fcode_bwd_ver:4;
		u32 fcode_bwd_hor:4;
		u32 fcode_fwd_ver:4;
		u32 fcode_fwd_hor:4;
		u32 alt_scan_flag_e:1;
		u32 reserved1:12;
	} sw136;

	u32 sw137;
	u32 sw138;
	u32 sw139;
	u32 sw140;
	u32 sw141;
	u32 sw142;
	u32 sw143;
	u32 sw144;
	u32 sw145;
	u32 sw146;
	u32 sw147;

	struct {
		u32 refer1_topc_e:1;
		u32 refer1_field_e:1;
		u32 refer1_base:30;
	} sw148;

	u32 sw149_sw158[10];
} __attribute__((packed));

#endif
