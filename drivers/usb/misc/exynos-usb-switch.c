/*
 * exynos-usb-switch.c - USB switch driver for Exynos
 *
 * Copyright (c) 2010-2011 Samsung Electronics Co., Ltd.
 * Yulgon Kim <yulgon.kim@samsung.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>

#include <plat/devs.h>
#include <plat/ehci.h>

#include <mach/regs-clock.h>

#include "../gadget/s3c-hsotg.h"
#include "exynos-usb-switch.h"

#define DRIVER_DESC "Exynos USB Switch Driver"
#define SWITCH_WAIT_TIME	500
#define WAIT_TIMES		10

extern struct s3c_hsotg *dev_controller;
static const char switch_name[] = "exynos_usb_switch";

#if defined(CONFIG_BATTERY_SAMSUNG)
void exynos_usb_cable_connect(void)
{
	samsung_cable_check_status(1);
}

void exynos_usb_cable_disconnect(void)
{
	samsung_cable_check_status(0);
}
#endif

static void exynos_host_port_power_off(void)
{
#ifdef CONFIG_USB_EHCI_S5P
	s5p_ehci_port_power_off(&s5p_device_ehci);
#endif
#ifdef CONFIG_USB_OHCI_EXYNOS
	s5p_ohci_port_power_off(&exynos4_device_ohci);
#endif
}

static void __maybe_unused exynos_host_port_power_on(void)
{
#ifdef CONFIG_USB_EHCI_S5P
	s5p_ehci_port_power_on(&s5p_device_ehci);
#endif
#ifdef CONFIG_USB_OHCI_EXYNOS
	s5p_ohci_port_power_on(&exynos4_device_ohci);
#endif
}

static int is_host_detect(struct exynos_usb_switch *usb_switch)
{
	return !gpio_get_value(usb_switch->gpio_host_detect);
}

static int is_device_detect(struct exynos_usb_switch *usb_switch)
{
	return gpio_get_value(usb_switch->gpio_device_detect);
}

static void set_host_vbus(struct exynos_usb_switch *usb_switch, int value)
{
	gpio_set_value(usb_switch->gpio_host_vbus, value);
}

static int exynos_change_usb_mode(struct exynos_usb_switch *usb_switch,
				enum usb_cable_status mode)
{
	struct s3c_hsotg *udc = dev_controller;
	int ret = 0;

	if (atomic_read(&usb_switch->connect)) {
		if (mode == USB_DEVICE_ATTACHED || mode == USB_HOST_ATTACHED) {
			pr_err(
				"Skip requested mode (%d), current mode=%d\n",
				mode, atomic_read(&usb_switch->connect));
			return -EPERM;
		}
	} else {
		if (mode == USB_DEVICE_DETACHED || mode == USB_HOST_DETACHED) {
			pr_err(
				"Skip requested mode (%d), current mode=%d\n",
				mode, atomic_read(&usb_switch->connect));
			return -EPERM;
		}
	}

	switch (mode) {
	case USB_DEVICE_DETACHED:
		if (atomic_read(&usb_switch->usb_status) == USB_HOST_ATTACHED)
			break;
		udc->gadget.ops->pullup(&udc->gadget, 0);
		atomic_set(&usb_switch->connect, 0);
		atomic_set(&usb_switch->usb_status, USB_DEVICE_DETACHED);
		udc->connect = false;
#if defined(CONFIG_BATTERY_SAMSUNG)
		exynos_usb_cable_disconnect();
#endif
		break;
	case USB_DEVICE_ATTACHED:
		udc->gadget.ops->pullup(&udc->gadget, 1);
		atomic_set(&usb_switch->connect, 1);
		atomic_set(&usb_switch->usb_status, USB_DEVICE_ATTACHED);
		udc->connect = true;
#if defined(CONFIG_BATTERY_SAMSUNG)
		exynos_usb_cable_connect();
#endif
		break;
	case USB_HOST_DETACHED:
		if (atomic_read(&usb_switch->usb_status)
						== USB_DEVICE_ATTACHED)
			break;
#ifdef CONFIG_USB_OHCI_EXYNOS
		pm_runtime_put(&exynos4_device_ohci.dev);
#endif
#ifdef CONFIG_USB_EHCI_S5P
		pm_runtime_put(&s5p_device_ehci.dev);
#endif
		if (usb_switch->gpio_host_vbus)
			set_host_vbus(usb_switch, 0);

		enable_irq(usb_switch->device_detect_irq);
#if defined(CONFIG_BATTERY_SAMSUNG)
		exynos_usb_cable_disconnect();
#endif
		atomic_set(&usb_switch->connect, 0);
		atomic_set(&usb_switch->usb_status, USB_HOST_DETACHED);
		break;
	case USB_HOST_ATTACHED:
#if defined(CONFIG_BATTERY_SAMSUNG)
		exynos_usb_cable_connect();
#endif
		udelay(50);
		disable_irq(usb_switch->device_detect_irq);
		if (usb_switch->gpio_host_vbus)
			set_host_vbus(usb_switch, 1);

#ifdef CONFIG_USB_EHCI_S5P
		pm_runtime_get_sync(&s5p_device_ehci.dev);
#endif
#ifdef CONFIG_USB_OHCI_EXYNOS
		pm_runtime_get_sync(&exynos4_device_ohci.dev);
#endif
		atomic_set(&usb_switch->connect, 1);
		atomic_set(&usb_switch->usb_status, USB_HOST_ATTACHED);
		break;
	default:
		pr_err("Does not changed\n");
	}
	pr_info("usb cable = %d\n", mode);

	return ret;
}

static void exnos_usb_switch_worker(struct work_struct *work)
{
	struct exynos_usb_switch *usb_switch =
		container_of(work, struct exynos_usb_switch, switch_work);
	int cnt = 0;

	mutex_lock(&usb_switch->mutex);
	/* If already device detached or host_detected, */
	if (!is_device_detect(usb_switch) || is_host_detect(usb_switch))
		goto done;

	while (!pm_runtime_suspended(&s5p_device_ehci.dev)
#ifdef CONFIG_USB_OHCI_EXYNOS
		|| !pm_runtime_suspended(&exynos4_device_ohci.dev)) {
#else
							) {
#endif
		mutex_unlock(&usb_switch->mutex);
		msleep(SWITCH_WAIT_TIME);
		mutex_lock(&usb_switch->mutex);

		/* If already device detached or host_detected, */
		if (!is_device_detect(usb_switch) || is_host_detect(usb_switch))
			goto done;

		if (cnt++ > WAIT_TIMES) {
			pr_err("%s:device not attached by host\n",
				__func__);
			goto done;
		}

	}

	if (cnt > 1)
		pr_info("Device wait host power during %d\n", (cnt-1));

	/* Check Device, VBUS PIN high active */
	exynos_change_usb_mode(usb_switch, USB_DEVICE_ATTACHED);
