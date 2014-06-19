/* linux/arch/arm/mach-exynos4/cpuidle.c
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpuidle.h>
#include <linux/cpu_pm.h>
#include <linux/io.h>
#include <linux/export.h>
#include <linux/time.h>

#include <asm/proc-fns.h>
#include <asm/smp_scu.h>
#include <asm/suspend.h>
#include <asm/unified.h>
#include <asm/cpuidle.h>
#include <mach/regs-pmu.h>
#include <mach/regs-clock.h>
#include <mach/pmu.h>
#include <mach/asv.h>
#include <mach/smc.h>

#include <plat/cpu.h>
#include <plat/pm.h>
#include <plat/devs.h>
#ifdef CONFIG_SEC_WATCHDOG_RESET
#include <plat/regs-watchdog.h>
#endif
#include <mach/regs-audss.h>

#ifdef CONFIG_EXYNOS_LPA
#define LPA_ENABLE	1
#else
#define LPA_ENABLE	0
#endif

#ifdef CONFIG_ARM_TRUSTZONE
#define REG_DIRECTGO_ADDR	(S5P_VA_SYSRAM_NS + 0x24)
#define REG_DIRECTGO_FLAG	(S5P_VA_SYSRAM_NS + 0x20)
#else
#define REG_DIRECTGO_ADDR	(samsung_rev() == EXYNOS4210_REV_1_1 ? \
			S5P_INFORM7 : (samsung_rev() == EXYNOS4210_REV_1_0 ? \
			(S5P_VA_SYSRAM + 0x24) : S5P_INFORM0))
#define REG_DIRECTGO_FLAG	(samsung_rev() == EXYNOS4210_REV_1_1 ? \
			S5P_INFORM6 : (samsung_rev() == EXYNOS4210_REV_1_0 ? \
			(S5P_VA_SYSRAM + 0x20) : S5P_INFORM1))
#endif

#define S5P_CHECK_AFTR		0xfcba0d10

#define S3C_HSMMC_CLKCON	0x2c
#define S3C_HSMMC_CLOCK_CARD_EN 0x0004

#define MSHCI_STATUS		0x48
#define MSHCI_DATA_BUSY		0x1<<9
#define MSHCI_DATA_STAT_BUSY    0x1<<10
#define MSHCI_ENCLK		0x1

#define GPIO_OFFSET             0x20
#define GPIO_PUD_OFFSET         0x08
#define GPIO_CON_PDN_OFFSET     0x10
#define GPIO_PUD_PDN_OFFSET     0x14
#define GPIO_END_OFFSET         0x200

#ifdef	CONFIG_SND_SAMSUNG_ALP
extern bool srp_run_state;
#endif
enum hc_type {
	HC_SDHC,
	HC_MSHC,
};

void __iomem *reg_directgo_addr, *reg_directgo_flag;
static unsigned lpa_enable;

extern int exynos4_check_usb_op(void);
static int exynos4_enter_lowpower(struct cpuidle_device *dev,
				struct cpuidle_driver *drv,
				int index);

static struct cpuidle_state exynos4_cpuidle_set[] __initdata = {
	[0] = ARM_CPUIDLE_WFI_STATE,
	[1] = {
		.enter			= exynos4_enter_lowpower,
		.exit_latency		= 300,
		.target_residency	= 100000,
		.flags			= CPUIDLE_FLAG_TIME_VALID,
		.name			= "C1",
		.desc			= "ARM power down",
	},
};

static DEFINE_PER_CPU(struct cpuidle_device, exynos4_cpuidle_device);

static struct cpuidle_driver exynos4_idle_driver = {
	.name			= "exynos4_idle",
	.owner			= THIS_MODULE,
	.en_core_tk_irqen	= 1,
};

struct check_device_op {
	void __iomem            *base;
	struct platform_device  *pdev;
	enum hc_type            type;
};

static struct check_device_op chk_sdhc_op[] = {
#if defined(CONFIG_EXYNOS4_DEV_DWMCI)
	{.base = 0, .pdev = &exynos_device_dwmci, .type = HC_MSHC},
#endif
#if defined(CONFIG_S3C_DEV_HSMMC)
	{.base = 0, .pdev = &s3c_device_hsmmc0, .type = HC_SDHC},
#endif
#if defined(CONFIG_S3C_DEV_HSMMC1)
	{.base = 0, .pdev = &s3c_device_hsmmc1, .type = HC_SDHC},
#endif
#if defined(CONFIG_S3C_DEV_HSMMC2)
	{.base = 0, .pdev = &s3c_device_hsmmc2, .type = HC_SDHC},
#endif
#if defined(CONFIG_S3C_DEV_HSMMC3)
	{.base = 0, .pdev = &s3c_device_hsmmc3, .type = HC_SDHC},
#endif
};

#ifdef CONFIG_SEC_WATCHDOG_RESET
static struct sleep_save exynos4_aftr_save[] = {
        SAVE_ITEM(S3C2410_WTDAT),
        SAVE_ITEM(S3C2410_WTCNT),
        SAVE_ITEM(S3C2410_WTCON),
};
#endif

static struct sleep_save exynos4_lpa_save[] = {
	/* CMU side */
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_TOP),
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_CAM),
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_TV),
        SAVE_ITEM(EXYNOS4_CLKSRC_MASK_LCD0),
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_MAUDIO),
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_FSYS),
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_PERIL0),
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_PERIL1),
	SAVE_ITEM(EXYNOS4_CLKSRC_MASK_DMC),
