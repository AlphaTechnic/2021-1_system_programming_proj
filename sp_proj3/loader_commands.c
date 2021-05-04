//
// Created by 김주호 on 2021/04/17.
//

#include "loader_commands.h"

/*------------------------------------------------------------------------------------*/
/*함수 : set_PROGADDR*/
/*목적 : 사용자로부터 입력받은 address로 program이 load될 메모리 시작주소 위치를 set한다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR set_PROGADDR(char *addr_hexstr) {
    int addr_int;

    addr_int = hexstr_to_decint(addr_hexstr);

    // address에 대한 유효성 검사
    if (addr_int == RANGE_ERR) { // 적절하지 않은 형식의 address가 입력된 경우
        printf("range err!\n");
        return RANGE_ERR;
    }
    if (addr_int < 0 || addr_int >= MEM_SIZE) { // 입력된 address가 메모리 주소 범위를 초과한 경우
        printf("range err!\n");
        return RANGE_ERR;
    }

    // 사용자로부터 입력 받은 address를
    // 1. program의 address로 설정
    // 2. PC 초기값으로 설정
    PROGRAM_ADDR = REG[regPC] = addr_int;
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : loader*/
/*목적 : pass1과 pass2 알고리즘을 call 하여, ES table을 생성(pass1)하고, 이를 이용한 load 작업을
 * 수행(pass2)한다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR loader(char filenames[MAX_FILES_NUM][MAX_FILENAME_LEN]) {
    FILE *fp;

    // loader()를 호출하면, 먼저 이전에 저장된 ESTAB을 할당 해제한다.
    free_ESTAB();

    // ********** pass1 ***********
    printf("%-7s  %-7s  %-7s  %-7s\n", "control", "symbol", "address", "length");
    printf("%-7s  %-7s\n", "section", "name");
    printf("-----------------------------------\n");
    // pass1 진입 이전에 초기조건 설정
    PROGRAM_LEN = 0;
    CS_ADDR = FIRST_INSTRUCTION_ADDR = PROGRAM_ADDR;

    // 사용자로부터 입력받은 모든 obj 파일에 대하여 pass1 알고리즘을 수행
    char *obj_file = filenames[1];
    for (int i = 2; obj_file != NULL && *obj_file != '\0'; i++) {
        fp = fopen(obj_file, "r");
        if (!fp) {
            printf("err! no file to open\n");
            return FILE_ERR;
        }

        load_pass1(fp);
        fclose(fp);
        obj_file = filenames[i];
    }
    printf("-----------------------------------\n");
    printf("%-7s  %-7s  %-7s  %04X   \n", " ", "  total", "length", PROGRAM_LEN);


    // ********** pass2 ***********
    // pass2 진입 이전에 초기조건 설정
    CS_ADDR = FIRST_INSTRUCTION_ADDR = PROGRAM_ADDR;

    // 입력받은 모든 obj 파일에 대하여 pass2 알고리즘을 수행
    obj_file = filenames[1];
    for (int i = 2; obj_file != NULL && *obj_file != '\0'; i++) {
        fp = fopen(obj_file, "r");
        if (!fp) {
            printf("wrong object code!\n");
            return FILE_ERR;
        }

        if (load_pass2(fp) != OK) {
            fclose(fp);
            return FILE_ERR;
        }
        fclose(fp);
        obj_file = filenames[i];
    }

    // reset REG
    for (int i = 0; i < REG_NUM; i++) REG[i] = 0;

    // 프로그램의 길이를 return address로 한다. (0번지에 sub program을 올린다는 가정)
    REG[regL] = PROGRAM_LEN;
    REG[regPC] = FIRST_INSTRUCTION_ADDR;
    PRINT_FLAG = 0;

    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : load_pass1*/