done:
	mutex_unlock(&usb_switch->mutex);
}

static irqreturn_t exynos_host_detect_thread(int irq, void *data)
{
	struct exynos_usb_switch *usb_switch = data;

	mutex_lock(&usb_switch->mutex);

	if (is_host_detect(usb_switch))
		exynos_change_usb_mode(usb_switch, USB_HOST_ATTACHED);
	else
		exynos_change_usb_mode(usb_switch, USB_HOST_DETACHED);

	mutex_unlock(&usb_switch->mutex);

	return IRQ_HANDLED;
}

static irqreturn_t exynos_device_detect_thread(int irq, void *data)
{
	struct exynos_usb_switch *usb_switch = data;

	mutex_lock(&usb_switch->mutex);

	if (is_host_detect(usb_switch)) {
		pr_err("Unexpected situation\n");
	} else if (is_device_detect(usb_switch)) {
		if (usb_switch->gpio_host_vbus)
			exynos_change_usb_mode(usb_switch, USB_DEVICE_ATTACHED);
		else
			queue_work(usb_switch->workqueue,
						&usb_switch->switch_work);
	} else {
		/* VBUS PIN low */
		exynos_change_usb_mode(usb_switch, USB_DEVICE_DETACHED);
	}

	mutex_unlock(&usb_switch->mutex);

	return IRQ_HANDLED;
}

static int exynos_usb_status_init(struct exynos_usb_switch *usb_switch)
{
	if (atomic_read(&usb_switch->connect))
		pr_err("Already setg\n");
	else if (is_host_detect(usb_switch)) {
		mutex_lock(&usb_switch->mutex);
		exynos_change_usb_mode(usb_switch, USB_HOST_ATTACHED);
		mutex_unlock(&usb_switch->mutex);
	} else if (is_device_detect(usb_switch)) {
		if (usb_switch->gpio_host_vbus) {
			mutex_lock(&usb_switch->mutex);
			exynos_change_usb_mode(usb_switch, USB_DEVICE_ATTACHED);
			mutex_unlock(&usb_switch->mutex);
		} else {
			queue_work(usb_switch->workqueue,
						&usb_switch->switch_work);
		}
	} else {
		atomic_set(&usb_switch->connect, 0);
	}
	return 0;
}

#ifdef CONFIG_PM
static int exynos_usbswitch_suspend(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct exynos_usb_switch *usb_switch = platform_get_drvdata(pdev);

	if (atomic_read(&usb_switch->connect)) {
		mutex_lock(&usb_switch->mutex);
		if (atomic_read(&usb_switch->usb_status) == USB_HOST_ATTACHED)
			exynos_change_usb_mode(usb_switch, USB_HOST_DETACHED);
		else
			exynos_change_usb_mode(usb_switch,
							USB_DEVICE_DETACHED);
		mutex_unlock(&usb_switch->mutex);
	}

	return 0;
}

