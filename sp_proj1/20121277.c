#include "20121277.h"



int main() {
    command cmd;
    while (1) {
        printf("sicsim> ");

        flush_input();
        scanf("%[^\n]", input);
        getchar();

        // 1. 긴 명령어를 입력하거나 2. token의 개수가 5개 이상이면 error 처리
        if (!input_split_by_comma()) {
            printf("command err!\n");
            continue;
        }
        // 3. 예약되어있지 않은 명령어를 입력하면 error 처리
        cmd = get_command();
        if (cmd == wrong_input) {
            printf("command err!\n");
            continue;
        }
        make_refined_input();
        save_cmd(refined_input);
        execute_cmd(cmd);
    }
    // 이 line에서 malloc 같은 것들 해제
    return 0;
}

// input[]과 input_split[], NUM_OF_OPERAND를 초기 상태로 만듦
void flush_input() {
    input[0] = '\0';
    for (int i = 0; i < MAXNUM_OF_TOKEN; i++) {
        strcpy(input_split[i], "\0");
    }
    NUM_OF_TOKEN = 0;
}


int input_split_by_comma() {
    int cur_ind, dx;

    for (cur_ind = 0; input[cur_ind] == ' ' || input[cur_ind] == '\t'; cur_ind++); // s는 dump의 d를 가리키게 됨
    while (cur_ind < MAX_INPUT_LEN) {
        // utils.h에서 정의한 함수 get_dx_to_nxt_token()
        dx = get_dx_to_nxt_token(input + cur_ind);

        // 종료 조건 : 1. next 토큰이 더이상 없다면, -> 정상 종료 2. TOKEN 개수가 5개 이상 -> error 3. TOKEN의 길이가 너무 길거나 -> error
        if (dx == -1) break;
        if (NUM_OF_TOKEN >= MAXNUM_OF_TOKEN - 1) return 0;
        if (strlen(input + cur_ind) > MAX_TOKEN_LEN) return 0;

        strcpy(input_split[NUM_OF_TOKEN++], input + cur_ind);
        cur_ind += dx;
    }
    return 1;
}

command get_command() {
    char *cmd = input_split[0];
    // shell command
    if ((strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0) && NUM_OF_TOKEN == 1) return help_command;
    else if ((strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) && NUM_OF_TOKEN == 1) return quit_command;
    else if ((strcmp(cmd, "d") == 0 || strcmp(cmd, "dir") == 0) && NUM_OF_TOKEN == 1) return dir_command;
    else if ((strcmp(cmd, "hi") == 0 || strcmp(cmd, "history") == 0) && NUM_OF_TOKEN == 1) return history_command;

        // memory command
    else if ((strcmp(cmd, "du") == 0 || strcmp(cmd, "dump") == 0) && (NUM_OF_TOKEN >= 1 || NUM_OF_TOKEN <= 3))
        return dump_command;
    else if ((strcmp(cmd, "e") == 0 || strcmp(cmd, "edit") == 0) && NUM_OF_TOKEN == 3) return edit_command;
    else if ((strcmp(cmd, "f") == 0 || strcmp(cmd, "fill") == 0) && NUM_OF_TOKEN == 4) return fill_command;

        // opcode table command
    else if (strcmp(cmd, "reset") == 0 && NUM_OF_TOKEN == 1) return reset_command;
    else if (strcmp(cmd, "opcode") == 0 && NUM_OF_TOKEN == 2) return opcode_mnemonic_command;
    else if (strcmp(cmd, "opcodelist") == 0 && NUM_OF_TOKEN == 1) return opcodelist_command;
    return wrong_input;
}

int execute_cmd(command cmd) {
    switch (cmd) {
        // shell command
        case help_command:
            help();
            break;
        case quit_command:
            exit(1);
            break;
        case dir_command:
            dir();
            break;
        case history_command:
            history();
            break;
            /*
       // memory command
        case dump_command:
            dump();
            break;
        case edit_command:
            edit();
            break;
        case fill_command:
            fill();
            break;

        // opcode table command
        case reset_command:
            reset();
            break;
        case opcode_mnemonic_command:
            opcode_mnemonic();
            break;
        case opcodelist_command:
            opcodelist();
            break;
             */
        case wrong_input:
            printf("command err!\n");
            break;
    }
}

void make_refined_input() {
    for (int i = 0; i < NUM_OF_TOKEN; i++) {
        if (i == 0) {
            strcpy(refined_input, input_split[i]);
        } else if (i == 1) {
            strcat(refined_input, " ");
            strcat(refined_input, input_split[i]);
        } else {
            strcat(refined_input, ", ");
            strcat(refined_input, input_split[i]);
        }
    }
}