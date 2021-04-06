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

    B = 0;

    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : symbol*/
/*목적 : assemble 과정 중에 생성된 symbol table을 화면에 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void print_symbols() {
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
        fgets(line, LINE_LEN, fp);
        line[strlen(line) - 1] = '\0';
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
            else if (type == _RESW) dl = 3 * atoi(OP1);
            else if (type == _RESB) dl = atoi(OP1);
            else if (type == _BYTE) dl = find_byte_len(OP1);
            else {
                printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                return ASSEMBLY_CODE_ERR;
                // 여기에 itm 파일 지우는 코드 들어가야함.
            }
            LOCCTR += dl;
        }
        else
            fprintf(fp_itm, "%04X %-10s %s\n", LOCCTR, LABEL,
                    MNEMONIC);//printf("%04X %-10s %s\n", LOCCTR, LABEL, MNEMONIC);

        fgets(line, LINE_LEN, fp);
        line[strlen(line) - 1] = '\0';
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
int find_byte_len(char *constant) {
    int len = 0;
    char *ptr;
    if (constant[0] == 'C') {
        ptr = strtok(constant, "C`'");
        len = (int) strlen(ptr);
    }
    else if (constant[0] == 'X') {
        ptr = strtok(constant, "X`'");
        len = (int) strlen(ptr) / 2;
    }
    return len;
}

void free_SYMTAB(SYM_node *head) {
    SYM_node *cur_node = head;
    SYM_node *pre_node = NULL;
    SYM_node *origin_node = head;

    for (; cur_node; pre_node = cur_node, cur_node = cur_node->nxt) {
        free(pre_node);
    }
    origin_node = NULL;
}

OK_or_ERR pass2(char *filename, int PROGRAM_SIZE) {
    char filename_itm[NAME_LEN];
    FILE *fp_itm;

    strcpy(filename_itm, filename);
    strcat(filename_itm, ".itm");
    fp_itm = fopen(filename_itm, "r");
    if (!fp_itm) return FILE_ERR;

    char line[LINE_LEN];
    char LABEL[LABEL_LEN];
    char MNEMONIC[MNEMONIC_LEN];
    char OP1[OPERAND_LEN];
    char OP2[OPERAND_LEN];
    FILE *fp_lst, *fp_obj;
    char target_file[NAME_LEN];
    INSTRUCTION type;
    int LOCCTR = 0, LINE_NUM = 1, STARTING_ADDR = 0;
    char PROGRAM_NAME[NAME_LEN];
    char ONELINE_T_RECORD[ONELINE_T_RECORD_LINE_SIZE];
    char obj_code[OBJ_CODE_LEN];
    OPCODE_MNEMONIC_MAP *opcode_mnemonic_map_node;
    OK_or_ERR state;
    SYM_node *symbol_node;
    int T_RECORD_ACCUMULATED_BYTE = 0;
    int var_flag = 0, new_line_flag = 0, comment_flag = 0;

    // init M_records
    M_RECORD_NUM = 0;
    for (int i = 0; i < TOTAL_M_RECORD_SIZE; i++) {
        strcpy(M_RECORDS[i], "\0");
    }

    strcpy(target_file, filename);
    fp_lst = fopen(strcat(filename, ".lst"), "w");
    strcpy(target_file, filename);
    fp_obj = fopen(strcat(filename, ".obj"), "w");

    fgets(line, LINE_LEN, fp_itm);
    line[strlen(line) - 1] = '\0';
    type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);

    if (type == _START) {
        // write listing file
        printf("-lst-%3d %-40s\n", (LINE_NUM++) * LINE_NUM_SCALE, line);
        //fprintf(fp_lst, "%3d %-40s\n", (LINE_NUM++)*LINE_NUM_SCALE, line);
        STARTING_ADDR = LOCCTR;
        strcpy(PROGRAM_NAME, LABEL);

        // read nxt line
        fgets(line, LINE_LEN, fp_itm);
        line[strlen(line) - 1] = '\0';
        type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);
    }

    printf("-obj-H%-6s%06X%06X\n", PROGRAM_NAME, STARTING_ADDR, PROGRAM_SIZE);
    //fprintf(fp_obj, "H%-6s%06X%06X\n", PROGRAM_NAME, STARTING_ADDR, PROGRAM_SIZE);
    sprintf(ONELINE_T_RECORD, "T%06XLL", STARTING_ADDR);

    while (type != _END) {
        if (feof(fp_itm)) {
            printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
            return ASSEMBLY_CODE_ERR;
            // obj, lst 파일 삭제
        }

        char tmp[OBJ_CODE_LEN];
        strcpy(obj_code, '\0');
        if (type != _COMMENT) {
            if (MNEMONIC[0] == '+') strcpy(tmp, MNEMONIC + 1);
            else strcpy(tmp, MNEMONIC);

            if (type == _OPERATION && (opcode_mnemonic_map_node = get_opcode2(tmp)) != NULL) {
                state = make_obj_code_by_a_line(obj_code, LOCCTR, MNEMONIC, OP1, OP2);
                if (state != OK) {
                    printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                    return ASSEMBLY_CODE_ERR;
                    // 파일 닫고 .lst .obj 삭제해야함
                }
            }
            else if (type == _BYTE) {
                char tmp_op1[OPERAND_LEN];
                if (OP1[0] == 'C') {
                    strcpy(tmp_op1, OP1);
                    char tmpbuf[3];
                    char *ptr = strtok(tmp_op1, " C'`");
                    for (int i = 0; i < (int) strlen(ptr); i++) {
                        sprintf(tmpbuf, "%02X", (int) *(ptr + i));
                        strcat(obj_code, tmpbuf);
                    }
                }
                else if (OP1[0] == 'X') {
                    strcpy(tmp_op1, OP1);
                    char *ptr = strtok(tmp_op1, " X'`");
                    strcpy(obj_code, ptr);
                }
            }
            else if (type == _WORD) sprintf(obj_code, "%06X", atoi(OP1));
            else if (type == _BASE) {
                symbol_node = find_symbol(OP1);
                {
                    if (!symbol_node) {
                        printf("Symbol ");
                        printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                        return ASSEMBLY_CODE_ERR;
                        // 파일 닫고 .lst .obj 삭제해야함
                    }
                    B = symbol_node->address;
                    LOCCTR = -1;
                }
            }
            else if (type == _RESW || type == _RESB) {
                while (type == _RESW || type == _RESB) {
                    //fprintf(fp_lst, "%3d %-40s %-s\n", (LINE_NUM++) * LINE_NUM_SCALE, line, obj_code);
                    printf("%3d %-40s %-s\n", (LINE_NUM++) * LINE_NUM_SCALE, line, obj_code);
                    fgets(line, LINE_LEN, fp_itm);
                    line[strlen(line) - 1] = '\0';
                    type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);
                }
                var_flag = 1;
                new_line_flag = 1;
            }
            // 현재 obj_code가 정해진 line 길이를 벗어난다면,
            if (T_RECORD_ACCUMULATED_BYTE + strlen(obj_code) / 2 > ONELINE_T_RECORD_BYTE_SIZE) new_line_flag = 1;
            if (new_line_flag) {
                char tmp[3];
                sprintf(tmp, "%02X", T_RECORD_ACCUMULATED_BYTE);
                ONELINE_T_RECORD[7] = tmp[0];
                ONELINE_T_RECORD[8] = tmp[1];
                //fprintf(fp_obj, "%s\n", ONELINE_T_RECORD);
                printf("%s\n", ONELINE_T_RECORD);
                T_RECORD_ACCUMULATED_BYTE = 0;
                new_line_flag = 0;
            }
            T_RECORD_ACCUMULATED_BYTE += strlen(obj_code) / 2;
            strcat(ONELINE_T_RECORD, obj_code);

            if (var_flag){
                var_flag = 0;
                continue;
            }
        }
        else{ // type == _COMMENT
            comment_flag = 1;
            strcpy(obj_code, "\0");
        }

        if (comment_flag){
            fprintf(fp_lst, "%3d %-4s %-35s %-s\n", (LINE_NUM++)*LINE_NUM_SCALE, "", line + 5, obj_code);
            //printf("%3d %-4s %-35s %-s\n", (LINE_NUM++)*LINE_NUM_SCALE, "", line + 5, obj_code);
            comment_flag = 0;
        }
        else{
            fprintf(fp_lst, "%3d %-40s %-s\n", (LINE_NUM++)*LINE_NUM_SCALE, line, obj_code);
            //printf("%3d %-40s %-s\n", (LINE_NUM++)*LINE_NUM_SCALE, line, obj_code);
        }
        fgets(line, LINE_LEN, fp_itm);
        line[strlen(line) - 1] = '\0';
        type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);
    }
    // _END 출력
    char tmp[3];
    sprintf(tmp, "%02X", T_RECORD_ACCUMULATED_BYTE);
    ONELINE_T_RECORD[7] = tmp[0];
    ONELINE_T_RECORD[8] = tmp[1];
    //fprintf(fp_obj, "%s\n", ONELINE_T_RECORD);
    printf("%s\n", ONELINE_T_RECORD);

    // M_record 출력
    for(int i=0; i<M_RECORD_NUM;i++){
        //fprintf(fp_obj, "%s\n", M_RECORDS[i]);
        printf("%s\n", M_RECORDS[i]);
    }
    fprintf(fp_obj, "E%06X", STARTING_ADDR);

    fprintf(fp_lst, "%3d %-4s %-35s", (LINE_NUM++)*LINE_NUM_SCALE, "", line+5);

    fclose(fp_obj);
    fclose(fp_lst);
    fclose(fp_itm);
    // itm file 삭제
}

INSTRUCTION line_split2(char *line, int *LOCCTR, char *LABEL, char *MNEMONIC, char *OP1, char *OP2) {
    char buf[LINE_LEN];
    char *ptr;

    strcpy(buf, line);
    ptr = strtok(buf, " \t");
    if (!ptr) return _ELSE;
    if (!(*ptr)) return _ELSE;
    *LOCCTR = hexstr_to_decint(ptr);

    return line_split(ptr + strlen(ptr) + 1, LABEL, MNEMONIC, OP1, OP2);
}

OK_or_ERR make_obj_code_by_a_line(char *ret, int PC_val, char *MNEMONIC, char *OP1, char *OP2) {
    int format = -1;
    int n, i, x, b, p, e;
    int TA = 0;
    int B_val = -1;
    char *ptr;
    char m_record[M_RECORD_LEN];

    OPCODE_MNEMONIC_MAP *opcode_memonic_map_node;
    SYM_node *sym1, *sym2;

    char *tmp_mnemonic;
    if (MNEMONIC[0] == '+') {
        strcpy(tmp_mnemonic, MNEMONIC + 1);
        e = 1;
    }
    else {
        strcpy(tmp_mnemonic, MNEMONIC);
        e = 0;
    }
    opcode_memonic_map_node = get_opcode2(tmp_mnemonic);

    if (e == 1) format = 4;
    else if (strcmp(opcode_memonic_map_node->format, "3/4") == 0) format = 3;
    else if (strcmp(opcode_memonic_map_node->format, "2") == 0) format = 2;
    else if (strcmp(opcode_memonic_map_node->format, "1") == 0) format = 1;
    PC_val += format;

    char tmp_op1[OPERAND_LEN];
    int REG1, REG2;
    switch (format) {
        case 1:
            sprintf(ret, "%02X", opcode_memonic_map_node->opcode);
            break;
        case 2:
            if (*OP1 == '\0') {
                printf("\"%s\" need a operand\n");
                return ASSEMBLY_CODE_ERR;
            }
            strcpy(tmp_op1, OP1);
            if (OP1[strlen(OP1) - 1] == ',') tmp_op1[strlen(OP1) - 1] = '\0';
            REG1 = hexstr_to_decint(OP1);
            if (REG1 >= 10 || REG1 == 7) {
                printf("symbol error: %s\n", tmp_op1);
                return ASSEMBLY_CODE_ERR;
            }

            if (*OP2 == '\0') {
                sprintf(ret, "%02X%X%X", opcode_memonic_map_node->opcode, REG1, 0);
                break;
            }
            else {
                if (OP1[strlen(OP1) - 1] != ',') {
                    printf("operand comma err!\n");
                    return COMMA_ERR;
                }
                strtok(OP1, " ,");
                REG2 = hexstr_to_decint(OP2);
                if (REG2 >= 10 || REG2 == 7) {
                    printf("symbol error: %s\n", OP2);
                    return ASSEMBLY_CODE_ERR;
                }
                sprintf(ret, "%02X%X%X", opcode_memonic_map_node->opcode, REG2, 0);
            }
            break;
        case 3:
            // n, i
            if (OP1[0] == '#') n = 0, i = 1;
            else if (OP1[0] == '@') n = 1, i = 0;
            else n = 1, i = 1;

            // x
            if (OP2[0] == 'X') x = 1;
            else x = 0;

            // b, p : PC_val relative 먼저 시도해보고, 불가능하다면 BASE relative
            ptr = strtok(OP1, " #@,");
            if (!ptr) b = 0, p = 0, TA = 0;
            else {
                sym1 = find_symbol(ptr);
                if (sym1) {
                    if (strcmp(MNEMONIC, "LDB") == 0) B_val = sym1->address; // set BASE

                    if (-2048 <= sym1->address - PC_val && sym1->address - PC_val <= 2047) {
                        b = 0, p = 1;
                        TA = sym1->address - PC_val;
                    }
                    else if (B_val > -1 && 0 <= sym1->address - B_val && sym1->address - B_val <= 4095) {
                        b = 1, p = 0;
                        TA = sym1->address - B_val;
                    }
                    else return ASSEMBLY_CODE_ERR;
                }
                else if (OP1[0] == '#') {
                    for (int i = 0; i < (int) strlen(OP1); i++) {
                        if (!isdigit(OP1[i])) return ASSEMBLY_CODE_ERR;
                    }
                    TA = atoi(OP1 + 1);
                    b = 0, p = 0;
                }
                else {
                    printf("symbol err: %s\n", ptr);
                    return ASSEMBLY_CODE_ERR;
                }
            }
            sprintf(ret, "%02X%0X%03X", opcode_memonic_map_node->opcode + 2 * n + i, 8 * x + 4 * b + 2 * p + e,
                    TA & 0xFFF);
            break;
        case 4:
            // n, i
            if (OP1[0] == '#') n = 0, i = 1;
            else if (OP1[0] == '@') n = 1, i = 0;
            else n = 1, i = 1;

            // x
            if (OP2[0] == 'X') x = 1;
            else x = 0;

            // b, p : PC_val relative 먼저 시도해보고, 불가능하다면 BASE relative
            ptr = strtok(OP1, " #@,");
            if (!ptr) b = 0, p = 0, TA = 0;
            else {
                sym1 = find_symbol(ptr);
                if (sym1) {
                    if (strcmp(MNEMONIC, "LDB") == 0) B_val = sym1->address; // set BASE

                    b = 0, p = 0;
                    TA = sym1->address;
                    sprintf(m_record, "M%06X%02X", PC_val - 3, FORMAT4_LEN);
                    strcpy(M_RECORDS[M_RECORD_NUM++], m_record);
                }
                else if (OP1[0] == '#') {
                    for (int i = 0; i < (int) strlen(OP1); i++) {
                        if (!isdigit(OP1[i])) return ASSEMBLY_CODE_ERR;
                    }
                    TA = atoi(OP1 + 1);
                    b = 0, p = 0;
                }
                else {
                    printf("symbol err: %s\n", ptr);
                    return ASSEMBLY_CODE_ERR;
                }
            }
            sprintf(ret, "%02X%0X%05X", opcode_memonic_map_node->opcode + 2 * n + i, 8 * x + 4 * b + 2 * p + e,
                    TA & 0xFFFFF);
            break;
    }
    return OK;
}

SYM_node *find_symbol(char *symbol) {
    SYM_node *cur_node = SYMTAB_HEAD;
    while (cur_node != NULL) {
        if (strcmp(cur_node->symbol, symbol) == 0) return cur_node;
    }
    return cur_node;
}