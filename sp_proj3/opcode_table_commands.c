//
// Created by 김주호 on 2021/03/17.
//

#include "opcode_table_commands.h"

/*------------------------------------------------------------------------------------*/
/*함수 : init_OPTAB*/
/*목적 : opcode 정보들이 적힌 file을 읽어서 해당파일을 열고, 이를 프로그램에서 선언한 hash table에 저장한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void init_OPTAB(char *filename) {
    int ind_by_mnemonic_hash, ind_by_opcode_hash;
    FILE *fp = fopen(filename, "r");
    OP_NODE *new_node = malloc(sizeof(OP_NODE));

    for (int i = 0; i < MAX_HASHTABLE_SIZE; i++) {
        OPTAB_by_mnemonic[i] = NULL;
        OPTAB_by_opcode[i] = NULL;
    }
    while (1) {
        if (fscanf(fp, "%X%s%s", &(new_node->opcode), new_node->mnemonic, new_node->format) == EOF) break;
        ind_by_mnemonic_hash = hash_func_by_mnemonic(new_node->mnemonic, MAX_HASHTABLE_SIZE);
        ind_by_opcode_hash = hash_func_by_opcode(new_node->opcode);

        // connect
        new_node->nxt_by_mnemonic = OPTAB_by_mnemonic[ind_by_mnemonic_hash];
        new_node->nxt_by_opcode = OPTAB_by_opcode[ind_by_opcode_hash];

        OPTAB_by_mnemonic[ind_by_mnemonic_hash] = new_node;
        OPTAB_by_opcode[ind_by_opcode_hash] = new_node;
        // malloc
        new_node = malloc(sizeof(OP_NODE));
    }
    free(new_node);
    fclose(fp);
}

/*------------------------------------------------------------------------------------*/
/*함수 : print_opcode_by_mnemonic*/
/*목적 : mnemonic을 입력 받아 해당 mnemonic에 맞는 opcode를 알려준다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OK_or_ERR print_opcode_by_mnemonic(char *mnemonic) {
    const int OK = 1;
    const int ERR = 0;
    OP_NODE *cur_node = OPTAB_by_mnemonic[hash_func_by_mnemonic(mnemonic, MAX_HASHTABLE_SIZE)];
    while (cur_node != NULL) {
        if (strcmp(cur_node->mnemonic, mnemonic) == 0) {
            printf("opcode is %02X\n", cur_node->opcode);
            return OK;
        }
        cur_node = cur_node->nxt_by_mnemonic;
    }
    printf("err: no opcode for the mnemonic %s\n", mnemonic);
    return ERR;
}

/*------------------------------------------------------------------------------------*/
/*함수 : get_opcode_or_NULL_by_opcode*/
/*목적 : opcode를 입력 받아 해당 opcode에 맞는 OP_node를 return 한다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경우*/
/*------------------------------------------------------------------------------------*/
OP_NODE *get_opcode_or_NULL_by_opcode(int opcode) {
    int ind = hash_func_by_opcode(opcode);
    for(OP_NODE *cur_node = OPTAB_by_opcode[ind]; cur_node; cur_node = cur_node->nxt_by_opcode){
        if(cur_node->opcode == opcode){
            return cur_node;
        }
    }
    return NULL;
}

/*------------------------------------------------------------------------------------*/
/*함수 : get_opcode_or_NULL_by_mnemonic*/
/*목적 : 주어진 mnemonic에 맞는 opcode를 return하는 함수이다*/
/*리턴값 : 주어진 mnemonic이 opcode table에 있다면 해당 node를 리턴,    없다면 NULL을 리턴*/
/*------------------------------------------------------------------------------------*/
OP_NODE *get_opcode_or_NULL_by_mnemonic(char *mnemonic) {
    OP_NODE *cur_node = OPTAB_by_mnemonic[hash_func_by_mnemonic(mnemonic, MAX_HASHTABLE_SIZE)];
    while (cur_node != NULL) {
        if (strcmp(cur_node->mnemonic, mnemonic) == 0) {
            return cur_node;
        }
        cur_node = cur_node->nxt_by_mnemonic;
    }
    return NULL;
}

/*------------------------------------------------------------------------------------*/
/*함수 : opcodelist*/
/*목적 : hash table의 모든 index를 방문하면서, 해당 index에 저장되어있는 모든 node들을 형식에 맞게 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void opcodelist() {
    OP_NODE *cur_node;
    for (int i = 0; i < MAX_HASHTABLE_SIZE; i++) {
        printf("%d : ", i);
        cur_node = OPTAB_by_mnemonic[i];
        if (cur_node == NULL) {
            printf("\n");
            continue;
        }
        printf("[%s,%02X]", cur_node->mnemonic, cur_node->opcode);
        cur_node = cur_node->nxt_by_mnemonic;

        while (cur_node != NULL){
            printf(" -> ");
            printf("[%s,%02X]", cur_node->mnemonic, cur_node->opcode);
            cur_node = cur_node->nxt_by_mnemonic;
        }
        printf("\n");
    }
}

/*------------------------------------------------------------------------------------*/
/*함수 : free_OPTAB*/
/*목적 : hash table에 저장된 모든 node들의 할당을 해제한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void free_OPTAB(){
    OP_NODE *pre_node;
    for (int i=0; i<MAX_HASHTABLE_SIZE; i++){
        for (OP_NODE *cur_node = OPTAB_by_mnemonic[i]; cur_node;){
            pre_node = cur_node;
            cur_node = cur_node->nxt_by_mnemonic;
            free(pre_node);
        }
        OPTAB_by_mnemonic[i] = NULL;
    }
    for (int i = 0; i < MAX_HASH_SIZE; i++) OPTAB_by_opcode[i] = NULL;
}