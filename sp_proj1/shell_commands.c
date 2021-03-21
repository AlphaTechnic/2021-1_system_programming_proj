//
// Created by 김주호 on 2021/03/12.
//

#include "shell_commands.h"

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
}

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

void history() {
    CMD *cur_node;
    int cnt = 1;
    for (cur_node = HEAD; cur_node != NULL; cur_node = cur_node->nxt) {
        printf("%4d %s\n", cnt++, cur_node->cmd);
    }
}

void free_log_of_instructions() {
    CMD *tmp_node;
    while (HEAD != NULL) {
        tmp_node = HEAD->nxt;
        free(HEAD);
        HEAD = tmp_node;
    }
    TAIL = NULL;
}
