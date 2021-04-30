//
// Created by 김주호 on 2021/04/17.
//

#include "loader_commands.h"

OK_or_ERR prog_addr(char *addr_hexstr) {
    int addr_int = -1;

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

OK_or_ERR loader(char filename) {
    // print loadmap
    printf("%-7s  %-7s  %-7s  %-7s\n", "control", "symbol", "address", "length");
    printf("%-7s  %-7s\n", "section", "name");
    printf("-----------------------------------\n");

}

void load_pass1(FILE *fp) {
    char *ptr;
    char line[LINE_LEN];


    while (!feof(fp)) {
        fgets(fp, LINE_LEN, line);
        if (line[0] == 'H') {
            // get CS name
            char cs_name[SYMBOL_LEN];
            char CS_start_addr_hexstr[STR_ADDR_LEN];
            int CS_start_addr_int;

            ptr = strtok(line, " ");
            ptr++;
            if (strlen(ptr) > 6) {
                strncpy(cs_name, ptr, 6);
                cs_name[6] = '\0';
                ptr += 6;
            }
            else {
                strcpy(cs_name, ptr);
                ptr = strtok(NULL, " ");
            }

            // get starting address
            strncpy(CS_start_addr_hexstr, ptr, 6);
            CS_start_addr_hexstr[6] = '\0';
            CS_start_addr_int = hexstr_to_decint(CS_start_addr_hexstr);
            push_to_ESTAB(cs_name, CS_start_addr_int);

            // get control section 길이
            strncpy(CS_start_addr_hexstr, ptr + 6, 3);
            CS_LEN = hexstr_to_decint(CS_start_addr_hexstr);
            TOTAL_LEN += CS_LEN;

            printf("%-7s  %-7s  %04X     %04X   \n", cs_name, " ", CS_ADDR + CS_start_addr_int, CS_LEN);
        }
        else if (line[0] == 'D') {
            char sym_name[SYMBOL_LEN];
            char sym_addr_hexstr[STR_ADDR_LEN];
            int sym_addr_int;

            ptr = strtok(line, " ");
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
                    ptr = strtok(NULL, " ");
                }

                // get address of symbol
                strncpy(sym_addr_hexstr, ptr, 6);
                sym_addr_hexstr[6] = '\0';
                sym_addr_int = hexstr_to_decint(sym_addr_hexstr);
                push_to_ESTAB(sym_name, sym_addr_int);

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
    int addr_int;

    int rf_table[MAX_RF_NUM];
    int rf_ind;


    while (!feof(fp)) {
        fgets(fp, LINE_LEN, line);
        if (line[0] == 'H') {
            char cs_name[NAME_LEN];
            ptr = strtok(line, " ");
            strcpy(cs_name, ptr + 1);

            ES_NODE *cs_node = find_ESNODE_or_NULL(cs_name);
            rf_table[1] = cs_node->addr;
            CS_ADDR = rf_table[1];
        }
        else if (line[0] == 'R') {
            char rf_ind_hexstr[NAME_LEN];
            char es_name[NAME_LEN];
            ptr = strtok(line, " ");
            ptr++;

            for (; ptr; ptr = strtok(NULL, " ")) {
                strncpy(rf_ind_hexstr, ptr, 2);
                es_name[2] = '\0';
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
            ptr = strtok(line, " ");
            ptr++;
            strcpy(starting_addr_hexstr, ptr + 2);
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
                byte_int = hexstr_to_decint(byte_hexstr);
                if (byte_int == RANGE_ERR) {
                    printf("err! : wrong byte value!\n");
                    return;
                }
            }
        }
        else if (line[0] == 'M'){
            char loc_to_be_modified_hexstr[STR_ADDR_LEN];
            char len_to_be_modified_hexstr[STR_ADDR_LEN];
            int loc_to_be_modified_int;
            char len_to_be_modified_int;

            // get "modified될 명령의 시작 위치"
            ptr = line + 1;
            strncpy(loc_to_be_modified_hexstr, ptr, 6);
            loc_to_be_modified_hexstr[6] = '\0';
            loc_to_be_modified_int = CS_ADDR + hexstr_to_decint(loc_to_be_modified_hexstr);
            ptr += 6;

            // get "수정될 halfbyte의 길이"
            strncpy(loc_to_be_modified_hexstr, ptr, 2);
            loc_to_be_modified_hexstr[2] = '\0';

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
        ESTAB[ind] = cur_node;
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

void free_ESTAB() {
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
}
