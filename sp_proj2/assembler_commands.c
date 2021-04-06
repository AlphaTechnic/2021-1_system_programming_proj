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

    char *name, *extension, tmp[NAME_LEN];
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

    SYMTAB_HEAD = NULL;

    // 여기서 pass1, pass2
    OK_or_ERR pass1_state = pass1(fp, name, &PROGRAM_SIZE);
    fclose(fp);
    if (pass1_state != OK) {
        free_SYMTAB(SYMTAB_HEAD);
        return FILE_ERR;
    }

    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : symbol*/
/*목적 : assemble 과정 중에 생성된 symbol table을 화면에 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void print_symbol() {
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

    char line[LINE_LEN];
    char LABEL[LABEL_LEN], MNEMONIC[MNEMONIC_LEN], OP1[MNEMONIC_LEN], OP2[MNEMONIC_LEN];
    char filename_itm[NAME_LEN];
    FILE *fp_itm;
    INSTRUCTION type;
    OPCODE_MNEMONIC_MAP *opcode_mnemonic_map_node;

    int LOCCTR = 0, STARTING_ADDR = 0;
    int dl, LINE_NUM = 0;  // 'dl' means 'delta line'

    strcpy(filename_itm, filename);
    strcat(filename_itm, ".itm");
    fp_itm = fopen(filename_itm, "w");
    fgets(line, LINE_LEN, fp);
    line[strlen(line) - 1] = '\0';


    type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
    if (type == _START) {
        STARTING_ADDR = atoi(OP1);
        LOCCTR = STARTING_ADDR;

        fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
        //printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
        fgets(line, LINE_LEN, fp); line[strlen(line) - 1] = '\0';
        type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
    }

    while (type != _END) {
        if (feof(fp)) {
            printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
            return ASSEMBLY_CODE_ERR;
            // 여기서 itm 파일 삭제
        }
        LINE_NUM++;
        if (type != _COMMENT) {
            if (*LABEL != '\0') {
                if (is_in_symtab(LABEL) == ASSEMBLY_CODE_ERR) {
                    printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                    return ASSEMBLY_CODE_ERR;
                    // 여기서 itm 파일 삭제
                }
                push_to_symtab(LABEL, LOCCTR);
            }
            fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
            //printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);

            char tmp[MNEMONIC_LEN];
            if (MNEMONIC[0] == '+') strcpy(tmp, MNEMONIC + 1);
            else
                strcpy(tmp, MNEMONIC);

            if (type == _OPERATION && (opcode_mnemonic_map_node = get_opcode2(tmp)) != NULL) {
                if (strcmp(opcode_mnemonic_map_node->format, "3/4") == 0) {
                    if (MNEMONIC[0] == '+') dl = 4;
                    else dl = 3;
                }
                else if (strcmp(opcode_mnemonic_map_node->format, "1") == 0) dl = 1;
                else if (strcmp(opcode_mnemonic_map_node->format, "2") == 0) dl = 2;
            }
            else if (type == _BASE) dl = 0;
            else if (type == _WORD) dl = 3;
            else if (type == _RESW) dl = 3*atoi(OP1);
            else if (type == _RESB) dl = atoi(OP1);
            else if (type == _BYTE) dl = find_byte_len(OP1);
            else{
                printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                return ASSEMBLY_CODE_ERR;
                // 여기에 itm 파일 지우는 코드 들어가야함.
            }
            LOCCTR += dl;
        }
        else fprintf(fp_itm, "%04X %-10s %s\n", LOCCTR, LABEL, MNEMONIC);//printf("%04X %-10s %s\n", LOCCTR, LABEL, MNEMONIC); 

        fgets(line, LINE_LEN, fp); line[strlen(line) - 1] = '\0';
        type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
    }
    fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
    //printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);

    fclose(fp_itm);
    *LENGTH = LOCCTR - STARTING_ADDR;
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : */
/*목적 : */
/*리턴값 : */
/*------------------------------------------------------------------------------------*/
INSTRUCTION line_split(char *line, char *LABEL, char *MNEMONIC, char *OP1, char *OP2) {
    char buf[LINE_LEN];
    char *ptr;
    INSTRUCTION type;

    // line, label, mnemonic, op1, op2 초기화
    strcpy(buf, "\0");
    strcpy(buf, line);
    strcpy(LABEL, "\0");
    strcpy(MNEMONIC, "\0");
    strcpy(OP1, "\0");
    strcpy(OP2, "\0");

    ptr = strtok(buf, " \t\r");
    if (!ptr) return _ELSE;
    if (!(*ptr)) return _ELSE;
    if (isalpha(buf[0])) {
        strcpy(LABEL, ptr);
        ptr = strtok(NULL, " \t\r");
    }
    if (buf[0] == '.') {
        strcpy(LABEL, ptr);
        ptr = strtok(NULL, " \t\r");
        if (!ptr) return _COMMENT;

        *(ptr + strlen(ptr)) = ' ';
        strcpy(MNEMONIC, ptr);
        return _COMMENT;
    }

    strcpy(MNEMONIC, ptr);
    type = get_instruction(MNEMONIC);

    ptr = strtok(NULL, " \t\r");
    if (!ptr) return type;
    strcpy(OP1, ptr);

    ptr = strtok(NULL, " \t\r");
    if (!ptr) return type;
    strcpy(OP2, ptr);
    return type;
}

