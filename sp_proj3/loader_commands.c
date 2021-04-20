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
    char cs_name[LINE_LEN];
    char addr_str[LINE_LEN];
    int addr_int;

    while(!feof(fp)){
        fgets(fp, LINE_LEN, line);
        if (line[0] == 'H'){
            // get CS name
            ptr = strtok(line, " ");
            ptr ++;
            strcpy(cs_name, ptr);
            ptr = strtok(NULL, " ");

            // get starting addr
            strncpy(addr_str, ptr, 2*3);
            addr_str[2*3] = '\0';
            addr_int = hexstr_to_decint(addr_str);

            TOTAL_LEN += CS_LEN;
            printf("%-7s  %-7s  %04X     %04X   \n", cs_name, " ", CS_ADDR + addr_int, CS_LEN);
        }

    }
}
