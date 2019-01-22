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

#include <asm/cacheflush.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/types.h>
#include <linux/of_platform.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <soc/rockchip/pm_domains.h>

#include <linux/videodev2.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mem2mem.h>

#include <linux/pm_runtime.h>

#include "mpp_debug.h"
#include "mpp_dev_common.h"
#include "vdpu2/hal.h"

#define RKVDPU2_DRIVER_NAME		"mpp_vdpu2"
#define RKVDPU2_NODE_NAME		"vpu-service"

#define to_rkvdpu_task(ctx)		\
		container_of(ctx, struct rkvdpu_task, mpp_task)
#define to_rkvdpu_dev(dev)		\
		container_of(dev, struct rockchip_rkvdpu_dev, mpp_dev)

static int debug;
module_param(debug, int, 0644);
MODULE_PARM_DESC(debug, "bit switch for vdpu2 debug information");

struct rockchip_rkvdpu_dev {
	struct rockchip_mpp_dev mpp_dev;

	struct reset_control *rst_a;
	struct reset_control *rst_h;

	void *current_task;
};

struct rkvdpu_task {
	struct mpp_task mpp_task;

	u32 reg[ROCKCHIP_VDPU2_REG_NUM];
	u32 idx;

	u32 strm_base;
	u32 irq_status;
};

static struct rockchip_mpp_control vdpu_controls[] = {
	{
	 .codec = V4L2_PIX_FMT_MPEG2_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_MPEG2_SLICE_PARAMS,
	 .elem_size = sizeof(struct v4l2_ctrl_mpeg2_slice_params),
	 },
	{
	 .codec = V4L2_PIX_FMT_MPEG2_SLICE,
	 .id = V4L2_CID_MPEG_VIDEO_MPEG2_QUANTIZATION,
	 .elem_size = sizeof(struct v4l2_ctrl_mpeg2_quantization),
	 },
};

static struct v4l2_pix_format_mplane fmt_out_templ[] = {
	{
	 .pixelformat = V4L2_PIX_FMT_MPEG2_SLICE,
	 },
	{.pixelformat = 0},
};

static struct v4l2_pix_format_mplane fmt_cap_templ[] = {
	{
	 .pixelformat = V4L2_PIX_FMT_NV12M,
	 },
	{.pixelformat = 0},
};

static const struct mpp_dev_variant rkvdpu_v2_data = {
	/* Exclude the register of the Performance counter */
	.reg_len = 159,
	.node_name = RKVDPU2_NODE_NAME,
	.vfd_func = MEDIA_ENT_F_PROC_VIDEO_DECODER,
};

static int rkvdpu_open(struct file *filp);

static const struct v4l2_file_operations rkvdpu_fops = {
	.open = rkvdpu_open,
	.release = rockchip_mpp_dev_release,
	.poll = v4l2_m2m_fop_poll,
	.unlocked_ioctl = video_ioctl2,
	.mmap = v4l2_m2m_fop_mmap,
};

#if 1
static struct v4l2_ioctl_ops rkvdpu_ioctl_ops = { 0, };
#endif

static void *rockchip_rkvdpu2_get_drv_data(struct platform_device *pdev);

#if 0
static int rockchip_mpp_queue_setup(struct vb2_queue *vq,
				    unsigned int *num_buffers,
				    unsigned int *num_planes,
				    unsigned int sizes[],
				    struct device *alloc_devs[])
{
	struct mpp_session *session = vb2_get_drv_priv(vq);
	struct v4l2_pix_format_mplane pixfmt;
	unsigned int size;

	switch (vq->type) {
	case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
		pixfmt = &session->fmt_out;
		break;
	case V4L2_BUF_TYPE_VIDEO_CAPTURE:
		pixfmt = &session->fmt_cap;
		break;
	default:
		return -EINVAL;
	}

	if (*num_planes) {
		if (*num_planes != pixfmt->num_planes)
			return -EINVAL;
		for (i = 0; i < pixfmt->num_planes; ++i)
			if (sizes[i] < pixfmt->plane_fmt[i].sizeimage)
				return -EINVAL;

		return 0;
	}

