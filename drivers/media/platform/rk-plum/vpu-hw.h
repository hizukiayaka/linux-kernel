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

#ifndef _VPU_HW_H_
#define _VPU_HW_H_

#include <media/media-entity.h>
#include <media/v4l2-device.h>
#include <linux/device.h>
#include <linux/io.h>
#include <linux/iommu.h>
#include <linux/platform_device.h>

#include "vpu-common.h"

struct vpu_hw_ops {
	int (*probe)(struct vpu_dev *vpu);
	int (*remove)(struct vpu_dev *vpu);
};

void vpu_hw_init_ops(struct vpu_dev *vpu);

int vpu_hw_probe(struct vpu_dev *vpu);
int vpu_hw_remove(struct vpu_dev *vpu);

#endif
