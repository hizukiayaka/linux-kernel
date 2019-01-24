// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2019 Randy Li <ayaka@soulik.info>
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

#ifndef _RKVDEC_REGS_H_
#define _RKVDEC_REGS_H_

struct vp9_segid_grp {
	/* this bit is only used by segid0 */
	u32 abs_delta:1;
	u32 frame_qp_delta_en:1;
	u32 frame_qp_delta:9;
	u32 frame_loopfitler_value_en:1;
	u32 frame_loopfilter_value:7;
	u32 referinfo_en:1;
	u32 referinfo:2;
	u32 frame_skip_en:1;
	u32 reserved0:9;
};

struct rkvdec_regs {
	struct {
		u32 minor_ver:8;
		u32 level:1;
		u32 dec_support:3;
		u32 profile:1;
		u32 reserved0:1;
		u32 codec_flag:1;
		u32 reserved1:1;
		u32 prod_num:16;
	} sw_id;

	struct {
		u32 dec_e:1;
		u32 dec_clkgate_e:1;
		u32 reserved0:1;
		u32 timeout_mode:1;
		u32 dec_irq_dis:1;
		u32 dec_timeout_e:1;
		u32 buf_empty_en:1;
		u32 stmerror_waitdecfifo_empty:1;
		u32 dec_irq:1;
		u32 dec_irq_raw:1;
		u32 reserved1:2;
		u32 dec_rdy_sta:1;
		u32 dec_bus_sta:1;
		u32 dec_error_sta:1;
		u32 dec_empty_sta:1;
		u32 colmv_ref_error_sta:1;
		u32 cabu_end_sta:1;
		u32 h264orvp9_error_mode:1;
		u32 softrst_en_p:1;
		u32 force_softreset_valid:1;
		u32 softreset_rdy:1;
		u32 wr_ddr_align_en:1;
		u32 scl_down_en:1;
		u32 allow_not_wr_unref_bframe:1;
		u32 reserved2:6;
	} sw_interrupt;

	struct {
		u32 in_endian:1;
		u32 in_swap32_e:1;
		u32 in_swap64_e:1;
		u32 str_endian:1;
		u32 str_swap32_e:1;
		u32 str_swap64_e:1;
		u32 out_endian:1;
		u32 out_swap32_e:1;
		u32 out_cbcr_swap:1;
		u32 reserved0:1;
		u32 rlc_mode_direct_write:1;
		u32 rlc_mode:1;
		u32 strm_start_bit:7;
		u32 reserved1:1;
		u32 dec_mode:2;
		u32 reserved2:2;
		u32 rps_mode:1;
		u32 stream_mode:1;
		u32 stream_lastpacket:1;
		u32 firstslice_flag:1;
		u32 frame_orslice:1;
		u32 buspr_slot_disable:1;
		u32 colmv_mode:1;
		u32 ycacherd_prior:1;
	} sw_sysctrl;

	struct {
		u32 y_hor_virstride:10;
		u32 uv_hor_virstride_highbit:1;
		u32 slice_num_highbit:1;
		u32 uv_hor_virstride:9;
		u32 slice_num_lowbits:11;
	} sw_picparameter;

	u32 sw_strm_rlc_base;
	u32 sw_stream_len;
	u32 sw_cabactbl_base;
	u32 sw_decout_base;
	u32 sw_y_virstride;
	u32 sw_yuv_virstride;
	/* SWREG 10 */
	union {
		struct {
			union {
				struct {
					u32 ref_valid_flag:4;
					u32 ref_base:28;
				};
				struct {
					u32 ref_field:1;
					u32 ref_topfield_used:1;
					u32 ref_botfield_used:1;
					u32 ref_colmv_use_flag:1;
					u32 padding:28;
				};
			} sw_refer_base[15];
			u32 sw_refer_poc[15];
		};