	*num_planes = pixfmt->num_planes;
	for (i = 0; i < pixfmt->num_planes; ++i)
		sizes[i] = pixfmt->plane_fmt[i].sizeimage;

	return 0;
}

static const struct vb2_ops rkvdpu_queue_ops = {
	.queue_setup = rockchip_mpp_queue_setup,
	.wait_prepare = vb2_ops_wait_prepare,
	.wait_finish = vb2_ops_wait_finish,
	/* TODO */
	.start_streaming = NULL,
	.stop_streaming = NULL,
	.buf_queue = = rockchip_mpp_buf_queue,
	.buf_request_complete = rockchip_mpp_buf_request_complete,
};

static int rkvdpu_try_fmt_vid_out(struct file *file, void *fh,
				  struct v4l2_format *f)
{
	struct rockchip_mpp_dev *mpp_dev = video_drvdata(filp);
	struct mpp_session *session = container_of(filp->private_data,
						   struct mpp_session, fh);
	struct v4l2_pix_format *pix = &f->fmt.pix;

	return 0;
}

static int rkvdpu_try_fmt_vid_cap(struct file *file, void *fh,
				  struct v4l2_format *f)
{
	struct rockchip_mpp_dev *mpp_dev = video_drvdata(filp);
	struct mpp_session *session = container_of(filp->private_data,
						   struct mpp_session, fh);
	struct v4l2_pix_format *pix = &f->fmt.pix;

	return 0;
}
#endif

