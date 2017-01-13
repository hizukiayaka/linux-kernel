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

#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/err.h>        
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/log2.h>
#include <linux/sizes.h>

#include <asm/dma-iommu.h>

#include <media/v4l2-common.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-device.h>
#include <media/v4l2-event.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-mem2mem.h>
#include <media/v4l2-device.h>
#include <media/v4l2-ioctl.h>
#include <media/v4l2-ctrls.h>
#include <media/v4l2-event.h>
#include <media/videobuf2-v4l2.h>
#include <media/videobuf2-dma-contig.h> 
#include <media/videobuf2-vmalloc.h>

#include "vpu-common.h"
#include "vpu-hw.h"

#define VPU_MODULE_NAME "rockchip-vpu"

static struct v4l2_m2m_ops m2m_ops = {
};

static int vpu_runtime_get(struct platform_device *pdev)
{
	int r;

	dev_dbg(&pdev->dev, "vpe_runtime_get\n");

	r = pm_runtime_get_sync(&pdev->dev);
	WARN_ON(r < 0);
	return r < 0 ? r : 0;
}

static void vpu_runtime_put(struct platform_device *pdev)
{
	int r;

	dev_dbg(&pdev->dev, "vpe_runtime_put\n");

	r = pm_runtime_put_sync(&pdev->dev);
	WARN_ON(r < 0 && r != -ENOSYS);
}

static void vpu_detach_iommu(struct vpu_dev *vpu)
{
	arm_iommu_release_mapping(vpu->mapping);
	vpu->mapping = NULL;
	iommu_group_remove_device(&vpu->plat_dev->dev);
}

static int vpu_attach_iommu(struct vpu_dev *vpu)
{
	struct dma_iommu_mapping *mapping;
	struct iommu_group *group;
	int ret;

	/* Create a device group and add the device to it. */
	group = iommu_group_alloc();
	if (IS_ERR(group)) {
		dev_err(&vpu->plat_dev->dev,
				"failed to allocate IOMMU group\n");
		return PTR_ERR(group);
	}

	ret = iommu_group_add_device(group, &vpu->plat_dev->dev);
	iommu_group_put(group);

	if (ret < 0) {
		dev_err(&vpu->plat_dev->dev,
				"failed to add device to IOMMU group\n");
		return ret;
	}

	/*
	 * Create the ARM mapping, used by the ARM DMA mapping core to allocate
	 * VAs. This will allocate a corresponding IOMMU domain.
	 */
	mapping = arm_iommu_create_mapping(&platform_bus_type, SZ_1G, SZ_2G);
	if (IS_ERR(mapping)) {
		dev_err(&vpu->plat_dev->dev,
				"failed to create ARM IOMMU mapping\n");
		ret = PTR_ERR(mapping);
		goto error;
	}

	vpu->mapping = mapping;

	/* Attach the ARM VA mapping to the device. */
	ret = arm_iommu_attach_device(&vpu->plat_dev->dev, mapping);
	if (ret < 0) {
		dev_err(&vpu->plat_dev->dev,
				"failed to attach device to VA mapping\n");
		goto error;
	}

	return 0;

error:
	vpu_detach_iommu(vpu);
	return ret;
}

static void *vpu_get_drv_data(struct platform_device *pdev);

static int rockchip_vpu_probe(struct platform_device *pdev)
{
	struct vpu_dev *vpu;
	int ret;

	vpu = devm_kzalloc(&pdev->dev, sizeof(*vpu), GFP_KERNEL);
	if (!vpu)
		return -ENOMEM;

	spin_lock_init(&vpu->lock);
	vpu->plat_dev = pdev;
	vpu->drvdata = vpu_get_drv_data(pdev);

	ret = v4l2_device_register(&pdev->dev, &vpu->v4l2_dev);
	if (ret)
		return ret;

	mutex_init(&vpu->dev_mutex);

	vpu_hw_init_ops(vpu);

	/* IP registers range and clock initilization */
	if (vpu_hw_probe(vpu))
		goto v4l2_dev_unreg;

	vpu_attach_iommu(vpu);

	platform_set_drvdata(pdev, vpu);

	vpu->m2m_dev = v4l2_m2m_init(&m2m_ops);
	if (IS_ERR(vpu->m2m_dev)) {
		dev_err(&pdev->dev, "Failed to init mem2mem device\n");
		ret = PTR_ERR(vpu->m2m_dev);
		goto v4l2_dev_unreg;
	}

	pm_runtime_enable(&pdev->dev);
	
	ret = vpu_runtime_get(pdev);
	if (ret) 
		goto rel_m2m;
	
	return 0;

rel_m2m:
	pm_runtime_disable(&pdev->dev);
	v4l2_m2m_release(vpu->m2m_dev);
v4l2_dev_unreg:
	v4l2_device_unregister(&vpu->v4l2_dev);
	return ret; 
}

static int rockchip_vpu_remove(struct platform_device *pdev)
{
	struct vpu_dev *vpu = platform_get_drvdata(pdev);
	v4l2_info(&vpu->v4l2_dev, "Removing " VPU_MODULE_NAME);

	v4l2_m2m_release(vpu->m2m_dev);
	if (vpu->vfd)
		video_unregister_device(vpu->vfd);

	vpu_hw_remove(vpu);
	vpu_runtime_put(pdev);
	pm_runtime_disable(&pdev->dev);

	return 0;
}

static const struct vpu_drvdata vpu_drvdata_vepu120 = {
	.version	= VPU_ENC_VERSION_120,
	.version_bit	= VPU_V120_BIT,
};

static const struct vpu_drvdata vpu_drvdata_vdpu120 = {
	.version	= VPU_DEC_VERSION_120,
	.version_bit	= VPU_V120_BIT,
};

static const struct of_device_id rockchip_vpu_of_match[] = {
	{ 
		.compatible = "rockchip,vepu1",
		.data = &vpu_drvdata_vdpu120,
	},
	{ 
		.compatible = "rockchip,vdpu1",
		.data = &vpu_drvdata_vdpu120,
	},
	{ /* sentinel */ },
};

MODULE_DEVICE_TABLE(of, rockchip_vpu_of_match);

static void *vpu_get_drv_data(struct platform_device *pdev)
{
	struct vpu_drvdata *driver_data = NULL;
	const struct of_device_id *match;

	match = of_match_node(rockchip_vpu_of_match, pdev->dev.of_node);
	if (match)
		driver_data = (struct vpu_drvdata *)match->data;

	 return driver_data;
}

static struct platform_driver rockchip_vpu_driver = {
	.probe		= rockchip_vpu_probe,
	.remove		= rockchip_vpu_remove,
	.driver		= {
		.name	= VPU_MODULE_NAME,
		.of_match_table = of_match_ptr(rockchip_vpu_of_match),
	},
};

module_platform_driver(rockchip_vpu_driver);
