/*
 * Rockchip RK3288 VPU codec driver
 *
 * Copyright (C) 2016 Rockchip Electronics Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/random.h>
#include <linux/sort.h>
#include <linux/types.h>

#include "rk3288_vpu_regs.h"
#include "rockchip_vpu_common.h"
#include "rockchip_vpu_hw.h"

#define IS_BUFFER_ALIGNED(a)    (0 == ((a) & 7))

int rk3288_vpu_enc_jpeg_init(struct rockchip_vpu_ctx *ctx)
{
	return 0;
}

void rk3288_vpu_enc_jpeg_exit(struct rockchip_vpu_ctx *ctx)
{
	return;
}

static inline u32 enc_in_img_ctrl(struct rockchip_vpu_ctx *ctx)
{
	struct v4l2_pix_format_mplane *pix_fmt = &ctx->src_fmt;
	struct v4l2_rect *crop = &ctx->src_crop;
	unsigned bytes_per_line, overfill_r, overfill_b;

	bytes_per_line = pix_fmt->width;
	overfill_r = (pix_fmt->width - crop->width) / 4;
	overfill_b = pix_fmt->height - crop->height;

	return VEPU_REG_IN_IMG_CTRL_ROW_LEN(bytes_per_line)
		| VEPU_REG_IN_IMG_CTRL_OVRFLR_D4(overfill_r)
		| VEPU_REG_IN_IMG_CTRL_OVRFLB_D4(overfill_b)
		| VEPU_REG_IN_IMG_CTRL_FMT(ctx->vpu_src_fmt->enc_fmt);
}

static void rk3288_vpu_enc_jpeg_set_buffers(struct rockchip_vpu_dev *vpu,
					struct rockchip_vpu_ctx *ctx)
{
	struct v4l2_pix_format_mplane *pix_fmt = &ctx->src_fmt;
	const uint32_t header_bytes = ctx->run.enc_jpeg.header_bytes;

	dma_addr_t dst_dma, dst_align_dma;
	size_t dst_size;

	vpu_debug_enter();

	dst_dma = vb2_dma_contig_plane_dma_addr(&ctx->run.dst->vb.vb2_buf, 0)
		+ header_bytes;
	dst_size = vb2_plane_size(&ctx->run.dst->vb.vb2_buf, 0) - header_bytes;

	/* Destination buffer. */
	if (!IS_BUFFER_ALIGNED(dst_dma)) {
		/*
		 * NOTE: aligment down! That is what that register is
		 * used for.
		 */
		dst_align_dma = round_down(dst_dma, 8);
		vepu_write_relaxed(vpu, ((dst_dma - dst_align_dma) * 8)
				<< VEPU_REG_RLC_CTRL_STR_OFFS_SHIFT,
				VEPU_REG_RLC_CTRL);
		dst_dma = dst_align_dma;
	}

	vepu_write_relaxed(vpu, dst_dma, VEPU_REG_ADDR_OUTPUT_STREAM);
	vepu_write_relaxed(vpu, dst_size, VEPU_REG_STR_BUF_LIMIT);

	/* Source buffer */
	/* FIXME not work for NV21 */
	if (1 == pix_fmt->num_planes) {
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 0),
					VEPU_REG_ADDR_IN_LUMA);
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 0)
				+ ctx->vpu_src_fmt->chroma_offset,
					VEPU_REG_ADDR_IN_CB);
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 0)
				+ ctx->vpu_src_fmt->chroma_offset,
					VEPU_REG_ADDR_IN_CR);
	}
	if (2 == pix_fmt->num_planes) {
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 0),
					VEPU_REG_ADDR_IN_LUMA);
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 1),
					VEPU_REG_ADDR_IN_CB);
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 1),
					VEPU_REG_ADDR_IN_CR);
	}
	else if (3 == pix_fmt->num_planes) {
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 0),
					VEPU_REG_ADDR_IN_LUMA);
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 1),
					VEPU_REG_ADDR_IN_CB);
		vepu_write_relaxed(vpu, vb2_dma_contig_plane_dma_addr(
					&ctx->run.src->vb.vb2_buf, 2),
					VEPU_REG_ADDR_IN_CR);
	}

	vpu_debug_leave();
}

static void rk3288_vpu_enc_jpeg_set_params(struct rockchip_vpu_dev *vpu,
				       struct rockchip_vpu_ctx *ctx)
{
	const struct v4l2_ctrl_jpeg_qmatrix *qmatrix =
		ctx->run.enc_jpeg.qmatrix;
	uint32_t reg;
	uint32_t i;

	/* Source input image parameters. */
	vepu_write_relaxed(vpu, enc_in_img_ctrl(ctx), VEPU_REG_IN_IMG_CTRL);

	/* FIXME only work for NV12 */
	reg = VEPU_REG_ENC_CTRL4_JPEG_MODE(0);
	vepu_write_relaxed(vpu, reg, VEPU_REG_ENC_CTRL4);