static int rkvdpu_s_fmt_vid_out_mplane(struct file *filp, void *priv,
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

#if 0
	ret = rkvdpu_try_fmt_out(filp, priv, f);
	if (ret)
		return ret;
#endif
	for (i = 0; i < pix_mp->num_planes; i++) {
		sizes += pix_mp->plane_fmt[i].sizeimage;
	}
	/* strm_len is 24 bits */
	if (sizes >= SZ_16M)
		return -EINVAL;

	if (!pix_mp->num_planes)
		pix_mp->num_planes = 1;

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

static int rkvdpu_s_fmt_vid_cap_mplane(struct file *filp, void *priv,
				       struct v4l2_format *f)
{
	struct mpp_session *session = container_of(filp->private_data,
						   struct mpp_session, fh);
	struct v4l2_pix_format_mplane *pix_mp = &f->fmt.pix_mp;
	struct vb2_queue *vq;

	vq = v4l2_m2m_get_vq(session->fh.m2m_ctx, f->type);
	if (vb2_is_streaming(vq))
		return -EBUSY;

#if 0
	ret = rkvdpu_try_fmt_cap(filp, priv, f);
	if (ret)
		return ret;
#endif
	switch (pix_mp->pixelformat) {
	case V4L2_PIX_FMT_NV12M:
		pix_mp->plane_fmt[0].bytesperline = ALIGN(pix_mp->width, 16);
		pix_mp->plane_fmt[1].bytesperline = ALIGN(pix_mp->width, 16);
		pix_mp->plane_fmt[0].sizeimage = ALIGN(pix_mp->width, 16) *
		/*
		 * FIXME: the plane 1 may map to a lower address than plane 0
		 * before solve this allocator problem, it can pass the test
		 */
		    ALIGN(pix_mp->height, 16) * 2;
		/* Additional space for motion vector */
		pix_mp->plane_fmt[1].sizeimage = ALIGN(pix_mp->width, 16) *
		    ALIGN(pix_mp->height, 16);
		pix_mp->num_planes = 2;
		break;
	default:
		return -EINVAL;
	}

	session->fmt_cap = *pix_mp;

	return 0;
}

#if 0
static int rkvdpu_queue_init(void *priv, struct vb2_queue *src_vq,
			     struct vb2_queue *dst_vq)
{
	struct mpp_session *session = priv;
	int ret;

	rockchip_mpp_queue_init(priv, src_vq, dst_vq);

	src_vq->ops = rkvdpu_queue_ops;
	ret = vb2_queue_init(src_vq);
	if (ret)
		return ret;

	dst_vq->ops = rkvdpu_queue_ops;
	return vb2_queue_init(dst_vq);
}

static int mpp_dev_open(struct file *filp)
{
	struct rockchip_mpp_dev *mpp_dev = video_drvdata(filp);
	struct video_device *vdev = video_devdata(filp);
	struct mpp_session *session = NULL;
	int error = 0;

	mpp_debug_enter();

	session = rockchip_alloc_session(mpp_dev);
	if (IS_ERR(session))
		return PTR_ERR(session);

	session->fh.m2m_ctx = v4l2_m2m_ctx_init(mpp_dev->m2m_dev, session,
						rockchip_mpp_queue_init);
	if (IS_ERR(session->fh.m2m_ctx)) {
		error = PTR_ERR(session->fb.m2m_ctx);
		goto fail;
	}
	v4l2_fh_init(&session->fh, vdev);
	filp->private_data = &session->fh;
	v4l2_fh_add(&session->fh);

	/* TODO: setup default formats */

	/* TODO: install v4l2 ctrl */
	if (error) {
		dev_err(mpp_dev->dev, "Failed to set up controls\n");
		goto err_fh;
	}

	session->fb.ctrl_handler = session->ctrl_handler;

	mpp_dev_power_on(mpp);
	mpp_debug_leave();

	return 0;

err_fh:
	v4l2_fh_del(&session->fh);
	v4l2_fh_exit(&session->fh);
fail:
	kfree(session);
	return error;
}
#endif

static int vdpu_setup_ctrls(struct rockchip_mpp_dev *mpp_dev,
			    struct mpp_session *session)
{
	struct v4l2_ctrl_handler *hdl = &session->ctrl_handler;
	struct v4l2_ctrl *ctrl;
	unsigned int num_ctrls = ARRAY_SIZE(vdpu_controls);
	unsigned int i;

	v4l2_ctrl_handler_init(hdl, num_ctrls);
	if (hdl->error) {
		v4l2_err(&mpp_dev->v4l2_dev,
			 "Failed to initialize control handler\n");
		return hdl->error;
	}
#if 0
	ctrls_size = sizeof(ctrl) * num_ctrls + 1;
	session->ctrls = kzalloc(ctrls_size, GFP_KERNEL);
#endif

	for (i = 0; i < num_ctrls; i++) {
		struct v4l2_ctrl_config cfg = { };

		cfg.id = vdpu_controls[i].id;
		cfg.elem_size = vdpu_controls[i].elem_size;

		ctrl = v4l2_ctrl_new_custom(hdl, &cfg, NULL);
		if (hdl->error) {
			v4l2_err(&mpp_dev->v4l2_dev,
				 "Failed to create new custom %d control\n",
				 cfg.id);
			goto fail;
		}
#if 0
		session->ctrls[i] = ctrl;
#endif
	}

	session->fh.ctrl_handler = hdl;
	v4l2_ctrl_handler_setup(hdl);

	return 0;
fail:
	v4l2_ctrl_handler_free(hdl);
#if 0
	kfree(session->ctrls);
#endif
	return hdl->error;
}

static int rkvdpu_open(struct file *filp)
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

	error = vdpu_setup_ctrls(mpp_dev, session);
	if (error) {
		kfree(session);
		return error;
	}

	filp->private_data = &session->fh;
	pm_runtime_get_sync(mpp_dev->dev);

	mpp_debug_leave();
	return 0;
}

static void *rockchip_mpp_rkvdpu_alloc_task(struct mpp_session *session,
					    void __user * src, u32 size)
{
	struct rkvdpu_task *task = NULL;
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
	case V4L2_PIX_FMT_MPEG2_SLICE:
		err = rkvdpu_mpeg2_gen_reg(session, task->reg, src_buf);
		break;
	default:
		goto fail;
	}

	if (err)
		goto fail;

	v4l2_ctrl_request_complete(src_buf->vb2_buf.req_obj.req,
				   &session->ctrl_handler);

	mpp_debug_leave();

	return &task->mpp_task;

