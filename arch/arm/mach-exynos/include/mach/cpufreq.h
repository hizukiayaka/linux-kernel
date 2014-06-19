/* linux/arch/arm/mach-exynos/include/mach/cpufreq.h
 *
 * Copyright (c) 2010 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com
 *
 * EXYNOS - CPUFreq support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef EXYNOS_CPUFREQ_H
#define EXYNOS_CPUFREQ_H
enum cpufreq_level_index {
	L0, L1, L2, L3, L4,
	L5, L6, L7, L8, L9,
	L10, L11, L12, L13, L14,
	L15, L16, L17, L18, L19,
	L20,
};

struct exynos_dvfs_info {
	unsigned long	mpll_freq_khz;
	unsigned int	pll_safe_idx;
	unsigned int	pm_lock_idx;
	unsigned int	max_support_idx;
	unsigned int	min_support_idx;
	struct clk	*cpu_clk;
	unsigned int	*volt_table;
	struct cpufreq_frequency_table	*freq_table;
	void (*set_freq)(unsigned int, unsigned int);
	bool (*need_apll_change)(unsigned int, unsigned int);
};

#define MAX_CPU_FREQ 1600000
#ifdef CONFIG_ARM_EXYNOS_CPUFREQ
void exynos_cpufreq_lock_freq(bool lock_en, unsigned int freq);
#else
static void exynos_cpufreq_lock_freq(bool lock_en, unsigned int freq)
{
	return;
}
#endif

extern int exynos4210_cpufreq_init(struct exynos_dvfs_info *);
extern int exynos4x12_cpufreq_init(struct exynos_dvfs_info *);
extern int exynos5250_cpufreq_init(struct exynos_dvfs_info *);
#endif  /* EXYNOS_CPUFREQ_H */
