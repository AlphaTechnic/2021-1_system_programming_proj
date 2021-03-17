//
// Created by 김주호 on 2021/03/17.
//

#include "opcode_table_commands.h"


void init_hash_table(char *filename) {
    int ind;
    FILE *fp = fopen(filename, "r");
    OPCODE_MNEMONIC_MAP *cur_node = malloc(sizeof(OPCODE_MNEMONIC_MAP));

    for (int i = 0; i < MAX_HASHTABLE_SIZE; i++) HASH_TABLE[i] = NULL;
    while (1) {
        if (fscanf(fp, "%s%s%s", cur_node->opcode, cur_node->mnemonic, cur_node->format) == EOF) break;
        ind = hash_func(cur_node->mnemonic);

        // connect
        cur_node->nxt = HASH_TABLE[ind];
        HASH_TABLE[ind] = cur_node;
        // malloc
        cur_node = malloc(sizeof(OPCODE_MNEMONIC_MAP));
    }
    free(cur_node);
}

int hash_func(char *mnemonic) {
    int total = 0;
    for (int i = 0; i < strlen(mnemonic); i++) total += mnemonic[i];
    return total % MAX_HASHTABLE_SIZE;
}

void get_opcode(char *mnemonic) {
    OPCODE_MNEMONIC_MAP *cur_node = HASH_TABLE[hash_func(mnemonic)];
    while (cur_node != NULL) {
        if (strcmp(cur_node->mnemonic, mnemonic) == 0) {
            printf("opcode is %s\n", cur_node->opcode);
            return;
        }
        cur_node = cur_node->nxt;
    }
    printf("err: no opcode for the mnemonic %s\n", mnemonic);
}

void opcodelist() {
    OPCODE_MNEMONIC_MAP *cur_node;
    for (int i = 0; i < MAX_HASHTABLE_SIZE; i++) {
        printf("%d : ", i);
        cur_node = HASH_TABLE[i];
        if (cur_node == NULL) break;
        printf("[%s,%s]", cur_node->mnemonic, cur_node->opcode);
        cur_node = cur_node->nxt;

        while (cur_node != NULL){
            printf(" -> ");
            printf("[%s,%s]", cur_node->mnemonic, cur_node->opcode);
            cur_node = cur_node->nxt;
        }
        printf("\n");
    }
}