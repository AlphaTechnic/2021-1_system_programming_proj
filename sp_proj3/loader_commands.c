//
// Created by 김주호 on 2021/04/17.
//

#include "loader_commands.h"

OK_or_ERR set_PROGADDR(char *addr_hexstr) {
    int addr_int;

    addr_int = hexstr_to_decint(addr_hexstr);
    if (addr_int == RANGE_ERR) { // 적절하지 않은 형식의 address가 입력된 경우
        printf("range err!\n");
        return RANGE_ERR;
    }
    if (addr_int < 0 || addr_int >= MEM_SIZE) { // 입력된 address가 메모리 주소 범위를 초과한 경우
        printf("range err!\n");
        return RANGE_ERR;
    }
    PROG_ADDR = addr_int;
    REG[regPC] = addr_int;
    return OK;
}

OK_or_ERR loader(char filenames[MAX_FILES_NUM][MAX_FILENAME_LEN]) {
    FILE *fp;

    // free ESTAB
    ES_NODE *cur_node, *pre_node;
    for (int i = 0; i < ESTAB_HASH_SIZE; i++) {
        cur_node = ESTAB[i];
        while (cur_node) {
            pre_node = cur_node;
            cur_node = cur_node->nxt;
            free(pre_node);
        }
        ESTAB[i] = NULL;
    }

    printf("%-7s  %-7s  %-7s  %-7s\n", "control", "symbol", "address", "length");
    printf("%-7s  %-7s\n", "section", "name");
    printf("-----------------------------------\n");

    // pass1 진입 이전에 초기조건 설정
    TOTAL_LEN = 0;
    CS_ADDR = PROG_ADDR;
    FIRST_INSTRUCTION_ADDR = PROG_ADDR;

    // load pass1
    char *obj_file = filenames[1];
    for (int ind = 2; obj_file != NULL && *obj_file != '\0'; ind++){
        fp = fopen(obj_file, "r");
        if (!fp) return FILE_ERR;

        load_pass1(fp);
        fclose(fp);
        obj_file = filenames[ind];
    }
    printf("-----------------------------------\n");
    printf("%-7s  %-7s  %-7s  %04X   \n", " ", "  total", "length", TOTAL_LEN);


    // pass2 진입 이전에 초기조건 설정
    CS_ADDR = PROG_ADDR;
    FIRST_INSTRUCTION_ADDR = PROG_ADDR;

    // load pass2
    obj_file = filenames[1];
    for (int ind = 2; obj_file != NULL && *obj_file != '\0'; ind++){
        fp = fopen(obj_file, "r");
        if (!fp) return FILE_ERR;

        load_pass2(fp);
        fclose(fp);
        obj_file = filenames[ind];
    }
    // total len, exec addr
    // reset register
    for(int i = 0; i < REG_NUM; i++) REG[i] = 0;
    REG[regL] = TOTAL_LEN; //////////////////////
    REG[regPC] = FIRST_INSTRUCTION_ADDR;
    bp_visited = 0;

    return OK;
}

void load_pass1(FILE *fp) {
    char *ptr;
    char line[LINE_LEN];


    while (!feof(fp)) {
        fgets(line, LINE_LEN, fp);
        if (line[strlen(line)-1]=='\n') line[strlen(line)-1] = '\0';
        if (line[0] == 'H') {
            // get CS name
            char cs_name[SYMBOL_LEN];
            char CS_start_addr_hexstr[STR_ADDR_LEN];
            char CS_len_hexstr[STR_ADDR_LEN];
            int CS_start_addr_int;
            int CS_len_hexstr_int;

            ptr = strtok(line, " \r");
            ptr++;
            if (strlen(ptr) > 6) {
                strncpy(cs_name, ptr, 6);
                cs_name[6] = '\0';
                ptr += 6;
            }
            else {
                strcpy(cs_name, ptr);
                ptr = strtok(NULL, " \r");
            }

            // get starting address
            strncpy(CS_start_addr_hexstr, ptr, 6);
            CS_start_addr_hexstr[6] = '\0';
            CS_start_addr_int = hexstr_to_decint(CS_start_addr_hexstr);
            push_to_ESTAB(cs_name, CS_ADDR + CS_start_addr_int);

            // get control section 길이
            strncpy(CS_len_hexstr, ptr + 6, 6);
            CS_len_hexstr[6] = '\0';
            CS_LEN = hexstr_to_decint(CS_len_hexstr);
            TOTAL_LEN += CS_LEN;

            printf("%-7s  %-7s  %04X     %04X   \n", cs_name, " ", CS_ADDR + CS_start_addr_int, CS_LEN);
        }
        else if (line[0] == 'D') {
            char sym_name[SYMBOL_LEN];
            char sym_addr_hexstr[STR_ADDR_LEN];
            int sym_addr_int;

            ptr = strtok(line, " \r");
            ptr++;
            while (1) {
                // get name of symbol
                if (strlen(ptr) > 6) {
                    strncpy(sym_name, ptr, 6);
                    sym_name[6] = '\0';
                    ptr += 6;
                }
                else {
                    strcpy(sym_name, ptr);
                    ptr = strtok(NULL, " \r");
                }

                // get address of symbol
                strncpy(sym_addr_hexstr, ptr, 6);
                sym_addr_hexstr[6] = '\0';
                sym_addr_int = hexstr_to_decint(sym_addr_hexstr);
                push_to_ESTAB(sym_name, CS_ADDR + sym_addr_int);

                printf("%-7s  %-7s  %04X   \n", " ", sym_name, CS_ADDR + sym_addr_int);
                ptr += 6;
                if (*(ptr) == '\0') break;
            }
        }
        else if (line[0] == 'E') {
            CS_ADDR += CS_LEN;
        }
    }
}

