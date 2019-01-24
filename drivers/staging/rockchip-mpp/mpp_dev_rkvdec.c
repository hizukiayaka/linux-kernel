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

#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <soc/rockchip/pm_domains.h>
#include <soc/rockchip/rockchip_sip.h>

#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mem2mem.h>

#include <linux/pm_runtime.h>

#include "mpp_debug.h"
#include "mpp_dev_common.h"
#include "rkvdec/hal.h"

#define RKVDEC_DRIVER_NAME		"mpp_rkvdec"

#define RKVDEC_VER_RK3328_BIT		BIT(1)
#define IOMMU_GET_BUS_ID(x)		(((x) >> 6) & 0x1f)
#define IOMMU_PAGE_SIZE			SZ_4K

#define RKVDEC_NODE_NAME		"rkvdec"
#define RK_HEVCDEC_NODE_NAME		"hevc-service"

#define to_rkvdec_task(ctx)		\
		container_of(ctx, struct rkvdec_task, mpp_task)
#define to_rkvdec_dev(dev)		\
		container_of(dev, struct rockchip_rkvdec_dev, mpp_dev)

static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "bit switch for rkvdec debug information");

enum RKVDEC_STATE {
	RKVDEC_STATE_NORMAL,
	RKVDEC_STATE_LT_START,
	RKVDEC_STATE_LT_RUN,
};

struct rockchip_rkvdec_dev {
	struct rockchip_mpp_dev mpp_dev;

	struct reset_control *rst_a;
	struct reset_control *rst_h;
	struct reset_control *rst_niu_a;
	struct reset_control *rst_niu_h;
	struct reset_control *rst_core;
	struct reset_control *rst_cabac;

	enum RKVDEC_STATE state;

	unsigned long aux_iova;
	struct page *aux_page;

	void *current_task;
};

struct rkvdec_task {
	struct mpp_task mpp_task;

	u32 reg[ROCKCHIP_RKVDEC_REG_NUM];
	u32 idx;

	u32 irq_status;
};

static struct rockchip_mpp_control hevc_controls[] = {
	{
	 .codec = V4L2_PIX_FMT_HEVC_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_HEVC_SPS,
	 .elem_size = sizeof(struct v4l2_ctrl_hevc_sps),
	},
	{
	 .codec = V4L2_PIX_FMT_HEVC_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_HEVC_PPS,
	 .elem_size = sizeof(struct v4l2_ctrl_hevc_pps),
	},
	{
	 .codec = V4L2_PIX_FMT_HEVC_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_HEVC_SLICE_PARAMS,
	 .elem_size = sizeof(struct v4l2_ctrl_hevc_slice_params),
	},
};

static struct rockchip_mpp_control rkvdec_controls[] = {
	{
	 .codec = V4L2_PIX_FMT_HEVC_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_HEVC_SPS,
	 .elem_size = sizeof(struct v4l2_ctrl_hevc_sps),
	},
	{
	 .codec = V4L2_PIX_FMT_HEVC_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_HEVC_PPS,
	 .elem_size = sizeof(struct v4l2_ctrl_hevc_pps),
	},
	{
	 .codec = V4L2_PIX_FMT_HEVC_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_HEVC_SLICE_PARAMS,
	 .elem_size = sizeof(struct v4l2_ctrl_hevc_slice_params),
	},

};

static const struct v4l2_pix_format_mplane fmt_out_templ[] = {
	{
	 .pixelformat = V4L2_PIX_FMT_H264_SLICE,
	 },
	{
	 .pixelformat = V4L2_PIX_FMT_HEVC_SLICE,
	 },
};

static const struct v4l2_pix_format_mplane fmt_cap_templ[] = {
	{
	 .pixelformat = V4L2_PIX_FMT_NV12M,
	 },
	{
	 .pixelformat = V4L2_PIX_FMT_NV16M,
	 },
};

static const struct mpp_dev_variant rkvdec_v1_data = {
	.reg_len = 76,
	.node_name = RKVDEC_NODE_NAME,
	.version_bit = BIT(0),
};

static const struct mpp_dev_variant rkvdec_v1p_data = {
	.reg_len = 76,
	.node_name = RKVDEC_NODE_NAME,
	.version_bit = RKVDEC_VER_RK3328_BIT,
};

static const struct mpp_dev_variant rk_hevcdec_data = {
	.reg_len = 48,
	.node_name = RK_HEVCDEC_NODE_NAME,
	.version_bit = BIT(0),
};

