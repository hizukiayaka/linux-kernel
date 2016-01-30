/* linux/arch/arm/mach-exynos/setup-fimc-is.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * FIMC-IS gpio and clock configuration
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/gpio.h>
#include <linux/clk.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <plat/clock.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <plat/map-s5p.h>
#include <plat/cpu.h>
#include <mach/map.h>
#include <mach/regs-clock.h>
#include <media/exynos_fimc_is.h>

struct platform_device; /* don't need the contents */

/*------------------------------------------------------*/
/*		Exynos4 series - FIMC-IS		*/
/*------------------------------------------------------*/
void exynos_fimc_is_cfg_gpio(struct platform_device *pdev)
{
	int ret;
	/* 1. UART setting for FIMC-IS */
	/* GPM3[5] : TXD_UART_ISP */
	ret = gpio_request(EXYNOS4X12_GPM3(5), "GPM3");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM3_5 ####\n");
	s3c_gpio_cfgpin(EXYNOS4X12_GPM3(5), (0x3<<20));
	s3c_gpio_setpull(EXYNOS4X12_GPM3(5), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM3(5));

	/* GPM3[7] : RXD_UART_ISP */
	ret = gpio_request(EXYNOS4X12_GPM3(7), "GPM3");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM3_7 ####\n");
	s3c_gpio_cfgpin(EXYNOS4X12_GPM3(7), (0x3<<28));
	s3c_gpio_setpull(EXYNOS4X12_GPM3(7), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM3(7));

	/* 2. GPIO setting for FIMC-IS */
	ret = gpio_request(EXYNOS4X12_GPM4(0), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS4X12_GPM4(0), (0x2<<0));
	s3c_gpio_setpull(EXYNOS4X12_GPM4(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM4(0));

	ret = gpio_request(EXYNOS4X12_GPM4(1), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_1 ####\n");
	s3c_gpio_cfgpin(EXYNOS4X12_GPM4(1), (0x2<<4));
	s3c_gpio_setpull(EXYNOS4X12_GPM4(1), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM4(1));

	ret = gpio_request(EXYNOS4X12_GPM4(2), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_2 ####\n");
	s3c_gpio_cfgpin(EXYNOS4X12_GPM4(2), (0x2<<8));
	s3c_gpio_setpull(EXYNOS4X12_GPM4(2), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM4(2));

	ret = gpio_request(EXYNOS4X12_GPM4(3), "GPM4");
	if (ret)
		printk(KERN_ERR "#### failed to request GPM4_3 ####\n");
	s3c_gpio_cfgpin(EXYNOS4X12_GPM4(3), (0x2<<12));
	s3c_gpio_setpull(EXYNOS4X12_GPM4(3), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS4X12_GPM4(3));
}

int exynos_fimc_is_clk_get(struct platform_device *pdev)
{
	struct exynos4_platform_fimc_is *pdata;
	pdata = to_fimc_is_plat(&pdev->dev);

	/* 1. Get clocks for CMU_ISP clock divider setting */
	/* UART_ISP_SEL - CLK_SRC_ISP (0x1003 C238) , [15:12] */
	pdata->div_clock[0] = clk_get(&pdev->dev, "mout_mpll_user");
	if (IS_ERR(pdata->div_clock[0])) {
		printk(KERN_ERR "failed to get mout_mpll_user\n");
		goto err_clk1;
	}
	/* UART_ISP_RATIO - CLK_DIV_ISP (0x1003 C538) , [31:28] */
	pdata->div_clock[1] = clk_get(&pdev->dev, "sclk_uart_isp");
	if (IS_ERR(pdata->div_clock[1])) {
		printk(KERN_ERR "failed to get uart_isp\n");
		goto err_clk2;
	}

	/* 2. Get clocks for CMU_ISP clock gate setting */
	/* CLK_MTCADC_ISP - CLK_GATE_IP_ISP0 (0x1004 8800), [27] */
	pdata->control_clock[0] = clk_get(&pdev->dev, "mtcadc");
	if (IS_ERR(pdata->control_clock[0])) {
		printk(KERN_ERR "failed to get mtcadc\n");
		goto err_clk3;
	}
	/* CLK_MPWM_ISP - CLK_GATE_IP_ISP0 (0x1004 8800), [24] */
	pdata->control_clock[1] = clk_get(&pdev->dev, "mpwm_isp");
	if (IS_ERR(pdata->control_clock[1])) {
		printk(KERN_ERR "failed to get mpwm_isp\n");
		goto err_clk4;
	}
	/* CLK_PPMUISPX  - CLK_GATE_IP_ISP0 (0x1004 8800), [21] */
	/* CLK_PPMUISPMX - CLK_GATE_IP_ISP0 (0x1004 8800), [20] */
	pdata->control_clock[2] = clk_get(&pdev->dev, "ppmuisp");
	if (IS_ERR(pdata->control_clock[2])) {
		printk(KERN_ERR "failed to get ppmuisp\n");
		goto err_clk5;
	}
	/* CLK_SMMU_LITE1 - CLK_GATE_IP_ISP0 (0x1004 8800), [12] */
	pdata->control_clock[3] = clk_get(&pdev->dev, "sysmmu_lite1");
	if (IS_ERR(pdata->control_clock[8])) {
		printk(KERN_ERR "failed to get sysmmu_lite1\n");
		goto err_clk6;
	}
	/* CLK_SMMU_LITE0 - CLK_GATE_IP_ISP0 (0x1004 8800), [11] */
	pdata->control_clock[4] = clk_get(&pdev->dev, "sysmmu_lite0");
	if (IS_ERR(pdata->control_clock[9])) {
		printk(KERN_ERR "failed to get sysmmu_lite0\n");
		goto err_clk7;
	}
	/* CLK_SPI1_ISP - CLK_GATE_IP_ISP0 (0x1004 8804), [13] */
	pdata->control_clock[5] = clk_get(&pdev->dev, "spi1_isp");
	if (IS_ERR(pdata->control_clock[10])) {
		printk(KERN_ERR "failed to get spi1_isp\n");
		goto err_clk8;
	}
	/* CLK_SPI0_ISP - CLK_GATE_IP_ISP0 (0x1004 8804), [12] */
	pdata->control_clock[6] = clk_get(&pdev->dev, "spi0_isp");
	if (IS_ERR(pdata->control_clock[11])) {
		printk(KERN_ERR "failed to get spi0_isp\n");
		goto err_clk9;
	}
	/* CLK_SMMU_FD - CLK_GATE_IP_ISP0 (0x1004 8800), [10] */
	pdata->control_clock[7] = clk_get(&pdev->dev, "sysmmu_fd");
	if (IS_ERR(pdata->control_clock[12])) {
		printk(KERN_ERR "failed to get sysmmu_fd\n");
		goto err_clk10;
	}
	/* CLK_SMMU_DRC - CLK_GATE_IP_ISP0 (0x1004 8800), [9] */
	pdata->control_clock[8] = clk_get(&pdev->dev, "sysmmu_drc");
	if (IS_ERR(pdata->control_clock[13])) {
		printk(KERN_ERR "failed to get sysmmu_drc\n");
		goto err_clk11;
	}
	/* CLK_SMMU_ISP - CLK_GATE_IP_ISP0 (0x1004 8800), [8] */
	pdata->control_clock[9] = clk_get(&pdev->dev, "sysmmu_isp");
	if (IS_ERR(pdata->control_clock[14])) {
		printk(KERN_ERR "failed to get sysmmu_isp\n");
		goto err_clk12;
	}
	/* CLK_SMMU_ISPCX - CLK_GATE_IP_ISP0 (0x1004 8804), [4] */
	pdata->control_clock[10] = clk_get(&pdev->dev, "sysmmu_ispcx");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get sysmmu_ispcx\n");
		goto err_clk13;
	}
	pdata->control_clock[11] = clk_get(&pdev->dev, "wdt_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get sysmmu_ispcx\n");
		goto err_clk14;
	}
	pdata->control_clock[12] = clk_get(&pdev->dev, "pwm_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk15;
	}
	pdata->control_clock[13] = clk_get(&pdev->dev, "i2c1_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk16;
	}
	pdata->control_clock[14] = clk_get(&pdev->dev, "i2c0_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk17;
	}
	pdata->control_clock[15] = clk_get(&pdev->dev, "mcuctl_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk18;
	}
	pdata->control_clock[16] = clk_get(&pdev->dev, "gicisp_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk19;
	}
	pdata->control_clock[17] = clk_get(&pdev->dev, "mcuisp_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk20;
	}
	pdata->control_clock[18] = clk_get(&pdev->dev, "fd_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk21;
	}
	pdata->control_clock[19] = clk_get(&pdev->dev, "drc_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk22;
	}
	pdata->control_clock[20] = clk_get(&pdev->dev, "isp_isp");
	if (IS_ERR(pdata->control_clock[15])) {
		printk(KERN_ERR "failed to get \n");
		goto err_clk23;
	}
	return 0;

err_clk23:
	clk_put(pdata->control_clock[19]);
err_clk22:
	clk_put(pdata->control_clock[18]);
err_clk21:
	clk_put(pdata->control_clock[17]);
err_clk20:
	clk_put(pdata->control_clock[16]);
err_clk19:
	clk_put(pdata->control_clock[15]);
err_clk18:
	clk_put(pdata->control_clock[14]);
err_clk17:
	clk_put(pdata->control_clock[13]);
err_clk16:
	clk_put(pdata->control_clock[12]);
err_clk15:
	clk_put(pdata->control_clock[11]);
err_clk14:
	clk_put(pdata->control_clock[10]);
err_clk13:
	clk_put(pdata->control_clock[9]);
err_clk12:
	clk_put(pdata->control_clock[8]);
err_clk11:
	clk_put(pdata->control_clock[7]);
err_clk10:
	clk_put(pdata->control_clock[6]);
err_clk9:
	clk_put(pdata->control_clock[5]);
err_clk8:
	clk_put(pdata->control_clock[4]);
err_clk7:
	clk_put(pdata->control_clock[3]);
err_clk6:
	clk_put(pdata->control_clock[2]);
err_clk5:
	clk_put(pdata->control_clock[1]);
err_clk4:
	clk_put(pdata->control_clock[0]);
err_clk3:
	clk_put(pdata->div_clock[1]);
err_clk2:
	clk_put(pdata->div_clock[0]);
err_clk1:
	return -EINVAL;
}

int exynos_fimc_is_cfg_clk(struct platform_device *pdev)
{
	struct exynos4_platform_fimc_is *pdata;
	unsigned int tmp;
printk("cfg clk ---------------------------------------------\n");
	pdata = to_fimc_is_plat(&pdev->dev);

	/* 1. MCUISP */
	__raw_writel(0x00000011, EXYNOS4_CLKDIV_ISP0);
	/* 2. ACLK_ISP */
	__raw_writel(0x00000030, EXYNOS4_CLKDIV_ISP1);
	/* 3. Set mux - CLK_SRC_TOP1(0x1003 C214) [24],[20]*/
	tmp = __raw_readl(EXYNOS4_CLKSRC_TOP1);
	tmp |= (0x1 << EXYNOS4_CLKDIV_TOP1_ACLK200_SUB_SHIFT |
		0x1 << EXYNOS4_CLKDIV_TOP1_ACLK400_MCUISP_SUB_SHIFT);
	__raw_writel(tmp, EXYNOS4_CLKSRC_TOP1);

	/* For debugging */
	printk(KERN_INFO "FIMC-IS MUX TOP1 = 0x%08x\n",
					__raw_readl(EXYNOS4_CLKSRC_TOP1));
	printk(KERN_INFO "FIMC-IS DIV = 0x%08x, 0x%08x\n",
					__raw_readl(EXYNOS4_CLKDIV_ISP0),
					__raw_readl(EXYNOS4_CLKDIV_ISP1));
	/* 4. UART-ISP */
	clk_set_parent(pdata->div_clock[UART_ISP_RATIO],
					pdata->div_clock[UART_ISP_SEL]);
	clk_set_rate(pdata->div_clock[UART_ISP_RATIO], 50 * 1000000);

	return 0;
}

int exynos_fimc_is_clk_on(struct platform_device *pdev)
{
	struct exynos4_platform_fimc_is *pdata;
	int i;
	pdata = to_fimc_is_plat(&pdev->dev);

	/* 1. CLK_GATE_IP_ISP (0x1003 C938)*/
	clk_enable(pdata->div_clock[UART_ISP_RATIO]);

	/* 2. CLK_GATE_IP_ISP0, CLK_GATE_IP_ISP1 (0x1004 8800) (0x1004 8804)*/
	for (i = 11; i < (EXYNOS4_FIMC_IS_MAX_CONTROL_CLOCKS ); i++)
		clk_enable(pdata->control_clock[i]);
	/* In case of CMA, clocks related system MMU off */
	/* In case of CMA, clocks related system MMU off */
	return 0;
}

int exynos_fimc_is_clk_off(struct platform_device *pdev)
{
	struct exynos4_platform_fimc_is *pdata;
	int i;
	pdata = to_fimc_is_plat(&pdev->dev);
	for (i = 11; i < (EXYNOS4_FIMC_IS_MAX_CONTROL_CLOCKS ); i++)
		clk_disable(pdata->control_clock[i]);

	/* 1. CLK_GATE_IP_ISP (0x1003 C938)*/
	clk_disable(pdata->div_clock[UART_ISP_RATIO]);

	return 0;
}

int exynos_fimc_is_clk_put(struct platform_device *pdev)
{
	struct exynos4_platform_fimc_is *pdata;
	int i;
	pdata = to_fimc_is_plat(&pdev->dev);

	for (i = 0; i < EXYNOS4_FIMC_IS_MAX_DIV_CLOCKS; i++)
		clk_put(pdata->div_clock[i]);
	for (i = 0; i < EXYNOS4_FIMC_IS_MAX_CONTROL_CLOCKS; i++)
		clk_put(pdata->control_clock[i]);
	return 0;
}

/*------------------------------------------------------*/
/*		Exynos5 series - FIMC-IS		*/
/*------------------------------------------------------*/
void exynos5_fimc_is_cfg_gpio(struct platform_device *pdev)
{
	int ret;

	/* 1. UART setting for FIMC-IS */
	ret = gpio_request(EXYNOS5_GPE0(0), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(0), (0x2<<0));
	s3c_gpio_setpull(EXYNOS5_GPE0(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(0));

	ret = gpio_request(EXYNOS5_GPE0(1), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_1 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(1), (0x2<<4));
	s3c_gpio_setpull(EXYNOS5_GPE0(1), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(1));

	ret = gpio_request(EXYNOS5_GPE0(2), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_2 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(2), (0x3<<8));
	s3c_gpio_setpull(EXYNOS5_GPE0(2), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(2));

	ret = gpio_request(EXYNOS5_GPE0(3), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_3 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(3), (0x3<<12));
	s3c_gpio_setpull(EXYNOS5_GPE0(3), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(3));

	ret = gpio_request(EXYNOS5_GPE0(4), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_4 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(4), (0x3<<16));
	s3c_gpio_setpull(EXYNOS5_GPE0(4), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(4));

	ret = gpio_request(EXYNOS5_GPE0(5), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_5 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(5), (0x3<<20));
	s3c_gpio_setpull(EXYNOS5_GPE0(5), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(5));

	ret = gpio_request(EXYNOS5_GPE0(6), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_6 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(6), (0x3<<24));
	s3c_gpio_setpull(EXYNOS5_GPE0(6), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(6));

	ret = gpio_request(EXYNOS5_GPE0(7), "GPE0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE0_7 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE0(7), (0x3<<28));
	s3c_gpio_setpull(EXYNOS5_GPE0(7), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE0(7));

	ret = gpio_request(EXYNOS5_GPE1(0), "GPE1");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE1_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE1(0), (0x3<<0));
	s3c_gpio_setpull(EXYNOS5_GPE1(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE1(0));

	ret = gpio_request(EXYNOS5_GPE1(1), "GPE1");
	if (ret)
		printk(KERN_ERR "#### failed to request GPE1_1 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPE1(1), (0x3<<4));
	s3c_gpio_setpull(EXYNOS5_GPE1(1), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPE1(1));

	/* 2. GPIO setting for FIMC-IS */
	ret = gpio_request(EXYNOS5_GPF0(0), "GPF0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPF0_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPF0(0), (0x2<<0));
	s3c_gpio_setpull(EXYNOS5_GPF0(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPF0(0));

	ret = gpio_request(EXYNOS5_GPF0(1), "GPF0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPF0_1 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPF0(1), (0x2<<4));
	s3c_gpio_setpull(EXYNOS5_GPF0(1), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPF0(1));

	ret = gpio_request(EXYNOS5_GPF0(2), "GPF0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPF0_2 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPF0(2), (0x2<<8));
	s3c_gpio_setpull(EXYNOS5_GPF0(2), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPF0(2));

	ret = gpio_request(EXYNOS5_GPF0(0), "GPF0");
	if (ret)
		printk(KERN_ERR "#### failed to request GPF0_3 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPF0(3), (0x2<<12));
	s3c_gpio_setpull(EXYNOS5_GPF0(3), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPF0(3));

	ret = gpio_request(EXYNOS5_GPF1(0), "GPF1");
	if (ret)
		printk(KERN_ERR "#### failed to request GPF1_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPF1(0), (0x3<<0));
	s3c_gpio_setpull(EXYNOS5_GPF1(0), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPF1(0));

	ret = gpio_request(EXYNOS5_GPF1(1), "GPF1");
	if (ret)
		printk(KERN_ERR "#### failed to request GPF1_0 ####\n");
	s3c_gpio_cfgpin(EXYNOS5_GPF1(1), (0x3<<4));
	s3c_gpio_setpull(EXYNOS5_GPF1(1), S3C_GPIO_PULL_NONE);
	gpio_free(EXYNOS5_GPF1(1));

	/* CAM A port(b0010) : PCLK, VSYNC, HREF, CLK_OUT */
	s3c_gpio_cfgrange_nopull(EXYNOS5_GPH0(3), 1, S3C_GPIO_SFN(2));

	/* Camera A */
	ret = gpio_request(EXYNOS5_GPX1(2), "GPX1");
	if (ret)
		printk(KERN_ERR "#### failed to request GPX1_2 ####\n");

	s3c_gpio_setpull(EXYNOS5_GPX1(2), S3C_GPIO_PULL_NONE);
	gpio_direction_output(EXYNOS5_GPX1(2), 0);
	gpio_direction_output(EXYNOS5_GPX1(2), 1);
	gpio_free(EXYNOS5_GPX1(2));
}

int exynos5_fimc_is_cfg_clk(struct platform_device *pdev)
{
	struct clk *aclk_mcuisp = NULL;
	struct clk *aclk_266 = NULL;
	struct clk *aclk_mcuisp_div0 = NULL;
	struct clk *aclk_mcuisp_div1 = NULL;
	struct clk *aclk_266_div0 = NULL;
	struct clk *aclk_266_div1 = NULL;
	struct clk *aclk_266_mpwm = NULL;
	struct clk *sclk_uart_isp = NULL;
	struct clk *sclk_uart_isp_div = NULL;
	struct clk *mout_mpll = NULL;
	struct clk *sclk_mipi = NULL;
	struct clk *cam_src = NULL;
	struct clk *cam_A_clk = NULL;
	unsigned long mcu_isp_400;
	unsigned long isp_266;
	unsigned long isp_uart;
	unsigned long mipi;
	unsigned long epll;

	/*
	 * initialize Clocks
	*/

	printk(KERN_DEBUG "exynos5_fimc_is_cfg_clk\n");
	/* 1. MCUISP */
	aclk_mcuisp = clk_get(&pdev->dev, "aclk_400_isp");
	if (IS_ERR(aclk_mcuisp))
		return PTR_ERR(aclk_mcuisp);

	aclk_mcuisp_div0 = clk_get(&pdev->dev, "aclk_400_isp_div0");
	if (IS_ERR(aclk_mcuisp_div0))
		return PTR_ERR(aclk_mcuisp_div0);

	aclk_mcuisp_div1 = clk_get(&pdev->dev, "aclk_400_isp_div1");
	if (IS_ERR(aclk_mcuisp_div1))
		return PTR_ERR(aclk_mcuisp_div1);

	clk_set_rate(aclk_mcuisp_div0, 400 * 1000000);
	clk_set_rate(aclk_mcuisp_div1, 400 * 1000000);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp);
	printk(KERN_DEBUG "mcu_isp_400 : %ld\n", mcu_isp_400);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp_div0);
	printk(KERN_DEBUG "mcu_isp_400_div0 : %ld\n", mcu_isp_400);

	mcu_isp_400 = clk_get_rate(aclk_mcuisp_div1);
	printk(KERN_DEBUG "aclk_mcuisp_div1 : %ld\n", mcu_isp_400);

	clk_put(aclk_mcuisp);
	clk_put(aclk_mcuisp_div0);
	clk_put(aclk_mcuisp_div1);

	/* 2. ACLK_ISP */
	aclk_266 = clk_get(&pdev->dev, "aclk_266_isp");
	if (IS_ERR(aclk_266))
		return PTR_ERR(aclk_266);
	aclk_266_div0 = clk_get(&pdev->dev, "aclk_266_isp_div0");
	if (IS_ERR(aclk_266_div0))
		return PTR_ERR(aclk_266_div0);
	aclk_266_div1 = clk_get(&pdev->dev, "aclk_266_isp_div1");
	if (IS_ERR(aclk_266_div1))
		return PTR_ERR(aclk_266_div1);
	aclk_266_mpwm = clk_get(&pdev->dev, "aclk_266_isp_divmpwm");
	if (IS_ERR(aclk_266_mpwm))
		return PTR_ERR(aclk_266_mpwm);

	clk_set_rate(aclk_266_div0, 134 * 1000000);
	clk_set_rate(aclk_266_div1, 68 * 1000000);

	isp_266 = clk_get_rate(aclk_266);
	printk(KERN_DEBUG "isp_266 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_div0);
	printk(KERN_DEBUG "isp_266_div0 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_div1);
	printk(KERN_DEBUG "isp_266_div1 : %ld\n", isp_266);

	isp_266 = clk_get_rate(aclk_266_mpwm);
	printk(KERN_DEBUG "isp_266_mpwm : %ld\n", isp_266);

	clk_put(aclk_266);
	clk_put(aclk_266_div0);
	clk_put(aclk_266_div1);
	clk_put(aclk_266_mpwm);

	/* 3. UART-ISP */
	sclk_uart_isp = clk_get(&pdev->dev, "sclk_uart_src_isp");
	if (IS_ERR(sclk_uart_isp))
		return PTR_ERR(sclk_uart_isp);

	sclk_uart_isp_div = clk_get(&pdev->dev, "sclk_uart_isp");
	if (IS_ERR(sclk_uart_isp_div))
		return PTR_ERR(sclk_uart_isp_div);

	clk_set_parent(sclk_uart_isp_div, sclk_uart_isp);
	clk_set_rate(sclk_uart_isp_div, 50 * 1000000);

	isp_uart = clk_get_rate(sclk_uart_isp);
	printk(KERN_DEBUG "isp_uart : %ld\n", isp_uart);
	isp_uart = clk_get_rate(sclk_uart_isp_div);
	printk(KERN_DEBUG "isp_uart_div : %ld\n", isp_uart);

	clk_put(sclk_uart_isp);
	clk_put(sclk_uart_isp_div);

	/* MIPI-CSI */
	mout_mpll = clk_get(&pdev->dev, "mout_mpll_user");
	if (IS_ERR(mout_mpll))
		return PTR_ERR(mout_mpll);
	sclk_mipi = clk_get(&pdev->dev, "sclk_gscl_wrap");
	if (IS_ERR(sclk_mipi))
		return PTR_ERR(sclk_mipi);

	clk_set_parent(sclk_mipi, mout_mpll);
	clk_set_rate(sclk_mipi, 266 * 1000000);

	mipi = clk_get_rate(mout_mpll);
	printk(KERN_DEBUG "mipi_src : %ld\n", mipi);
	mipi = clk_get_rate(sclk_mipi);
	printk(KERN_DEBUG "mipi_div : %ld\n", mipi);

	clk_put(mout_mpll);
	clk_put(sclk_mipi);

	/* camera A */
	cam_src = clk_get(&pdev->dev, "xxti");
	if (IS_ERR(cam_src))
		return PTR_ERR(cam_src);
	cam_A_clk = clk_get(&pdev->dev, "sclk_cam0");
	if (IS_ERR(cam_A_clk))
		return PTR_ERR(cam_A_clk);

	epll = clk_get_rate(cam_src);
	printk(KERN_DEBUG "epll : %ld\n", epll);

	clk_set_parent(cam_A_clk, cam_src);
	clk_set_rate(cam_A_clk, 24 * 1000000);

	clk_put(cam_src);
	clk_put(cam_A_clk);

	return 0;
}

int exynos5_fimc_is_clk_on(struct platform_device *pdev)
{
	struct clk *gsc_ctrl = NULL;
	struct clk *isp_ctrl = NULL;
	struct clk *mipi_ctrl = NULL;
	struct clk *cam_A_clk = NULL;

	printk(KERN_DEBUG "exynos5_fimc_is_clk_on\n");

	gsc_ctrl = clk_get(&pdev->dev, "gscl");
	if (IS_ERR(gsc_ctrl))
		return PTR_ERR(gsc_ctrl);

	clk_enable(gsc_ctrl);
	clk_put(gsc_ctrl);

	isp_ctrl = clk_get(&pdev->dev, "isp");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_enable(isp_ctrl);
	clk_put(isp_ctrl);

	mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap");
	if (IS_ERR(mipi_ctrl))
		return PTR_ERR(mipi_ctrl);

	clk_enable(mipi_ctrl);
	clk_put(mipi_ctrl);

	cam_A_clk = clk_get(&pdev->dev, "sclk_cam0");
	if (IS_ERR(cam_A_clk))
		return PTR_ERR(cam_A_clk);

	clk_enable(cam_A_clk);
	clk_put(cam_A_clk);

	return 0;
}

int exynos5_fimc_is_clk_off(struct platform_device *pdev)
{
	struct clk *gsc_ctrl = NULL;
	struct clk *isp_ctrl = NULL;
	struct clk *mipi_ctrl = NULL;
	struct clk *cam_A_clk = NULL;

	printk(KERN_DEBUG "exynos5_fimc_is_clk_on\n");

	gsc_ctrl = clk_get(&pdev->dev, "gscl");
	if (IS_ERR(gsc_ctrl))
		return PTR_ERR(gsc_ctrl);

	clk_disable(gsc_ctrl);
	clk_put(gsc_ctrl);

	isp_ctrl = clk_get(&pdev->dev, "isp");
	if (IS_ERR(isp_ctrl))
		return PTR_ERR(isp_ctrl);

	clk_disable(isp_ctrl);
	clk_put(isp_ctrl);

	mipi_ctrl = clk_get(&pdev->dev, "gscl_wrap");
	if (IS_ERR(mipi_ctrl))
		return PTR_ERR(mipi_ctrl);

	clk_disable(mipi_ctrl);
	clk_put(mipi_ctrl);

	cam_A_clk = clk_get(&pdev->dev, "sclk_cam0");
	if (IS_ERR(cam_A_clk))
		return PTR_ERR(cam_A_clk);

	clk_disable(cam_A_clk);
	clk_put(cam_A_clk);

	return 0;
}
