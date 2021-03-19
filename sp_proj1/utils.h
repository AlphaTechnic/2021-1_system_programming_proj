//
// Created by 김주호 on 2021/03/12.
//

#ifndef SP_PROJ1_UTILS_H
#define SP_PROJ1_UTILS_H

#include <string.h>

typedef enum {
    NO_COMMA = 0,
    YES_COMMA
} func_option;
func_option IS_COMMA;

int get_dx_to_nxt_token(char *start_ptr);

int hexstr_to_dec(char *hex);

#endif //SP_PROJ1_UTILS_H
