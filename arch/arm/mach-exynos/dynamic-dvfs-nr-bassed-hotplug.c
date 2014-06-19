/* linux/arch/arm/mach-exynos/dynamic-dvfs-nr-bassed-hotplug.c
 *
 * Yadwinder Singh Brar <yadi.brar@samsung.com>
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *		http://www.samsung.com/
 *
 * EXYNOS4 - Integrated DVFS CPU hotplug
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/sched.h>
#include <linux/cpufreq.h>
#include <linux/cpu.h>
#include <linux/err.h>
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/suspend.h>
#include <linux/io.h>

#include <plat/cpu.h>

#define CPU_0			0
#define CPU_NO			1
#define DEFAULT_WORK_DELAY	100
#define INITIAL_WORK_DELAY	10

enum events {
	CPU_FREQ_POSTCHANGE,
	SUSPEND,
	RESUME,
};

struct cpu_hotplug_policy {
	char *name;
	void (*target) (void *, unsigned int event);
};

struct cpu_hotplug_dvfs {
	struct cpufreq_freqs		freqs;
	unsigned int			freq_trg_out;
	unsigned int			freq_trg_in;
	struct delayed_work		work;
	unsigned int			work_delay;
	spinlock_t			lock;
} cpu_p;

struct cpu_hotplug {
	unsigned int			can_hotplug;
	struct cpu_hotplug_policy	policy;
	void				*data;		/* policy's data */
} h_cpu;

static u32 hotplug_in_cnt;		/* frequency hotplug in trigger */
static unsigned int hotplug_out_cnt;		/* frequency hotplug out trigger */

#define NR2_COUNTER_INC		0x00000001
#define NR3_COUNTER_INC		0x00000100
#define NR4_COUNTER_INC		0x00010000
#define TRIGGER_COUNT_INC	0x01000000
#define NR2_COUNTER_MASK	0x000000FF
#define NR3_COUNTER_MASK	0x0000FF00
#define NR4_COUNTER_MASK	0x00FF0000
#define TRIGGER_COUNT_MASK	0xFF000000