void load_pass2(FILE *fp) {
    char *ptr;
    char line[MAX_LINE_NUM];
    int rf_table[MAX_RF_NUM];

    while (!feof(fp)) {
        fgets(line, LINE_LEN, fp);
        if (line[strlen(line)-1]=='\n') line[strlen(line)-1] = '\0';
        if (line[0] == 'H') {
            char cs_name[NAME_LEN];
            ptr = strtok(line, " \r");
            strcpy(cs_name, ptr + 1);

            ES_NODE *cs_node = find_ESNODE_or_NULL(cs_name);
            rf_table[1] = cs_node->addr;
            CS_ADDR = rf_table[1];
        }
        else if (line[0] == 'R') {
            char rf_ind_hexstr[NAME_LEN];
            char es_name[NAME_LEN];
            int rf_ind;
            ptr = strtok(line, " \r");
            ptr++;

            for (; ptr; ptr = strtok(NULL, " \r")) {
                strncpy(rf_ind_hexstr, ptr, 2);
                rf_ind_hexstr[2] = '\0';

                rf_ind = hexstr_to_decint(rf_ind_hexstr);
                if (rf_ind != RANGE_ERR) {
                    strcpy(es_name, ptr + 2);
                    ES_NODE *es_node = find_ESNODE_or_NULL(es_name);
                    rf_table[rf_ind] = es_node->addr;
                }
            }
        }
        else if (line[0] == 'T'){
            char starting_addr_hexstr[STR_ADDR_LEN];
            char one_line_T_record_len_hexstr[STR_ADDR_LEN];
            char byte_hexstr[STR_ADDR_LEN];
            int one_line_T_record_len_int;
            int byte_int;

            // get "obj_code가 올라갈 MEM의 시작주소"
            ptr = strtok(line, " \r");
            ptr++;
            strncpy(starting_addr_hexstr, ptr, 6);
            starting_addr_hexstr[6] = '\0';
            REG[regPC] = CS_ADDR + hexstr_to_decint(starting_addr_hexstr);
            ptr += 6;

            // get len of one line T record len
            strncpy(one_line_T_record_len_hexstr, ptr, 2);
            one_line_T_record_len_hexstr[2] = '\0';
            one_line_T_record_len_int = REG[regPC] + hexstr_to_decint(one_line_T_record_len_hexstr);
            ptr += 2;

            // MEM에 적재
            for(;REG[regPC] <= one_line_T_record_len_int; MEMORY[REG[regPC]++] = byte_int) {
                strncpy(byte_hexstr, ptr, 2);
                byte_hexstr[2] = '\0';
                ptr += 2;
                byte_int = hexstr_to_decint(byte_hexstr);
                if (byte_int == RANGE_ERR) {
                    printf("err! : wrong byte value!\n");
                    return;
                }
            }
        }
        else if (line[0] == 'M'){
            char loc_to_be_modified_hexstr[STR_ADDR_LEN];
            char halfbytes_len_to_be_modified_hexstr[STR_ADDR_LEN];
            char half_bytes_to_be_modified_str[STR_ADDR_LEN];
            int loc_to_be_modified_int;
            int halfbytes_len_to_be_modified_int;
            int rf_ind_int;
            int dx; // modify를 적용하는 양
            ES_NODE *es_node;


            // get "modified될 명령의 시작 위치"
            ptr = line + 1;
            strncpy(loc_to_be_modified_hexstr, ptr, 6);
            loc_to_be_modified_hexstr[6] = '\0';
            loc_to_be_modified_int = CS_ADDR + hexstr_to_decint(loc_to_be_modified_hexstr);
            ptr += 6;

            // get "수정될 halfbyte의 길이"
            strncpy(halfbytes_len_to_be_modified_hexstr, ptr, 2);
            halfbytes_len_to_be_modified_hexstr[2] = '\0';
            halfbytes_len_to_be_modified_int = hexstr_to_decint(halfbytes_len_to_be_modified_hexstr);
            ptr += 2;

            dx = 0;
            if(ptr){
                char rf_ind_or_rf_name[STR_ADDR_LEN];
                strcpy(rf_ind_or_rf_name, ptr + 1);
                strtok(rf_ind_or_rf_name, " \r");
                rf_ind_int = hexstr_to_decint(rf_ind_or_rf_name);
                if (rf_ind_int != RANGE_ERR){ // rf number를 이용한 modify가 적혀있는 경우(ex> +02)
                    dx = rf_table[rf_ind_int];
                }
                else{ // rf name을 이용한 modify가 적혀있는 경우 (ex> +LISTB)
                    es_node = find_ESNODE_or_NULL(rf_ind_or_rf_name);
                    dx = es_node->addr;
                }
                if (*ptr == '-') dx = -dx;
            }

            // 수정할 obj code를 가져온다.
            char tmp[STR_ADDR_LEN];
            strcpy(half_bytes_to_be_modified_str, "");
            for (int i=0; i<3; i++){
                sprintf(tmp, "%02X", MEMORY[loc_to_be_modified_int + i]);
                strcat(half_bytes_to_be_modified_str, tmp);
            }

            // dx 만큼 modified된 address 생성
            int addr_modified;
            if (halfbytes_len_to_be_modified_int == 5){
                addr_modified = hexstr_to_decint(half_bytes_to_be_modified_str + 1) + dx;
                sprintf(tmp, "%02X", MEMORY[loc_to_be_modified_int]);
                sprintf(half_bytes_to_be_modified_str, "%c%05X", tmp[0], addr_modified);
            }
            else if (halfbytes_len_to_be_modified_int == 6){
                addr_modified = twos_complement_str_to_decint(half_bytes_to_be_modified_str) + dx;
                sprintf(half_bytes_to_be_modified_str, "%06X", addr_modified);
                if (addr_modified < 0) {
                    for (int i = 0; i <= 6; i++) half_bytes_to_be_modified_str[i] = half_bytes_to_be_modified_str[i + 2];
                }
            }

            for (int i=0; i<3; i++){
                int byte_val;
                strncpy(tmp, half_bytes_to_be_modified_str + 2*i, 2);
                tmp[2] = '\0';
                byte_val = hexstr_to_decint(tmp);
                MEMORY[loc_to_be_modified_int + i] = byte_val;
            }
        }
        else if (line[0] == 'E'){
            if (line[1] != '\0'){
                char first_instrcution_addr_str[STR_ADDR_LEN];
                int first_instruction_addr_int;
                strncpy(first_instrcution_addr_str, line+1, 6);
                first_instruction_addr_int = hexstr_to_decint(first_instrcution_addr_str);
                FIRST_INSTRUCTION_ADDR = CS_ADDR + first_instruction_addr_int;
                break;
            }
        }
    }
}