static int rkvdec_open(struct file *filp);

static const struct v4l2_file_operations rkvdec_fops = {
	.open = rkvdec_open,
	.release = rockchip_mpp_dev_release,
	.poll = v4l2_m2m_fop_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap = v4l2_m2m_fop_mmap,
};

static struct v4l2_ioctl_ops rkvdec_ioctl_ops = { 0, };

static void *rockchip_rkvdec_get_drv_data(struct platform_device *pdev);

static int rkvdec_s_fmt_vid_out_mplane(struct file *filp, void *priv,
				       struct v4l2_format *f)
{
	struct mpp_session *session = container_of(filp->private_data,
						   struct mpp_session, fh);
	struct v4l2_pix_format_mplane *pix_mp = &f->fmt.pix_mp;
	struct vb2_queue *vq;
	int sizes = 0;
	int i;

	/* TODO: We can change width and height at streaming on */
	vq = v4l2_m2m_get_vq(session->fh.m2m_ctx, f->type);
	if (vb2_is_streaming(vq))
		return -EBUSY;

	if (!pix_mp->num_planes)
		pix_mp->num_planes = 1;

	for (i = 0; i < pix_mp->num_planes; i++) {
		sizes += pix_mp->plane_fmt[i].sizeimage;
	}
	/* strm_len is 24 bits */
	if (sizes >= SZ_16M - SZ_1K)
		return -EINVAL;

	/* FIXME: For those slice header data, put it in a better place */
	pix_mp->plane_fmt[pix_mp->num_planes - 1].sizeimage += SZ_4M;

	session->fmt_out = *pix_mp;

	/* Copy the pixel format information from OUTPUT to CAPUTRE */
	session->fmt_cap.pixelformat = V4L2_PIX_FMT_NV12M;
	session->fmt_cap.width = pix_mp->width;
	session->fmt_cap.height = pix_mp->height;
	session->fmt_cap.colorspace = pix_mp->colorspace;
	session->fmt_cap.ycbcr_enc = pix_mp->ycbcr_enc;
	session->fmt_cap.xfer_func = pix_mp->xfer_func;
	session->fmt_cap.quantization = pix_mp->quantization;

	return 0;
}

static int rkvdec_s_fmt_vid_cap_mplane(struct file *filp, void *priv,
				       struct v4l2_format *f)
{
	struct mpp_session *session = container_of(filp->private_data,
						   struct mpp_session, fh);
	struct v4l2_pix_format_mplane *pix_mp = &f->fmt.pix_mp;
	struct vb2_queue *vq;

	vq = v4l2_m2m_get_vq(session->fh.m2m_ctx, f->type);
	if (vb2_is_streaming(vq))
		return -EBUSY;

	switch (pix_mp->pixelformat) {
	case V4L2_PIX_FMT_NV12M:
		/* TODO: adaptive based by cache settings */
		pix_mp->plane_fmt[0].bytesperline =
		    ALIGN(pix_mp->width, 256) | 256;
		pix_mp->plane_fmt[1].bytesperline =
		    ALIGN(pix_mp->width, 256) | 256;
#if 0
		/* TODO: align with 16 for H.264 */
		pix_mp->plane_fmt[0].sizeimage =
		    pix_mp->plane_fmt[0].bytesperline * ALIGN(pix_mp->height,
							      8);
		/* Additional space for motion vector */
		pix_mp->plane_fmt[1].sizeimage =
		    pix_mp->plane_fmt[1].bytesperline * ALIGN(pix_mp->height,
							      8);
#else
		pix_mp->plane_fmt[0].sizeimage =
		    pix_mp->plane_fmt[0].bytesperline * ALIGN(pix_mp->height,
							      8);
		/* Additional space for motion vector */
		pix_mp->plane_fmt[0].sizeimage *= 2;
#endif
		break;
	case V4L2_PIX_FMT_NV16M:
		pix_mp->plane_fmt[0].bytesperline =
		    ALIGN(pix_mp->width, 256) | 256;
		pix_mp->plane_fmt[1].bytesperline =
		    ALIGN(pix_mp->width, 256) | 256;
		pix_mp->plane_fmt[0].sizeimage =
		    pix_mp->plane_fmt[0].bytesperline * ALIGN(pix_mp->height,
							      8);
		/* Additional space for motion vector */
		pix_mp->plane_fmt[1].sizeimage =
		    pix_mp->plane_fmt[1].bytesperline * ALIGN(pix_mp->height,
							      8) * 3 / 2;
		break;
	default:
		return -EINVAL;
	}