	for (i = 0; i < 16; i++) {
		uint32_t j = i * 4;

		reg = (qmatrix->lum_quantiser_matrix[j] << 24)
			| (qmatrix->lum_quantiser_matrix[j + 1] << 16)
			| (qmatrix->lum_quantiser_matrix[j + 2] << 8)
			| (qmatrix->lum_quantiser_matrix[j + 3] << 0);

		vepu_write_relaxed(vpu, reg, 
				VEPU_REG_JPEG_LUMA_QUAT_BASE + (j));

		reg = (qmatrix->chroma_quantiser_matrix[j] << 24)
			| (qmatrix->chroma_quantiser_matrix[j + 1] << 16)
			| (qmatrix->chroma_quantiser_matrix[j + 2] << 8)
			| (qmatrix->chroma_quantiser_matrix[j + 3] << 0);

		vepu_write_relaxed(vpu, reg, 
				VEPU_REG_JPEG_CHROMA_QUAT_BASE + (j));
	}
		
}

#if 0
static void rk3288_fill_enc_register(struct rockchip_vpu_dev *vpu)
{
	uint32_t i, j;

	for (i = 0; i < 164; i++) {
		j = i * 4;
		switch (j) {
		case VEPU_REG_INTERRUPT:
		case VEPU_REG_AXI_CTRL:
		case VEPU_REG_ADDR_OUTPUT_STREAM:
		case VEPU_REG_ADDR_OUTPUT_CTRL:
		case VEPU_REG_ADDR_IN_LUMA:
		case VEPU_REG_ADDR_IN_CB:
		case VEPU_REG_ADDR_IN_CR:
		case VEPU_REG_ENC_CTRL:
		case VEPU_REG_IN_IMG_CTRL:
		case VEPU_REG_ENC_CTRL4:
		case VEPU_REG_STR_BUF_LIMIT:
		case VEPU_REG_JPEG_LUMA_QUAT_BASE ... (VEPU_REG_JPEG_CHROMA_QUAT_BASE + 4 * 15):
			continue;
			break;
		default:
			vepu_write(vpu, 0x0FFFFFFF & (i << 16), j);
			break;
		}
	}
	wmb();
}
#endif

void rk3288_vpu_enc_jpeg_run(struct rockchip_vpu_ctx *ctx)
{
	struct rockchip_vpu_dev *vpu = ctx->dev;
	u32 reg;

	/*
	 * Program the hardware.
	 */
	rockchip_vpu_power_on(vpu);

	vepu_write_relaxed(vpu, VEPU_REG_ENC_CTRL_ENC_MODE_JPEG,
			VEPU_REG_ENC_CTRL);

	rk3288_vpu_enc_jpeg_set_params(vpu, ctx);
	rk3288_vpu_enc_jpeg_set_buffers(vpu, ctx);

	/* Make sure that all registers are written at this point. */
	wmb();

	/* Set the watchdog. */
	schedule_delayed_work(&vpu->watchdog_work, msecs_to_jiffies(2000));

	/* Start the hardware. */
	reg = VEPU_REG_AXI_CTRL_OUTPUT_SWAP16
		| VEPU_REG_AXI_CTRL_INPUT_SWAP16
		| VEPU_REG_AXI_CTRL_BURST_LEN(16)
		| VEPU_REG_AXI_CTRL_GATE_BIT
		/* YUV need set all swap flags */
		| VEPU_REG_AXI_CTRL_OUTPUT_SWAP32
		| VEPU_REG_AXI_CTRL_INPUT_SWAP32
		| VEPU_REG_AXI_CTRL_OUTPUT_SWAP8
		| VEPU_REG_AXI_CTRL_INPUT_SWAP8;
	vepu_write(vpu, reg, VEPU_REG_AXI_CTRL);

	vepu_write(vpu, 0, VEPU_REG_INTERRUPT);

	/* FIXME check whether all the request bits is set */
	reg = VEPU_REG_ENC_CTRL_WIDTH(MB_WIDTH(ctx->src_fmt.width))
		| VEPU_REG_ENC_CTRL_HEIGHT(MB_HEIGHT(ctx->src_fmt.height))
		| VEPU_REG_PIC_TYPE(1)
		| VEPU_REG_ENC_CTRL_ENC_MODE_JPEG
		| VEPU_REG_ENC_CTRL_EN_BIT;

	vepu_write(vpu, reg, VEPU_REG_ENC_CTRL);
}

void rk3288_vpu_enc_jpeg_done(struct rockchip_vpu_ctx *ctx,
			  enum vb2_buffer_state result)
{
	struct rockchip_vpu_dev *vpu = ctx->dev;

	ctx->run.dst->enc_jpeg.encoded_size =
		vepu_read(vpu, VEPU_REG_STR_BUF_LIMIT) / 8;

	rockchip_vpu_run_done(ctx, result);
}
