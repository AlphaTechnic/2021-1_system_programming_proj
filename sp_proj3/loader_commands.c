//
// Created by 김주호 on 2021/04/17.
//

#include "loader_commands.h"

OK_or_ERR loader(char filename){
    // print loadmap
    printf("%-7s  %-7s  %-7s  %-7s\n", "control", "symbol", "address", "length");
    printf("%-7s  %-7s\n", "section", "name");
    printf("-----------------------------------\n");

}

void load_pass1(FILE *fp){
    char *ptr;
    char line[LINE_LEN];

    //
}