static int exynos_usbswitch_resume(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct exynos_usb_switch *usb_switch = platform_get_drvdata(pdev);

	exynos_usb_status_init(usb_switch);

	if (!atomic_read(&usb_switch->connect) && !usb_switch->gpio_host_vbus)
		exynos_host_port_power_off();

	return 0;
}
#else
#define exynos_usbswitch_suspend	NULL
#define exynos_usbswitch_resume		NULL
#endif

static int __devinit exynos_usbswitch_probe(struct platform_device *pdev)
{
	struct s5p_usbswitch_platdata *pdata = dev_get_platdata(&pdev->dev);
	struct device *dev = &pdev->dev;
	struct exynos_usb_switch *usb_switch;
	struct s3c_hsotg *udc = dev_controller;
	int irq;
	int ret;

	/* Defer the probe of switch driver if otg driver is not ready */
	if (!udc)
		return -EPROBE_DEFER;

	usb_switch = kzalloc(sizeof(struct exynos_usb_switch), GFP_KERNEL);
	if (!usb_switch) {
		ret = -ENOMEM;
		goto fail;
	}

	mutex_init(&usb_switch->mutex);
	usb_switch->workqueue = create_singlethread_workqueue("usb_switch");
	INIT_WORK(&usb_switch->switch_work, exnos_usb_switch_worker);
	spin_lock_init(&usb_switch->lock);

	if (!pdata) {
		dev_err(&pdev->dev, "No platform data defined\n");
		ret = -ENODEV;
		goto fail;
	}
	usb_switch->gpio_host_detect = pdata->gpio_host_detect;
	usb_switch->gpio_device_detect = pdata->gpio_device_detect;
	usb_switch->gpio_host_vbus = pdata->gpio_host_vbus;

	/* USB Device detect IRQ */
	irq = platform_get_irq(pdev, 1);
	if (!irq) {
		dev_err(&pdev->dev, "Failed to get IRQ\n");
		ret = -ENODEV;
		goto fail;
	}

	ret = request_threaded_irq(irq, NULL, exynos_device_detect_thread,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
			"DEVICE_DETECT", usb_switch);
	if (ret) {
		dev_err(dev, "Failed to allocate an DEVICE interrupt(%d)\n",
			irq);
		goto fail;
	}
	usb_switch->device_detect_irq = irq;

	/* USB Host detect IRQ */
	irq = platform_get_irq(pdev, 0);
	if (!irq) {
		dev_err(&pdev->dev, "Failed to get IRQ\n");
		ret = -ENODEV;
		goto fail;
	}

	ret = request_threaded_irq(irq, NULL, exynos_host_detect_thread,
		IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING | IRQF_ONESHOT,
			"HOST_DETECT", usb_switch);
	if (ret) {
		dev_err(dev, "Failed to allocate an HOST interrupt(%d)\n", irq);
		goto fail;
	}

	usb_switch->host_detect_irq = irq;
	exynos_usb_status_init(usb_switch);

	platform_set_drvdata(pdev, usb_switch);

	return ret;
fail:
	cancel_work_sync(&usb_switch->switch_work);
	destroy_workqueue(usb_switch->workqueue);
	mutex_destroy(&usb_switch->mutex);
	return ret;
}

static int __devexit exynos_usbswitch_remove(struct platform_device *pdev)
{
	struct exynos_usb_switch *usb_switch = platform_get_drvdata(pdev);
	struct device *dev = &pdev->dev;

	free_irq(usb_switch->device_detect_irq, dev);
	free_irq(usb_switch->device_detect_irq, dev);
	platform_set_drvdata(pdev, 0);

	cancel_work_sync(&usb_switch->switch_work);
	destroy_workqueue(usb_switch->workqueue);
	mutex_destroy(&usb_switch->mutex);
	kfree(usb_switch);

	return 0;
}

static const struct dev_pm_ops exynos_usbswitch_pm_ops = {
	.suspend                = exynos_usbswitch_suspend,
	.resume                 = exynos_usbswitch_resume,
};

static struct platform_driver exynos_usbswitch_driver = {
	.probe		= exynos_usbswitch_probe,
	.remove		= __devexit_p(exynos_usbswitch_remove),
	.driver		= {
		.name	= "exynos-usb-switch",
		.owner	= THIS_MODULE,
		.pm	= &exynos_usbswitch_pm_ops,
	},
};

module_platform_driver(exynos_usbswitch_driver);

MODULE_DESCRIPTION("Exynos USB switch driver");
MODULE_AUTHOR("<yulgon.kim@samsung.com>");
MODULE_LICENSE("GPL");
