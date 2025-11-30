#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/abi.h"

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage:\n"
        "  %s print             # _IO   : 커널에 Hello 메시지 요청\n"
        "  %s get               # _IOR  : 커널에서 id 읽기\n"
        "  %s set <id>          # _IOW  : 커널의 id 설정\n",
        "  %s msg <text>        # _IOWR : 구조체로 메시지 교환\n",
        prog, prog, prog, prog);

static int open_device(void) {
    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        fprintf(stderr,
                "hint: 커널 모듈이 로드되어 있고 %s 이 존재하는지 확인하세요.\n",
                DEVICE_PATH);
    }
    return fd;
}

int main(int argc, char *argv[]) {
    int fd;
    if (argc < 2) { usage(argv[0]); return EXIT_FAILURE; }

    fd = open_device();
    if (fd < 0) return EXIT_FAILURE;

    if (strcmp(argv[1], "print") == 0) {

         if (ioctl(fd, IOCTL_PRINT) == -1) {
        perror("ioctl(IOCTL_PRINT)");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("[_IO] IOCTL_PRINT 호출 완료. dmesg 에서 커널 로그를 확인해보세요.\n");

    } else if (strcmp(argv[1], "get") == 0) {

        int id = 0;

        if (ioctl(fd, IOCTL_GET_ID, &id) == -1) {
            perror("ioctl(IOCTL_GET_ID)");
            close(fd);
            return EXIT_FAILURE;
        }

        printf("[_IOR] IOCTL_GET_ID: id = %d\n", id);

    } else if (strcmp(argv[1], "set") == 0) {

         if (argc < 3) {
        usage(argv[0]);
        close(fd);
        return EXIT_FAILURE;
        }

        char *endptr = NULL;
        long val = strtol(argv[2], &endptr, 10);
        if (*argv[2] == '\0' || (endptr && *endptr != '\0')) {
            fprintf(stderr, "invalid id: %s\n", argv[2]);
            close(fd);
            return EXIT_FAILURE;
        }

        int id = (int)val;

        if (ioctl(fd, IOCTL_SET_ID, &id) == -1) {
            perror("ioctl(IOCTL_SET_ID)");
            close(fd);
            return EXIT_FAILURE;
        }

        printf("[_IOW] IOCTL_SET_ID: id -> %d 로 설정 요청 완료.\n", id);

    } else {
        usage(argv[0]);
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);
    return EXIT_SUCCESS;
}