#ifdef CONFIG_EXYNOS_DVFS_NR_RUNNING_POLICY
static void dvfs_nr_based_hotplug(void *data, unsigned int event)
{
	struct cpu_hotplug_dvfs *p_data = (struct cpu_hotplug_dvfs *)data;
	unsigned int online = 0;
	unsigned int new_freq, nr_run;

	spin_lock(&p_data->lock);
	new_freq = p_data->freqs.new;

	spin_unlock(&p_data->lock);

	if (event != CPU_FREQ_POSTCHANGE) {
		if (event == SUSPEND) {
			goto suspend;
		} else {
			if (event == RESUME)
				goto resume;
			else
				pr_err("%s: Unknown event\n", __func__);
		}
	}

	online |= cpu_online(3);
	online |= cpu_online(2) << 1;
	online |= cpu_online(1) << 2;

	nr_run = nr_running();

	if (new_freq <= p_data->freq_trg_out) {
		hotplug_in_cnt = 0;

		switch (nr_run) {
		case 0:
		case 1:
			hotplug_out_cnt += (NR2_COUNTER_INC + NR3_COUNTER_INC +
					    NR4_COUNTER_INC);
			break;
		case 2:
			hotplug_out_cnt += (NR3_COUNTER_INC + NR4_COUNTER_INC);
			break;
		case 3:
			hotplug_out_cnt += NR4_COUNTER_INC;
			break;
		default:
			hotplug_out_cnt = 0;
		}

		goto hotplug_out;
	} else {
		if(new_freq >= p_data->freq_trg_in) {
			hotplug_out_cnt = 0;

			switch (nr_run) {
			case 0:
			case 1:
				hotplug_in_cnt = 0;
				break;
			case 2:
				hotplug_in_cnt += NR2_COUNTER_INC;
				break;
			case 3:
				hotplug_in_cnt += (NR2_COUNTER_INC +
							NR3_COUNTER_INC);
				break;
			default:
				hotplug_in_cnt += (NR2_COUNTER_INC +
					     NR3_COUNTER_INC + NR4_COUNTER_INC);
				break;
			}
			goto hotplug_in;
		} else {
			/* freq is in normal range, reset counters and return */
			hotplug_in_cnt = 0;
			hotplug_out_cnt = 0;
			return ;
		}
	}

hotplug_in:
	switch (online) {
	case 0:
		if ((hotplug_in_cnt & NR2_COUNTER_MASK) >= 0x3) {
			hotplug_in_cnt = 0;
			cpu_up(1);
		} else
			hotplug_in_cnt &= NR2_COUNTER_MASK;
		goto schd_work;

	case 4:
		if ((hotplug_in_cnt & NR3_COUNTER_MASK) >= 0x400) {
			hotplug_in_cnt = 0;
			cpu_up(2);
		} else
			hotplug_in_cnt &= NR3_COUNTER_MASK;
		goto schd_work;

	case 6:
		if ((hotplug_in_cnt & NR4_COUNTER_MASK) > 0x50000) {
			hotplug_in_cnt = 0;
			if (cpu_up(3))
				goto schd_work;
		} else {
			hotplug_in_cnt &= NR4_COUNTER_MASK;
			goto schd_work;
		}

	case 7:
		hotplug_in_cnt = 0;
		return ;
	default:
		pr_err("Unknown condition\n");
		return ;
	}

hotplug_out:
	switch (online) {
	case 7:
		if ((hotplug_out_cnt & NR4_COUNTER_MASK) > 0x50000) {
			hotplug_out_cnt = 0;
			cpu_down(3);
		} else
			hotplug_out_cnt &= NR4_COUNTER_MASK;
		goto schd_work;

	case 6:
		if ((hotplug_out_cnt & NR3_COUNTER_MASK) > 0x400) {
			hotplug_out_cnt = 0;
			if(cpu_down(2))
				printk("Unable to hotplug out cpu 2>>>>>>\n");
		} else
			hotplug_out_cnt &= NR3_COUNTER_MASK;
		goto schd_work;

	case 4:
		if ((hotplug_out_cnt & NR2_COUNTER_MASK) > 0x3) {
			hotplug_out_cnt = 0;
			if (cpu_down(1))
				goto schd_work;
		} else {
			hotplug_out_cnt &= NR2_COUNTER_MASK;
			goto schd_work;
		}

	case 0:
		hotplug_out_cnt = 0;
		return ;

	default:
		pr_err("Unknown condition\n");
		return ;
	}

resume:
schd_work:
	schedule_delayed_work_on(0, &p_data->work, p_data->work_delay);
	return ;
suspend:
	hotplug_out_cnt = 0;
	hotplug_in_cnt = 0;
	cancel_delayed_work_sync(&p_data->work);
	return ;
}
#endif

