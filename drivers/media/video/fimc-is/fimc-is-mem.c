/*
 * Samsung Exynos4 SoC series FIMC-IS slave interface driver
 *
 * v4l2 subdev driver interface
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd
 * Contact: Younghwan Joo, <yhwan.joo@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <asm/cacheflush.h>
#include <asm/pgtable.h>
#include <media/videobuf2-core.h>
#include <asm/cacheflush.h>
#if defined(CONFIG_VIDEOBUF2_ION)
#include <media/videobuf2-ion.h>
#endif
#include "fimc-is-core.h"
#include "fimc-is-param.h"
#define SIZE_THRESHOLD SZ_1M

struct vb2_buffer *is_vb;
void *buf_start;
struct fimc_is_dev *g_dev;
#if 1
struct vb2_ion_conf {
	struct device		*dev;
	const char		*name;

	struct ion_client	*client;

	unsigned long		align;
	bool			contig;
	bool			sharable;
	bool			cacheable;
	bool			use_mmu;
	atomic_t		mmu_enable;

	spinlock_t		slock;
};

struct vb2_ion_buf {
	struct vm_area_struct		*vma;
	struct vb2_ion_conf		*conf;
	struct vb2_vmarea_handler	*handler;

	struct ion_handle		*handle;	/* Kernel space */
	int				fd;		/* User space */

	dma_addr_t			kva;
	dma_addr_t			dva;
	size_t				offset;
	unsigned long			size;

	struct scatterlist		*sg;
	int				nents;

	atomic_t			ref;

	bool				cacheable;
};
#endif

void fimc_is_mem_cache_clean(const void *start_addr, unsigned long size)
{

return;
}

void fimc_is_mem_cache_inv(const void *start_addr, unsigned long size)
{
return ;
}

int fimc_is_cache_flush(struct vb2_buffer *vb,
                                const void *start_addr, unsigned long size1)
{

return 0;
}
int fimc_is_init_mem_mgr(struct fimc_is_dev *dev)
{
		int ret;
		g_dev=dev;
        dev->mem.size = FIMC_IS_A5_MEM_SIZE;
	dev->mem.base = 0x58000000;
	dev->mem.kvaddr = ioremap(0x58000000, dev->mem.size);
        if (!dev->mem.kvaddr) {
                printk(KERN_ERR "Bitprocessor memory remap failed\n");
                return -EIO;
        }
	memset(dev->mem.kvaddr, 0, FIMC_IS_A5_MEM_SIZE);
	dev->is_p_region =
		(struct is_region *)(dev->mem.kvaddr +
			FIMC_IS_A5_MEM_SIZE - FIMC_IS_REGION_SIZE);
	dev->is_shared_region =
                (struct is_share_region *)(dev->mem.kvaddr +
                                FIMC_IS_SHARED_REGION_ADDR);
	memset((void *)dev->is_p_region, 0,
                (unsigned long)sizeof(struct is_region));

	return 0;
}

