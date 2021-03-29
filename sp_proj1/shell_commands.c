//
// Created by 김주호 on 2021/03/12.
//

#include "shell_commands.h"

/*------------------------------------------------------------------------------------*/
/*함수 : help*/
/*목적 : 사용자에게 해당 프로그램에 명령할 수 있는 옵션들이 어떤 것들이 있는지 화면에 print하여 알려준다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void help() {
    printf("h[elp]\n");
    printf("d[ir]\n");
    printf("q[uit]\n");
    printf("hi[story]\n");
    printf("du[mp] [start, end]\n");
    printf("e[dit] address, value\n");
    printf("f[ill] start, end, value\n");
    printf("reset\n");
    printf("opcode mnemonic\n");
    printf("opcodelist\n");
    printf("assemble filename\n");
    printf("type filename\n");
    printf("symbol\n");
}

/*------------------------------------------------------------------------------------*/
/*함수 : dir*/
/*목적 : 현재 directory에 있는 파일들을 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void dir() {
    DIR *pwd = opendir("."); // opendir함수 : DIR형 오브젝트에 대한 포인터를 반환
    /*
    struct dirent { // struct dirent : 파일, 또는 디렉토리가 가지고 있는 정보 구조체
    long d_ino;                         // 아이노드
    off_t d_off;                        // dirent 의 offset
    unsigned short d_reclen;            // d_name 의 길이
    char d_name [NAME_MAX+1];   // 파일 이름(없다면 NULL로 종료)
    }
    */
    struct dirent *pwd_info = NULL;
    /*
    struct stat {
        dev_t st_dev; // ID of device containing file
        ino_t st_ino; // inode number
        mode_t st_mode; // 파일의 종류 및 접근권한
        nlink_t st_nlink; // hardlink 된 횟수
        uid_t st_uid; // 파일의 owner
        gid_t st_gid; // group ID of owner
        dev_t st_rdev; // device ID (if special file)
        off_t st_size; // 파일의 크기(bytes)
        blksize_t st_blksize; // blocksize for file system I/O
        blkcnt_t st_blocks; // number of 512B blocks allocated
        time_t st_atime; // time of last access
        time_t st_mtime; // time of last modification
        time_t st_ctime; // time of last status change
    };
    */
    struct stat stat_info;


    if (pwd != NULL) {
        pwd_info = readdir(pwd);
        while (pwd_info != NULL) {
            printf("%s", pwd_info->d_name);
            lstat(pwd_info->d_name, &stat_info); // 파일을 path로 넘기면, 파일의 정보를 얻는다

            // st_mode는 파일의 유형값으로 직접 bit & 연산으로 여부를 확인가능
            if (stat_info.st_mode & S_IFREG) { // S_IFREG : 일반 파일 여부
                if (stat_info.st_mode & S_IXUSR || // S_IXUSR 00100 owner has execute permission
                    stat_info.st_mode & S_IXGRP || // S_IXGRP 00010 group has execute permission
                    stat_info.st_mode & S_IXOTH) { // S_IXOTH 00001 others have execute permission
                    printf("*\n");
                } else {
                    printf("\n");
                }
            } else if (stat_info.st_mode & S_IFDIR) { // S_IFDIR : 디렉토리 여부
                printf("/\n");
            }
            pwd_info = readdir(pwd);
        }
    }
}

/*------------------------------------------------------------------------------------*/
/*함수 : save_instructions*/
/*목적 : 사용자가 입력한 command들을 linked list에 저장한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void save_instructions(char *refined_cmd) {
    // create new_node
    CMD *new_node = malloc(sizeof(CMD));
    strcpy(new_node->cmd, refined_cmd);
    new_node->nxt = NULL;

    // connect
    if (HEAD == NULL) {
        HEAD = TAIL = new_node;
    } else {
        TAIL->nxt = new_node;
        TAIL = new_node;
    }
}

/*------------------------------------------------------------------------------------*/
/*함수 : history*/
/*목적 : 현재까지 사용자가 입력한 명령어들을 순서대로 번호와 함께 보여준다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void history() {
    CMD *cur_node;
    int cnt = 1;
    for (cur_node = HEAD; cur_node != NULL; cur_node = cur_node->nxt) {
        printf("%4d %s\n", cnt++, cur_node->cmd);
    }
}

/*------------------------------------------------------------------------------------*/
/*함수 : free_log_of_instructions*/
/*목적 : 명령어들을 저장하고 있는 linked list의 메모리 할당을 해제한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void free_log_of_instructions() {
    CMD *tmp_node;
    if (HEAD == TAIL) {
        return;
    }
    while (HEAD != NULL) {
        tmp_node = HEAD->nxt;
        free(HEAD);
        HEAD = tmp_node;
    }
    TAIL = NULL;
}


/*------------------------------------------------------------------------------------*/
/*함수 : type*/
/*목적 : filename에 해당하는 파일을 현재 디렉터리에서 읽어서 화면에 출력한다.*/
/*리턴값 : OK - 성공, FILE_ERR - 파일 읽기 에러*/
/*------------------------------------------------------------------------------------*/
int type(char *filename) {
    DIR *dir = opendir(filename);
    if (!dir) return FILE_ERR;
    FILE *fp = fopen(filename, "r");
    if (!fp) return FILE_ERR;

    char ch;
    while (1) {
        ch = fgetc(fp);
        if (ch == EOF) break;
        printf("%c", ch);
    }
    printf("\n");
    fclose(fp);
    return OK;
}
