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
    if (start_ptr[dx] == '\0') return dx;

    start_ptr[dx++] = '\0'; // token 끝에 NULL을 삽입
    for (; start_ptr[dx] == ' ' || start_ptr[dx] == '\t'; dx++); // 다음 토큰의 첫 ind까지 접근
    return dx;
}