		struct {
			struct {
				u32 vp9_cprheader_offset:16;
				u32 reserved0:16;
			};
			struct {
				u32 reserved1:4;
				u32 vp9last_base:28;
			};
			struct {
				u32 reserved2:4;
				u32 vp9golden_base:28;
			};
			struct {
				u32 reserved3:4;
				u32 vp9alfter_base:28;
			};
			struct {
				u32 vp9count_update_en:1;
				u32 reserved4:2;
				u32 vp9count_base:29;
			};
			struct {
				u32 reserved5:4;
				u32 vp9segidlast_base:28;
			};
			struct {
				u32 reserved6:4;
				u32 vp9segidcur_base:28;
			};
			struct {
				u32 framewidth_last:16;
				u32 frameheight_last:16;
			};
			struct {
				u32 framewidth_golden:16;
				u32 frameheight_golden:16;
			};
			struct {
				u32 framewidth_alfter:16;
				u32 frameheight_alfter:16;
			};
			/* SWREG 20 (segid_grp0) to SWREG 27 (segid_grp7) */
			struct vp9_segid_grp grp[8];
			/* cprheader_config */
			struct {
				u32 tx_mode:3;
				u32 frame_reference_mode:2;
				u32 reserved7:27;
			};
			/* lref_scale */
			struct {
				u32 lref_hor_scale:16;
				u32 lref_ver_scale:16;
			};
			/* gref_scale */
			struct {
				u32 gref_hor_scale:16;
				u32 gref_ver_scale:16;
			};
			/* aref_scale */
			struct {
				u32 aref_hor_scale:16;
				u32 aref_ver_scale:16;
			};
			/* ref_deltas_lastframe */
			struct {
				u32 ref_deltas_lastframe:28;
				u32 reserved8:4;
			};
			/* info_lastframe */
			struct {
				u32 mode_deltas_lastframe:14;
				u32 reserved9:2;
				u32 segmentation_enable_lastframe:1;
				u32 last_show_frame:1;
				u32 last_intra_only:1;
				u32 last_widthheight_eqcur:1;
				u32 color_space_lastkeyframe:3;
				u32 reserved10:9;
			};
			/* intercmd_base */
			struct {
				u32 reserved11:4;
				u32 intercmd_base:28;
			};
			/* intercmd_num */
			struct {
				u32 intercmd_num:24;
				u32 reserved12:8;
			};
			/* lasttile_size */
			struct {
				u32 lasttile_size:24;
				u32 reserved13:8;
			};
			/* lastf_hor_virstride */
			struct {
				u32 lastfy_hor_virstride:10;
				u32 reserved14:6;
				u32 lastfuv_hor_virstride:10;
				u32 reserved15:6;
			};
			/* goldenf_hor_virstride */
			struct {
				u32 goldenfy_hor_virstride:10;
				u32 reserved16:6;
				u32 goldenuv_hor_virstride:10;
				u32 reserved17:6;
			};
			/* altreff_hor_virstride */
			struct {
				u32 altrey_hor_virstride:10;
				u32 reserved18:6;
				u32 altreuv_hor_virstride:10;
				u32 reserved19:6;
			};
		} vp9;
	};

	/* SWREG 40 */
	u32 sw_cur_poc;
	u32 sw_rlcwrite_base;
	u32 sw_pps_base;
	u32 sw_rps_base;
	/* in HEVC, it is called cabac_error_en */
	u32 sw_strmd_error_en;
	/* SWREG 45, cabac_error_status, vp9_error_info0 */
	struct {
		u32 strmd_error_status:28;
		u32 colmv_error_ref_picidx:4;
	} sw_strmd_error_status;

	struct cabac_error_ctu {
		u32 strmd_error_ctu_xoffset:8;
		u32 strmd_error_ctu_yoffset:8;
		u32 streamfifo_space2full:7;
		u32 reserved0:1;
		u32 vp9_error_ctu0_en:1;
		u32 reversed1:7;
	} cabac_error_ctu;

	/* SWREG 47 */
	struct sao_ctu_position {
		u32 sw_saowr_xoffset:9;
		u32 reversed0:7;
		u32 sw_saowr_yoffset:10;
		u32 reversed1:6;
	} sao_ctu_position;

	/* SWREG 48 */
	union {
		u32 sw_ref_valid;
		struct {
			u32 ref15_base:10;
			u32 ref15_field:1;
			u32 ref15_topfield_used:1;
			u32 ref15_botfield_used:1;
			u32 ref15_colmv_use_flag:1;
			u32 reverse0:18;
		} sw48_h264;

		struct {
			u32 lastfy_virstride:20;
			u32 reserved0:12;
		} sw48_vp9;
	};

	/* SWREG 49 - 63 */
	union {
		u32 sw_refframe_index[15];
		struct {
			struct {
				u32 goldeny_virstride:20;
				u32 reserved0:12;
			};
			struct {
				u32 altrefy_virstride:20;
				u32 reserved1:12;
			};
			struct {
				u32 lastref_yuv_virstride:20;
				u32 reserved2:12;
			};
			struct {
				u32 reserved3:4;
				u32 refcolmv_base:28;
			};
			u32 padding[11];
		} vp9_vir;

	};

	/* SWREG 64 */
	u32 performance_cycle;
	u32 axi_ddr_rdata;
	u32 axi_ddr_wdata;
	/* SWREG 67 */
	u32 fpgadebug_reset;

	struct {
		u32 perf_cnt0_sel:6;
		u32 reserved2:2;
		u32 perf_cnt1_sel:6;
		u32 reserved1:2;
		u32 perf_cnt2_sel:6;
		u32 reserved0:10;
	} sw68_perf_sel;

	u32 sw69_perf_cnt0;
	u32 sw70_perf_cnt1;
	u32 sw71_perf_cnt2;
	u32 sw72_h264_refer30_poc;
	u32 sw73_h264_refer31_poc;
	u32 sw74_h264_cur_poc1;
	u32 sw75_errorinfo_base;

	union {
		struct {
			u32 slicedec_num:14;
			u32 reserved1:1;
			u32 strmd_detect_error_flag:1;
			u32 sw_error_packet_num:14;
			u32 reserved0:2;
		} sw76_errorinfo_num;

		struct {
			u32 error_ctu1_x:6;
			u32 reserved0:2;
			u32 vp9_error_ctu1_y:6;
			u32 reserved1:1;
			u32 vp9_error_ctu1_en:1;
			u32 reserved2:16;
		} swreg76_vp9_error_ctu1;
	};

	/* SWREG 77 */
	struct {
		u32 error_en_highbits:28;
		u32 strmd_error_en:2;
		u32 reserved0:2;
	} extern_error_en;
};

#endif
