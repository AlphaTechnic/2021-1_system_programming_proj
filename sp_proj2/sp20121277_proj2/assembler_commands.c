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

    char *pure_filename, *extension;
    int PROGRAM_SIZE;

    // 파일 열기에서 오류가 생겼을 때 처리
    char tmp[NAME_LEN];
    strcpy(tmp, filename);
    pure_filename = strtok(tmp, ".");
    if (!pure_filename) {
        fclose(fp);
        return FILE_ERR;
    }
    extension = strtok(NULL, "");
    if (strcmp(extension, "asm") != 0) {
        fclose(fp);
        return FILE_ERR;
    }

    // SYMTAB과 B register의 값을 초기화
    SYMTAB_HEAD = NULL;
    B_val = -1;

    // pass1 실행
    OK_or_ERR pass1_state = pass1(fp, pure_filename, &PROGRAM_SIZE);
    fclose(fp);
    if (pass1_state != OK) {
        free_SYMTAB(SYMTAB_HEAD);
        return FILE_ERR;
    }

    // pass2 실행
    OK_or_ERR pass2_state = pass2(pure_filename, PROGRAM_SIZE);
    if (pass2_state != OK) {
        free_SYMTAB(SYMTAB_HEAD);
        return FILE_ERR;
    }
    free_SYMTAB(LATEST_SYMTAB);

    // 가장 최근의 assemble에서 생성한 SYMTAB을 저장
    LATEST_SYMTAB = SYMTAB_HEAD;
    SYMTAB_HEAD = NULL;

    // assemble 성공 메세지
    printf("[%s.lst], [%s.obj]\n", pure_filename, pure_filename);
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : pass1*/
/*목적 : 1. .asm 파일을 한 줄씩 읽어들이면서, 각 line에 location counter를 할당하면서 intermediate(확장자 .imt) 파일을 생성한다.
        2. pass2에서 object code를 만들 때 참조할 SYMBOL TABLE을 생성한다.*/
