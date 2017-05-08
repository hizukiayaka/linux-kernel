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

#ifndef _VPU_HW_COMMON_H_
#define _VPU_HW_COMMON_H_

#include <linux/reset.h> 

#include "vpu-common.h"

struct vpu_hw_ops {
	int (*probe)(struct vpu_dev *vpu);
	int (*remove)(struct vpu_dev *vpu);
	int (*reset)(struct vpu_dev *vpu);
};

unsigned int vpu_read(struct vpu_dev *dev, u32 reg);
void vpu_write(struct vpu_dev *dev, u32 data, u32 reg);

int try_reset_assert(struct reset_control *rst);
int try_reset_deassert(struct reset_control *rst);

#endif