fail:
	if (unlikely(debug & DEBUG_DUMP_ERR_REG))
		mpp_debug_dump_reg_mem(task->reg, ROCKCHIP_VDPU2_REG_NUM);

	kfree(task);
	return ERR_PTR(err);
}

static int rockchip_mpp_rkvdpu_prepare(struct rockchip_mpp_dev *mpp_dev,
				       struct mpp_task *mpp_task)
{
	struct rkvdpu_task *task = container_of(mpp_task, struct rkvdpu_task,
						mpp_task);

	return rkvdpu_mpeg2_prepare_buf(mpp_task->session, task->reg);
}

static int rockchip_mpp_rkvdpu_run(struct rockchip_mpp_dev *mpp_dev,
				   struct mpp_task *mpp_task)
{
	struct rkvdpu_task *task = NULL;
	struct rockchip_rkvdpu_dev *dec_dev = NULL;

	mpp_debug_enter();

	task = to_rkvdpu_task(mpp_task);
	dec_dev = to_rkvdpu_dev(mpp_dev);

	/* FIXME: spin lock here */
	dec_dev->current_task = task;
	/* NOTE: Only write the decoder part */
	mpp_dev_write_seq(mpp_dev, RKVDPU2_REG_DEC_CTRL,
			  &task->reg[RKVDPU2_REG_DEC_CTRL_INDEX],
			  RKVDPU2_REG_DEC_DEV_CTRL_INDEX
			  - RKVDPU2_REG_DEC_CTRL_INDEX);

	mpp_dev_write_seq(mpp_dev, RKVDPU2_REG59,
			  &task->reg[RKVDPU2_REG59_INDEX],
			  mpp_dev->variant->reg_len - RKVDPU2_REG59_INDEX);
	/* Flush the registers */
	wmb();
	mpp_dev_write(mpp_dev, RKVDPU2_REG_DEC_DEV_CTRL,
		      task->reg[RKVDPU2_REG_DEC_DEV_CTRL_INDEX]
		      | RKVDPU2_DEC_START);

	mpp_debug_leave();

	return 0;
}

static int rockchip_mpp_rkvdpu_finish(struct rockchip_mpp_dev *mpp_dev,
				      struct mpp_task *mpp_task)
{
	struct rkvdpu_task *task = to_rkvdpu_task(mpp_task);

	mpp_debug_enter();

	/* NOTE: Only read the decoder part */
	mpp_dev_read_seq(mpp_dev, RKVDPU2_REG_DEC_CTRL,
			 &task->reg[RKVDPU2_REG_DEC_CTRL_INDEX],
			 mpp_dev->variant->reg_len
			 - RKVDPU2_REG_DEC_CTRL_INDEX);

	task->reg[RKVDPU2_REG_DEC_INT_EN_INDEX] = task->irq_status;

	mpp_debug_leave();

	return 0;
}

static int rockchip_mpp_rkvdpu_result(struct rockchip_mpp_dev *mpp_dev,
				      struct mpp_task *mpp_task,
				      u32 __user * dst, u32 size)
{
	struct rkvdpu_task *task = to_rkvdpu_task(mpp_task);
	u32 err_mask;

	err_mask = RKVDPU2_INT_TIMEOUT
	    | RKVDPU2_INT_STRM_ERROR
	    | RKVDPU2_INT_ASO_ERROR
	    | RKVDPU2_INT_BUF_EMPTY
	    | RKVDPU2_INT_BUS_ERROR;

	if (err_mask & task->irq_status)
		return VB2_BUF_STATE_ERROR;

	return VB2_BUF_STATE_DONE;
}

static int rockchip_mpp_rkvdpu_free_task(struct mpp_session *session,
					 struct mpp_task *mpp_task)
{
	struct rkvdpu_task *task = to_rkvdpu_task(mpp_task);

	mpp_dev_task_finalize(session, mpp_task);
	kfree(task);

	return 0;
}