	session->fmt_cap = *pix_mp;

	return 0;
}

static int rkvdec_setup_ctrls(struct rockchip_mpp_dev *mpp_dev,
			      struct mpp_session *session)
{
	struct v4l2_ctrl_handler *hdl = &session->ctrl_handler;
	struct v4l2_ctrl *ctrl;
	unsigned int num_ctrls = ARRAY_SIZE(rkvdec_controls);
	unsigned int i;

	v4l2_ctrl_handler_init(hdl, num_ctrls);
	if (hdl->error) {
		v4l2_err(&mpp_dev->v4l2_dev,
			 "Failed to initialize control handler\n");
		return hdl->error;
	}

	for (i = 0; i < num_ctrls; i++) {
		struct v4l2_ctrl_config cfg = { };

		cfg.id = rkvdec_controls[i].id;
		cfg.elem_size = rkvdec_controls[i].elem_size;

		ctrl = v4l2_ctrl_new_custom(hdl, &cfg, NULL);
		if (hdl->error) {
			v4l2_err(&mpp_dev->v4l2_dev,
				 "Failed to create new custom %d control\n",
				 cfg.id);
			goto fail;
		}
	}

	session->fh.ctrl_handler = hdl;
	v4l2_ctrl_handler_setup(hdl);

	return 0;
fail:
	v4l2_ctrl_handler_free(hdl);
	return hdl->error;
}

static int rkvdec_open(struct file *filp)
{
	struct rockchip_mpp_dev *mpp_dev = video_drvdata(filp);
	struct video_device *vdev = video_devdata(filp);
	struct mpp_session *session = NULL;
	/* TODO: install ctrl based on register report */
	int error = 0;

	mpp_debug_enter();

	session = rockchip_mpp_alloc_session(mpp_dev, vdev);
	if (IS_ERR_OR_NULL(session))
		return PTR_ERR(session);

	error = rkvdec_setup_ctrls(mpp_dev, session);
	if (error) {
		kfree(session);
		return error;
	}

	filp->private_data = &session->fh;
	pm_runtime_get_sync(mpp_dev->dev);

	mpp_debug_leave();
	return 0;
}

#if 0
/*
 * NOTE: rkvdec/rkhevc put scaling list address in pps buffer hardware will read
 * it by pps id in video stream data.
 *
 * So we need to translate the address in iommu case. The address data is also
 * 10bit fd + 22bit offset mode.
 * Because userspace decoder do not give the pps id in the register file sets
 * kernel driver need to translate each scaling list address in pps buffer which
 * means 256 pps for H.264, 64 pps for H.265.
 *
 * In order to optimize the performance kernel driver ask userspace decoder to
 * set all scaling list address in pps buffer to the same one which will be used
 * on current decoding task. Then kernel driver can only translate the first
 * address then copy it all pps buffer.
 */
static int fill_scaling_list_pps(struct rkvdec_task *task, int fd, int offset,
				 int count, int pps_info_size,
				 int sub_addr_offset)
{
	struct device *dev = NULL;
	struct dma_buf *dmabuf = NULL;
	void *vaddr = NULL;
	u8 *pps = NULL;
	u32 base = sub_addr_offset;
	u32 scaling_fd = 0;
	u32 scaling_offset;
	int ret = 0;

	/* FIXME: find a better way, it only be used for debugging purpose */
	dev = task->mpp_task.session->mpp_dev->dev;
	if (!dev)
		return -EINVAL;

	dmabuf = dma_buf_get(fd);
	if (IS_ERR_OR_NULL(dmabuf)) {
		dev_err(dev, "invliad pps buffer\n");
		return -ENOENT;
	}

	ret = dma_buf_begin_cpu_access(dmabuf, DMA_FROM_DEVICE);
	if (ret) {
		dev_err(dev, "can't access the pps buffer\n");
		goto done;
	}

	vaddr = dma_buf_vmap(dmabuf);
	if (!vaddr) {
		dev_err(dev, "can't access the pps buffer\n");
		ret = -EIO;
		goto done;
	}
	pps = vaddr + offset;

	memcpy(&scaling_offset, pps + base, sizeof(scaling_offset));
	scaling_offset = le32_to_cpu(scaling_offset);

	scaling_fd = scaling_offset & 0x3ff;
	scaling_offset = scaling_offset >> 10;

	if (scaling_fd > 0) {
		struct mpp_mem_region *mem_region = NULL;
		dma_addr_t tmp = 0;
		int i = 0;

		mem_region = mpp_dev_task_attach_fd(&task->mpp_task,
						    scaling_fd);
		if (IS_ERR(mem_region)) {
			ret = PTR_ERR(mem_region);
			goto done;
		}

		tmp = mem_region->iova;
		tmp += scaling_offset;
		tmp = cpu_to_le32(tmp);
		mpp_debug(DEBUG_PPS_FILL,
			  "pps at %p, scaling fd: %3d => %pad + offset %10d\n",
			  pps, scaling_fd, &mem_region->iova, offset);

		/* Fill the scaling list address in each pps entries */
		for (i = 0; i < count; i++, base += pps_info_size)
			memcpy(pps + base, &tmp, sizeof(tmp));
	}

done:
	dma_buf_vunmap(dmabuf, vaddr);
	dma_buf_end_cpu_access(dmabuf, DMA_FROM_DEVICE);
	dma_buf_put(dmabuf);

	return ret;
}
#endif

