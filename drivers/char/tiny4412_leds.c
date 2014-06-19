#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/moduleparam.h>
#include <linux/slab.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/delay.h>
 
#include <linux/gpio.h>
#include <mach/gpio.h>
#include <plat/gpio-cfg.h>


#define DEVICE_NAME "leds"

static int led_gpios[] = {
	EXYNOS4X12_GPM4(0),
	EXYNOS4X12_GPM4(1),
	EXYNOS4X12_GPM4(2),
	EXYNOS4X12_GPM4(3),
};

#define LED_NUM		ARRAY_SIZE(led_gpios)


static long tiny4412_leds_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	switch(cmd) {
		case 0:
		case 1:
			if (arg > LED_NUM) {
				return -EINVAL;
			}

			gpio_set_value(led_gpios[arg], !cmd);
			//printk(DEVICE_NAME": %d %d\n", arg, cmd);
			break;

		default:
			return -EINVAL;
	}

	return 0;
}

static struct file_operations tiny4412_led_dev_fops = {
	.owner			= THIS_MODULE,
	.unlocked_ioctl	= tiny4412_leds_ioctl,
};

static struct miscdevice tiny4412_led_dev = {
	.minor			= MISC_DYNAMIC_MINOR,
	.name			= DEVICE_NAME,
	.fops			= &tiny4412_led_dev_fops,
};

static int __init tiny4412_led_dev_init(void) {
	int ret;
	int i;

	for (i = 0; i < LED_NUM; i++) {
		ret = gpio_request(led_gpios[i], "LED");
		if (ret) {
			printk("%s: request GPIO %d for LED failed, ret = %d\n", DEVICE_NAME,
					led_gpios[i], ret);
			return ret;
		}

		s3c_gpio_cfgpin(led_gpios[i], S3C_GPIO_OUTPUT);
		gpio_set_value(led_gpios[i], 1);
	}

	ret = misc_register(&tiny4412_led_dev);

	printk(DEVICE_NAME"\tinitialized\n");

	return ret;
}

static void __exit tiny4412_led_dev_exit(void) {
	int i;

	for (i = 0; i < LED_NUM; i++) {
		gpio_free(led_gpios[i]);
	}

	misc_deregister(&tiny4412_led_dev);
}

module_init(tiny4412_led_dev_init);
module_exit(tiny4412_led_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("FriendlyARM Inc.");

