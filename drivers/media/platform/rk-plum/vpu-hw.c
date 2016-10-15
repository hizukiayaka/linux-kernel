/*
 * Rockchip VPU codec driver, based on the virtual v4l2-mem2mem example driver
 *
 * Copyright (C) 2014 - 2016 Rockchip Electronics Co., Ltd.
 *	Randy Li <randy.li@rock-chips.com>
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

#include "vpu-hw.h"
#include "vpu-hw-rk3288.h"

void vpu_hw_init_ops(struct vpu_dev *vpu)
{
	struct vpu_hw_ops *vpu_ops;
	if (vpu->drvdata->version == VPU_VERSION_RK3288)
		vpu_ops = vpu_init_hw_ops_rk3288();

	vpu->hw_ops = vpu_ops;
}

int vpu_hw_probe(struct vpu_dev *vpu)
{
	if (vpu->hw_ops == NULL)
		return -EINVAL;
	if(vpu->hw_ops->probe == NULL)
		return -EINVAL;

	return vpu->hw_ops->probe(vpu);
}

int vpu_hw_remove(struct vpu_dev *vpu)
{
	if (vpu->hw_ops == NULL)
		return -EINVAL ;
	if(vpu->hw_ops->remove == NULL)
		return -EINVAL ;

	return vpu->hw_ops->remove(vpu);
}
