//
// Created by 김주호 on 2021/04/17.
//

#include "loader_commands.h"

OK_or_ERR prog_addr(char* addr_hexstr){
    int addr_int = -1;

    addr_int = hexstr_to_decint(addr_hexstr);
    if (addr_int == RANGE_ERR){ // 적절하지 않은 형식의 address가 입력된 경우
        printf("range err!\n");
        return RANGE_ERR;
    }
    if (addr_int < 0 || addr_int >= MEM_SIZE) { // 입력된 address가 메모리 주소 범위를 초과한 경우
        printf("range err!\n");
        return RANGE_ERR;
    }
    PROG_ADDR = addr_int;
    REG[regPC] = addr_int;
    return OK;
}

OK_or_ERR loader(char filename){
    // print loadmap
    printf("%-7s  %-7s  %-7s  %-7s\n", "control", "symbol", "address", "length");
    printf("%-7s  %-7s\n", "section", "name");
    printf("-----------------------------------\n");

}

void load_pass1(FILE *fp){
    char *ptr;
    char line[LINE_LEN];
    char addr_str[LINE_LEN];
    int addr_int;

    while(!feof(fp)){
        fgets(fp, LINE_LEN, line);
        if (line[0] == 'H'){
            // get CS name
            char cs_name[LINE_LEN];

            ptr = strtok(line, " ");
            ptr++;
            strcpy(cs_name, ptr);
            ptr = strtok(NULL, " ");

            // get starting addr
            strncpy(addr_str, ptr, 6);
            addr_str[6] = '\0';
            addr_int = hexstr_to_decint(addr_str);

            TOTAL_LEN += CS_LEN;
            printf("%-7s  %-7s  %04X     %04X   \n", cs_name, " ", CS_ADDR + addr_int, CS_LEN);
        }
        else if (line[0] == 'D'){
            char symbol_name[LINE_LEN];

            ptr = strtok(line, " ");
            ptr++;
            while(1){
                // get name of symbol
                if (strlen(ptr) > 6){
                    strncpy(symbol_name, ptr, 6);
                    symbol_name[6] = '\0';
                    ptr += 6;
                }
                else{
                    strcpy(symbol_name, ptr);
                    ptr = strtok(NULL, " ");
                }

                // get address of symbol
                strncpy(addr_str, ptr, 6);
                addr_str[6] = '\0';
                addr_int = hexstr_to_decint(addr_str);

                // es_table에 push

                printf("%-7s  %-7s  %04X   \n", " ", symbol_name, CS_ADDR + addr_int);
            }
        }
        else if (line[0] == 'E'){
            CS_ADDR += CS_LEN;
        }
    }
}

void load_pass2(FILE *fp){
    char *ptr;
    char line[MAX_LINE_NUM];
    int addr;

    while (!feof(fp)){
        fgets(fp, LINE_LEN, line);
    }
}