/*목적 : object file을 열어 H 레코드와 D 레코드를 참조하여, ES table을 완성한다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR load_pass1(FILE *fp) {
    char *ptr;
    char line[LINE_LEN];

    while (!feof(fp)) {
        fgets(line, LINE_LEN, fp);
        if (line[0] == 'H') {
            char CS_name[SYMBOL_LEN];
            char CS_start_addr_hexstr[STR_ADDR_LEN];
            char CS_len_hexstr[STR_ADDR_LEN];
            int CS_start_addr_int;

            line[strlen(line) - 1] = '\0';
            ptr = strtok(line, " \r");
            ptr++;

            // get CS_name
            strncpy(CS_name, ptr, 6);
            CS_name[6] = '\0';
            strtok(CS_name, " ");
            ptr += 6;

            // get starting address
            strncpy(CS_start_addr_hexstr, ptr, 6);
            CS_start_addr_hexstr[6] = '\0';
            CS_start_addr_int = hexstr_to_decint(CS_start_addr_hexstr);
            if (push_to_ESTAB(CS_name, CS_ADDR + CS_start_addr_int) == OBJ_CODE_ERR) {
                free_ESTAB();
                printf("CS name duplicate!\n");
                return OBJ_CODE_ERR;
            }

            // get Control Section 길이
            strncpy(CS_len_hexstr, ptr + 6, 6);
            CS_len_hexstr[6] = '\0';
            CS_LEN = hexstr_to_decint(CS_len_hexstr);
            // PROGRAM 길이에 CS 길이를 누적시킴
            PROGRAM_LEN += CS_LEN;

            printf("%-7s  %-7s  %04X     %04X   \n", CS_name, " ", CS_ADDR + CS_start_addr_int, CS_LEN);
        }
        else if (line[0] == 'D') {
            char SYM_name[SYMBOL_LEN];
            char sym_addr_hexstr[STR_ADDR_LEN];
            int sym_addr_int;

            line[strlen(line) - 1] = '\0';
            ptr = strtok(line, " \r");
            ptr++;
            while (1) {
                // get name of symbol
                strncpy(SYM_name, ptr, 6);
                SYM_name[6] = '\0';
                strtok(SYM_name, " ");
                ptr += 6;

                // get address of symbol and push to ESTAB
                strncpy(sym_addr_hexstr, ptr, 6);
                sym_addr_hexstr[6] = '\0';
                sym_addr_int = hexstr_to_decint(sym_addr_hexstr);
                if (push_to_ESTAB(SYM_name, CS_ADDR + sym_addr_int) == OBJ_CODE_ERR) {
                    free_ESTAB();
                    printf("symbol name duplicate!\n");
                    return OBJ_CODE_ERR;
                }

                printf("%-7s  %-7s  %04X   \n", " ", SYM_name, CS_ADDR + sym_addr_int);
                ptr += 6;
                if (*(ptr) == '\0') break;
            }
        }
        else if (line[0] == 'E') {
            // CS address를 다음 program이 load될 주소로 조정해둔다.
            CS_ADDR += CS_LEN;
            break;
        }
    }
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : load_pass2*/
/*목적 : pass1에서 완성한 ES table과 R, T, M 레코드를 참조하여 memory에 object 코드들을 적재한다*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR load_pass2(FILE *fp) {
    char *ptr;
    char line[MAX_LINE_NUM];
    int RFTAB[MAX_RF_NUM];

    while (!feof(fp)) {
        fgets(line, LINE_LEN, fp);
        if (line[0] == 'H') {
            char CS_name[NAME_LEN];

            line[strlen(line) - 1] = '\0';
            ptr = strtok(line, " \r");
            ptr++;

            // get CS_name
            strncpy(CS_name, ptr, 6);
            CS_name[6] = '\0';
            strtok(CS_name, " ");

            // CS_name의 address를 RFTAB[1]에 저장한다.
            ES_NODE *cs_node = find_ESNODE_or_NULL(CS_name);
            CS_ADDR = RFTAB[1] = cs_node->addr;
        }
        else if (line[0] == 'R') {
            char rf_ind_hexstr[NAME_LEN];
            int rf_ind;
            char ES_name[NAME_LEN];

            line[strlen(line) - 1] = '\0';
            ptr = strtok(line, " \r");
            ptr++;

            // RFTAB[rf_ind]에 해당 symbol의 address를 저장하는 작업
            for (; ptr; ptr = strtok(NULL, " \r")) {
                strncpy(rf_ind_hexstr, ptr, 2);
                rf_ind_hexstr[2] = '\0';

                rf_ind = hexstr_to_decint(rf_ind_hexstr);
                if (rf_ind != RANGE_ERR) {
                    strncpy(ES_name, ptr + 2, 6);
                    ES_name[6] = '\0';

                    ES_NODE *es_node = find_ESNODE_or_NULL(ES_name);
                    if (!es_node) {
                        free_ESTAB();
                        printf("err! check ES table\n");
                        return OBJ_CODE_ERR;
                    }
                    RFTAB[rf_ind] = es_node->addr;
                }
                else {
                    free_ESTAB();
                    printf("err! wrong reference number\n");
                    return OBJ_CODE_ERR;
                }
            }
        }
        else if (line[0] == 'T') {
            char starting_addr_hexstr[STR_ADDR_LEN];
            char one_line_T_record_len_hexstr[STR_ADDR_LEN];
            char byte_hexstr[STR_ADDR_LEN];
            int one_line_T_record_len_int;
            int byte_int;
            int LOC_to_push;

            line[strlen(line) - 1] = '\0';
            ptr = strtok(line, " \r");
            ptr++;

            // get "obj_code가 올라갈 MEM의 시작주소"
            strncpy(starting_addr_hexstr, ptr, 6);
            starting_addr_hexstr[6] = '\0';

            LOC_to_push = CS_ADDR + hexstr_to_decint(starting_addr_hexstr);
            ptr += 6;

            // get "LEN of one line T record"
            strncpy(one_line_T_record_len_hexstr, ptr, 2);
            one_line_T_record_len_hexstr[2] = '\0';
            one_line_T_record_len_int = LOC_to_push + hexstr_to_decint(one_line_T_record_len_hexstr);
            ptr += 2;

            // T record 1byte씩 접근하여 MEM에 적재
            for (; LOC_to_push <= one_line_T_record_len_int; MEMORY[LOC_to_push++] = byte_int) {
                strncpy(byte_hexstr, ptr, 2);
                byte_hexstr[2] = '\0';
                ptr += 2;
                byte_int = hexstr_to_decint(byte_hexstr);
                if (byte_int == RANGE_ERR) {
                    printf("err! : wrong byte value!\n");
                    return RANGE_ERR;
                }
            }
        }
        else if (line[0] == 'M') {
            char LOC_to_be_modified_hexstr[STR_ADDR_LEN];
            char halfbytes_len_to_be_modified_hexstr[STR_ADDR_LEN];
            char half_bytes_to_be_modified_str[STR_ADDR_LEN];
            int LOC_to_be_modified_int;
            int halfbytes_len_to_be_modified_int;
            int rf_ind_int;
            int dx; // modify를 적용하는 증분(음수일 수 있음). target object code에 dx만큼의 조정을 가하게 된다.

            // get "modified될 명령의 시작 위치"
            line[strlen(line) - 1] = '\0';
            ptr = line + 1;
            strncpy(LOC_to_be_modified_hexstr, ptr, 6);
            LOC_to_be_modified_hexstr[6] = '\0';
            LOC_to_be_modified_int = CS_ADDR + hexstr_to_decint(LOC_to_be_modified_hexstr);
            ptr += 6;

            // get "수정될 half byte의 길이"
            strncpy(halfbytes_len_to_be_modified_hexstr, ptr, 2);
            halfbytes_len_to_be_modified_hexstr[2] = '\0';
            halfbytes_len_to_be_modified_int = hexstr_to_decint(halfbytes_len_to_be_modified_hexstr);
            ptr += 2;

            // modify를 적용하는 증분을 RFTAB에서 꺼내온다.
            dx = 0;
            if (ptr) {
                char rf_ind_or_rf_name[STR_ADDR_LEN];
                strcpy(rf_ind_or_rf_name, ptr + 1);
                strtok(rf_ind_or_rf_name, " \r");
                rf_ind_int = hexstr_to_decint(rf_ind_or_rf_name);

                // rf number를 이용한 M레코드가 적여있다고 가정 (ex> +02)
                dx = RFTAB[rf_ind_int];
                if (*ptr == '-') dx = -dx;
            }

            // 수정할 obj code 3byte(=6 halfbyte)를 가져온다.
            char tmp[STR_ADDR_LEN];
            strcpy(half_bytes_to_be_modified_str, "");
            for (int i = 0; i < 3; i++) {
                sprintf(tmp, "%02X", MEMORY[LOC_to_be_modified_int + i]);
                strcat(half_bytes_to_be_modified_str, tmp);
            }

            // dx 만큼 modified된 objcode 생성
            int modified_val;
            // modify 해야할 object code가 5 half byte인 경우, 해당 부분은 주솟값이기 때문에 음의 값은 올 수 없고,
            // hexstr -> int 작업만 수행하면 된다.
            if (halfbytes_len_to_be_modified_int == 5) {
                modified_val = hexstr_to_decint(half_bytes_to_be_modified_str + 1) + dx;
                sprintf(tmp, "%02X", MEMORY[LOC_to_be_modified_int]);
                sprintf(half_bytes_to_be_modified_str, "%c%05X", tmp[0], modified_val);
            }
                // modify 해야할 object code가 6 half byte인 경우 해당 부분은 2의 보수표현 음수가 올 수 있어서
                // 혹 음수가 온 경우 이를 음숫값으로 계산하여 modified value를 계산해야한다.
                // ex) 0xFFFFFA를 16777210로 계산하면 안되고, -10이라고 계산해야한다.
            else if (halfbytes_len_to_be_modified_int == 6) {
                modified_val = twos_complement_str_to_decint(half_bytes_to_be_modified_str) + dx;
                sprintf(half_bytes_to_be_modified_str, "%06X", modified_val);
                if (modified_val < 0) {
                    for (int i = 0; i <= 6; i++)
                        half_bytes_to_be_modified_str[i] = half_bytes_to_be_modified_str[i + 2];
                }
            }

            // 생성한 modified val를 한 byte씩 메모리에 심는다.
            for (int i = 0; i < 3; i++) {
                int byte_val;
                strncpy(tmp, half_bytes_to_be_modified_str + 2 * i, 2);
                tmp[2] = '\0';
                byte_val = hexstr_to_decint(tmp);
                MEMORY[LOC_to_be_modified_int + i] = byte_val;
            }
        }
        else if (line[0] == 'E') {
            if (line[1] != '\0') {
                char first_instrcution_addr_str[STR_ADDR_LEN];
                strncpy(first_instrcution_addr_str, line + 1, 6);
                FIRST_INSTRUCTION_ADDR = CS_ADDR + hexstr_to_decint(first_instrcution_addr_str);
            }
            break;
        }
    }
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : push_to_ESTAB*/
/*목적 : ES table에 es node를 push하는 작업을 수행한다. ES table은 hash size 20인 hash map으로 구현되어있다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR push_to_ESTAB(char *es_name, int es_addr) {
    int ind = hash_func_by_mnemonic(es_name, ESTAB_HASH_SIZE);
    ES_NODE *cur_node = ESTAB[ind];
    ES_NODE *new_node = malloc(sizeof(ES_NODE));

    // create new node
    strcpy(new_node->name, es_name);
    new_node->addr = es_addr;
    new_node->nxt = NULL;

    // ES table의 해당 index 자리가 비어있는 경우 head에 삽입
    if (cur_node == NULL) {
        ESTAB[ind] = new_node;
        return OK;
    }
    // 비어있지 않다면, tail 에 삽입
    for (; cur_node->nxt; cur_node = cur_node->nxt) {
        // ES table에 이미 존재한다면, 오류
        if (strcmp(cur_node->name, es_name) == 0) {
            free(new_node);
            return OBJ_CODE_ERR;
        }
    }
    cur_node->nxt = new_node;
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : find_ESNODE_or_NULL*/
/*목적 : es name을 입력받아 ES table을 탐색하여 이에 해당하는 node를 돌려준다. 해당 es name이 없다면,
 * NULL 값을 return한다.*/