#ifdef CONFIG_EXYNOS_DVFS_NR_RUNNING_QUICK_POLICY
static void dvfs_nr_based_hotplug(void *data, unsigned int event)
{
	struct cpu_hotplug_dvfs *p_data = (struct cpu_hotplug_dvfs *)data;
	unsigned int online = 0;
	unsigned int new_freq, nr_run;

	spin_lock(&p_data->lock);
	new_freq = p_data->freqs.new;

	spin_unlock(&p_data->lock);

	if (event != CPU_FREQ_POSTCHANGE) {
		if (event == SUSPEND) {
			goto suspend;
		} else {
			if (event == RESUME)
				goto resume;
			else
				pr_err("%s: Unknown event\n", __func__);
		}
	}

	online |= cpu_online(3);
	online |= cpu_online(2) << 1;
	online |= cpu_online(1) << 2;

	nr_run = nr_running();

	if (new_freq <= p_data->freq_trg_out) {
		hotplug_in_cnt = 0;

		switch (nr_run) {
		case 0:
		case 1:
			hotplug_out_cnt += (NR2_COUNTER_INC + NR3_COUNTER_INC +
					    NR4_COUNTER_INC);
			break;
		case 2:
			hotplug_out_cnt += (NR3_COUNTER_INC + NR4_COUNTER_INC);
			break;
		case 3:
			hotplug_out_cnt += NR4_COUNTER_INC;
			break;
		default:
			hotplug_out_cnt = 0;
		}

		goto hotplug_out;
	} else {
		if (new_freq >= p_data->freq_trg_in) {
			hotplug_out_cnt = 0;

			switch (nr_run) {
			case 0:
			case 1:
				hotplug_in_cnt = 0;
				break;
			case 2:
				hotplug_in_cnt += NR2_COUNTER_INC;
				hotplug_in_cnt &= NR2_COUNTER_MASK;
				break;
			case 3:
				hotplug_in_cnt += (NR2_COUNTER_INC +
							NR3_COUNTER_INC);
				hotplug_in_cnt &= ~NR4_COUNTER_MASK;
				break;
			default:
				hotplug_in_cnt += (NR2_COUNTER_INC +
					     NR3_COUNTER_INC + NR4_COUNTER_INC);
				break;
			}
			goto hotplug_in;
		} else {
			/* freq is in normal range, reset counters and return */
			hotplug_in_cnt = 0;
			hotplug_out_cnt = 0;
			p_data->work_delay =
					msecs_to_jiffies(INITIAL_WORK_DELAY);
			return ;
		}
	}

hotplug_in:
	switch (online) {
	case 0:
	case 1:
	case 2:
		if ((hotplug_in_cnt & NR2_COUNTER_MASK) >= 0x1) {
			if (!cpu_up(1))
				online |= 4;
		} else {
			goto schd_work;
		}
	case 4:
	case 5:
		hotplug_in_cnt &= ~NR2_COUNTER_MASK;
		if ((hotplug_in_cnt & NR3_COUNTER_MASK) >= 0x100) {
			if (!cpu_up(2))
				online |= 2;
		} else {
			goto schd_work;
		}
	case 6:
		hotplug_in_cnt &= NR4_COUNTER_MASK;
		if ((hotplug_in_cnt & NR4_COUNTER_MASK) >= 0x10000) {
			if (!cpu_up(3))
				online |= 1;
		} else {
			goto schd_work;
		}

		if (online < 7)
			goto schd_work;
	case 7:
		hotplug_in_cnt = 0;
		return ;
	default:
		pr_err("Unknown condition\n");
		return ;
	}

hotplug_out:
	switch (online) {
	case 7:
		if ((hotplug_out_cnt & NR4_COUNTER_MASK) > 0x50000) {
			hotplug_out_cnt = 0;
			cpu_down(3);
		} else
			hotplug_out_cnt &= NR4_COUNTER_MASK;
		goto schd_work;

	case 6:
		if ((hotplug_out_cnt & NR3_COUNTER_MASK) > 0x400) {
			hotplug_out_cnt = 0;
			cpu_down(2);
		} else
			hotplug_out_cnt &= NR3_COUNTER_MASK;
		goto schd_work;

	case 4:
		if ((hotplug_out_cnt & NR2_COUNTER_MASK) > 0x3) {
			hotplug_out_cnt = 0;
			if (cpu_down(1))
				goto schd_work;
		} else {
			hotplug_out_cnt &= NR2_COUNTER_MASK;
			goto schd_work;
		}

	case 0:
		hotplug_out_cnt = 0;
		p_data->work_delay = msecs_to_jiffies(INITIAL_WORK_DELAY);
		return ;

	default:
		pr_err("Unknown condition\n");
		return ;
	}

resume:
schd_work:
	schedule_delayed_work_on(0, &p_data->work, p_data->work_delay);
	return ;
suspend:
	hotplug_out_cnt = 0;
	hotplug_in_cnt = 0;
	cancel_delayed_work_sync(&p_data->work);
	return ;
}
#endif

static void hotplug_work(struct work_struct *h_work)
{
	h_cpu.policy.target(h_cpu.data, CPU_FREQ_POSTCHANGE);
}