static irqreturn_t mpp_rkvdpu_isr(int irq, void *dev_id)
{
	struct rockchip_rkvdpu_dev *dec_dev = dev_id;
	struct rockchip_mpp_dev *mpp_dev = &dec_dev->mpp_dev;
	struct rkvdpu_task *task = NULL;
	struct mpp_task *mpp_task = NULL;
	u32 irq_status;
	u32 err_mask;

	irq_status = mpp_dev_read(mpp_dev, RKVDPU2_REG_DEC_INT_EN);
	if (!(irq_status & RKVDPU2_DEC_INT_RAW))
		return IRQ_NONE;

	mpp_dev_write(mpp_dev, RKVDPU2_REG_DEC_INT_EN, 0);
	mpp_dev_write(mpp_dev, RKVDPU2_REG_DEC_DEV_CTRL,
		      RKVDPU2_DEC_CLOCK_GATE_EN);

	/* FIXME use a spin lock here */
	task = (struct rkvdpu_task *)dec_dev->current_task;
	if (!task) {
		dev_err(dec_dev->mpp_dev.dev, "no current task\n");
		return IRQ_HANDLED;
	}

	mpp_task = &task->mpp_task;
	mpp_debug_time_diff(mpp_task);
	task->irq_status = irq_status;
	mpp_debug(DEBUG_IRQ_STATUS, "irq_status: %08x\n", task->irq_status);

	err_mask = RKVDPU2_INT_TIMEOUT
	    | RKVDPU2_INT_STRM_ERROR
	    | RKVDPU2_INT_ASO_ERROR
	    | RKVDPU2_INT_BUF_EMPTY
	    | RKVDPU2_INT_BUS_ERROR;

	if (err_mask & task->irq_status)
		atomic_set(&mpp_dev->reset_request, 1);

	mpp_dev_task_finish(mpp_task->session, mpp_task);

	mpp_debug_leave();
	return IRQ_HANDLED;
}

static int rockchip_mpp_rkvdpu_assign_reset(struct rockchip_rkvdpu_dev *dec_dev)
{
	struct rockchip_mpp_dev *mpp_dev = &dec_dev->mpp_dev;

	dec_dev->rst_a = devm_reset_control_get_shared(mpp_dev->dev, "video_a");
	dec_dev->rst_h = devm_reset_control_get_shared(mpp_dev->dev, "video_h");

	if (IS_ERR_OR_NULL(dec_dev->rst_a)) {
		mpp_err("No aclk reset resource define\n");
		dec_dev->rst_a = NULL;
	}

	if (IS_ERR_OR_NULL(dec_dev->rst_h)) {
		mpp_err("No hclk reset resource define\n");
		dec_dev->rst_h = NULL;
	}

	return 0;
}

static int rockchip_mpp_rkvdpu_reset(struct rockchip_mpp_dev *mpp_dev)
{
	struct rockchip_rkvdpu_dev *dec = to_rkvdpu_dev(mpp_dev);

	if (dec->rst_a && dec->rst_h) {
		mpp_debug(DEBUG_RESET, "reset in\n");

		safe_reset(dec->rst_a);
		safe_reset(dec->rst_h);
		udelay(5);
		safe_unreset(dec->rst_h);
		safe_unreset(dec->rst_a);

		mpp_dev_write(mpp_dev, RKVDPU2_REG_DEC_DEV_CTRL, 0);
		mpp_dev_write(mpp_dev, RKVDPU2_REG_DEC_INT_EN, 0);
		dec->current_task = NULL;
		mpp_debug(DEBUG_RESET, "reset out\n");
	}

	return 0;
}

static struct mpp_dev_ops rkvdpu_ops = {
	.alloc_task = rockchip_mpp_rkvdpu_alloc_task,
	.prepare = rockchip_mpp_rkvdpu_prepare,
	.run = rockchip_mpp_rkvdpu_run,
	.finish = rockchip_mpp_rkvdpu_finish,
	.result = rockchip_mpp_rkvdpu_result,
	.free_task = rockchip_mpp_rkvdpu_free_task,
	.reset = rockchip_mpp_rkvdpu_reset,
};