/*리턴값 : 해당 es_node - ES table에 존재하는 경우, NULL - ES table에 es name이 존재하지 않는 경*/
/*------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------*/
/*함수 : free_ESTAB*/
/*목적 : ESTAB에 할당된 메모리를 해제하는 함수이다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------*/
/*함수 : bp_command*/
/*목적 : bp 관련한 사용자의 명령에 따라 관련 작업을 수행하고, 그 내용은 아래와 같다.
 * 1. bp : bp가 걸린 메모리 주소를 보여준다.
 * 2. bp clear : set 되어 있는 bp값들을 모두 해제한다.
 * 3. bp [address] : 해당 [address]에 bp를 설정한다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR bp_command(int num_of_tokens, char *addr_hexstr_or_claer_str) {
    // bp 명령
    if (num_of_tokens == 1) {
        printf("\t\tbreakpoint\n");
        printf("\t\t----------\n");
        for (int i = 0; i < NUM_OF_BP; i++) printf("\t\t%X\n", BP_ADDR[i]);
    }

    // bp clear 명령
    else if (num_of_tokens == 2 && strcmp(addr_hexstr_or_claer_str, "clear") == 0) {
        for (int i = 0; i < NUM_OF_BP; i++){
            BP_CHK[BP_ADDR[i]] = 0;
        }
        NUM_OF_BP = 0;

        printf("\t\t[" CYN"ok"RESET "] clear all breakpoints\n");
    }

    // bp [address] 명령
    else if (num_of_tokens == 2) {
        int addr_int = hexstr_to_decint(addr_hexstr_or_claer_str);
        if (addr_int < 0) {
            printf("err! wrong address\n");
            return RANGE_ERR;
        }
        if (addr_int > MEM_SIZE) {
            printf("err! wrong address\n");
            return RANGE_ERR;
        }

        // bp가 걸린 address를 자료구조에 표시해둔다.
        // 1. MEMORY와 동일한 크기의 배열(BP_CHK)을 정의하고 해당 메모리 주소에 true(1)를 저장시킴
        // 2. true를 저장하고 있는 MEM 주소를 BP_ADDR에 저장한다.
        BP_CHK[addr_int] = 1;
        BP_ADDR[NUM_OF_BP++] = addr_int;
        printf("\t\t[" CYN"ok"RESET "] create breakpoint %s\n", addr_hexstr_or_claer_str);
    }
    else {
        printf("wrong command!\n");
        return CMD_ERR;
    }
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : run*/
/*목적 : machine instruction을 수행시키며, break이 걸린 point들을 잡아내어 그 때마다 register의 내용을
 * print한다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR run() {
    int END = PROGRAM_ADDR + CS_LEN;
    while (REG[regPC] < END) {
        if (BP_CHK[REG[regPC]] && PRINT_FLAG) {
            // print register
            printf("A : %06X   X : %06X\n", REG[regA], REG[regX]);
            printf("L : %06X  PC : %06X\n", REG[regL], REG[regPC]);
            printf("B : %06X   S : %06X\n", REG[regB], REG[regS]);
            printf("T : %06X\n", REG[regT]);
            printf("\t\tStop at checkpoint[%X]\n", REG[regPC]);

            // bp 지점에서 다시 run을 수행시, 해당위치에서 register 값을 또 출력하지 않도록 조정
            PRINT_FLAG = 0;
            return OK;
        }
        execute_instructions();
        PRINT_FLAG = 1;
    }
    // 프로그램 run 수행이 끝나면 print register
    printf("A : %06X   X : %06X\n", REG[regA], REG[regX]);
    printf("L : %06X  PC : %06X\n", REG[regL], REG[regPC]);
    printf("B : %06X   S : %06X\n", REG[regB], REG[regS]);
    printf("T : %06X\n", REG[regT]);
    printf("\t\tEnd Program\n");

    // 프로그램 수행이 끝나면 프로그램이 다시 처음부터 수행되도록 설정
    REG[regL] = PROGRAM_LEN;
    REG[regPC] = FIRST_INSTRUCTION_ADDR;
    PRINT_FLAG = 0;
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : execute_instructions*/
/*목적 : 메모리에 적재된 object code들을 메모리에서 읽어들여 machine instruction을 수행시킨다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR execute_instructions() {
    int opcode, format;
    int reg1, reg2;
    int disp = 0;
    int ni, x, b, p, e;
    ni = x = b = p = e = 0;

    int TA = 0;
    int start_loc = REG[regPC];
    int operand;

    OP_NODE *op_node;

    // 메모리에서 1 byte 읽어옴.
    opcode = MEMORY[start_loc];
    // get opcode and ni
    ni = opcode & 0b00000011;
    opcode -= ni;
    op_node = get_opcode_or_NULL_by_opcode(opcode);
    if (!op_node) {
        printf("wrong object code!\n");
        return OBJ_CODE_ERR;
    }

    // op_node에 기록된 format을 읽어옴.
    if (*(op_node->format) == '1') format = 1;
    else if (*(op_node->format) == '2') format = 2;
    else { // format == 3 또는 format == 4
        e = (MEMORY[start_loc + 1] & 0b10000) >> 4;
        if (e == 1) format = 4;
        else format = 3;
    }
    REG[regPC] += format;

    // get operands
    switch (format) {
        case 2:
            reg1 = (MEMORY[start_loc + 1] & 0xF0) >> 4;
            reg2 = MEMORY[start_loc + 1] & 0x0F;
            break;
        case 3:
            x = (MEMORY[start_loc + 1] & 0b10000000) >> 7;
            b = (MEMORY[start_loc + 1] & 0b1000000) >> 6;
            p = (MEMORY[start_loc + 1] & 0b100000) >> 5;
            // MEMORY[start_loc + 1]의 오른쪽 half byte와 MEMORY[start_loc + 2]를 결합해 disp 만듦
            disp = ((MEMORY[start_loc + 1] & 0x0F) << 8) + MEMORY[start_loc + 2];
            break;
        case 4:
            x = (MEMORY[start_loc + 1] & 0b10000000) >> 7;
            b = (MEMORY[start_loc + 1] & 0b1000000) >> 6;
            p = (MEMORY[start_loc + 1] & 0b100000) >> 5;
            // MEMORY[start_loc + 1]의 오른쪽 half byte와 MEMORY[start_loc + 2]와 MEMORY[start_loc + 3]를 결합해 disp 만듦
            disp = ((MEMORY[start_loc + 1] & 0x0F) << 16) + (MEMORY[start_loc + 2] << 8) + MEMORY[start_loc + 3];
            break;
        default:
            break;
    }

    // disp가 음수인 경우, 2의 보수표현 -> int 값으로 가져온다
    // "| 0xFFFFF000"은 disp값(3 halfbyte)에 앞에 FFFFF를 붙여서, 이를 32bit 2의 보수표현으로 읽히게 하는 작업이다
    if (format == 3 && (disp & 0x800)) disp = (int)(disp | 0xFFFFF000);

    // operand 값 정하기
    if (format == 3 || format == 4) {
        if (b == 1 && p == 0) TA = REG[regB] + disp; // b-relative addressing
        else if (b == 0 && p == 1) TA = REG[regPC] + disp; // p-relative addressing
        else if (b == 0 && p == 0) TA = disp; // direct addressing
    }
    if (x == 1) TA += REG[regX];

    // execute instructions
    switch (opcode) {
        case 0XB4: // CLEAR
            REG[reg1] = 0;
            break;
        case 0x28: // COMP
            operand = LD_related_instruction(ni, TA, format, 3);
            if (REG[regA] > operand) CC = '>';
            else if (REG[regA] < operand) CC = '<';
            else CC = '=';
            break;
        case 0xA0: // COMPR
            if (REG[reg1] > REG[reg2]) CC = '>';
            else if (REG[reg1] < REG[reg2]) CC = '<';
            else CC = '=';
            break;
        case 0x3C: // J
            REG[regPC] = J_related_instruction(ni, TA, format);
            break;
        case 0x30: // JEQ
            if (CC == '=') REG[regPC] = J_related_instruction(ni, TA, format);
            break;
        case 0x38: // JLT
            if (CC == '<') REG[regPC] = J_related_instruction(ni, TA, format);
            break;
        case 0x48: // JSUB
            REG[regL] = REG[regPC];
            REG[regPC] = J_related_instruction(ni, TA, format);
            break;
        case 0x00: // LDA
            REG[regA] = LD_related_instruction(ni, TA, format, 3);
            break;
        case 0x68: // LDB
            REG[regB] = LD_related_instruction(ni, TA, format, 3);
            break;
        case 0x50: // LDCH
            operand = LD_related_instruction(ni, TA, format, 1);
            REG[regA] = (int)((REG[regA] & 0xFFFFFF00) | (0x000000FF & operand)); // rightmost byte
            break;
        case 0x70: // LDF
            REG[regF] = LD_related_instruction(ni, TA, format, 6);
            break;
        case 0x08: // LDL
            REG[regL] = LD_related_instruction(ni, TA, format, 3);
            break;
        case 0x6C: // LDS
            REG[regS] = LD_related_instruction(ni, TA, format, 3);
            break;
        case 0x74: // LDT
            REG[regT] = LD_related_instruction(ni, TA, format, 3);
            break;
        case 0x04: // LDX
            REG[regX] = LD_related_instruction(ni, TA, format, 3);
            break;
        case 0xD8: // RD
            // input device로부터 아무것도 받지 못했다는 가정. input device로부터 NULL(ascii==0)을 읽어온다
            REG[regA] = (int)(REG[regA] & 0xFFFFFF00);
            break;
        case 0x4C: // RSUB
            REG[regPC] = REG[regL];
            break;
        case 0x0C: // STA
            ST_related_instruction(ni, TA, REG[regA], 3);
            break;
        case 0x78: // STB
            ST_related_instruction(ni, TA, REG[regB], 3);
            break;
        case 0x54: // STCH
            ST_related_instruction(ni, TA, REG[regA], 1);
            break;
        case 0x80: // STF
            ST_related_instruction(ni, TA, REG[regF], 6);
            break;
        case 0x14: // STL
            ST_related_instruction(ni, TA, REG[regL], 3);
            break;
        case 0x7C: // STS
            ST_related_instruction(ni, TA, REG[regS], 3);
            break;
        case 0xE8: // STSW
            ST_related_instruction(ni, TA, REG[regSW], 3);
            break;
        case 0x84: // STT
            ST_related_instruction(ni, TA, REG[regT], 3);
            break;
        case 0x10: // STX
            ST_related_instruction(ni, TA, REG[regX], 3);
            break;
        case 0xE0: // TD
            CC = '<';
            break;
        case 0xB8: // TIXR
            REG[regX] += 1;
            if (REG[regX] > REG[reg1]) CC = '>';
            else if (REG[regX] < REG[reg1]) CC = '<';
            else CC = '=';
            break;
        case 0xDC: // WD
            break;
        default:
            printf("wrong access! : 0x%02X\n", REG[regPC]);
            return OBJ_CODE_ERR;
            break;
    }
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : LD_related_instruction*/
/*목적 : operand로 (m..m+2)가 오는 즉, operand로 'MEM가 가지고 있는 value'가 오는 경우의 instruction을
 * 처리한다.*/
