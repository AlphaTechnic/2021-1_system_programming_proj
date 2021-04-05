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
    FILE *fp = fopen(filename, "r");
    DIR *dir = opendir(filename);
    if (!fp) return FILE_ERR;
    if (dir) return FILE_ERR;

    char *name, *extension, tmp[MAX_NAME_SIZE];
    int size_of_name = strlen(filename);
    int PROGRAM_SIZE;


    strcpy(tmp, filename);
    name = strtok(tmp, ".");
    if (!name) {
        fclose(fp);
        return FILE_ERR;
    }
    extension = strtok(NULL, "");
    if (strcmp(extension, "asm") != 0) {
        fclose(fp);
        return FILE_ERR;
    }

    // 여기서 pass1, pass2
    pass1(fp, name, &PROGRAM_SIZE);
}

/*------------------------------------------------------------------------------------*/
/*함수 : symbol*/
/*목적 : assemble 과정 중에 생성된 symbol table을 화면에 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void symbol() {
    SYM_node *cur_node = LATEST_SYMTAB;
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
OK_or_ERR pass1(FILE *fp, char *filename, int *LENGTH) {
    if (!fp) return FILE_ERR;

    char line[MAX_LINE_SIZE];
    char LABEL[MAX_LABEL_SIZE], MNEMONIC[MAX_OPCODE_SIZE], OP1[MAX_OPCODE_SIZE], OP2[MAX_OPCODE_SIZE];
    char filename_itm[MAX_NAME_SIZE];
    FILE *fp_itm;
    INSTRUCTION type;
    OPCODE_MNEMONIC_MAP *opcode_mnemonic_map_node;

    int LOCCTR = 0, STARTING_ADDR = 0;
    int dl, LINE_NUM;  // 'dl' means 'delta line'

    strcpy(filename_itm, filename);
    strcat(filename_itm, ".itm");
    fp_itm = fopen(filename_itm, "w");
    fgets(line, MAX_LINE_SIZE, fp);
    line[strlen(line) - 1] = '\0';


    type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
    if (type == START) {
        STARTING_ADDR = atoi(OP1);
        LOCCTR = STARTING_ADDR;

        //fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
        printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
        fgets(line, MAX_LINE_SIZE, fp); line[strlen(line) - 1] = '\0';
        type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
    }

    while (type != END) {
        if (feof(fp)) {
            printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
            return ASSEMBLY_CODE_ERR;
            // 여기서 itm 파일 삭제
        }
        LINE_NUM++;
        if (type != COMMENT) {
            if (*LABEL != '\0') {
                if (is_in_symtab(LABEL) == ASSEMBLY_CODE_ERR) {
                    printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                    return ASSEMBLY_CODE_ERR;
                    // 여기서 itm 파일 삭제
                }
                push_to_symtab(LABEL, LOCCTR);
            }
            //fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
            printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);

            char tmp[MAX_OPCODE_SIZE];
            if (MNEMONIC[0] == '+') strcpy(tmp, MNEMONIC + 1);
            else
                strcpy(tmp, MNEMONIC);

            if (type == OPERATION && (opcode_mnemonic_map_node = get_opcode2(tmp)) != NULL) {
                if (strcmp(opcode_mnemonic_map_node->format, "3/4") == 0) {
                    if (MNEMONIC[0] == '+') dl = 4;
                    else dl = 3;
                }
                else if (strcmp(opcode_mnemonic_map_node->format, "1") == 0) dl = 1;
                else if (strcmp(opcode_mnemonic_map_node->format, "2") == 0) dl = 2;
            }
            else if (type == BASE) dl = 0;
            else if (type == WORD) dl = 3;
            else if (type == RESW) dl = 3*atoi(OP1);
            else if (type == RESB) dl = atoi(OP1);
            else if (type == BYTE) dl = find_byte_len(OP1);
            else{
                printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                return ASSEMBLY_CODE_ERR;
                // 여기에 itm 파일 지우는 코드 들어가야함.
            }
            LOCCTR += dl;
        }
        else printf("%04X %-10s %s\n", LOCCTR, LABEL, MNEMONIC); //fprintf(fp_itm, "%04X %-10s %s\n", LOCCTR, LABEL, MNEMONIC);

        fgets(line, MAX_LINE_SIZE, fp); line[strlen(line) - 1] = '\0';
        type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
    }
    //fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
    printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);

    fclose(fp_itm);
    *LENGTH = LOCCTR - STARTING_ADDR;
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : */
/*목적 : */
/*리턴값 : */
/*------------------------------------------------------------------------------------*/
INSTRUCTION line_split(char *line, char *label, char *mnemonic, char *op1, char *op2) {
    char buf[MAX_LINE_SIZE];
    char *ptr;
    INSTRUCTION type;

    // line, label, mnemonic, op1, op2 초기화
    strcpy(buf, line);
    strcpy(label, "\0");
    strcpy(mnemonic, "\0");
    strcpy(op1, "\0");
    strcpy(op2, "\0");

    ptr = strtok(buf, " \t\r");
    if (!ptr) return ELSE;
    if (!(*ptr)) return ELSE;
    if (isalpha(buf[0])) {
        strcpy(label, ptr);
        ptr = strtok(NULL, " \t\r");
    }
    if (buf[0] == '.') {
        strcpy(label, ptr);
        ptr = strtok(NULL, " \t\r");
        if (!ptr) return COMMENT;

        *(ptr + strlen(ptr)) = ' ';
        strcpy(mnemonic, ptr);
        return COMMENT;
    }

    strcpy(mnemonic, ptr);
    type = get_instruction(mnemonic);

    ptr = strtok(NULL, " \t\r");
    if (!ptr) return type;
    strcpy(op1, ptr);

    ptr = strtok(NULL, " \t\r");
    if (!ptr) return type;
    strcpy(op2, ptr);
    return type;
}

