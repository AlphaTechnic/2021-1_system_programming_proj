//
// Created by 김주호 on 2021/03/12.
//

// cur token의 첫번째 index부터 ~ nxt token의 첫번째 index까지의 차이를 구하는 함수
#include "utils.h"

int get_dx_to_nxt_token(char *start_ptr) {
    int ind = 0;
    if (start_ptr[ind] == '\0') return -1;
    for (ind = 0;
         start_ptr[ind] != ' ' && start_ptr[ind] != '\t' && start_ptr[ind] != '\0' && start_ptr[ind] != ',';
         ind++);
    if (start_ptr[ind] == '\0') return ind;

    start_ptr[ind++] = '\0'; // token 끝에 NULL을 삽입
    for (; start_ptr[ind] == ' ' || start_ptr[ind] == '\t'; ind++); // 다음 토큰의 첫 ind까지 접근
    return ind;
}
