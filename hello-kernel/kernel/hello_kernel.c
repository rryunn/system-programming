#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/ioctl.h>

#include "../include/abi.h"

#define STUDENT_ID 202224210 // Don't modify

MODULE_LICENSE("GPL");
MODULE_AUTHOR("202224210");
MODULE_DESCRIPTION("Hello Kernel ioctl example (IO/IOR/IOW/IOWR + ABI)");
MODULE_VERSION("0.1");

static dev_t _dev;
static struct cdev _cdev;
static struct class *_class;

static int  hello_open   (struct inode *inode, struct file *file);
static int  hello_release(struct inode *inode, struct file *file);
static long hello_ioctl  (struct file *file, unsigned int cmd, unsigned long arg);

static int  __init hello_init(void);
static void __exit hello_exit(void);

static const struct file_operations _fops = {
    .owner          = THIS_MODULE,
    .open           = hello_open,
    .release        = hello_release,
    .unlocked_ioctl = hello_ioctl,
};

module_init(hello_init);
module_exit(hello_exit);

static int current_id = STUDENT_ID;

static int hello_open(struct inode *inode, struct file *file) {
    pr_info("hello_kernel: device opened\n");
    return 0;
}

static int hello_release(struct inode *inode, struct file *file) {
    pr_info("hello_kernel: device closed\n");
    return 0;
}

static long hello_ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {

    case IOCTL_PRINT:    /* _IO */
        pr_info("hello_kernel: Hello, User! I'm %d\n", current_id);
        return 0;

    case IOCTL_GET_ID:   /* _IOR */
    {
        int tmp = current_id;
        if (copy_to_user((void __user *)arg, &tmp, sizeof(tmp)))
            return -EFAULT;
        pr_info("hello_kernel: GET_ID -> %d\n", tmp);
        return 0;
    }

    case IOCTL_SET_ID:   /* _IOW */
    {
        int new_id;
        if (copy_from_user(&new_id, (void __user *)arg, sizeof(new_id)))
            return -EFAULT;
        pr_info("hello_kernel: SET_ID %d -> %d\n", current_id, new_id);
        current_id = new_id;
        return 0;
    }

    default:
        return -ENOTTY;
    }
}


static int __init hello_init(void) {
 int ret;
    current_id = STUDENT_ID;

    ret = alloc_chrdev_region(&_dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("hello_kernel: failed to alloc_chrdev_region\n");
        return ret;
    }

    cdev_init(&_cdev, &_fops);
    _cdev.owner = THIS_MODULE;

    ret = cdev_add(&_cdev, _dev, 1);
    if (ret < 0) {
        pr_err("hello_kernel: failed to cdev_add\n");
        unregister_chrdev_region(_dev, 1);
        return ret;
    }

    _class = class_create(CLASS_NAME);  
    if (IS_ERR(_class)) {
        pr_err("hello_kernel: failed to create class\n");
        cdev_del(&_cdev);
        unregister_chrdev_region(_dev, 1);
        return PTR_ERR(_class);
    }

    if (IS_ERR(device_create(_class, NULL, _dev, NULL, DEVICE_NAME))) {
        pr_err("hello_kernel: failed to create device\n");
        class_destroy(_class);
        cdev_del(&_cdev);
        unregister_chrdev_region(_dev, 1);
        return -EINVAL;
    }

    pr_info("hello_kernel: module loaded (major=%d, minor=%d)\n",
            MAJOR(_dev), MINOR(_dev));
    return 0;
}

static void __exit hello_exit(void) {
    device_destroy(_class, _dev);
    class_destroy(_class);
    cdev_del(&_cdev);
    unregister_chrdev_region(_dev, 1);
 pr_info("hello_kernel: module unloaded\n");
}