#ifdef CONFIG_SEC_WATCHDOG_RESET
        SAVE_ITEM(S3C2410_WTDAT),
        SAVE_ITEM(S3C2410_WTCNT),
        SAVE_ITEM(S3C2410_WTCON),
#endif
};

static struct sleep_save exynos4_set_clksrc[] = {
	{ .reg = EXYNOS4_CLKSRC_MASK_TOP	, .val = 0x00000001, },
	{ .reg = EXYNOS4_CLKSRC_MASK_CAM	, .val = 0x11111111, },
	{ .reg = EXYNOS4_CLKSRC_MASK_TV		, .val = 0x00000111, },
	{ .reg = EXYNOS4_CLKSRC_MASK_LCD0	, .val = 0x00001111, },
	{ .reg = EXYNOS4_CLKSRC_MASK_MAUDIO	, .val = 0x00000001, },
	{ .reg = EXYNOS4_CLKSRC_MASK_FSYS	, .val = 0x01011111, },
	{ .reg = EXYNOS4_CLKSRC_MASK_PERIL0	, .val = 0x01111111, },
	{ .reg = EXYNOS4_CLKSRC_MASK_PERIL1	, .val = 0x01110111, },
	{ .reg = EXYNOS4_CLKSRC_MASK_DMC	, .val = 0x00010000, },
};

static void exynos4_gpio_conpdn_reg(void)
{
	void __iomem *gpio_base = S5P_VA_GPIO;
	unsigned int val;

	do {
		/* Keep the previous state in didle mode */
		__raw_writel(0xffff, gpio_base + GPIO_CON_PDN_OFFSET);

		/* Pull up-down state in didle is same as normal */
		val = __raw_readl(gpio_base + GPIO_PUD_OFFSET);
		__raw_writel(val, gpio_base + GPIO_PUD_PDN_OFFSET);

		gpio_base += GPIO_OFFSET;

		if (gpio_base == S5P_VA_GPIO + GPIO_END_OFFSET)
			gpio_base = S5P_VA_GPIO2;

	} while (gpio_base <= S5P_VA_GPIO2 + GPIO_END_OFFSET);

	/* set the GPZ */
	gpio_base = S5P_VA_GPIO3;
	__raw_writel(0xffff, gpio_base + GPIO_CON_PDN_OFFSET);

	val = __raw_readl(gpio_base + GPIO_PUD_OFFSET);
	__raw_writel(val, gpio_base + GPIO_PUD_PDN_OFFSET);

	return;
}

/* Ext-GIC nIRQ/nFIQ is the only wakeup source in AFTR */
static void exynos4_set_wakeupmask(void)
{
	__raw_writel(0x0000ff3e, S5P_WAKEUP_MASK);
}

static unsigned int g_pwr_ctrl, g_diag_reg;

static void save_cpu_arch_register(void)
{
	/*read power control register*/
	asm("mrc p15, 0, %0, c15, c0, 0" : "=r"(g_pwr_ctrl) : : "cc");
	/*read diagnostic register*/
	asm("mrc p15, 0, %0, c15, c0, 1" : "=r"(g_diag_reg) : : "cc");
	return;
}

static void restore_cpu_arch_register(void)
{
#if defined(CONFIG_ARM_TRUSTZONE)
	exynos_smc(SMC_CMD_C15RESUME, g_pwr_ctrl, g_diag_reg, 0);
#else
	/*write power control register*/
	asm("mcr p15, 0, %0, c15, c0, 0" : : "r"(g_pwr_ctrl) : "cc");
	/*write diagnostic register*/
	asm("mcr p15, 0, %0, c15, c0, 1" : : "r"(g_diag_reg) : "cc");
#endif
	return;
}