/*------------------------------------------------------------------------------------*/
/*함수 : */
/*목적 : */
/*리턴값 : */
/*------------------------------------------------------------------------------------*/
INSTRUCTION get_instruction(char *mnemonic) {
    INSTRUCTION type;
    if (strcmp(mnemonic, "START") == 0) type = _START;
    else if (strcmp(mnemonic, "END") == 0) type = _END;
    else if (strcmp(mnemonic, "COMMENT") == 0) type = _COMMENT;
    else if (strcmp(mnemonic, "BASE") == 0) type = _BASE;
    else if (strcmp(mnemonic, "BYTE") == 0) type = _BYTE;
    else if (strcmp(mnemonic, "WORD") == 0) type = _WORD;
    else if (strcmp(mnemonic, "RESB") == 0) type = _RESB;
    else if (strcmp(mnemonic, "RESW") == 0) type = _RESW;
    else type = _OPERATION;

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
    SYM_node *node_to_insert = malloc(sizeof(SYM_node));

    strcpy(node_to_insert->symbol, symbol);
    node_to_insert->address = addr;
    node_to_insert->nxt = NULL;

    if (!SYMTAB_HEAD) {
        SYMTAB_HEAD = node_to_insert;
        return;
    }

    for (; cur_node; pre_node = cur_node, cur_node = cur_node->nxt) {
        if (strcmp(cur_node->symbol, node_to_insert->symbol) > 0) {
            if (!pre_node) { // 삽입할 노드가 알파벳 첫순서
                //printf("1111\n");
                //printf("1111\n");
                SYMTAB_HEAD = node_to_insert;
                //printf("12345\n");
                node_to_insert->nxt = cur_node;
                //printf("45678\n");
            }
            else {
                //printf("2222\n");
                //printf("2222\n");
                pre_node->nxt = node_to_insert;
                node_to_insert->nxt = cur_node;
            }
            return;
        }
    }
    // 삽입할 노드가 알파벳 마지막 순서
    pre_node->nxt = node_to_insert;
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
    int len = 0;
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

void free_SYMTAB(SYM_node* head) {
    SYM_node* cur_node = head;
    SYM_node* pre_node = NULL;
    SYM_node* origin_node = head;

    for (; cur_node; pre_node = cur_node, cur_node = cur_node->nxt) {
        free(pre_node);
    }
    origin_node = NULL;
}

OK_or_ERR pass2(char* filename, int PROGRAM_SIZE) {
    char filename_itm[NAME_LEN];
    FILE* fp_itm;

    strcpy(filename_itm, filename);
    strcat(filename_itm, ".itm");
    fp_itm = fopen(filename_itm, "r");
    if (!fp_itm) return FILE_ERR;

    char line[LINE_LEN]; char LABEL[LABEL_LEN]; char MNEMONIC[MNEMONIC_LEN];
    char OP1[OPERAND_LEN]; char OP2[OPERAND_LEN];
    FILE* fp_lst, * fp_obj;
    char target_file[NAME_LEN];
    INSTRUCTION type;
    int LOCCTR = 0, LINE_NUM = 1, STARTING_ADDR = 0;
    char PROGRAM_NAME[NAME_LEN];
    char cur_T_record[ONELINE_T_RECORD_SIZE];
    char obj_code[OBJ_CODE_LEN];
    OPCODE_MNEMONIC_MAP *opcode_mnemonic_map_node;

    // init M_records
    M_RECORD_NUM = 0;
    for(int i=0; i<TOTAL_M_RECORD_SIZE; i++){
        strcpy(M_RECORDS[i], "\0");
    }

    strcpy(target_file, filename);
    fp_lst = fopen(strcat(filename, ".lst"), "w");
    strcpy(target_file, filename);
    fp_obj = fopen(strcat(filename, ".obj"), "w");

    fgets(line, LINE_LEN, fp_itm); line[strlen(line) - 1] = '\0';
    type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);

    if (type == _START){
        // write listing file
        printf("-lst-%3d %-40s\n", (LINE_NUM++)*LINE_NUM_SCALE, line);
        //fprintf(fp_lst, "%3d %-40s\n", (LINE_NUM++)*LINE_NUM_SCALE, line);
        STARTING_ADDR = LOCCTR;
        strcpy(PROGRAM_NAME, LABEL);

        // read nxt line
        fgets(line, LINE_LEN, fp_itm); line[strlen(line) - 1] = '\0';
        type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);
    }

    printf("-obj-H%-6s%06X%06X\n", PROGRAM_NAME, STARTING_ADDR, PROGRAM_SIZE);
    //fprintf(fp_obj, "H%-6s%06X%06X\n", PROGRAM_NAME, STARTING_ADDR, PROGRAM_SIZE);
    sprintf(cur_T_record, "T%06XLL", STARTING_ADDR);

    while (type!=_END){
        if(feof(fp_itm)){
            printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
            return FILE_ERR;
            // obj, lst 파일 삭제
        }

        char tmp[OBJ_CODE_LEN];
        strcpy(obj_code, '\0');
        if (type != _COMMENT){
            if (MNEMONIC[0] == '+') strcpy(tmp, MNEMONIC+1);
            else strcpy(tmp, MNEMONIC);
        }

        if (type == _OPERATION && (opcode_mnemonic_map_node = get_opcode2(tmp)) != NULL){
            // object_code를 생성해야함.
        }
    }




}

INSTRUCTION line_split2(char* line, int* LOCCTR, char* LABEL, char* MNEMONIC, char* OP1, char* OP2) {
    char buf[LINE_LEN];
    char* ptr;

    strcpy(buf, line);
    ptr = strtok(buf, " \t");
    if (!ptr) return _ELSE;
    if (!(*ptr)) return _ELSE;
    *LOCCTR = hexstr_to_decint(ptr);

    return line_split(ptr + strlen(ptr) + 1, LABEL, MNEMONIC, OP1, OP2);
}