static void *rockchip_mpp_rkvdec_alloc_task(struct mpp_session *session,
					    void __user * src, u32 size)
{
	struct rkvdec_task *task = NULL;
	struct vb2_v4l2_buffer *src_buf;
	u32 fmt = 0;
	int err = -EFAULT;

	mpp_debug_enter();

	task = kzalloc(sizeof(*task), GFP_KERNEL);
	if (!task)
		return NULL;

	mpp_dev_task_init(session, &task->mpp_task);

	src_buf = v4l2_m2m_next_src_buf(session->fh.m2m_ctx);
	v4l2_ctrl_request_setup(src_buf->vb2_buf.req_obj.req,
				&session->ctrl_handler);

	fmt = session->fmt_out.pixelformat;
	switch (fmt) {
	case V4L2_PIX_FMT_HEVC_SLICE:
		err = rkvdec_hevc_gen_reg(session, task->reg, src_buf);
		break;
	default:
		goto fail;
	}

	if (err)
		goto fail;

	v4l2_ctrl_request_complete(src_buf->vb2_buf.req_obj.req,
				   &session->ctrl_handler);

#if 0
	if (fmt == RKVDEC_FMT_VP9D) {
		struct mpp_mem_region *mem_region = NULL;
		dma_addr_t iova = 0;
		u32 offset = task->reg[RKVDEC_REG_VP9_REFCOLMV_BASE_INDEX];
		int fd = task->reg[RKVDEC_REG_VP9_REFCOLMV_BASE_INDEX] & 0x3ff;

		offset = offset >> 10 << 4;
		mem_region = mpp_dev_task_attach_fd(&task->mpp_task, fd);
		if (IS_ERR(mem_region)) {
			err = PTR_ERR(mem_region);
			goto fail;
		}

		iova = mem_region->iova;
		task->reg[RKVDEC_REG_VP9_REFCOLMV_BASE_INDEX] = iova + offset;
	}
#endif

#if 0
	pps_fd = task->reg[RKVDEC_REG_PPS_BASE_INDEX] & 0x3ff;
	pps_offset = task->reg[RKVDEC_REG_PPS_BASE_INDEX] >> 10;
	if (pps_fd > 0) {
		int pps_info_offset;
		int pps_info_count;
		int pps_info_size;
		int scaling_list_addr_offset;

		switch (fmt) {
		case RKVDEC_FMT_H264D:
			pps_info_offset = pps_offset;
			pps_info_count = 256;
			pps_info_size = 32;
			scaling_list_addr_offset = 23;
			break;
		case RKVDEC_FMT_H265D:
			pps_info_offset = pps_offset;
			pps_info_count = 64;
			pps_info_size = 80;
			scaling_list_addr_offset = 74;
			break;
		default:
			pps_info_offset = 0;
			pps_info_count = 0;
			pps_info_size = 0;
			scaling_list_addr_offset = 0;
			break;
		}

		mpp_debug(DEBUG_PPS_FILL, "scaling list filling parameter:\n");
		mpp_debug(DEBUG_PPS_FILL,
			  "pps_info_offset %d\n", pps_info_offset);
		mpp_debug(DEBUG_PPS_FILL,
			  "pps_info_count  %d\n", pps_info_count);
		mpp_debug(DEBUG_PPS_FILL,
			  "pps_info_size   %d\n", pps_info_size);
		mpp_debug(DEBUG_PPS_FILL,
			  "scaling_list_addr_offset %d\n",
			  scaling_list_addr_offset);

		if (pps_info_count) {
			err = fill_scaling_list_pps(task, pps_fd,
						    pps_info_offset,
						    pps_info_count,
						    pps_info_size,
						    scaling_list_addr_offset);
			if (err) {
				mpp_err("fill pps failed\n");
				goto fail;
			}
		}
	}
#endif

	mpp_debug_leave();

	return &task->mpp_task;

fail:
	kfree(task);
	return ERR_PTR(err);
}

