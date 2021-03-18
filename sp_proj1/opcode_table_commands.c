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
    for (int i = 0; i < (int)strlen(mnemonic); i++) total += mnemonic[i];
    return total % MAX_HASHTABLE_SIZE;
}

int get_opcode(char *mnemonic) {
    int OK = 1;
    int ERR = 0;
    OPCODE_MNEMONIC_MAP *cur_node = HASH_TABLE[hash_func(mnemonic)];
    while (cur_node != NULL) {
        if (strcmp(cur_node->mnemonic, mnemonic) == 0) {
            printf("opcode is %s\n", cur_node->opcode);
            return OK;
        }
        cur_node = cur_node->nxt;
    }
    printf("err: no opcode for the mnemonic %s\n", mnemonic);
    return ERR;
}

void opcodelist() {
    OPCODE_MNEMONIC_MAP *cur_node;
    for (int i = 0; i < MAX_HASHTABLE_SIZE; i++) {
        printf("%d : ", i);
        cur_node = HASH_TABLE[i];
        if (cur_node == NULL) {
            printf("\n");
            continue;
        }
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

void free_hash_table(){
    OPCODE_MNEMONIC_MAP * cur_node;
    for (int i=0; i<MAX_HASHTABLE_SIZE; i++){
        cur_node = HASH_TABLE[i];
        while(cur_node!= NULL){
            OPCODE_MNEMONIC_MAP *tmp_node;
            tmp_node = cur_node->nxt;
            free(cur_node);
            cur_node = tmp_node;
        }
    }
}