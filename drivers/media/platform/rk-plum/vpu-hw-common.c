/*
 * Rockchip VPU codec driver
 *
 * Copyright (C) 2014 - 2016 Randy Li <ayaka@soulik.info>.
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

#include "vpu-hw-common.h"

inline int try_reset_assert(struct reset_control *rst)
{
	if (!IS_ERR_OR_NULL(rst))
		return reset_control_assert(rst);
	return -EINVAL;
}

inline int try_reset_deassert(struct reset_control *rst)
{
	if (!IS_ERR_OR_NULL(rst))
		return reset_control_deassert(rst);
	return -EINVAL;
}

unsigned int vpu_read(struct vpu_dev *dev, u32 reg)
{
	u32 data;

	data = readl(dev->regs_base + reg);
	v4l2_dbg(2, vpu_debug, &dev->v4l2_dev,
		 "%s: data=0x%x, reg=0x%x\n", __func__, data, reg);
	return data;
}

void vpu_write(struct vpu_dev *dev, u32 data, u32 reg)
{
	v4l2_dbg(2, vpu_debug, &dev->v4l2_dev,
		 "%s: data=0x%x, reg=0x%x\n", __func__, data, reg);
	writel(data, dev->regs_base + reg);
}
