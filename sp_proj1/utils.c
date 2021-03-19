//
// Created by 김주호 on 2021/03/12.
//

// cur token의 첫번째 index부터 ~ nxt token의 첫번째 index까지의 차이를 구하는 함수
#include "utils.h"

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

int hexstr_to_dec(char *hex) {
    int cur, res = 0;
    int scale = 1;
    for (int i = strlen(hex) - 1; i >= 0; i--) {
        if (hex[i] >= '0' && hex[i] <= '9') cur = hex[i] - '0';
        else if (hex[i] >= 'A' && hex[i] <= 'F') cur = hex[i] - 'A' + 10;
        else if (hex[i] >= 'a' && hex[i] <= 'f') cur = hex[i] - 'a' + 10;
        else return -1; // contain wrong hex symbol

        res += cur * scale;
        scale *= 16;
    }
    return res;
}