static int rockchip_mpp_rkvdec_prepare(struct rockchip_mpp_dev *mpp_dev,
				       struct mpp_task *task)
{
	struct rockchip_rkvdec_dev *dec_dev = to_rkvdec_dev(mpp_dev);

	if (dec_dev->state == RKVDEC_STATE_NORMAL)
		return -EINVAL;
	/*
	 * Don't do soft reset before running or you will meet 0x00408322
	 * if you will decode a HEVC stream. Different error for the AVC.
	 */

	return 0;
}

static int rockchip_mpp_rkvdec_run(struct rockchip_mpp_dev *mpp_dev,
				   struct mpp_task *mpp_task)
{
	struct rockchip_rkvdec_dev *dec_dev = NULL;
	struct rkvdec_task *task = NULL;
	u32 reg = 0;

	mpp_debug_enter();

	dec_dev = to_rkvdec_dev(mpp_dev);
	task = to_rkvdec_task(mpp_task);

	switch (dec_dev->state) {
	case RKVDEC_STATE_NORMAL:
		/* FIXME: spin lock here */
		dec_dev->current_task = task;

		reg = RKVDEC_CACHE_PERMIT_CACHEABLE_ACCESS
		    | RKVDEC_CACHE_PERMIT_READ_ALLOCATE;
		if (!(debug & DEBUG_CACHE_32B))
			reg |= RKVDEC_CACHE_LINE_SIZE_64_BYTES;

		mpp_dev_write(mpp_dev, RKVDEC_REG_CACHE_ENABLE(0), reg);
		mpp_dev_write(mpp_dev, RKVDEC_REG_CACHE_ENABLE(1), reg);

		mpp_dev_write_seq(mpp_dev, RKVDEC_REG_SYS_CTRL,
				  &task->reg[RKVDEC_REG_SYS_CTRL_INDEX],
				  mpp_dev->variant->reg_len
				  - RKVDEC_REG_SYS_CTRL_INDEX);

		/* Flush the register before the start the device */
		wmb();
		mpp_dev_write(mpp_dev, RKVDEC_REG_DEC_INT_EN,
			      task->reg[RKVDEC_REG_DEC_INT_EN_INDEX]
			      | RKVDEC_DEC_START);
		break;
	default:
		break;
	}

	mpp_debug_leave();

	return 0;
}

static int rockchip_mpp_rkvdec_finish(struct rockchip_mpp_dev *mpp_dev,
				      struct mpp_task *mpp_task)
{
	struct rockchip_rkvdec_dev *dec_dev = to_rkvdec_dev(mpp_dev);
	struct rkvdec_task *task = to_rkvdec_task(mpp_task);

	mpp_debug_enter();

	switch (dec_dev->state) {
	case RKVDEC_STATE_NORMAL:{
			mpp_dev_read_seq(mpp_dev, RKVDEC_REG_SYS_CTRL,
					 &task->reg[RKVDEC_REG_SYS_CTRL_INDEX],
					 mpp_dev->variant->reg_len
					 - RKVDEC_REG_SYS_CTRL_INDEX);
			task->reg[RKVDEC_REG_DEC_INT_EN_INDEX] =
			    task->irq_status;
		}
		break;
	default:
		break;
	}

	mpp_debug_leave();

	return 0;
}

static int rockchip_mpp_rkvdec_result(struct rockchip_mpp_dev *mpp_dev,
				      struct mpp_task *mpp_task,
				      u32 __user * dst, u32 size)
{
	return 0;
}

static int rockchip_mpp_rkvdec_free_task(struct mpp_session *session,
					 struct mpp_task *mpp_task)
{
	struct rkvdec_task *task = to_rkvdec_task(mpp_task);

	mpp_dev_task_finalize(session, mpp_task);
	kfree(task);

	return 0;
}

