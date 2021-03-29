//
// Created by 김주호 on 2021/03/29.
//

#ifndef SP_PROJ1_ASSEMBLER_COMMANDS_H
#define SP_PROJ1_ASSEMBLER_COMMANDS_H

#define MAX_SYMBOL_LEN 100

typedef struct symbol_node{
    char symbol[MAX_SYMBOL_LEN];
    int address;
    struct symbol_node *nxt;
}symbol_node;


#endif //SP_PROJ1_ASSEMBLER_COMMANDS_H
