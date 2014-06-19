/*
 * linux/arch/arm/mach-exynos4/dev-dwmci.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * Platform device for Synopsys DesignWare Mobile Storage IP
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/kernel.h>
#include <linux/dma-mapping.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/mmc/dw_mmc.h>
#include <linux/mmc/host.h>
#include <linux/clk.h>

#include <plat/devs.h>

#include <mach/map.h>

#define DWMCI_CLKSEL    0x09c

static int exynos_dwmci_get_bus_wd(u32 slot_id)
{
	return 4;
}

static int exynos_dwmci_init(u32 slot_id, irq_handler_t handler, void *data)
{
	return 0;
}

static void exynos_dwmci_set_io_timing(void *data, unsigned char timing)
{
	struct dw_mci *host = (struct dw_mci *)data;
	u32 clksel;

	if (timing == MMC_TIMING_MMC_HS200 ||
			timing == MMC_TIMING_UHS_SDR104) {
		if(host->bus_hz != 200 * 1000 * 1000) {
			host->bus_hz = 200 * 1000 * 1000;
			clk_set_rate(host->cclk, 800 * 1000 * 1000);
		}
		clksel = __raw_readl(host->regs + DWMCI_CLKSEL);
		clksel = (clksel & 0xfff8ffff) | (host->pdata->clk_drv << 16);
		__raw_writel(clksel, host->regs + DWMCI_CLKSEL);
	} else if (timing == MMC_TIMING_UHS_DDR50) {
		if (host->bus_hz != 100 * 1000 * 1000) {
			host->bus_hz = 100 * 1000 * 1000;
			clk_set_rate(host->cclk, 400 * 1000 * 1000);
			host->current_speed = 0;
		}
		__raw_writel(host->pdata->ddr_timing,
			host->regs + DWMCI_CLKSEL);
	} else {
		if (host->bus_hz != 50 * 1000 * 1000) {
			host->bus_hz = 50 * 1000 * 1000;
			clk_set_rate(host->cclk, 200 * 1000 * 1000);
		}
		__raw_writel(host->pdata->sdr_timing,
			host->regs + DWMCI_CLKSEL);
	}
}

static struct resource exynos_dwmci_resource[] = {
	[0] = DEFINE_RES_MEM(EXYNOS4_PA_DWMCI, SZ_4K),
	[1] = DEFINE_RES_IRQ(EXYNOS4_IRQ_DWMCI),
};

static struct dw_mci_board exynos_dwci_pdata = {
	.num_slots			= 1,
	.quirks				= DW_MCI_QUIRK_BROKEN_CARD_DETECTION,
	.bus_hz				= 80 * 1000 * 1000,
	.detect_delay_ms	= 200,
	.init				= exynos_dwmci_init,
	.get_bus_wd			= exynos_dwmci_get_bus_wd,
};

static u64 exynos_dwmci_dmamask = DMA_BIT_MASK(32);

struct platform_device exynos_device_dwmci = {
	.name		= "dw_mmc",
	.id		= -1,
	.num_resources	= ARRAY_SIZE(exynos_dwmci_resource),
	.resource	= exynos_dwmci_resource,
	.dev		= {
		.dma_mask		= &exynos_dwmci_dmamask,
		.coherent_dma_mask	= DMA_BIT_MASK(32),
		.platform_data	= &exynos_dwci_pdata,
	},
};

void __init exynos_dwmci_set_platdata(struct dw_mci_board *pd)
{
	struct dw_mci_board *npd;

	npd = s3c_set_platdata(pd, sizeof(struct dw_mci_board),
			&exynos_device_dwmci);

	if (!npd->init)
		npd->init = exynos_dwmci_init;
	if (!npd->get_bus_wd)
		npd->get_bus_wd = exynos_dwmci_get_bus_wd;
	if (!npd->set_io_timing)
		npd->set_io_timing = exynos_dwmci_set_io_timing;
}