static irqreturn_t mpp_rkvdec_isr(int irq, void *dev_id)
{
	struct rockchip_rkvdec_dev *dec_dev = dev_id;
	struct rockchip_mpp_dev *mpp_dev = &dec_dev->mpp_dev;
	struct rkvdec_task *task = NULL;
	struct mpp_task *mpp_task = NULL;
	u32 irq_status;
	u32 err_mask;

	irq_status = mpp_dev_read(mpp_dev, RKVDEC_REG_DEC_INT_EN);
	if (!(irq_status & RKVDEC_DEC_INT_RAW))
		return IRQ_NONE;

	mpp_dev_write(mpp_dev, RKVDEC_REG_DEC_INT_EN, RKVDEC_CLOCK_GATE_EN);
	/* FIXME use a spin lock here */
	task = (struct rkvdec_task *)dec_dev->current_task;
	if (!task) {
		dev_err(dec_dev->mpp_dev.dev, "no current task\n");
		return IRQ_HANDLED;
	}
	mpp_debug_time_diff(&task->mpp_task);

	task->irq_status = irq_status;
	switch (dec_dev->state) {
	case RKVDEC_STATE_NORMAL:
		mpp_debug(DEBUG_IRQ_STATUS, "irq_status: %08x\n",
			  task->irq_status);

		err_mask = RKVDEC_INT_BUF_EMPTY
		    | RKVDEC_INT_BUS_ERROR
		    | RKVDEC_INT_COLMV_REF_ERROR
		    | RKVDEC_INT_STRM_ERROR | RKVDEC_INT_TIMEOUT;

		if (err_mask & task->irq_status)
			atomic_set(&mpp_dev->reset_request, 1);

		mpp_task = &task->mpp_task;
		mpp_dev_task_finish(mpp_task->session, mpp_task);

		mpp_debug_leave();
		return IRQ_HANDLED;
	default:
		goto fail;
	}
fail:
	return IRQ_HANDLED;
}

static int rockchip_mpp_rkvdec_assign_reset(struct rockchip_rkvdec_dev *dec_dev)
{
	struct rockchip_mpp_dev *mpp_dev = &dec_dev->mpp_dev;

	dec_dev->rst_a = devm_reset_control_get_shared(mpp_dev->dev, "video_a");
	dec_dev->rst_h = devm_reset_control_get_shared(mpp_dev->dev, "video_h");
	/* The reset controller below are not shared with VPU */
	dec_dev->rst_niu_a = devm_reset_control_get(mpp_dev->dev, "niu_a");
	dec_dev->rst_niu_h = devm_reset_control_get(mpp_dev->dev, "niu_h");
	dec_dev->rst_core = devm_reset_control_get(mpp_dev->dev, "video_core");
	dec_dev->rst_cabac = devm_reset_control_get(mpp_dev->dev,
						    "video_cabac");

	if (IS_ERR_OR_NULL(dec_dev->rst_a)) {
		mpp_err("No aclk reset resource define\n");
		dec_dev->rst_a = NULL;
	}

	if (IS_ERR_OR_NULL(dec_dev->rst_h)) {
		mpp_err("No hclk reset resource define\n");
		dec_dev->rst_h = NULL;
	}

	if (IS_ERR_OR_NULL(dec_dev->rst_niu_a)) {
		mpp_err("No axi niu reset resource define\n");
		dec_dev->rst_niu_a = NULL;
	}

	if (IS_ERR_OR_NULL(dec_dev->rst_niu_h)) {
		mpp_err("No ahb niu reset resource define\n");
		dec_dev->rst_niu_h = NULL;
	}

	if (IS_ERR_OR_NULL(dec_dev->rst_core)) {
		mpp_err("No core reset resource define\n");
		dec_dev->rst_core = NULL;
	}

	if (IS_ERR_OR_NULL(dec_dev->rst_cabac)) {
		mpp_err("No cabac reset resource define\n");
		dec_dev->rst_cabac = NULL;
	}

	safe_unreset(dec_dev->rst_a);
	safe_unreset(dec_dev->rst_h);

	return 0;
}

