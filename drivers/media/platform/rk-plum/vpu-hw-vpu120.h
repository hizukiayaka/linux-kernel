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

#ifndef _VPU_HW_VPU120_H_
#define _VPU_HW_VPU120_H_

#include "vpu-common.h"

struct vpu_hw_ops *vpu_get_hw_ops_vepu120(void);
struct vpu_hw_ops *vpu_get_hw_ops_vdpu120(void);

#endif