/*리턴값 : tar_val*/
/*------------------------------------------------------------------------------------*/
int LD_related_instruction(int ni, int TA, int format, int num_of_bytes) {
    int Val = 0;

    if (format == 4) return TA;
    switch (ni) {
        case 0: // ni == 00 : considered as standard SIC instruction
            // COPY 프로그램에 해당 유형의 instruction은 존재하지 않는다고 가정.
            break;
        case 1 : // ni == 01 : immediate addressing
            Val = TA;
            break;
        case 2:  // ni == 10 : indirect addressing
            TA = MEMORY[TA];
            for (int i = 0; i < num_of_bytes; i++) Val += MEMORY[TA + num_of_bytes - 1 - i] << (i * 8);
            break;
        case 3: // ni == 11 : simple addressing
            for (int i = 0; i < num_of_bytes; i++) Val += MEMORY[TA + num_of_bytes - 1 - i] << (i * 8);
            break;
        default:
            break;
    }
    return Val;
}

/*------------------------------------------------------------------------------------*/
/*함수 : J_related_instruction*/
/*목적 : operand로 m 가 오는 즉, operand로 MEM의 주소가 오는 경우의 mnemonic을 처리한다.*/
/*리턴값 : tar_addr*/
/*------------------------------------------------------------------------------------*/
int J_related_instruction(int ni, int TA, int format) {
    int Addr = 0;
    if (format == 4) return TA;

    switch (ni) {
        case 2: // ni == 10 : indirect addressing
            for (int i = 0; i < 3; i++) Addr += MEMORY[TA + 2 - i] << (i * 8);
            break;
        case 3: // ni == 11 : simple addressing
            Addr = TA;
            break;
        default:
            break;
    }
    return Addr;
}