static int rockchip_mpp_rkvdec_reset(struct rockchip_mpp_dev *mpp_dev)
{
	struct rockchip_rkvdec_dev *dec = to_rkvdec_dev(mpp_dev);

	if (dec->rst_a && dec->rst_h) {
		mpp_debug(DEBUG_RESET, "reset in\n");
		rockchip_pmu_idle_request(mpp_dev->dev, true);

		safe_reset(dec->rst_niu_a);
		safe_reset(dec->rst_niu_h);
		safe_reset(dec->rst_a);
		safe_reset(dec->rst_h);
		safe_reset(dec->rst_core);
		safe_reset(dec->rst_cabac);
		udelay(5);
		safe_unreset(dec->rst_niu_h);
		safe_unreset(dec->rst_niu_a);
		safe_unreset(dec->rst_a);
		safe_unreset(dec->rst_h);
		safe_unreset(dec->rst_core);
		safe_unreset(dec->rst_cabac);

		rockchip_pmu_idle_request(mpp_dev->dev, false);

		mpp_dev_write(mpp_dev, RKVDEC_REG_DEC_INT_EN, 0);
		dec->current_task = NULL;
		mpp_debug(DEBUG_RESET, "reset out\n");
	}

	return 0;
}

static int rockchip_mpp_rkvdec_sip_reset(struct rockchip_mpp_dev *mpp_dev)
{
/* The reset flow in arm trustzone firmware */
#if CONFIG_ROCKCHIP_SIP
	sip_smc_vpu_reset(0, 0, 0);
#else
	return rockchip_mpp_rkvdec_reset(mpp_dev);
#endif
	return 0;
}

#if 0
static int rkvdec_rk3328_iommu_hdl(struct iommu_domain *iommu,
				   struct device *iommu_dev, unsigned long iova,
				   int status, void *arg)
{
	struct device *dev = (struct device *)arg;
	struct platform_device *pdev = NULL;
	struct rockchip_rkvdec_dev *dec_dev = NULL;
	struct rockchip_mpp_dev *mpp_dev = NULL;

	int ret = -EIO;

	pdev = container_of(dev, struct platform_device, dev);
	if (!pdev) {
		dev_err(dev, "invalid platform_device\n");
		ret = -ENXIO;
		goto done;
	}

	dec_dev = platform_get_drvdata(pdev);
	if (!dec_dev) {
		dev_err(dev, "invalid device instance\n");
		ret = -ENXIO;
		goto done;
	}
	mpp_dev = &dec_dev->mpp_dev;

	if (IOMMU_GET_BUS_ID(status) == 2) {
		unsigned long page_iova = 0;

		/* avoid another page fault occur after page fault */
		if (dec_dev->aux_iova != 0)
			iommu_unmap(mpp_dev->iommu_info->domain,
				    dec_dev->aux_iova, IOMMU_PAGE_SIZE);

		page_iova = round_down(iova, IOMMU_PAGE_SIZE);
		ret = iommu_map(mpp_dev->iommu_info->domain, page_iova,
				page_to_phys(dec_dev->aux_page),
				IOMMU_PAGE_SIZE, DMA_FROM_DEVICE);
		if (!ret)
			dec_dev->aux_iova = page_iova;
	}

done:
	return ret;
}
#endif

static struct mpp_dev_ops rkvdec_ops = {
	.alloc_task = rockchip_mpp_rkvdec_alloc_task,
	.prepare = rockchip_mpp_rkvdec_prepare,
	.run = rockchip_mpp_rkvdec_run,
	.finish = rockchip_mpp_rkvdec_finish,
	.result = rockchip_mpp_rkvdec_result,
	.free_task = rockchip_mpp_rkvdec_free_task,
	.reset = rockchip_mpp_rkvdec_reset,
};

#if 0
static struct mpp_dev_ops rkvdec_rk3328_ops = {
	.alloc_task = rockchip_mpp_rkvdec_alloc_task,
	.prepare = rockchip_mpp_rkvdec_prepare,
	.run = rockchip_mpp_rkvdec_run,
	.finish = rockchip_mpp_rkvdec_finish,
	.result = rockchip_mpp_rkvdec_result,
	.free_task = rockchip_mpp_rkvdec_free_task,
	.reset = rockchip_mpp_rkvdec_sip_reset,
};
#endif

static int rockchip_mpp_rkvdec_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rockchip_rkvdec_dev *dec_dev = NULL;
	struct rockchip_mpp_dev *mpp_dev = NULL;
	int ret = 0;

	dec_dev = devm_kzalloc(dev, sizeof(struct rockchip_rkvdec_dev),
			       GFP_KERNEL);
	if (!dec_dev)
		return -ENOMEM;

	mpp_dev = &dec_dev->mpp_dev;
	mpp_dev->variant = rockchip_rkvdec_get_drv_data(pdev);

