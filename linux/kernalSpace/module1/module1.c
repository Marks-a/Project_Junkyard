#include <linux/module.h>   
#include <linux/kernel.h> 

#define DEVICE_NAME "basic_Hello"
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Module1.c");
MODULE_DESCRIPTION("Simple Hello Kernel module");


static int __init hello(void) {
    pr_info("Hello, World!\n");
    return 0;
}

static void __exit goodbye(void) {
    pr_info("Goodbye, World!\n");
}

module_init(hello);
module_exit(goodbye);