void print_register(void) {
    printf("A : %06X   X : %06X\n", REG[regA], REG[regX]);
    printf("L : %06X  PC : %06X\n", REG[regL], REG[regPC]);
    printf("B : %06X   S : %06X\n", REG[regB], REG[regS]);
    printf("T : %06X\n", REG[regT]);
}

// **************ES table***********//
void push_to_ESTAB(char *es_name, int es_addr) {
    int ind = hash_func(es_name, ESTAB_HASH_SIZE);
    ES_NODE *cur_node = ESTAB[ind];
    ES_NODE *tar_node = malloc(sizeof(ES_NODE));

    // create node
    strcpy(tar_node->name, es_name);
    tar_node->addr = es_addr;
    tar_node->nxt = NULL;

    if (cur_node == NULL) {
        ESTAB[ind] = tar_node;
        return;
    }
    for (; cur_node->nxt; cur_node = cur_node->nxt) {
        if (strcmp(cur_node->name, es_name) == 0) { // err!
            free(tar_node);
            return;
        }
    }
    cur_node->nxt = tar_node;
}

ES_NODE *find_ESNODE_or_NULL(char *es_name) {
    int ind = hash_func(es_name, ESTAB_HASH_SIZE);
    ES_NODE *cur_node = ESTAB[ind];
    for (; cur_node; cur_node = cur_node->nxt) {
        if (strcmp(cur_node->name, es_name) == 0) {
            return cur_node;
        }
    }
    return NULL;
}