/*리턴값 : OK - 성공,    FILE_ERR 또는 ASSEMBLY_CODE_ERR - 실패*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR pass1(FILE *fp, char *filename, int *PROGRAM_SIZE) {
    if (!fp) return FILE_ERR;

    char line[LINE_LEN];
    char LABEL[LABEL_LEN], MNEMONIC[MNEMONIC_LEN], OP1[MNEMONIC_LEN], OP2[MNEMONIC_LEN];
    char filename_itm[NAME_LEN];
    FILE *fp_itm;
    INSTRUCTION type; // asm파일 각 line에서 지정하는 명령의 종류를 9가지로 분류하였고, 어떤 유형에 해당하는 지를 저장하는 변수이다.
    OPCODE_MNEMONIC_MAP *opcode_mnemonic_map_node;

    int LOCCTR = 0, STARTING_ADDR = 0;
    int dl, LINE_NUM = 0;  // 'dl' means 'delta LOCCTR'

    // itm file 쓰기모드로 open
    strcpy(filename_itm, filename);
    strcat(filename_itm, ".itm");
    fp_itm = fopen(filename_itm, "w");

    // read a line from .asm file and parsing
    fgets(line, LINE_LEN, fp);
    line[strlen(line) - 1] = '\0';
    type = line_split(line, LABEL, MNEMONIC, OP1, OP2);

    // asm 파일의 가장 가장 첫 line에 START 슈도명령이 없다면 오류
    if (type != _START) {
        printf("Error! Check line number \"%d\"\n", 0);
        return ASSEMBLY_CODE_ERR;
    }
    //STARTING_ADDR = atoi(OP1);
    STARTING_ADDR = hexstr_to_decint(OP1);
    LOCCTR = STARTING_ADDR;

    // asm 첫 명령을 itm 파일에 기록
    fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
    //printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);

    // asm 파일을 읽고 parsing
    fgets(line, LINE_LEN, fp);
    line[strlen(line) - 1] = '\0';
    type = line_split(line, LABEL, MNEMONIC, OP1, OP2);

    // END type의 line이 나올때까지 계속 명령을 읽어들이고 parsing을 수행한다.
    while (1) {
        if (feof(fp)) {
            printf("Error! Check line number \"%d\"\n", (LINE_NUM + 1) * LINE_NUM_SCALE);
            ////******************* 여기서 itm 파일 삭제 *******************///////////
            fclose(fp_itm);
            remove(filename_itm);
            return ASSEMBLY_CODE_ERR;

        }
        // 무한loop에 빠지지 않도록 line을 충분히 많이 읽어들였음에도, END symbol을 만나지 않았다면, 오류를 return
        if (LINE_NUM++ >= MAX_LINE_NUM) {
            printf("Err! There's no 'END' pseudo instruction!\n");
            return ASSEMBLY_CODE_ERR;
        }

        if (type == _END) {
            // itm 파일에 기록
            fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
            //printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);

            // 파일을 닫고, 프로그램의 SIZE를 기록
            fclose(fp_itm);
            *PROGRAM_SIZE = LOCCTR - STARTING_ADDR;
            break;
        }
        if (type == _COMMENT) {
            while (type == _COMMENT) {
                // itm 파일에 기록
                fprintf(fp_itm, "%04X %-10s %s\n", LOCCTR, LABEL, MNEMONIC);
                //printf("%04X %-10s %s\n", LOCCTR, LABEL, MNEMONIC);

                // asm 파일을 읽고 parsing
                fgets(line, LINE_LEN, fp);
                line[strlen(line) - 1] = '\0';
                type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
            }
            continue;
        }

        // LABEL을 symtab에 기록
        if (*LABEL != '\0') {
            // LABEL이 SYMTAB에 있다면 duplication error이다.
            if (find_symbol_or_NULL(LABEL) != NULL) {
                printf("Error! Check line number \"%d\"\n", (LINE_NUM + 1) * LINE_NUM_SCALE);
                ////******************* 여기서 itm 파일 삭제 *******************///////////
                fclose(fp_itm);
                remove(filename_itm);
                return ASSEMBLY_CODE_ERR;
            }
            push_to_symtab(LABEL, LOCCTR);
        }
        // itm 파일에 기록
        fprintf(fp_itm, "%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);
        //printf("%04X %-10s %-10s %s %s\n", LOCCTR, LABEL, MNEMONIC, OP1, OP2);


        // MNEMONIC의 유형에 따라 각기 다른 LOCCTR의 증가량을 부여
        char mnemonic_refined[MNEMONIC_LEN];
        if (MNEMONIC[0] == '+') strcpy(mnemonic_refined, MNEMONIC + 1);
        else
            strcpy(mnemonic_refined, MNEMONIC);

        if (type == _OPERATION && (opcode_mnemonic_map_node = get_opcode_or_NULL(mnemonic_refined)) != NULL) {
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
        else if (type == _BYTE) dl = get_byte_size(OP1);
        else {
            printf("Error! Check line number \"%d\"\n", (LINE_NUM + 1) * LINE_NUM_SCALE);
            ////******************* 여기서 itm 파일 삭제 *******************///////////
            fclose(fp_itm);
            remove(filename_itm);
            return ASSEMBLY_CODE_ERR;
        }
        // LOCCTR 증가
        LOCCTR += dl;

        // asm 파일을 읽고 parsing
        fgets(line, LINE_LEN, fp);
        line[strlen(line) - 1] = '\0';
        type = line_split(line, LABEL, MNEMONIC, OP1, OP2);
    }
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : pass2*/
/*목적 : itm 파일의 line을 읽으면서 lst 파일과 obj 파일을 생성하는 함수이다.*/
/*리턴값 : OK - 성공, ERR - 실패*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR pass2(char *filename, int PROGRAM_SIZE) {
    // sub routine 실행의 결과 상태를 관리하는 변수
    OK_or_ERR state;

    // fp_itm은 읽기모드로, fp_lst와 fp_obj는 쓰기모드로 파일을 연다
    FILE *fp_itm, *fp_lst, *fp_obj;
    if (file_open(filename, &fp_itm, &fp_lst, &fp_obj) != OK) {
        return FILE_ERR;
    }

    // init M_records
    M_RECORD_NUM = 0;
    for (int i = 0; i < TOTAL_M_RECORD_SIZE; i++) {
        strcpy(M_RECORDS[i], "\0");
    }

    // 하나의 line에 대한 parsing과 관련한 변수들
    INSTRUCTION type;
    SYM_node *symbol_node;
    char obj_code[OBJ_CODE_LEN];
    char line[LINE_LEN];
    char LABEL[LABEL_LEN], MNEMONIC[MNEMONIC_LEN], OP1[OPERAND_LEN], OP2[OPERAND_LEN];

    // 프로그램의 정보들을 관리 및 저장하는 변수들
    char PROGRAM_NAME[NAME_LEN];
    int LOCCTR = 0, LINE_NUM = 1, STARTING_ADDR = 0;
    char ONELINE_T_RECORD[ONELINE_T_RECORD_LINE_SIZE];
    int T_RECORD_ACCUMULATED_BYTE = 0;
    int var_flag = 0, new_line_flag = 0;

    // itm 파일을 읽고 parsing
    fgets(line, LINE_LEN, fp_itm);
    line[strlen(line) - 1] = '\0';
    line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);

    // 프로그램 이름과 starting address, START symbol lst 파일에 기록
    fprintf(fp_lst, "%3d %-40s\n", (LINE_NUM++) * LINE_NUM_SCALE, line);
    //printf("-lst-%3d %-40s\n", (LINE_NUM++) * LINE_NUM_SCALE, line);
    STARTING_ADDR = LOCCTR;
    strcpy(PROGRAM_NAME, LABEL);

    // itm 파일의 두번째 line을 읽고 parsing
    fgets(line, LINE_LEN, fp_itm);
    line[strlen(line) - 1] = '\0';
    type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);

    // obj 파일에 H record 기록한다
    // line byte size를 일단 비워두고, char 1차원 배열에 T 레코드를 기록해간다
    //printf("-obj-H%-6s%06X%06X\n", PROGRAM_NAME, STARTING_ADDR, PROGRAM_SIZE);
    fprintf(fp_obj, "H%-6s%06X%06X\n", PROGRAM_NAME, STARTING_ADDR, PROGRAM_SIZE);
    sprintf(ONELINE_T_RECORD, "T%06X__", STARTING_ADDR);

    int cnt = 0;
    while (1) {
        cnt++;
        if (feof(fp_itm)) {
            printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
            ////******************* 여기서 파일 닫고 itm, obj, lsm 파일 삭제 *******************///////////
            fclose(fp_obj); fclose(fp_lst); fclose(fp_itm);
            char filename_itm[NAME_LEN]; char filename_lst[NAME_LEN]; char filename_obj[NAME_LEN];
            strcpy(filename_itm, filename); strcpy(filename_lst, filename); strcpy(filename_obj, filename);
            strcat(filename_itm, ".itm"); strcat(filename_lst, ".lst"); strcat(filename_obj, ".obj");
            remove(filename_itm); remove(filename_lst); remove(filename_obj);
            return ASSEMBLY_CODE_ERR;
        }
        if (type == _END) {
            // line byte size를 T 레코드에 기록
            char line_byte_size[3];
            sprintf(line_byte_size, "%02X", T_RECORD_ACCUMULATED_BYTE);
            ONELINE_T_RECORD[7] = line_byte_size[0];
            ONELINE_T_RECORD[8] = line_byte_size[1];
            fprintf(fp_obj, "%s\n", ONELINE_T_RECORD);
            //printf("-obj-%s\n", ONELINE_T_RECORD);

            // M_record 출력
            for (int i = 0; i < M_RECORD_NUM; i++) {
                fprintf(fp_obj, "%s\n", M_RECORDS[i]);
                //printf("-obj-%s\n", M_RECORDS[i]);
            }
            fprintf(fp_obj, "E%06X", STARTING_ADDR);
            fprintf(fp_lst, "%3d %-4s %-35s", (LINE_NUM++) * LINE_NUM_SCALE, "", line + 5);
            //printf("-obj-E%06X", STARTING_ADDR);
            //printf("-lst-E%06X", STARTING_ADDR);

            fclose(fp_obj);
            fclose(fp_lst);
            fclose(fp_itm);
            ////******************* 여기서 itm 파일 삭제 *******************///////////
            char filename_itm[NAME_LEN];
            strcpy(filename_itm, filename);
            strcat(filename_itm, ".itm");
            remove(filename_itm);
            break;
        }

        if (type == _COMMENT) {
            while (type == _COMMENT) {
                strcpy(obj_code, "\0");
                fprintf(fp_lst, "%3d %-4s %-35s %-s\n", (LINE_NUM++) * LINE_NUM_SCALE, "", line + 5, obj_code);
                //printf("-lst-%3d %-4s %-35s %-s\n", (LINE_NUM++)*LINE_NUM_SCALE, "", line + 5, obj_code);

                // itm 파일을 읽고 parsing
                fgets(line, LINE_LEN, fp_itm);
                line[strlen(line) - 1] = '\0';
                type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);
            }
            continue;
        }

        // line의 type에 따라 다른 방식으로 obj_code를 생성한다
        char mnemonic_refined[OBJ_CODE_LEN];
        strcpy(obj_code, "\0");
        if (MNEMONIC[0] == '+') strcpy(mnemonic_refined, MNEMONIC + 1);
        else
            strcpy(mnemonic_refined, MNEMONIC);

        if (type == _OPERATION && get_opcode_or_NULL(mnemonic_refined) != NULL) {
            state = make_obj_code(obj_code, LOCCTR, MNEMONIC, OP1, OP2, STARTING_ADDR);
            if (state != OK) {
                printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                ////******************* 여기서 파일 닫고 itm, obj, lsm 파일 삭제 *******************///////////
                fclose(fp_obj); fclose(fp_lst); fclose(fp_itm);
                char filename_itm[NAME_LEN]; char filename_lst[NAME_LEN]; char filename_obj[NAME_LEN];
                strcpy(filename_itm, filename); strcpy(filename_lst, filename); strcpy(filename_obj, filename);
                strcat(filename_itm, ".itm"); strcat(filename_lst, ".lst"); strcat(filename_obj, ".obj");
                remove(filename_itm); remove(filename_lst); remove(filename_obj);
                return ASSEMBLY_CODE_ERR;
            }
        }
        else if (type == _BYTE) {
            if (OP1[0] == 'C') {
                char characters[OPERAND_LEN];
                strcpy(characters, OP1);
                char hexstr[3];
                char *ptr = strtok(characters, " C'`");
                for (int i = 0; i < (int) strlen(ptr); i++) {
                    sprintf(hexstr, "%02X", (int) *(ptr + i));
                    strcat(obj_code, hexstr);
                }
            }
            else if (OP1[0] == 'X') {
                char hexstr[OPERAND_LEN];
                strcpy(hexstr, OP1);
                char *ptr = strtok(hexstr, " X'`");
                strcpy(obj_code, ptr);
            }
        }
        else if (type == _WORD) sprintf(obj_code, "%06X", atoi(OP1));
        else if (type == _BASE) {
            symbol_node = find_symbol_or_NULL(OP1);
            if (!symbol_node) {
                printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
                ////******************* 여기서 파일 닫고 itm, obj, lsm 파일 삭제 *******************///////////
                fclose(fp_obj); fclose(fp_lst); fclose(fp_itm);
                char filename_itm[NAME_LEN]; char filename_lst[NAME_LEN]; char filename_obj[NAME_LEN];
                strcpy(filename_itm, filename); strcpy(filename_lst, filename); strcpy(filename_obj, filename);
                strcat(filename_itm, ".itm"); strcat(filename_lst, ".lst"); strcat(filename_obj, ".obj");
                remove(filename_itm); remove(filename_lst); remove(filename_obj);
                return ASSEMBLY_CODE_ERR;
            }
            B_val = symbol_node->address;
        }
        else if (type == _RESW || type == _RESB) {
            while (type == _RESW || type == _RESB) {
                fprintf(fp_lst, "%3d %-40s %-s\n", (LINE_NUM++) * LINE_NUM_SCALE, line, obj_code);
                //printf("-lst-%3d %-40s %-s\n", (LINE_NUM++) * LINE_NUM_SCALE, line, obj_code);
                fgets(line, LINE_LEN, fp_itm);
                line[strlen(line) - 1] = '\0';
                type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);
            }
            var_flag = 1;
            new_line_flag = 1;
        }
        else {
            printf("Error! Check line number \"%d\"\n", LINE_NUM * LINE_NUM_SCALE);
            ////******************* 여기서 파일 닫고 itm, obj, lsm 파일 삭제 *******************///////////
            fclose(fp_obj); fclose(fp_lst); fclose(fp_itm);
            char filename_itm[NAME_LEN]; char filename_lst[NAME_LEN]; char filename_obj[NAME_LEN];
            strcpy(filename_itm, filename); strcpy(filename_lst, filename); strcpy(filename_obj, filename);
            strcat(filename_itm, ".itm"); strcat(filename_lst, ".lst"); strcat(filename_obj, ".obj");
            remove(filename_itm); remove(filename_lst); remove(filename_obj);
            return ASSEMBLY_CODE_ERR;
        }

        // 개행을 하는 경우 1. line byte size를 초과했을 때, 2. RESW, RESB 명령을 만나서 메모리공간을 사용할 때,
        if (T_RECORD_ACCUMULATED_BYTE + strlen(obj_code) / 2 > ONELINE_T_RECORD_BYTE_SIZE) new_line_flag = 1;
        if (new_line_flag) {
            char line_byte_size[3];
            sprintf(line_byte_size, "%02X", T_RECORD_ACCUMULATED_BYTE);
            ONELINE_T_RECORD[7] = line_byte_size[0];
            ONELINE_T_RECORD[8] = line_byte_size[1];

            // T레코드 라인 출력
            fprintf(fp_obj, "%s\n", ONELINE_T_RECORD);
            //printf("-obj-%s\n", ONELINE_T_RECORD);
            T_RECORD_ACCUMULATED_BYTE = 0;

            // 새로운 line에서 T레코드 기록 시작
            sprintf(ONELINE_T_RECORD, "T%06X__", LOCCTR);
            new_line_flag = 0;
        }
        T_RECORD_ACCUMULATED_BYTE += (int) strlen(obj_code) / 2;
        strcat(ONELINE_T_RECORD, obj_code);

        if (var_flag) {
            var_flag = 0;
            continue;
        }

        // lst 파일에 기록
        fprintf(fp_lst, "%3d %-40s %-s\n", (LINE_NUM++) * LINE_NUM_SCALE, line, obj_code);
        //printf("-lst-%3d %-40s %-s\n", (LINE_NUM++)*LINE_NUM_SCALE, line, obj_code);

        // itm 파일을 읽고 parsing
        fgets(line, LINE_LEN, fp_itm);
        line[strlen(line) - 1] = '\0';
        type = line_split2(line, &LOCCTR, LABEL, MNEMONIC, OP1, OP2);
    }
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : make_obj_code*/
/*목적 : mnemonic과 operand를 입력받아 object code를 생성하는 함수*/
/*리턴값 : OK - 성공,    ERR - 실패*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR make_obj_code(char *obj_code, int PC_val, char *MNEMONIC, char *OP1, char *OP2, int STARTING_ADDR) {
    int format = -1;
    int n, i, x, b, p, e;
    int DISP;
    char m_record[M_RECORD_LEN];

    OPCODE_MNEMONIC_MAP *opcode_memonic_map_node;
    int is_digit_operand = 1;
    SYM_node *sym;

    char mnemonic_refined[MNEMONIC_LEN];
    if (MNEMONIC[0] == '+') { // 4형식
        strcpy(mnemonic_refined, MNEMONIC + 1);
        e = 1;
    }
    else {
        strcpy(mnemonic_refined, MNEMONIC);
        e = 0;
    }
    opcode_memonic_map_node = get_opcode_or_NULL(mnemonic_refined);

    if (e == 1) format = 4;
    else if (strcmp(opcode_memonic_map_node->format, "3/4") == 0) format = 3;
    else if (strcmp(opcode_memonic_map_node->format, "2") == 0) format = 2;
    else if (strcmp(opcode_memonic_map_node->format, "1") == 0) format = 1;
    PC_val += format;  // 현재 line의 format을 안다면, PC 값을 알 수가 있다.

    REG_num REG1, REG2;
    switch (format) {
        case 1:
            sprintf(obj_code, "%02X", opcode_memonic_map_node->opcode);
            break;
        case 2:
            if (*OP1 == '\0') {
                printf("\"%s\" need a operand!\n", mnemonic_refined);
                return ASSEMBLY_CODE_ERR;
            }

            char reg_str[OPERAND_LEN];
            strcpy(reg_str, OP1);
            if (OP1[strlen(OP1) - 1] == ',') reg_str[strlen(OP1) - 1] = '\0';
            REG1 = get_REG_num(reg_str);
            if (REG1 == non_exist) { // 존재하지 않는 reg에 접근
                printf("There's no reg \"%s\"\n", reg_str);
                return ASSEMBLY_CODE_ERR;
            }

            if (*OP2 == '\0') { // reg operand가 1개인 경우(OP2가 없다)
                sprintf(obj_code, "%02X%X%X", opcode_memonic_map_node->opcode, REG1, 0);
                break;
            }

            // reg operand가 2개인 경
            if (OP1[strlen(OP1) - 1] != ',') { // operand 2개를 입력하는데 comma를 사이에 넣지 않았다면 오류
                return COMMA_ERR;
            }
            strtok(OP1, " ,");
            REG2 = get_REG_num(OP2);
            if (REG2 == non_exist) { // 존재하지 않는 reg에 접근
                printf("There's no reg \"%s\"\n", OP2);
                return ASSEMBLY_CODE_ERR;
            }
            sprintf(obj_code, "%02X%X%X", opcode_memonic_map_node->opcode, REG1, REG2);
            break;

        case 3:
            // n, i를 set
            if (OP1[0] == '#') n = 0, i = 1;    // immediate addressing
            else if (OP1[0] == '@') n = 1, i = 0;   // indirect addressing
            else n = 1, i = 1;  // direct addressing

            // x를 set
            if (OP2[0] != 'X') x = 0;   // index
            else x = 1;

            char *ptr;
            // b, p를 set : PC relative addressing 먼저 시도해보고, 불가능하다면 BASE relative addressing
            ptr = strtok(OP1, " #@,");

            // RSUB 같은 명령어는 operand가 없는 3형식 명령
            if (!ptr) {
                b = 0, p = 0, DISP = 0;
                sprintf(obj_code, "%02X%0X%03X", opcode_memonic_map_node->opcode + 2 * n + i, 8 * x + 4 * b + 2 * p + e,
                        DISP & 0xFFF);
                break;
            }

            // digit operand라면 symtab을 탐색할 필요가 없다
            for (int i = 0; i < (int) strlen(ptr); i++) {
                if (!isdigit(ptr[i])) {
                    is_digit_operand = 0;
                    break;
                }
            }
            if (is_digit_operand) {
                DISP = atoi(OP1 + 1);
                if (strcmp(MNEMONIC, "LDB") == 0) B_val = DISP;

                b = 0, p = 0;
                sprintf(obj_code, "%02X%0X%03X", opcode_memonic_map_node->opcode + 2 * n + i, 8 * x + 4 * b + 2 * p + e,
                        DISP & 0xFFF);
                break;
            }

            // sym tab에서 symbol을 꺼내어 opcode 생성
            sym = find_symbol_or_NULL(ptr);
            if (sym) {
                // set BASE
                if (strcmp(MNEMONIC, "LDB") == 0) B_val = sym->address;

                // PC relative를 최우선으로 시도
                DISP = sym->address - PC_val;
                if (-2048 <= DISP && DISP <= 2047) {
                    b = 0, p = 1;
                }
                    // BASE relative를 다음으로 시도
                else if (B_val > -1 && 0 <= sym->address - B_val && sym->address - B_val <= 4095) {
                    DISP = sym->address - B_val;
                    b = 1, p = 0;
                }
                    // 3형식인데 relative 접근이 되지 않는다면 에러
                else return ASSEMBLY_CODE_ERR;
            }
            else {
                printf("symbol err: %s\n", ptr);
                return ASSEMBLY_CODE_ERR;
            }
            sprintf(obj_code, "%02X%0X%03X", opcode_memonic_map_node->opcode + 2 * n + i, 8 * x + 4 * b + 2 * p + e,
                    DISP & 0xFFF);
            break;

        case 4:
            // n, i를 set
            if (OP1[0] == '#') n = 0, i = 1;
            else if (OP1[0] == '@') n = 1, i = 0;
            else n = 1, i = 1;

            // x를 set
            if (OP2[0] == 'X') x = 1;
            else x = 0;

            // b, p : PC_val relative 먼저 시도해보고, 불가능하다면 BASE relative
            ptr = strtok(OP1, " #@,");
            if (!ptr) b = 0, p = 0, DISP = 0;

            // digit_operand라면 symtab을 탐색할 필요가 없다.
            is_digit_operand = 1;
            for (int i = 0; i < (int) strlen(ptr); i++) {
                if (!isdigit(ptr[i])) {
                    is_digit_operand = 0;
                    break;
                }
            }
            if (is_digit_operand) {
                DISP = atoi(OP1 + 1);
                if (strcmp(MNEMONIC, "LDB") == 0) B_val = DISP;

                b = 0, p = 0;
                sprintf(obj_code, "%02X%0X%05X", opcode_memonic_map_node->opcode + 2 * n + i, 8 * x + 4 * b + 2 * p + e,
                        DISP & 0xFFFFF);
                break;
            }

            // sym tab에서 symbol을 꺼내어 opcode 생성
            SYM_node *sym;
            sym = find_symbol_or_NULL(ptr);
            if (sym) {
                // set BASE
                if (strcmp(MNEMONIC, "LDB") == 0) B_val = sym->address;
                b = 0, p = 0;

                DISP = sym->address;
                sprintf(obj_code, "%02X%0X%05X", opcode_memonic_map_node->opcode + 2 * n + i, 8 * x + 4 * b + 2 * p + e,
                        DISP & 0xFFFFF);
                sprintf(m_record, "M%06X%02X", PC_val - STARTING_ADDR - 3, FORMAT4_TA_LEN);
                strcpy(M_RECORDS[M_RECORD_NUM++], m_record);
            }
            else {
                printf("symbol err: %s\n", ptr);
                return ASSEMBLY_CODE_ERR;
            }
            break;
        default:
            break;
    }
    return OK;
}

/*------------------------------------------------------------------------------------*/
/*함수 : print_symbols*/
/*목적 : assemble 과정 중에 생성된 symbol table을 화면에 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void print_symbols() {
    SYM_node *cur_node = LATEST_SYMTAB;
    if (cur_node == NULL){
        printf("There's no symbol table. Assemble first!\n");
    }
    while (cur_node) {
        printf("\t%-10s %04X\n", cur_node->symbol, cur_node->address);
        cur_node = cur_node->nxt;
    }
}

/*------------------------------------------------------------------------------------*/
/*함수 : find_symbol_or_NULL*/
/*목적 : symtab을 탐색하여, 인자로 주어진 symbol string에 해당하는 정보를 찾는다*/
/*리턴값 : symbol string이 symtab에 있다면, 해당 정보를 return,  그렇지 않다면, NULL을 return*/
/*------------------------------------------------------------------------------------*/
SYM_node *find_symbol_or_NULL(char *symbol) {
    SYM_node *cur_node = SYMTAB_HEAD;
    for (; cur_node != NULL; cur_node = cur_node->nxt) {
        if (strcmp(cur_node->symbol, symbol) == 0) return cur_node;
    }
    return cur_node;
}