static int idle_finisher(unsigned long flags)
{
#if defined(CONFIG_ARM_TRUSTZONE)
	exynos_smc(SMC_CMD_CPU0AFTR, 0, 0, 0);
#else
	cpu_do_idle();
#endif
	return 1;
}

static int sdmmc_dev_num;

static int check_sdmmc_op(unsigned int ch)
{
	unsigned int reg1, reg2;
	void __iomem *base_addr;

	if (unlikely(ch >= sdmmc_dev_num)) {
		printk(KERN_ERR "Invalid ch[%d] for SD/MMC\n", ch);
		return 0;
	}

	if (chk_sdhc_op[ch].type == HC_SDHC) {
		base_addr = chk_sdhc_op[ch].base;
		/* Check CLKCON [2]: ENSDCLK */
		reg2 = readl(base_addr + S3C_HSMMC_CLKCON);
		return !!(reg2 & (S3C_HSMMC_CLOCK_CARD_EN));
	} else if (chk_sdhc_op[ch].type == HC_MSHC) {
		base_addr = chk_sdhc_op[ch].base;
		/* Check STATUS [9] for data busy */
		reg1 = readl(base_addr + MSHCI_STATUS);
		return (reg1 & (MSHCI_DATA_BUSY)) ||
			(reg1 & (MSHCI_DATA_STAT_BUSY));
	}

	return 0;
}

/* Check all sdmmc controller */
static int loop_sdmmc_check(void)
{
	unsigned int iter;

	for (iter = 0; iter < sdmmc_dev_num; iter++) {
		if (check_sdmmc_op(iter)) {
			printk(KERN_DEBUG "SDMMC [%d] working\n", iter);
			return 1;
		}
	}
	return 0;
}

static int check_power_domain(void)
{
	unsigned long tmp;

	tmp = __raw_readl(S5P_PMU_LCD0_CONF);
	if ((tmp & S5P_INT_LOCAL_PWR_EN) == S5P_INT_LOCAL_PWR_EN)
		return 1;

	tmp = __raw_readl(S5P_PMU_MFC_CONF);
	if ((tmp & S5P_INT_LOCAL_PWR_EN) == S5P_INT_LOCAL_PWR_EN)
		return 2;

	tmp = __raw_readl(S5P_PMU_G3D_CONF);
	if ((tmp & S5P_INT_LOCAL_PWR_EN) == S5P_INT_LOCAL_PWR_EN)
		return 3;

	tmp = __raw_readl(S5P_PMU_TV_CONF);
	if ((tmp & S5P_INT_LOCAL_PWR_EN) == S5P_INT_LOCAL_PWR_EN)
		return 6;
	
	tmp = __raw_readl(S5P_PMU_CAM_CONF);
	if ((tmp & S5P_INT_LOCAL_PWR_EN) == S5P_INT_LOCAL_PWR_EN)
		return 4;

	tmp = __raw_readl(S5P_PMU_GPS_CONF);
	if ((tmp & S5P_INT_LOCAL_PWR_EN) == S5P_INT_LOCAL_PWR_EN)
		return 5;

	return 0;
}

static int check_usb_op(void)
{
	return exynos4_check_usb_op();
}

static int exynos_check_operation(void)
{
	int ret;
	if (!lpa_enable)
		return 1;
#ifdef	CONFIG_SND_SAMSUNG_ALP
	if(!srp_run_state){
		return 1;
	}
#endif
	ret = check_power_domain();
	if (ret) {
		return 1;
	}

	ret = check_usb_op();
	if (ret) {
		return 1;
	}

	if (loop_sdmmc_check())
		return 1;

	return 0;
}

