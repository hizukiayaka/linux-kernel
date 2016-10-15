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

#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include "vpu-hw.h"
#include "vpu-hw-rk3288.h"

static int rk3288_probe(struct vpu_dev *vpu)
{
	struct platform_device *pdev;

	pdev = vpu->plat_dev;

	vpu->clk_aclk = devm_clk_get(&vpu->plat_dev->dev, "aclk");
	if (IS_ERR(vpu->clk_aclk)) {
		dev_err(&pdev->dev, "failed to get aclk\n");
		return PTR_ERR(vpu->clk_aclk);
	}

	vpu->clk_hclk = devm_clk_get(&vpu->plat_dev->dev, "hclk");
	if (IS_ERR(vpu->clk_hclk)) {
		dev_err(&pdev->dev, "failed to get hclk\n");
		return PTR_ERR(vpu->clk_hclk);
	}

	return 0;
}

static struct vpu_hw_ops vpu_hw_ops_rk3288 = {
	.probe = rk3288_probe,
	.remove = NULL,
};

struct vpu_hw_ops *vpu_init_hw_ops_rk3288(void)
{
	return &vpu_hw_ops_rk3288;	
}
