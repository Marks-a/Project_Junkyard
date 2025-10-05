#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/wait.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/slab.h>

#define DEVICE_NAME "module2_Testing_kernal"
#define BUFFER_SIZE 256

static dev_t devt;
static struct cdev module2_cdev;
static struct class *module2_class;

static char message[BUFFER_SIZE];
static size_t message_len;
static wait_queue_head_t read_queue;
static int data_available;
static struct task_struct *producer_thread;
    
static ssize_t module2_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    ssize_t tocopy;
    /* block until data is available */
    if (wait_event_interruptible(read_queue, data_available != 0))
        return -ERESTARTSYS;
    tocopy = min(count, message_len);
    if (copy_to_user(buf, message, tocopy))
        return -EFAULT;

    
    data_available = 0;
    message_len = 0;
    return tocopy;
}

static unsigned int module2_poll(struct file *file, poll_table *wait)
{
    unsigned int mask = 0;

    poll_wait(file, &read_queue, wait);

    if (data_available)
        mask |= POLLIN | POLLRDNORM;
    return mask;
}

static int module2_open(struct inode *inode, struct file *file)
{
    return 0;
}

static const struct file_operations module2_fops = {
    .owner = THIS_MODULE,
    .read = module2_read,
    .open = module2_open,
    .poll = module2_poll,
};

static int producer_fn(void *arg)
{
    int counter = 0;

    while (!kthread_should_stop()) {
        message_len = snprintf(message, BUFFER_SIZE, "module2 message %d\n", counter++);
        data_available = 1;
        wake_up_interruptible(&read_queue);

        if (kthread_should_stop())
            break;
        sleep(2000);
    }
    return 0;
}

static int __init module2_init(void)
{
    int ret;
    struct device *dev_ret;
    ret = alloc_chrdev_region(&devt, 0, 1, DEVICE_NAME);
    if (ret) {
        pr_err("module2: alloc_chrdev_region failed\n");
        return ret;
    }

    cdev_init(&module2_cdev, &module2_fops);
    module2_cdev.owner = THIS_MODULE;

    ret = cdev_add(&module2_cdev, devt, 1);
    if (ret) {
        pr_err("module2: cdev_add failed\n");
        unregister_chrdev_region(devt, 1);
        return ret;
    }

    module2_class = class_create(THIS_MODULE, DEVICE_NAME);
    if (IS_ERR(module2_class)) {
        pr_err("module1: class_create failed\n");
        cdev_del(&module2_cdev);
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(module2_class);
    }

    dev_ret = device_create(module2_class, NULL, devt, NULL, DEVICE_NAME);
    if (IS_ERR(dev_ret)) {
        pr_err("module1: device_create failed\n");
        class_destroy(module2_class);
        cdev_del(&module2_cdev);
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(dev_ret);
    }

    init_waitqueue_head(&read_queue);
    data_available = 0;
    message_len = 0;

    producer_thread = kthread_run(producer_fn, NULL, "module2_producer");
    if (IS_ERR(producer_thread)) {
        pr_err("module2: kthread_run failed\n");
        device_destroy(module2_class, devt);
        class_destroy(module2_class);
        cdev_del(&module2_cdev);
        unregister_chrdev_region(devt, 1);
        return PTR_ERR(producer_thread);
    }

    pr_info("module2: loaded, major=%d\n", MAJOR(devt));
    return 0;
}

static void __exit module2_exit(void)
{
    if (producer_thread)
        kthread_stop(producer_thread);

    device_destroy(module2_class, devt);
    class_destroy(module2_class);
    cdev_del(&module2_cdev);
    unregister_chrdev_region(devt, 1);

    pr_info("module2: unloaded\n");
}

module_init(module2_init);
module_exit(module2_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("example");
MODULE_DESCRIPTION("Example messages for userspace");