/*------------------------------------------------------------------------------------*/
/*함수 : ST_related_instruction*/
/*목적 : operand로 m..m+2가 오며, 해당 메모리 주소에 val_to_push를 적재한다. */
/*리턴값 : 없음 */
/*------------------------------------------------------------------------------------*/
void ST_related_instruction(int ni, int TA, int val_to_push, int num_of_bytes) {
    if (ni == 2) TA = MEMORY[TA]; // ni == 10 : indirect addressing

    switch (num_of_bytes) {
        case 1:
            MEMORY[TA] = val_to_push & 0x000000FF;
            break;
        case 3:
            // binary로 표현된 tar_val를 오른쪽에서부터 8bit(=1byte)씩 접근하면서, 메모리에 적재
            for (int i = 0; i < num_of_bytes; i++)
                MEMORY[TA + 2 - i] = (val_to_push & (0x000000FF << (8 * i))) >> (8 * i);
            break;
        case 6:
            // binary로 표현된 tar_val를 오른쪽에서부터 8bit(=1byte)씩 접근하면서, 메모리에 적재
            for (int i = 0; i < num_of_bytes; i++)
                MEMORY[TA + 5 - i] = (val_to_push & (0x0000000000FF << (8 * i))) >> (8 * i);
            break;
        default:
            break;
    }
}

