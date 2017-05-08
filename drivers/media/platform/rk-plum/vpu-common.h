/*
 * Rockchip VPU codec driver, based on the virtual v4l2-mem2mem example driver
 *
 * Copyright (C) 2014 - 2016 Rockchip Electronics Co., Ltd.
 *	Randy Li <randy.li@rock-chips.com>
 *
 * Based on the virtual v4l2-mem2mem example driver
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

#ifndef _VPU_COMMON_H_
#define _VPU_COMMON_H_

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>
#include <linux/sizes.h>
#include <linux/videodev2.h>

#include <media/media-entity.h>
#include <media/v4l2-common.h>
#include <media/v4l2-device.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-dma-contig.h>

#define VPU_TYPE_DECODER	(1<<31)
#define VPU_TYPE_ENCODER	(1<<30)
#define VPU_DEC_VERSION_120	0x0120 | VPU_TYPE_DECODER
#define VPU_ENC_VERSION_120	0x0120 | VPU_TYPE_ENCODER
#define VPU_V120_BIT		BIT(0)

extern int vpu_debug;

struct vpu_drvdata {
	u32 version_bit;
	unsigned int version;
};

struct vpu_dev {
	struct v4l2_device      v4l2_dev;
	struct v4l2_m2m_dev     *m2m_dev;
	struct video_device     *vfd;
	struct platform_device  *plat_dev;

	struct workqueue_struct *workqueue;
	struct list_head        instances;
	struct mutex            dev_mutex;
	spinlock_t              lock;

	int			irq;
	
	struct clk              *clk_aclk;
	struct clk              *clk_hclk;

	struct reset_control    *rst_v;
	struct reset_control    *rst_a;
	struct reset_control    *rst_h;
	struct reset_control    *rst_niu_a;
	struct reset_control    *rst_niu_h;

	void __iomem		*regs_base;

	struct dma_iommu_mapping *mapping;

	struct vpu_drvdata	*drvdata;
	struct vpu_hw_ops	*hw_ops;
};

#endif
