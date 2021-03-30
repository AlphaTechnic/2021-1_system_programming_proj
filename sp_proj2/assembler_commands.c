//
// Created by 김주호 on 2021/03/29.
//

#include "assembler_commands.h"

/*------------------------------------------------------------------------------------*/
/*함수 : assemble*/
/*목적 : filename에 해당하는 소스파일을 읽어서 object파일(.obj)과 리스팅파일(.lst)을 만든다.*/
/*리턴값 : OK - 성공, FILE_ERR - 파일 에러*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR assemble(char *filename) {
    char *name, *extension, tmp[MAX_NAME_SIZE];
    FILE *fp = fopen(filename, "r");
    DIR *dir = opendir(filename);
    if (!fp) return FILE_ERR;
    if (!dir) return FILE_ERR;


    strcpy(tmp, filename);
    name = strtok(tmp, ".");
    if (!name) {
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
/*함수 : symbol*/
/*목적 : assemble 과정 중에 생성된 symbol table을 화면에 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void symbol() {
    symbol_node *cur_node = RECENT_SYMTAB_HEAD;
    while (cur_node) {
        printf("\t%-10s %04X\n", cur_node->symbol, cur_node->address);
        cur_node = cur_node->nxt;
    }
}

/*------------------------------------------------------------------------------------*/
/*함수 : */
/*목적 : */
/*리턴값 : */
/*------------------------------------------------------------------------------------*/
OK_or_ERR pass1(FILE* fp, char* filename, int* LENGTH){
    if (!fp) return FILE_ERR;

    char line[MAX_LINE_SIZE];
    char label[MAX_LABEL_SIZE], opcode[MAX_OPCODE_SIZE], op1[MAX_OPCODE_SIZE], op2[MAX_OPCODE_SIZE];
    char tmpfile_name[MAX_NAME_SIZE];
    FILE *tmpfile_ptr;

    strcpy(tmpfile_name, filename);
    strcat(tmpfile_name, ".tmp");
    tmpfile_ptr = fopen(tmpfile_name, "w");
    fgets(line, MAX_LINE_SIZE, fp); line[strlen(line) - 1] = '\0';

    // 읽어들인 line을 쪼개야.
}




