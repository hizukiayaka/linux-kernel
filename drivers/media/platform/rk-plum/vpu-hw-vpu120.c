/*
 * Rockchip VPU codec driver
 *
 * Copyright (C) 2014 - 2016 Rockchip Electronics Co., Ltd.
 *	Randy Li <randy.li@rock-chips.com>
 *	ayaka <ayaka@soulik.info>
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
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/irqreturn.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include "vpu-hw.h"
#include "vpu-hw-vpu120.h"
#include "regs-vpu120.h"

static inline void vdpu_reset(struct vpu_dev *vpu)
{
	vpu_write(vpu, VPU120_VDPU_REG_SOFT_RESET, 1);
}

static irqreturn_t vdpu120_irq_handler(int irq, void *data)
{
	struct vpu_dev *vpu = data;
	uint32_t irq_status = 0;

	irq_status = vpu_read(vpu, VPU120_VDPU_REG_INTERRUPT);
	vpu_write(vpu, VPU120_VDPU_REG_INTERRUPT, 0);

	vdpu_reset(vpu);

	return IRQ_HANDLED;
}

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

	vpu->clk_aclk = devm_clk_get(&pdev->dev, "aclk");
	if (IS_ERR_OR_NULL(vpu->clk_aclk)) {
		dev_err(&pdev->dev, "failed to get aclk\n");
		return PTR_ERR(vpu->clk_aclk);
	}

	vpu->clk_hclk = devm_clk_get(&pdev->dev, "hclk");
	if (IS_ERR_OR_NULL(vpu->clk_hclk)) {
		dev_err(&pdev->dev, "failed to get hclk\n");
		return PTR_ERR(vpu->clk_hclk);
	}

	vpu->rst_a = devm_reset_control_get(&pdev->dev, "video_a");
	if (IS_ERR_OR_NULL(vpu->rst_a))
		dev_info(&pdev->dev, "failed to get reset video_a\n");

	vpu->rst_h = devm_reset_control_get(&pdev->dev, "video_h");
	if (IS_ERR_OR_NULL(vpu->rst_h))
		dev_info(&pdev->dev, "failed to get reset video_h\n");

	return 0;
}

static int rk_vdpu120_probe(struct vpu_dev *vpu)
{
	int32_t ret, irq;
	struct platform_device *pdev;

	pdev = vpu->plat_dev;

	ret = rk_vpu120_probe(vpu);
	if (ret)
		return ret;

	/* IRQ */
	irq = platform_get_irq_byname(pdev, "vdpu"); 
	if (irq < 0)
		irq = platform_get_irq(pdev, 0);
	if (irq < 0) {
		dev_err(&pdev->dev, "failed to get irq resource\n");
		return irq;
	}

	/* share the same irq with the other devices in comobo */
	ret = devm_request_threaded_irq(&pdev->dev, irq, NULL,
					vdpu120_irq_handler, IRQF_SHARED,
					dev_name(&pdev->dev), vpu);
	if (ret < 0) {
		dev_err(&pdev->dev, "failed to request irq: %d\n", ret);
		return ret;
	}

	return 0;
}

static int rk_vpu120_reset(struct vpu_dev *vpu) {
	try_reset_assert(vpu->rst_niu_a);
	try_reset_assert(vpu->rst_niu_h);
	try_reset_assert(vpu->rst_a);
	try_reset_assert(vpu->rst_h);

	udelay(5);
	try_reset_deassert(vpu->rst_h);
	try_reset_deassert(vpu->rst_a);
	try_reset_deassert(vpu->rst_niu_h);
	try_reset_deassert(vpu->rst_niu_a);

	return 0;
}

static struct vpu_hw_ops vpu_hw_ops_vepu120 = {
	.probe = rk_vpu120_probe,
	.reset = rk_vpu120_reset,
	.remove = NULL,
};

static struct vpu_hw_ops vpu_hw_ops_vdpu120 = {
	.probe = rk_vdpu120_probe,
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