/*------------------------------------------------------------------------------------*/
/*함수 : push_to_symtab*/
/*목적 : symbol과 그에 대응하는 location 값을 symtab에 저장하는 함수이다.*/
/*리턴값 : 없음 */
/*------------------------------------------------------------------------------------*/
void push_to_symtab(char *symbol, int addr) {
    SYM_node *pre_node = NULL;
    SYM_node *cur_node = SYMTAB_HEAD;
    SYM_node *node_to_insert = malloc(sizeof(SYM_node));

    // 삽입할 노드 생성
    strcpy(node_to_insert->symbol, symbol);
    node_to_insert->address = addr;
    node_to_insert->nxt = NULL;

    // symtab이 비어있다면, node를 head에 바로 삽입
    if (!SYMTAB_HEAD) {
        SYMTAB_HEAD = node_to_insert;
        return;
    }

    // linked list를 순회하면서 알파벳 순서에 맞는 위치를 찾고 삽입하는 알고리즘
    for (; cur_node; pre_node = cur_node, cur_node = cur_node->nxt) {
        if (strcmp(cur_node->symbol, node_to_insert->symbol) > 0) {
            if (!pre_node) { // 삽입할 노드가 알파벳 가장 첫 순서라면, head에 삽입
                SYMTAB_HEAD = node_to_insert;
                node_to_insert->nxt = cur_node;
            }
            else { // 알파벳 순서에 맞게 삽입
                pre_node->nxt = node_to_insert;
                node_to_insert->nxt = cur_node;
            }
            return;
        }
    }
    // 삽입할 노드가 알파벳 마지막 순서라면, tail에 삽
    pre_node->nxt = node_to_insert;
}

/*------------------------------------------------------------------------------------*/
/*함수 : free_SYMTAB*/
/*목적 : SYMTAB에 할당되어 있는 데이터를 해제하는 함수*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void free_SYMTAB(SYM_node *head) {
    SYM_node *cur_node = head;
    SYM_node *pre_node = NULL;

    for (; cur_node; pre_node = cur_node, cur_node = cur_node->nxt) {
        free(pre_node);
    }
    head = NULL;
}