static int exynos4_enter_core0_aftr(struct cpuidle_device *dev,
				struct cpuidle_driver *drv,
				int index)
{
	unsigned long tmp, abb_val;
#ifdef CONFIG_SEC_WATCHDOG_RESET
        s3c_pm_do_save(exynos4_aftr_save, ARRAY_SIZE(exynos4_aftr_save));
#endif

 	exynos4_set_wakeupmask();

	/* Set value of power down register for aftr mode */
	exynos_sys_powerdown_conf(SYS_AFTR);

	__raw_writel(virt_to_phys(s3c_cpu_resume), reg_directgo_addr);
	__raw_writel(S5P_CHECK_AFTR, reg_directgo_flag);
	

	save_cpu_arch_register();

	/* Setting Central Sequence Register for power down mode */
	tmp = __raw_readl(S5P_CENTRAL_SEQ_CONFIGURATION);
	tmp &= ~S5P_CENTRAL_LOWPWR_CFG;
	__raw_writel(tmp, S5P_CENTRAL_SEQ_CONFIGURATION);

	/* Setting SEQ_OPTION register */
	tmp = S5P_USE_STANDBY_WFI0 | S5P_USE_STANDBY_WFE0;
	__raw_writel(tmp, S5P_CENTRAL_SEQ_OPTION);

#ifdef CONFIG_EXYNOS_ASV
	abb_val = exynos4x12_get_abb_member(ABB_ARM);
	exynos4x12_set_abb_member(ABB_ARM, ABB_MODE_085V);
#endif

	cpu_pm_enter();
	cpu_suspend(0, idle_finisher);

#ifdef CONFIG_SMP
#if defined(CONFIG_ARM_TRUSTZONE)
	if (!soc_is_exynos5250())
		scu_enable(S5P_VA_SCU);
#endif
#endif
	cpu_pm_exit();

#ifdef CONFIG_EXYNOS_ASV
	exynos4x12_set_abb_member(ABB_ARM, abb_val);
#endif

	restore_cpu_arch_register();

#ifdef CONFIG_SEC_WATCHDOG_RESET
        s3c_pm_do_restore_core(exynos4_aftr_save,
                                ARRAY_SIZE(exynos4_aftr_save));
#endif
	/*
	 * If PMU failed while entering sleep mode, WFI will be
	 * ignored by PMU and then exiting cpu_do_idle().
	 * S5P_CENTRAL_LOWPWR_CFG bit will not be set automatically
	 * in this situation.
	 */
	tmp = __raw_readl(S5P_CENTRAL_SEQ_CONFIGURATION);
	if (!(tmp & S5P_CENTRAL_LOWPWR_CFG)) {
		tmp |= S5P_CENTRAL_LOWPWR_CFG;
		__raw_writel(tmp, S5P_CENTRAL_SEQ_CONFIGURATION);
	}
	/* Clear wakeup state register */
	__raw_writel(0x0, S5P_WAKEUP_STAT);

	return index;
}

static int exynos4_enter_core0_lpa(struct cpuidle_device *dev,
				struct cpuidle_driver *drv,
				int index)
{
	unsigned long tmp, abb_val;
	/* Set wakeup sources */
	__raw_writel(0x3ff0000, S5P_WAKEUP_MASK);

	exynos4_gpio_conpdn_reg();

	s3c_pm_do_save(exynos4_lpa_save, ARRAY_SIZE(exynos4_lpa_save));
	s3c_pm_do_restore(exynos4_set_clksrc, ARRAY_SIZE(exynos4_set_clksrc));

	/* Set value of power down register for aftr mode */
	exynos_sys_powerdown_conf(SYS_LPA);

	save_cpu_arch_register();

	/* Store resume address and LPA flag */
	__raw_writel(virt_to_phys(s3c_cpu_resume), reg_directgo_addr);
	__raw_writel(S5P_CHECK_AFTR, reg_directgo_flag);

	/* Setting Central Sequence Register for power down mode */
	tmp = __raw_readl(S5P_CENTRAL_SEQ_CONFIGURATION);
	tmp &= ~S5P_CENTRAL_LOWPWR_CFG;
	__raw_writel(tmp, S5P_CENTRAL_SEQ_CONFIGURATION);

	/* Setting SEQ_OPTION register */
	tmp = S5P_USE_STANDBY_WFI0 | S5P_USE_STANDBY_WFE0;
	__raw_writel(tmp, S5P_CENTRAL_SEQ_OPTION);

#ifdef CONFIG_EXYNOS_ASV
	abb_val = exynos4x12_get_abb_member(ABB_ARM);
	exynos4x12_set_abb_member(ABB_ARM, ABB_MODE_085V);
#endif

	cpu_pm_enter();
	cpu_suspend(0, idle_finisher);

#ifdef CONFIG_SMP
#if defined(CONFIG_ARM_TRUSTZONE)
	if (!soc_is_exynos5250())
		scu_enable(S5P_VA_SCU);
#endif
#endif
	cpu_pm_exit();

#ifdef CONFIG_EXYNOS_ASV
	exynos4x12_set_abb_member(ABB_ARM, abb_val);
#endif

	restore_cpu_arch_register();

	/*
	 * If PMU failed while entering sleep mode, WFI will be
	 * ignored by PMU and then exiting cpu_do_idle().
	 * S5P_CENTRAL_LOWPWR_CFG bit will not be set automatically
	 * in this situation.
	 */
	tmp = __raw_readl(S5P_CENTRAL_SEQ_CONFIGURATION);
	if (!(tmp & S5P_CENTRAL_LOWPWR_CFG)) {
		tmp |= S5P_CENTRAL_LOWPWR_CFG;
		__raw_writel(tmp, S5P_CENTRAL_SEQ_CONFIGURATION);
	}

	s3c_pm_do_restore_core(exynos4_lpa_save, ARRAY_SIZE(exynos4_lpa_save));

	__raw_writel((1 << 28), S5P_PAD_RET_GPIO_OPTION);
	__raw_writel((1 << 28), S5P_PAD_RET_UART_OPTION);
        __raw_writel((1 << 28), S5P_PAD_RET_MMCA_OPTION);
        __raw_writel((1 << 28), S5P_PAD_RET_MMCB_OPTION);
        __raw_writel((1 << 28), S5P_PAD_RET_EBIA_OPTION);
        __raw_writel((1 << 28), S5P_PAD_RET_EBIB_OPTION);

	/* Clear wakeup state register */
	__raw_writel(0x0, S5P_WAKEUP_STAT);

	return index;
}

