//
// Created by 김주호 on 2021/03/12.
//

#ifndef SP_PROJ1_UTILS_H
#define SP_PROJ1_UTILS_H

/* 포함되는 파일 */
#include <string.h>

/* 사용자 정의 data type */
typedef enum {
    NO_COMMA = 0,
    YES_COMMA
} func_option;

/* 전역 변수 */
func_option IS_COMMA;

/* 함수 원형 */
int get_dx_to_nxt_token(char *start_ptr);
int hexstr_to_decint(char *hexstr);

#endif //SP_PROJ1_UTILS_H
