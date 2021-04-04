//
// Created by 김주호 on 2021/03/12.
//

#include "utils.h"

/*------------------------------------------------------------------------------------*/
/*함수 : get_dx_to_nxt_token*/
/*목적 : */
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
/*목적 : */
/*리턴값 : res - 문자열 hexstr을 int형 decimal 형태로 바꾼 결과*/
/*------------------------------------------------------------------------------------*/
int hexstr_to_decint(char *hexstr) {
    int cur, res = 0;
    int scale = 1;
    for (int i = strlen(hexstr) - 1; i >= 0; i--) {
        if (hexstr[i] >= '0' && hexstr[i] <= '9') cur = hexstr[i] - '0';
        else if (hexstr[i] >= 'A' && hexstr[i] <= 'F') cur = hexstr[i] - 'A' + 10;
        else if (hexstr[i] >= 'a' && hexstr[i] <= 'f') cur = hexstr[i] - 'a' + 10;
        else return -1; // contain wrong hexstr symbol

        res += cur * scale;
        scale *= 16;
    }
    return res;
}