static int exynos4_enter_lowpower(struct cpuidle_device *dev,
				struct cpuidle_driver *drv,
				int index)
{
	int new_index = index;

	/* This mode only can be entered when other core's are offline */
	if (num_online_cpus() > 1)
		new_index = drv->safe_state_index;

	if (new_index == 0)
		return arm_cpuidle_simple_enter(dev, drv, new_index);

	if (exynos_check_operation())
		return exynos4_enter_core0_aftr(dev, drv, new_index);
	else
		return exynos4_enter_core0_lpa(dev, drv, new_index);
}

static int __init exynos4_init_cpuidle(void)
{
	int i, max_cpuidle_state, cpu_id;
	struct cpuidle_device *device;
	struct cpuidle_driver *drv = &exynos4_idle_driver;
	struct platform_device *pdev;
	struct resource *res;

	/* Setup cpuidle driver */
	drv->state_count = (sizeof(exynos4_cpuidle_set) /
				       sizeof(struct cpuidle_state));
	max_cpuidle_state = drv->state_count;
	for (i = 0; i < max_cpuidle_state; i++) {
		memcpy(&drv->states[i], &exynos4_cpuidle_set[i],
				sizeof(struct cpuidle_state));
	}
	drv->safe_state_index = 0;
	cpuidle_register_driver(&exynos4_idle_driver);

	for_each_cpu(cpu_id, cpu_online_mask) {
		device = &per_cpu(exynos4_cpuidle_device, cpu_id);
		device->cpu = cpu_id;

		if (cpu_id == 0)
			device->state_count = (sizeof(exynos4_cpuidle_set) /
					       sizeof(struct cpuidle_state));
		else
			device->state_count = 1;	/* Support IDLE only */

		if (cpuidle_register_device(device)) {
			printk(KERN_ERR "CPUidle register device failed\n,");
			return -EIO;
		}
	}

	if (soc_is_exynos4412()) {
#ifdef CONFIG_ARM_TRUSTZONE
		reg_directgo_addr = S5P_VA_SYSRAM_NS + 0x24;
		reg_directgo_flag = S5P_VA_SYSRAM_NS + 0x20;
#else
		reg_directgo_addr = S5P_INFORM0;
		reg_directgo_flag = S5P_INFORM1;
#endif
	} else {
		reg_directgo_addr = REG_DIRECTGO_ADDR;
		reg_directgo_flag = REG_DIRECTGO_FLAG;
	}

	lpa_enable = LPA_ENABLE;

	sdmmc_dev_num = ARRAY_SIZE(chk_sdhc_op);

	for (i = 0; i < sdmmc_dev_num; i++) {
		pdev = chk_sdhc_op[i].pdev;

		res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
		if (!res) {
			printk(KERN_ERR "failed to get iomem region\n");
			return -EINVAL;
		}

		chk_sdhc_op[i].base = ioremap(res->start, resource_size(res));

		if (!chk_sdhc_op[i].base) {
			printk(KERN_ERR "failed to map io region\n");
			return -EINVAL;
		}
	}

	return 0;
}
device_initcall(exynos4_init_cpuidle);