static int hotplug_cpufreq_transition(struct notifier_block *nb,
					unsigned long val, void *data)
{
	struct cpufreq_freqs *freqs = (struct cpufreq_freqs *)data;

	if ((val == CPUFREQ_POSTCHANGE)) {
		spin_lock(&cpu_p.lock);
		cpu_p.freqs.old = freqs->old;
		cpu_p.freqs.new = freqs->new;
		cpu_p.freqs.cpu = freqs->cpu;
		spin_unlock(&cpu_p.lock);
	}

	if (h_cpu.can_hotplug)
		schedule_delayed_work_on(0, &cpu_p.work, cpu_p.work_delay);
		cpu_p.work_delay = msecs_to_jiffies(DEFAULT_WORK_DELAY);
	return 0;
}

static struct notifier_block dvfs_hotplug = {
	.notifier_call = hotplug_cpufreq_transition,
};

static int hotplug_pm_transition(struct notifier_block *nb,
					unsigned long val, void *data)
{
	switch (val) {
	case PM_SUSPEND_PREPARE:
			h_cpu.can_hotplug = 0;
			h_cpu.policy.target(h_cpu.data, SUSPEND);
		break;
	case PM_POST_RESTORE:
	case PM_POST_SUSPEND:
			h_cpu.can_hotplug = 1;
			h_cpu.policy.target(h_cpu.data, RESUME);
		break;
	}

	return 0;
}

static struct notifier_block pm_hotplug = {
	.notifier_call = hotplug_pm_transition,
};

static int set_hotplug_policy(struct cpu_hotplug_policy *policy) {
#ifdef CONFIG_EXYNOS_DVFS_NR_RUNNING_POLICY
	policy->name = "DVFS_NR_BASED_HOTPLUG";
	policy->target = dvfs_nr_based_hotplug;
#endif
#ifdef CONFIG_EXYNOS_DVFS_NR_RUNNING_QUICK_POLICY
	policy->name = "DVFS_NR_BASED_HOTPLUG";
	policy->target = dvfs_nr_based_hotplug;
#endif
	if (!policy->target)
		return(-EINVAL);

	return 0;
}

void *hotplug_policy_init(void)
{
	int ret = 0;
/*	int j,index = 0;
	struct cpufreq_frequency_table	*freq_table;
*/
	spin_lock_init(&cpu_p.lock);
	INIT_DELAYED_WORK(&cpu_p.work, hotplug_work);
	cpu_p.work_delay = msecs_to_jiffies(INITIAL_WORK_DELAY);

/*	freq_table = cpufreq_frequency_get_table(0);
		if (IS_ERR(freq_table)) {
			pr_err("%s: cpufreq_table for CPU:%d"
				" not found\n", __func__, 0);
			return freq_table;
		}

	for (j = 0; freq_table[j].frequency != CPUFREQ_TABLE_END; j++)
		index += 1;
	index /= 2;
	index += index / 2;

	cpu_p.freq_trg_out = freq_table[0].frequency;
	cpu_p.freq_trg_in = freq_table[index].frequency;
*/
	cpu_p.freq_trg_out = 200000;		/* tunnable */
	cpu_p.freq_trg_in = 800000;		/* tunnable */

	ret = register_pm_notifier(&pm_hotplug);
	if (ret)
		return ERR_PTR(-EINVAL);

	ret = cpufreq_register_notifier(&dvfs_hotplug,
					CPUFREQ_TRANSITION_NOTIFIER);
	if (ret)
		return ERR_PTR(-EINVAL);

	ret = set_hotplug_policy(&h_cpu.policy);

	if (ret) {
		pr_err("%s: Policy not defined\n", __func__);
		return ERR_PTR(ret);
	}

	pr_info("%s: intialised with policy : %s", __func__,
		h_cpu.policy.name);

	return ((void *)&cpu_p);
}

static int __init dvfs_based_hotplug_init(void)
{
	h_cpu.can_hotplug = 1;

	h_cpu.data = hotplug_policy_init();
	if (IS_ERR(h_cpu.data))
		return PTR_ERR(h_cpu.data);

	return 0;
}
late_initcall_sync(dvfs_based_hotplug_init);