/*------------------------------------------------------------------------------------*/
/*함수 : */
/*목적 : */
/*리턴값 : */
/*------------------------------------------------------------------------------------*/
INSTRUCTION get_instruction(char *mnemonic) {
    INSTRUCTION type;
    if (strcmp(mnemonic, "START") == 0) type = START;
    else if (strcmp(mnemonic, "END") == 0) type = END;
    else if (strcmp(mnemonic, "COMMENT") == 0) type = COMMENT;
    else if (strcmp(mnemonic, "BASE") == 0) type = BASE;
    else if (strcmp(mnemonic, "BYTE") == 0) type = BYTE;
    else if (strcmp(mnemonic, "WORD") == 0) type = WORD;
    else if (strcmp(mnemonic, "RESW") == 0) type = RESW;
    else type = OPERATION;

    return type;
}

OK_or_ERR is_in_symtab(char *symbol) {
    SYM_node *cur_node = SYMTAB_HEAD;
    for (; cur_node != NULL; cur_node = cur_node->nxt) {
        // SYMTAB 안에 해당 symbol이 있다면, duplication err이다.
        if (strcmp(symbol, cur_node->symbol) == 0) return ASSEMBLY_CODE_ERR;
    }
    return OK;
}

void push_to_symtab(char *symbol, int addr) {
    SYM_node *pre_node = NULL;
    SYM_node *cur_node = SYMTAB_HEAD;
    SYM_node *new_node = malloc(sizeof(SYM_node));

    strcpy(new_node->symbol, symbol);
    new_node->address = addr;
    new_node->nxt = NULL;

    if (SYMTAB_HEAD == NULL) {
        SYMTAB_HEAD = new_node;
        return;
    }

    for (; cur_node->nxt != NULL; cur_node = cur_node->nxt, pre_node = cur_node) {
        if (strcmp(cur_node->symbol, new_node->symbol) > 0) {
            if (pre_node == NULL) SYMTAB_HEAD = new_node;
            else pre_node->nxt = new_node;
            new_node->nxt = cur_node;
            return;
        }
    }
    cur_node->nxt = new_node;
    new_node = NULL;
}

/*------------------------------------------------------------------------------------*/
/*함수 : */
/*목적 : */
/*리턴값 : */
/*------------------------------------------------------------------------------------*/
OPCODE_MNEMONIC_MAP *get_opcode2(char *mnemonic) {
    OPCODE_MNEMONIC_MAP *cur_node = HASH_TABLE[hash_func(mnemonic)];
    while (cur_node != NULL) {
        if (strcmp(cur_node->mnemonic, mnemonic) == 0) {
            return cur_node;
        }
        cur_node = cur_node->nxt;
    }
    return NULL;
}

/*------------------------------------------------------------------------------------*/
/*함수 : */
/*목적 : */
/*리턴값 : */
/*------------------------------------------------------------------------------------*/
int find_byte_len(char* constant){
    int len;
    char* ptr;
    if (constant[0] == 'C'){
        ptr = strtok(constant, "C`'");
        len = strlen(ptr);
    }
    else if (constant[0] == 'X'){
        ptr = strtok(constant, "X`'");
        len = strlen(ptr) / 2;
    }
    return len;
}