static int rockchip_mpp_rkvdpu_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct rockchip_rkvdpu_dev *dec_dev = NULL;
	struct rockchip_mpp_dev *mpp_dev = NULL;
	int ret = 0;

	dec_dev = devm_kzalloc(dev, sizeof(struct rockchip_rkvdpu_dev),
			       GFP_KERNEL);
	if (!dec_dev)
		return -ENOMEM;

	mpp_dev = &dec_dev->mpp_dev;
	mpp_dev->variant = rockchip_rkvdpu2_get_drv_data(pdev);
	ret = mpp_dev_common_probe(mpp_dev, pdev, &rkvdpu_ops);
	if (ret)
		return ret;

	ret = devm_request_threaded_irq(dev, mpp_dev->irq, NULL, mpp_rkvdpu_isr,
					IRQF_SHARED | IRQF_ONESHOT,
					dev_name(dev), dec_dev);
	if (ret) {
		dev_err(dev, "register interrupter runtime failed\n");
		return ret;
	}

	rockchip_mpp_rkvdpu_assign_reset(dec_dev);

	rkvdpu_ioctl_ops = mpp_ioctl_ops_templ;
	rkvdpu_ioctl_ops.vidioc_s_fmt_vid_out_mplane =
	    rkvdpu_s_fmt_vid_out_mplane;
	rkvdpu_ioctl_ops.vidioc_s_fmt_vid_cap_mplane =
	    rkvdpu_s_fmt_vid_cap_mplane;

	ret = mpp_dev_register_node(mpp_dev, mpp_dev->variant->node_name,
				    &rkvdpu_fops, &rkvdpu_ioctl_ops);
	if (ret)
		dev_err(dev, "register char device failed: %d\n", ret);

	memcpy(mpp_dev->fmt_out, fmt_out_templ, sizeof(fmt_out_templ));
	memcpy(mpp_dev->fmt_cap, fmt_cap_templ, sizeof(fmt_cap_templ));
	dev_info(dev, "probing finish\n");

	platform_set_drvdata(pdev, dec_dev);

	return 0;
}

static int rockchip_mpp_rkvdpu_remove(struct platform_device *pdev)
{
	struct rockchip_rkvdpu_dev *dec_dev = platform_get_drvdata(pdev);

	mpp_dev_common_remove(&dec_dev->mpp_dev);

	return 0;
}

static const struct of_device_id mpp_rkvdpu2_dt_match[] = {
	{.compatible = "rockchip,vpu-decoder-v2",.data = &rkvdpu_v2_data},
	{},
};

static void *rockchip_rkvdpu2_get_drv_data(struct platform_device *pdev)
{
	struct mpp_dev_variant *driver_data = NULL;

	if (pdev->dev.of_node) {
		const struct of_device_id *match;

		match = of_match_node(mpp_rkvdpu2_dt_match, pdev->dev.of_node);
		if (match)
			driver_data = (struct mpp_dev_variant *)match->data;
	}
	return driver_data;
}

static struct platform_driver rockchip_rkvdpu2_driver = {
	.probe = rockchip_mpp_rkvdpu_probe,
	.remove = rockchip_mpp_rkvdpu_remove,
	.driver = {
		   .name = RKVDPU2_DRIVER_NAME,
		   .of_match_table = of_match_ptr(mpp_rkvdpu2_dt_match),
		   },
};

static int __init mpp_dev_rkvdpu2_init(void)
{
	int ret = platform_driver_register(&rockchip_rkvdpu2_driver);

	if (ret) {
		mpp_err("Platform device register failed (%d).\n", ret);
		return ret;
	}

	return ret;
}

static void __exit mpp_dev_rkvdpu2_exit(void)
{
	platform_driver_unregister(&rockchip_rkvdpu2_driver);
}

module_init(mpp_dev_rkvdpu2_init);
module_exit(mpp_dev_rkvdpu2_exit);

MODULE_DEVICE_TABLE(of, mpp_rkvdpu2_dt_match);
MODULE_LICENSE("GPL v2");
