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
        "  %s set <id>          # _IOW  : 커널의 id 설정\n"
        "  %s msg <text>        # _IOWR : 구조체로 메시지 교환\n",
        prog, prog, prog, prog);
}


static int open_device(void) {
    //디바이스 파일을 rw권한으로 연다.
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
    //인자 부족 시 사용법 출력 후 종료
    if (argc < 2) { usage(argv[0]); return EXIT_FAILURE; }

    //디바이스 드라이버 파일 열기 
    fd = open_device();
    if (fd < 0) return EXIT_FAILURE;

    //_IO 타입 구현. 데이터 전송 없이 커널에 로그 출력만 요청
    if (strcmp(argv[1], "print") == 0) {

        if (ioctl(fd, IOCTL_PRINT) == -1) {
        perror("ioctl(IOCTL_PRINT)");
        close(fd);
        return EXIT_FAILURE;
    }
    printf("[_IO] HELLO_IOCTL_PRINT 호출 완료. dmesg 에서 로그를 확인해보세요.\n");

    //_IOR 타입 구현. 커널이 가지고 있는 데이터를 유저 공간으로 읽어옴.
    } else if (strcmp(argv[1], "get") == 0) {

        int id = 0; //커널로부터 값을 받아올 변수
        // &id를 넘겨주면, 커널은 copy to user를 통해 이 주소에 값을 쓴다.
        if (ioctl(fd, IOCTL_GET_ID, &id) == -1) {
            perror("ioctl(IOCTL_GET_ID)");
            close(fd);
            return EXIT_FAILURE;
        }

        printf("[_IOR] HELLO_IOCTL_GET_ID: id = %d\n", id);
    //_IOW 타입 구현. 유저 공간의 데이터를 커널로 보냄.
    } else if (strcmp(argv[1], "set") == 0) {
        //값을 입력하지 않은 경우 예외처리
        if (argc < 3) {
        usage(argv[0]);
        close(fd);
        return EXIT_FAILURE;
        }
        //문자열을 INT로 변환
        int id = atoi(argv[2]); 

        // 만약 0이 유효한 ID가 아니라면 간단히 체크 가능
        if (id == 0 && strcmp(argv[2], "0") != 0) {
            fprintf(stderr, "Invalid id input\n");
            close(fd);
            return EXIT_FAILURE;
        }

        int id = (int)val; //전송할 데이터 준비

        //&id를 넘겨주면 커널은 copy form user를 통해 이 값을 읽어감.
        if (ioctl(fd, IOCTL_SET_ID, &id) == -1) {
            perror("ioctl(IOCTL_SET_ID)");
            close(fd);
            return EXIT_FAILURE;
        }
 
        printf("[_IOW] HELLO_IOCTL_SET_ID: id %d로 설정 요청 완료.\n", id);
    //_IOWR 타입 구현. 구조체 단위의 양방향 데이터 교환
    } else if(strcmp(argv[1], "msg")==0){
    
        if(argc <3){
            usage(argv[0]);
            close(fd);
            return EXIT_FAILURE;
        }

        struct hello_msg msg; //abi.h에 정의된 구조체 사용
        msg.id = 0; 
        strncpy(msg.text, argv[2], sizeof(msg.text)-1); //사용자가 입력한 메시지 복사.
        
        msg.text[sizeof(msg.text)-1] = '\0'; //문자열 끝 처리

        //ioctl 호출로 msg를 커널로 보내고, 커널이 수정한 msg를 다시 받아옴.
        if(ioctl(fd, IOCTL_MSG, &msg) == -1){
            perror("ioctl(IOCTL_MSG)");
            close(fd);
            return EXIT_FAILURE;
        }

        printf("[_IOWR] HELLO_IOCTL_MSG: id = %d, text = %s\n", msg.id, msg.text);
    }
    else {
        usage(argv[0]);
        close(fd);
        return EXIT_FAILURE;
    }
    //작업 완료 후 디바이스 파일 닫기
    close(fd);
    return EXIT_SUCCESS;
}
