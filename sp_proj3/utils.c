//
// Created by 김주호 on 2021/03/12.
//

#include "utils.h"

/*------------------------------------------------------------------------------------*/
/*함수 : get_dx_to_nxt_token*/
/*목적 : 현 token의 첫번째 index와 next token의 첫번째 index와의 차이를 계산한다*/
/*리턴값 : dx - 현 token의 첫번째 index와 next token의 첫번째 index와의 차이*/
/*------------------------------------------------------------------------------------*/
int get_dx_to_nxt_token(char *start_ptr) {
    int dx = 0;
    if (start_ptr[dx] == '\0') return -1;
    for (dx = 0;
         start_ptr[dx] != ' ' && start_ptr[dx] != '\t' && start_ptr[dx] != '\0' && start_ptr[dx] != ',';
         dx++);

    if (start_ptr[dx] == '\0') {
        IS_COMMA = NO_COMMA;
        return dx;
    }

    if (start_ptr[dx] == ',')  {
        IS_COMMA = YES_COMMA;
    }
    else { // start_ptr[dx] == ' ' or start_ptr[dx] == '\t' or start_ptr[dx] == '\0'
        IS_COMMA = NO_COMMA;
    }

    start_ptr[dx++] = '\0'; // token 끝에 NULL을 삽입
    for (; start_ptr[dx] == ' ' || start_ptr[dx] == '\t'; dx++); // 다음 토큰의 첫 ind까지 접근
    return dx;
}

/*------------------------------------------------------------------------------------*/
/*함수 : hexstr_to_decint*/
/*목적 : 문자열 hexstr을 int형 decimal 형태로 바꾼다*/
/*리턴값 : res - 문자열 hexstr을 int형 decimal 형태로 바꾼 결과*/
/*------------------------------------------------------------------------------------*/
int hexstr_to_decint(char *hexstr) {
    int cur, res = 0;
    int scale = 1;
    for (int i = strlen(hexstr) - 1; i >= 0; i--) {
        if (hexstr[i] >= '0' && hexstr[i] <= '9') cur = hexstr[i] - '0';
        else if (hexstr[i] >= 'A' && hexstr[i] <= 'F') cur = hexstr[i] - 'A' + 10;
        else if (hexstr[i] >= 'a' && hexstr[i] <= 'f') cur = hexstr[i] - 'a' + 10;
        else return RANGE_ERR; // contain wrong hexstr symbol

        res += cur * scale;
        scale *= 16;
    }
    return res;
}

/*------------------------------------------------------------------------------------*/
/*함수 : twos_complement_str_to_decint*/
/*목적 : 주어진 hex str에 대한 유효성 검사와 문자열 hex str을 signed 32bit hex 표현으로 간주하고,
 * 이에 조응하는 dec int값으로 변환한다.*/
/*리턴값 : res - 문자열 hexstr을 int형 signed 32bit 표현으로 바꾼 결과*/
/*------------------------------------------------------------------------------------*/
int twos_complement_str_to_decint(char *hexstr) {
    int res;
    // 주어진 hexstr을 decimal int 값로 바꾸며, 유효성 검사 실
    res = hexstr_to_decint(hexstr);
    if (res == RANGE_ERR) return RANGE_ERR; // range err!

    // ascii('0') == 48 and ascii('7') == 55
    // 주어진 hexstr 값이 양수라면, 값을 그대로 return
    if (hexstr[0] >= 48 && hexstr[0] <= 55) return res;

    // 주어진 hexstr 값이 음수라면, 값을 2의 보수표현으로 읽어 return
    // 주어진 hexstr bit 앞에 FF를 붙여 음수 bit가 반영된 32bit 수로 변환한다.
    return res | 0xFF000000;
}

/*------------------------------------------------------------------------------------*/
/*함수 : hash_func_by_mnemonic*/
/*목적 : string을 입력 받아 이를 0부터 19까지 20개의 숫자에 mapping한다. 입력 받은 명령어를 구성하는 모든 문자의
 * ASCII  값을 더하여, 20으로 나눈다.*/
/*리턴값 : total % max_hash_size*/
/*------------------------------------------------------------------------------------*/
int hash_func_by_mnemonic(char *string, int max_hash_size) {
    int total = 0;
    for (int i = 0; i < (int)strlen(string); i++) total += string[i];
    return total % max_hash_size;
}

/*------------------------------------------------------------------------------------*/
/*함수 : hash_func_by_opcode*/
/*목적 : int형 opcode를 이를 0부터 19까지 20개의 숫자에 mapping한다.*/
/*리턴값 : total % max_hash_size*/
/*------------------------------------------------------------------------------------*/
int hash_func_by_opcode(int opcode) {
    return opcode % MAX_HASH_SIZE;
}

