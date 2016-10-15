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

#include <media/media-entity.h>
#include <media/v4l2-device.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>
#include <linux/videodev2.h>
#include <linux/sizes.h>

#include <media/v4l2-common.h>
#include <media/v4l2-mem2mem.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-dma-contig.h>

#define VPU_VERSION_RK3288	0x3288
#define VPU_RK3288_BIT		BIT(0)

struct vpu_drvdata {
	u32 version_bit;
	unsigned int version;
};

struct vpu_dev {
	struct v4l2_device      v4l2_dev;
	struct v4l2_m2m_dev     *m2m_dev;
	struct video_device     hevc_vfd;
	struct video_device     vepu_vfd;
	struct video_device     vdpu_vfd;
	struct platform_device  *plat_dev;

	atomic_t                num_instances;  /* count of driver instances */
	struct mutex            dev_mutex;
	spinlock_t              lock;

	int			irq_hevc;
	int			irq_vepu;
	int			irq_vdpu;
	
	struct clk              *clk_aclk;
	struct clk              *clk_hclk;

	void __iomem		*hevc_base;
	void __iomem		*vepu_base;
	void __iomem		*vdpu_base;
	struct resource         *hevc_res;
	struct resource         *vepu_res;
	struct resource         *vdpu_res;

	struct dma_iommu_mapping *mapping;

	struct vpu_drvdata	*drvdata;
	struct vpu_hw_ops	*hw_ops;
};

#endif
