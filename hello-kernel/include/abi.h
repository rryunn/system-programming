#ifndef HELLO_KERNEL_ABI_H
#define HELLO_KERNEL_ABI_H

#ifdef __KERNEL__
#include <linux/ioctl.h>
#else
#include <sys/ioctl.h> 
#endif

#define DEVICE_NAME "hello_kernel"
#define DEVICE_PATH "/dev/hello_kernel"
#define CLASS_NAME  "hello"

#define IOCTL_MAGIC 'h'

#define IOCTL_PRINT _IO(IOCTL_MAGIC, 1)
#define IOCTL_GET_ID _IOR(IOCTL_MAGIC, 2, int)
#define IOCTL_SET_ID _IOW(IOCTL_MAGIC, 3, int)
//  #define IOCTL_SOMETHING _IOWR( /** args */)

struct hello_msg {
    int id; //학번 정보 교환용
    char text[32]; //문자열 메시지 데이터
};

//_IOWR를 이용하여 구조체 단위의 양방향 통신 정의 
#define IOCTL_MSG _IOWR(IOCTL_MAGIC, 4, struct hello_msg)
#endif 