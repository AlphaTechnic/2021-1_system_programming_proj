//
// Created by 김주호 on 2021/03/17.
//

#include "opcode_table_commands.h"

/*------------------------------------------------------------------------------------*/
/*함수 : init_hash_table*/
/*목적 : opcode 정보들이 적힌 file을 읽어서 해당파일을 열고, 이를 프로그램에서 선언한 hash table에 저장한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------*/
/*함수 : hash_func*/
/*목적 : mnemonic을 입력 받아 이를 0부터 19까지 20개의 숫자에 mapping한다. 입력 받은 명령어를 구성하는 모든 문자의
 * ASCII  값을 더하여, 20으로 나눈다.*/
/*리턴값 : total % MAX_HASHTABLE_SIZE*/
/*------------------------------------------------------------------------------------*/
int hash_func(char *mnemonic) {
    int total = 0;
    for (int i = 0; i < (int)strlen(mnemonic); i++) total += mnemonic[i];
    return total % MAX_HASHTABLE_SIZE;
}

/*------------------------------------------------------------------------------------*/
/*함수 : get_opcode*/
/*목적 : mnemonic을 입력 받아 해당 mnemonic에 맞는 opcode를 알려준다.*/
/*리턴값 : OK - 성공인 경우, ERR - 에러인 경*/
/*------------------------------------------------------------------------------------*/
int get_opcode(char *mnemonic) {
    const int OK = 1;
    const int ERR = 0;
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

/*------------------------------------------------------------------------------------*/
/*함수 : opcodelist*/
/*목적 : hash table의 모든 index를 방문하면서, 해당 index에 저장되어있는 모든 node들을 형식에 맞게 출력한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------------*/
/*함수 : free_hash_table*/
/*목적 : hash table에 저장된 모든 node들의 할당을 해제한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
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