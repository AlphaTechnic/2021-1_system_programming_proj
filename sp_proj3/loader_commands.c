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
    for (int ind = 2; obj_file != NULL && *obj_file != '\0'; ind++) {
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
    for (int ind = 2; obj_file != NULL && *obj_file != '\0'; ind++) {
        fp = fopen(obj_file, "r");
        if (!fp) return FILE_ERR;

        load_pass2(fp);
        fclose(fp);
        obj_file = filenames[ind];
    }

    // total len, exec addr
    // reset register
    for (int i = 0; i < REG_NUM; i++) REG[i] = 0;
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
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
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
        if (line[strlen(line) - 1] == '\n') line[strlen(line) - 1] = '\0';
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
        else if (line[0] == 'T') {
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
            for (; REG[regPC] <= one_line_T_record_len_int; MEMORY[REG[regPC]++] = byte_int) {
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
        else if (line[0] == 'M') {
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
            if (ptr) {
                char rf_ind_or_rf_name[STR_ADDR_LEN];
                strcpy(rf_ind_or_rf_name, ptr + 1);
                strtok(rf_ind_or_rf_name, " \r");
                rf_ind_int = hexstr_to_decint(rf_ind_or_rf_name);
                if (rf_ind_int != RANGE_ERR) { // rf number를 이용한 modify가 적혀있는 경우(ex> +02)
                    dx = rf_table[rf_ind_int];
                }
                else { // rf name을 이용한 modify가 적혀있는 경우 (ex> +LISTB)
                    es_node = find_ESNODE_or_NULL(rf_ind_or_rf_name);
                    dx = es_node->addr;
                }
                if (*ptr == '-') dx = -dx;
            }

            // 수정할 obj code를 가져온다.
            char tmp[STR_ADDR_LEN];
            strcpy(half_bytes_to_be_modified_str, "");
            for (int i = 0; i < 3; i++) {
                sprintf(tmp, "%02X", MEMORY[loc_to_be_modified_int + i]);
                strcat(half_bytes_to_be_modified_str, tmp);
            }

            // dx 만큼 modified된 address 생성
            int addr_modified;
            if (halfbytes_len_to_be_modified_int == 5) {
                addr_modified = hexstr_to_decint(half_bytes_to_be_modified_str + 1) + dx;
                sprintf(tmp, "%02X", MEMORY[loc_to_be_modified_int]);
                sprintf(half_bytes_to_be_modified_str, "%c%05X", tmp[0], addr_modified);
            }
            else if (halfbytes_len_to_be_modified_int == 6) {
                addr_modified = twos_complement_str_to_decint(half_bytes_to_be_modified_str) + dx;
                sprintf(half_bytes_to_be_modified_str, "%06X", addr_modified);
                if (addr_modified < 0) {
                    for (int i = 0; i <= 6; i++)
                        half_bytes_to_be_modified_str[i] = half_bytes_to_be_modified_str[i + 2];
                }
            }

            for (int i = 0; i < 3; i++) {
                int byte_val;
                strncpy(tmp, half_bytes_to_be_modified_str + 2 * i, 2);
                tmp[2] = '\0';
                byte_val = hexstr_to_decint(tmp);
                MEMORY[loc_to_be_modified_int + i] = byte_val;
            }
        }
        else if (line[0] == 'E') {
            if (line[1] != '\0') {
                char first_instrcution_addr_str[STR_ADDR_LEN];
                int first_instruction_addr_int;
                strncpy(first_instrcution_addr_str, line + 1, 6);
                first_instruction_addr_int = hexstr_to_decint(first_instrcution_addr_str);
                FIRST_INSTRUCTION_ADDR = CS_ADDR + first_instruction_addr_int;
                break;
            }
        }
    }
}

// **************ES table***********//
void push_to_ESTAB(char *es_name, int es_addr) {
    int ind = hash_func_by_mnemonic(es_name, ESTAB_HASH_SIZE);
    ES_NODE *cur_node = ESTAB[ind];
    ES_NODE *new_node = malloc(sizeof(ES_NODE));

    // create new node
    strcpy(new_node->name, es_name);
    new_node->addr = es_addr;
    new_node->nxt = NULL;

    if (cur_node == NULL) {
        ESTAB[ind] = new_node;
        return;
    }
    for (; cur_node->nxt; cur_node = cur_node->nxt) {
        if (strcmp(cur_node->name, es_name) == 0) { // err!
            free(new_node);
            return;
        }
    }
    cur_node->nxt = new_node;
}

ES_NODE *find_ESNODE_or_NULL(char *es_name) {
    int ind = hash_func_by_mnemonic(es_name, ESTAB_HASH_SIZE);
    ES_NODE *cur_node = ESTAB[ind];
    for (; cur_node; cur_node = cur_node->nxt) {
        if (strcmp(cur_node->name, es_name) == 0) {
            return cur_node;
        }
    }
    return NULL;
}

OK_or_ERR bp_command(int num_of_tokens, char *addr_hexstr_or_claer_str) {
    if (num_of_tokens == 1) {
        printf("\t\tbreakpoint\n");
        printf("\t\t----------\n");
        for (BP_NODE *cur_node = BP_LIST_HEAD; cur_node; cur_node = cur_node->nxt) printf("\t\t%X\n", cur_node->addr);
    }
    else if (num_of_tokens == 2 && strcmp(addr_hexstr_or_claer_str, "clear") == 0) {
        BP_NODE *cur_node, *pre_node;
        for (cur_node = BP_LIST_HEAD, pre_node = NULL; cur_node;
             pre_node = cur_node, cur_node = cur_node->nxt, free(pre_node)) {
            BP_CHK[cur_node->addr] = 0;
        }
        BP_LIST_HEAD = NULL;
        printf("\t\t[ok] clear all breakpoints\n");
    }
    else if (num_of_tokens == 2) {
        int addr_int = hexstr_to_decint(addr_hexstr_or_claer_str);
        if (addr_int < 0) return RANGE_ERR;
        if (addr_int > MEM_SIZE) return RANGE_ERR;

        // push to BPTABLE*************************
        BP_CHK[addr_int] = 1;
        printf("\t\t[ok] create breakpoint %s\n", addr_hexstr_or_claer_str);
    }
    else return RANGE_ERR;
}

void push_to_BPTAB(int addr) {
    BP_NODE *cur_node = BP_LIST_HEAD;
    BP_NODE *new_node = malloc(sizeof(BP_NODE));

    // create new node
    new_node->addr = addr;
    new_node->nxt = NULL;

    if (BP_LIST_HEAD == NULL) {
        BP_LIST_HEAD = new_node;
        return;
    }
    for (; cur_node->nxt; cur_node = cur_node->nxt);
    cur_node->nxt = new_node;
}

OK_or_ERR run(){
    int END = PROG_ADDR + CS_LEN;
    while(REG[regPC] < END){
        if (BP_CHK[REG[regPC]] && !bp_visited){
            // print register
            printf("A : %06X   X : %06X\n", REG[regA], REG[regX]);
            printf("L : %06X  PC : %06X\n", REG[regL], REG[regPC]);
            printf("B : %06X   S : %06X\n", REG[regB], REG[regS]);
            printf("T : %06X\n", REG[regT]);
            printf("\t\tStop at checkpoint[%X]\n", REG[regPC]);
            bp_visited = 1;
            return OK;
        }
        // 메모리의 명령을 수행******************
        bp_visited = 0;
    }
    // print register
    printf("A : %06X   X : %06X\n", REG[regA], REG[regX]);
    printf("L : %06X  PC : %06X\n", REG[regL], REG[regPC]);
    printf("B : %06X   S : %06X\n", REG[regB], REG[regS]);
    printf("T : %06X\n", REG[regT]);
    printf("\t\tEnd Program\n");
    return OK;
}

OK_or_ERR execute_instructions(){
    int opcode, format;
    int reg1, reg2;
    int disp = 0;
    int ni, x, b, p, e;

    int TA = 0;
    int start_loc = REG[regPC];
    int operand;

    OP_NODE *op_node;

    // 메모리에서 1 byte 읽어옴.
    opcode = MEMORY[start_loc];
    ni = opcode % 4;
    opcode -= ni;
    op_node = get_opcode_or_NULL_by_mnemonic(opcode);
    if (!op_node) return OBJ_CODE_ERR;

    // 해당 mnemonic의 format을 알아냄.
    if(*(op_node->format) == '1') format = 1;
    else if(*(op_node->format) == '2') format = 2;
    else{ // format == 3 또는 format == 4
        e = (MEMORY[start_loc + 1] & 0b10000) >> 4;
        if (e == 1) format = 4;
        else format = 3;
    }
    REG[regPC] += format;

    // get operands
    switch(format){
        case 2:
            reg1 = (MEMORY[start_loc + 1] & 0b11110000) >> 4;
            reg2 = MEMORY[start_loc + 1] & 0b00001111;
            break;
        case 3:
            x = (MEMORY[start_loc+1] & 0b10000000) >> 7;
            b =  (MEMORY[start_loc+1] & 0b1000000) >> 6;
            p =   (MEMORY[start_loc+1] & 0b100000) >> 5;
            disp = ((MEMORY[start_loc+1] & 0x0F) << 8) + MEMORY[start_loc+2];
            break;
        case 4:
            x = (MEMORY[start_loc+1] & 0b10000000) >> 7;
            b =  (MEMORY[start_loc+1] & 0b1000000) >> 6;
            p =   (MEMORY[start_loc+1] & 0b100000) >> 5;
            disp = ((MEMORY[start_loc+1] & 0x0F) << 16) + (MEMORY[start_loc+2] << 8) + MEMORY[start_loc+3];
            break;
        default:
            break;
    }

    // disp가 음수인 경우, 2의 보수표현 -> int 값으로 가져온다
    if(format == 3 && (disp & 0x800)) disp = disp | 0xFFFFF000;

    // operand 값 정하기
    if (format == 3 || format == 4){
        if (b == 1 && p == 0) TA = REG[regB] + disp; // b-relative addressing
        else if (b == 0 && p == 1) TA = REG[regPC] + disp; // p-relative addressing
        else if (b == 0 && p == 0) TA = disp; // direct addressing
    }
    if (x == 1) TA += REG[regX];

    // execute instructions
    switch (opcode){
        case 0XB4: // CLEAR
            REG[reg1] = 0;
            break;
        case 0x28: // COMP
            operand = LD_value(ni, TA, 3, format);
            if (REG[regA] > operand) CC = '>';
            else if (REG[regA] < operand) CC = '<';
            else CC = '=';
            break;
        case 0x3C: // J
            operand =

    }
}

int LD_value(int ni, int TA, int num_of_bytes, int format){
    int tar_val = 0;

    if (format == 4) return TA;
    switch (ni){
        case 0: // ni ==00 : considered as standard SIC instruction
            tar_val = -1;
        case 1 : // ni==01 : immediate addressing
            tar_val = TA;
            break;
        case 2:  // ni==10 : indirect addressing
            TA = MEMORY[TA];
            for (int i = 0; i < num_of_bytes; i++) tar_val += MEMORY[TA + num_of_bytes - 1 - i] << (i*8);
            break;
        case 3: // ni==11 : simple addressing
            for (int i = 0; i<num_of_bytes; i++) tar_val += MEMORY[TA + num_of_bytes - 1 - i] << (i*8);
            break;
        default:
            break;
    }
    return tar_val;
}

int J_addr(int ni, int TA, int format){
    int result = 0;

    if (format == 4) return TA;
    switch(ni){
        case 0: // ni == 00, J 관련 명령어에 해당 경우는 없다고 가정
            break;
        case 1: // ni == 10, J 관련 명령어에 해당 경우는 없다고 가정
            break;
        case 2: // ni == 10 : indirect addressing
            for (int i = 0; i < 3; i++) result += MEMORY[TA + 2 - i] << (i*8);
            break;
        case 3:
            result = TA;
        default:
            break;
    }
    return result;
}