/*------------------------------------------------------------------------------------*/
/*함수 : line_split*/
/*목적 : line을 char 포인터로 입력받아 parsing하여 line에서 LABEL, MNEMONIC, OP1, OP2를 추출하고 정하여 caller에게 알려준다.*/
/*리턴값 : type(어떤 유형의 line인지) */
/*------------------------------------------------------------------------------------*/
INSTRUCTION line_split(char *line, char *LABEL, char *MNEMONIC, char *OP1, char *OP2) {
    char buf[LINE_LEN];
    char *ptr;
    INSTRUCTION type;

    // buf, LABEL, MNEMONIC, OP1, OP2 초기화
    strcpy(buf, "\0"); strcpy(buf, line);
    strcpy(LABEL, "\0"); strcpy(MNEMONIC, "\0"); strcpy(OP1, "\0"); strcpy(OP2, "\0");

    ptr = strtok(buf, " \t\r");
    if (!ptr) return _ELSE;
    if (!(*ptr)) return _ELSE;

    // line이 주석이라면 바로 return.
    if (buf[0] == '.') {
        strcpy(LABEL, ptr);
        ptr = strtok(NULL, " \t\r");
        if (!ptr) return _COMMENT;

        *(ptr + strlen(ptr)) = ' ';
        strcpy(MNEMONIC, ptr);
        return _COMMENT;
    }

    // line에서 LABEL 추출
    if (isalpha(buf[0])) {
        strcpy(LABEL, ptr);
        ptr = strtok(NULL, " \t\r");
    }
    // line에서 MNEMONIC 추출
    strcpy(MNEMONIC, ptr);
    type = get_instruction(MNEMONIC);

    // line에서 OP1 추출
    ptr = strtok(NULL, " \t\r");
    if (!ptr) return type;
    strcpy(OP1, ptr);

    // line에서 OP2 추출
    ptr = strtok(NULL, " \t\r");
    if (!ptr) return type;
    strcpy(OP2, ptr);

    return type;
}

/*------------------------------------------------------------------------------------*/
/*함수 : line_split2*/
/*목적 : line_split 함수의 parsing 기능을 수행하면서 추가로 LOCCTR 값을 decimal int 값으로 저장한다*/
/*리턴값 : line의 종류*/
/*------------------------------------------------------------------------------------*/
INSTRUCTION line_split2(char *line, int *LOCCTR, char *LABEL, char *MNEMONIC, char *OP1, char *OP2) {
    char tmp[LINE_LEN];
    char *ptr;

    strcpy(tmp, line);
    ptr = strtok(tmp, " \t");
    if (!ptr) return _ELSE;
    if (!(*ptr)) return _ELSE;
    *LOCCTR = hexstr_to_decint(ptr);

    return line_split(ptr + strlen(ptr) + 1, LABEL, MNEMONIC, OP1, OP2);
}

/*------------------------------------------------------------------------------------*/
/*함수 : get_instruction*/
/*목적 : mnemonic에 해당하는 line type을 알려주는 함수*/
/*리턴값 : mnemonic의 type*/
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

/*------------------------------------------------------------------------------------*/
/*함수 : get_byte_size*/
/*목적 : BYTE 명령의 operand에 있는 상수가 몇 byte인지 계산하는 함수 */
/*리턴값 : byte size 정수값*/
/*------------------------------------------------------------------------------------*/
int get_byte_size(char *BYTE_operand) {
    int byte_size = 0;
    char *ptr;
    if (BYTE_operand[0] == 'C') {
        ptr = strtok(BYTE_operand, "C`'");
        byte_size = (int) strlen(ptr);
    }
    else if (BYTE_operand[0] == 'X') {
        ptr = strtok(BYTE_operand, "X`'");
        byte_size = (int) strlen(ptr) / 2;
    }
    return byte_size;
}

/*------------------------------------------------------------------------------------*/
/*함수 : file_open*/
/*목적 : fp_itm은 읽기모드로, fp_lst, fp_obj는 쓰기모드로 파일을 여는 함 */
/*리턴값 : OK - 파일 열기 성공, FILE_ERR - 파일 열기 실패*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR file_open(char* filename, FILE** fp_itm, FILE** fp_lst, FILE** fp_obj){
    char target_file[NAME_LEN];

    // intermediate 파일을 읽기모드로 open
    strcpy(target_file, filename);
    strcat(target_file, ".itm");
    *fp_itm = fopen(target_file, "r");
    if (!(*fp_itm)) return FILE_ERR;

    // lst 파일을 쓰기모드로 open
    strcpy(target_file, filename);
    *fp_lst = fopen(strcat(target_file, ".lst"), "w");
    if (!(*fp_lst)) return FILE_ERR;

    // obj 파일을 쓰기모드로 open
    strcpy(target_file, filename);
    *fp_obj = fopen(strcat(target_file, ".obj"), "w");
    if (!(*fp_obj)) return FILE_ERR;

    return OK;
}


/*------------------------------------------------------------------------------------*/
/*함수 : get_REG_num*/
/*목적 : string으로 주어지는 register에 대하여 해당하는 register number를 알려준다*/
/*리턴값 : register number*/
/*------------------------------------------------------------------------------------*/
REG_num get_REG_num(char *REG) {
    switch (hash_func_by_mnemonic(REG, MAX_HASH_SIZE)) {
        case 5:
            return regA;
        case 8:
            return regX;
        case 16:
            return regL;
        case 6:
            return regB;
        case 3:
            return regS;
        case 4:
            return regT;
        case 10:
            if (strcmp(REG, "F") == 0) return regF;
            else if (strcmp(REG, "SW") == 0) return regSW;
            else break;
        case 7:
            return regPC;
        default:
            return non_exist;
    }
    return non_exist;
}
