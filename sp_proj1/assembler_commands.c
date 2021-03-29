//
// Created by 김주호 on 2021/03/29.
//

#include "assembler_commands.h"

/*------------------------------------------------------------------------------------*/
/*함수 : assemble*/
/*목적 : filename에 해당하는 소스파일을 읽어서 object파일(.obj)과 리스팅파일(.lst)을 만든다.*/
/*리턴값 : OK - 성공, FILE_ERR - 파일 에러*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR assemble(char* filename){
    char *name, *extension, tmp[MAX_NAME_LEN];
    FILE *fp = fopen(filename, "r");
    DIR* dir = opendir(filename);
    if (!fp) return FILE_ERR;
    if (!dir) return FILE_ERR;


    strcpy(tmp, filename);
    name = strtok(tmp, ".");
    if (!name){
        fclose(fp);
        return FILE_ERR;
    }
    extension = strtok(NULL, "");
    if (!strcmp(extension, "asm")) {
        fclose(fp);
        return FILE_ERR;
    }

    // 여기서 pass1, pass2
}

/*------------------------------------------------------------------------------------*/
/*함수 : assemble*/
/*목적 : filename에 해당하는 소스파일을 읽어서 object파일(.obj)과 리스팅파일(.lst)을 만든다.*/
/*리턴값 : OK - 성공, FILE_ERR - 파일 에러*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR symbol(){
}


