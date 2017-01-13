/*
 * Rockchip VPU codec driver, based on the virtual v4l2-mem2mem example driver
 *
 * Copyright (C) 2014 - 2016 Rockchip Electronics Co., Ltd.
 *	Randy Li <randy.li@rock-chips.com>
 *	ayaka <ayaka@soulik.info>
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
#include "vpu-hw-vpu120.h"

static int rk_vpu120_probe(struct vpu_dev *vpu)
{
	struct platform_device *pdev;
	struct resource *res;

	pdev = vpu->plat_dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (IS_ERR(res))
		return PTR_ERR(res);
	vpu->regs_base = devm_ioremap_resource(&pdev->dev, res);
	if (IS_ERR(vpu->regs_base))
		return -ENOMEM;

	vpu->clk_aclk = devm_clk_get(&vpu->plat_dev->dev, "aclk");
	if (IS_ERR_OR_NULL(vpu->clk_aclk)) {
		dev_err(&pdev->dev, "failed to get aclk\n");
		return PTR_ERR(vpu->clk_aclk);
	}

	vpu->clk_hclk = devm_clk_get(&vpu->plat_dev->dev, "hclk");
	if (IS_ERR_OR_NULL(vpu->clk_hclk)) {
		dev_err(&pdev->dev, "failed to get hclk\n");
		return PTR_ERR(vpu->clk_hclk);
	}

	vpu->rst_a = devm_reset_control_get(&vpu->plat_dev->dev, "video_a");
	if (IS_ERR_OR_NULL(vpu->rst_a))
		dev_info(&pdev->dev, "failed to get reset video_a\n");

	vpu->rst_h = devm_reset_control_get(&vpu->plat_dev->dev, "video_h");
	if (IS_ERR_OR_NULL(vpu->rst_h))
		dev_info(&pdev->dev, "failed to get reset video_h\n");

	return 0;
}

static int rk_vpu120_reset(struct vpu_dev *vpu) {
	/* TODO: PMU idle first */
	try_reset_assert(vpu->rst_niu_a);
	try_reset_assert(vpu->rst_niu_h);
	try_reset_assert(vpu->rst_a);
	try_reset_assert(vpu->rst_h);

	udelay(5);
	try_reset_deassert(vpu->rst_h);
	try_reset_deassert(vpu->rst_a);
	try_reset_deassert(vpu->rst_niu_h);
	try_reset_deassert(vpu->rst_niu_a);
	/* TODO: PMU active */

	return 0;
}

static struct vpu_hw_ops vpu_hw_ops_vepu120 = {
	.probe = rk_vpu120_probe,
	.reset = rk_vpu120_reset,
	.remove = NULL,
};

static struct vpu_hw_ops vpu_hw_ops_vdpu120 = {
	.probe = rk_vpu120_probe,
	.reset = rk_vpu120_reset,
	.remove = NULL,
};

struct vpu_hw_ops *vpu_get_hw_ops_vepu120(void)
{
	return &vpu_hw_ops_vepu120;
}

struct vpu_hw_ops *vpu_get_hw_ops_vdpu120(void)
{
	return &vpu_hw_ops_vdpu120;
}
