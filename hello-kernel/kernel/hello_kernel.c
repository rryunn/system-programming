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
    //지금은 그냥 print역할로, 별도의 데이터 없이 현재 커널에 저장된 id 출력
        pr_info("hello_kernel: Hello, User! I'm %d\n", current_id);
        return 0;

    case IOCTL_GET_ID:   /* _IOR */
    //kernel ->user방향으로 데이터를 받을 때.
    //커널의 current id를 user가 읽어오는 방식
    {
        int tmp = current_id; //커널의 current_id를 tmp에 복사
        if (copy_to_user((void __user *)arg, &tmp, sizeof(tmp))) 
            return -EFAULT; // copy to user: 커널 공간에서 유저 공간으로 데이터 복사
        pr_info("hello_kernel: GET_ID -> %d\n", tmp);
        return 0;
    }

    case IOCTL_SET_ID:   /* _IOW */
    //user -> kernel방향으로 데이터를 보낼 때.
    //user가 int id를 커널에 보내서 current_id로 설정하는 방식
    {
        int new_id;
        if (copy_from_user(&new_id, (void __user *)arg, sizeof(new_id)))
            return -EFAULT; // copy from user: 유저 공간에서 커널 공간으로 데이터 복사
        pr_info("hello_kernel: SET_ID %d -> %d\n", current_id, new_id);
        current_id = new_id; // 학번 변경 적용
        return 0;
    }
    case IOCTL_MSG:     /* _IOWR */
    {
        struct hello_msg msg; //abi.h에 정의된 구조체 사용
        //user가 보낸 구조체 데이터를 커널로 가져옴
        if (copy_from_user(&msg, (void __user *)arg, sizeof(msg)))
            return -EFAULT;
        pr_info("hello_kernel: MSG SET id=%d, text=%s\n", msg.id, msg.text);
        
        //커널이 가지고 있는 current_id 값을 구조체에 담아 user로 보냄.
        msg.id = current_id;
        if(copy_to_user((void __user *)arg, &msg, sizeof(msg)))
            return -EFAULT;
        pr_info("hello_kernel: MSG GET id=%d, text=%s\n", msg.id, msg.text);
        return 0;
    }
    default:
        return -ENOTTY;
    }
}


static int __init hello_init(void) {
    int ret;
    current_id = STUDENT_ID;

    //캐릭터 디바이스 번호 동적 할당.
    ret = alloc_chrdev_region(&_dev, 0, 1, DEVICE_NAME);
    if (ret < 0) {
        pr_err("hello_kernel: failed to alloc_chrdev_region\n");
        return ret;
    }
    //cdev 구조체 초기화 및 디바이스 등록
    cdev_init(&_cdev, &_fops);
    _cdev.owner = THIS_MODULE;

    //cdev를 커널에 추가
    ret = cdev_add(&_cdev, _dev, 1);
    if (ret < 0) {
        pr_err("hello_kernel: failed to cdev_add\n");
        unregister_chrdev_region(_dev, 1);
        return ret;
    }
    //디바이스 클래스 생성
    _class = class_create(CLASS_NAME);  
    if (IS_ERR(_class)) {
        pr_err("hello_kernel: failed to create class\n");
        cdev_del(&_cdev);
        unregister_chrdev_region(_dev, 1);
        return PTR_ERR(_class);
    }
    //디바이스 노드 생성
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
    //디바이스 노드 및 클래스 제거, cdev 삭제, 디바이스 번호 해제
    device_destroy(_class, _dev);
    class_destroy(_class);
    cdev_del(&_cdev);
    unregister_chrdev_region(_dev, 1);
    pr_info("hello_kernel: module unloaded\n");
}