#include <linux/kernel.h>
#include <linux/module.h>

static int __init tiny4412_hello_module_init(void)
{
    printk("Hello, Tiny4412 module is installed !\n");
    return 0;
}

static void __exit tiny4412_hello_module_cleanup(void)
{
    printk("Good-bye, Tiny4412 module was removed!\n");
}

module_init(tiny4412_hello_module_init);
module_exit(tiny4412_hello_module_cleanup);

MODULE_LICENSE("GPL");

