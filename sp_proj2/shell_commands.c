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
    DIR* pwd = opendir(".");
    struct dirent* pwd_info = NULL;
    struct stat stat_info;

    if (pwd != NULL) {
        pwd_info = readdir(pwd);
        while (pwd_info != NULL) {
            printf("%s", pwd_info->d_name);
            lstat(pwd_info->d_name, &stat_info);
            if (stat_info.st_mode & S_IFREG) {
                if (stat_info.st_mode & S_IXUSR || stat_info.st_mode & S_IXGRP || stat_info.st_mode & S_IXOTH) {
                    printf("*\n");
                }
                else {
                    printf("\n");
                }
            }
            else if (stat_info.st_mode & S_IFDIR) {
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
    } 
    else {
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
OK_or_ERR type(char *filename) {
    DIR *dir = opendir(filename);
    FILE *fp = fopen(filename, "r");
    if (dir) return FILE_ERR;
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