#if 0
	if (mpp_dev->variant->version_bit & RKVDEC_VER_RK3328_BIT) {
		ret = mpp_dev_common_probe(mpp_dev, pdev, &rkvdec_rk3328_ops);

		dec_dev->aux_page = alloc_page(GFP_KERNEL);
		if (!dec_dev->aux_page) {
			dev_err(dev,
				"can't allocate a page for auxiliary usage\n");
			return ret;
		}
		dec_dev->aux_iova = 0;

		iommu_set_fault_handler(mpp_dev->iommu_info->domain,
					rkvdec_rk3328_iommu_hdl, dev);
	} else {
		ret = mpp_dev_common_probe(mpp_dev, pdev, &rkvdec_ops);
	}
#else
	ret = mpp_dev_common_probe(mpp_dev, pdev, &rkvdec_ops);
#endif
	if (ret)
		return ret;

	ret = devm_request_threaded_irq(dev, mpp_dev->irq, NULL, mpp_rkvdec_isr,
					IRQF_SHARED | IRQF_ONESHOT,
					dev_name(dev), dec_dev);
	if (ret) {
		dev_err(dev, "register interrupter runtime failed\n");
		return ret;
	}

	rockchip_mpp_rkvdec_assign_reset(dec_dev);
	dec_dev->state = RKVDEC_STATE_NORMAL;

	rkvdec_ioctl_ops = mpp_ioctl_ops_templ;
	rkvdec_ioctl_ops.vidioc_s_fmt_vid_out_mplane =
		rkvdec_s_fmt_vid_out_mplane;
	rkvdec_ioctl_ops.vidioc_s_fmt_vid_cap_mplane =
		rkvdec_s_fmt_vid_cap_mplane;

	ret = mpp_dev_register_node(mpp_dev, mpp_dev->variant->node_name,
				    &rkvdec_fops, &rkvdec_ioctl_ops);
	if (ret)
		dev_err(dev, "register char device failed: %d\n", ret);

	memcpy(mpp_dev->fmt_out, fmt_out_templ, sizeof(fmt_out_templ));
	memcpy(mpp_dev->fmt_cap, fmt_cap_templ, sizeof(fmt_cap_templ));
	dev_info(dev, "probing finish\n");

	platform_set_drvdata(pdev, dec_dev);

	return 0;
}

static int rockchip_mpp_rkvdec_remove(struct platform_device *pdev)
{
	struct rockchip_rkvdec_dev *dec_dev = platform_get_drvdata(pdev);

	mpp_dev_common_remove(&dec_dev->mpp_dev);

	return 0;
}

static const struct of_device_id mpp_rkvdec_dt_match[] = {
	{.compatible = "rockchip,video-decoder-v1p",.data = &rkvdec_v1p_data},
	{.compatible = "rockchip,video-decoder-v1",.data = &rkvdec_v1_data},
	{.compatible = "rockchip,hevc-decoder-v1",.data = &rk_hevcdec_data},
	{},
};

static void *rockchip_rkvdec_get_drv_data(struct platform_device *pdev)
{
	struct mpp_dev_variant *driver_data = NULL;

	if (pdev->dev.of_node) {
		const struct of_device_id *match;

		match = of_match_node(mpp_rkvdec_dt_match, pdev->dev.of_node);
		if (match)
			driver_data = (struct mpp_dev_variant *)match->data;
	}
	return driver_data;
}

static struct platform_driver rockchip_rkvdec_driver = {
	.probe = rockchip_mpp_rkvdec_probe,
	.remove = rockchip_mpp_rkvdec_remove,
	.driver = {
		   .name = RKVDEC_DRIVER_NAME,
		   .of_match_table = of_match_ptr(mpp_rkvdec_dt_match),
		   },
};

static int __init mpp_dev_rkvdec_init(void)
{
	int ret = platform_driver_register(&rockchip_rkvdec_driver);

	if (ret) {
		mpp_err("Platform device register failed (%d).\n", ret);
		return ret;
	}

	return ret;
}

static void __exit mpp_dev_rkvdec_exit(void)
{
	platform_driver_unregister(&rockchip_rkvdec_driver);
}

module_init(mpp_dev_rkvdec_init);
module_exit(mpp_dev_rkvdec_exit);

MODULE_DEVICE_TABLE(of, mpp_rkvdec_dt_match);
MODULE_LICENSE("GPL v2");
