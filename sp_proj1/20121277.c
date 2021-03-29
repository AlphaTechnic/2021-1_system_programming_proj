#include "20121277.h"

/* 프로그램 시작 */
int main() {
    command cmd;
    init();
    while (1) {
        printf("sicsim> ");

        refresh_input();
        scanf("%[^\n]", INPUT);
        getchar();

        // 1. 긴 명령어를 입력하거나 2. token의 개수가 5개 이상이면 error 처리
        if (!input_split_by_comma()) {
            printf("command err!\n");
            continue;
        }
        // 3. 예약되어있지 않은 명령어를 입력하거
        cmd = get_command();
        if (cmd == wrong_cmd) {
            printf("err : wrong cmd or wrong number of parameters!\n");
            continue;
        } else if (cmd == history_command) {
            make_refined_input();
            save_instructions(REFINED_INPUT);
            execute_cmd(cmd);
            continue;
        } else if (cmd == quit_command) {
            free_log_of_instructions();
            free_hash_table();
            break;
        }

        if (execute_cmd(cmd) == FAIL) continue;

        // store instructions
        make_refined_input();
        save_instructions(REFINED_INPUT);
    }
    return 0;
}

/*------------------------------------------------------------------------------------*/
/*함수 : init*/
/*목적 : 명령어들의 history를 저장하기 위한 linked list를 초기화하며, memory의 마지막 주소를 초기화한다.*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void init() {
    HEAD = NULL;
    TAIL = NULL;
    NUM_OF_TOKENS = 0;
    LAST_ADDR = -1;
    init_hash_table("opcode.txt");
}

/*------------------------------------------------------------------------------------*/
/*함수 : refresh_input*/
/*목적 : 사용자로부터 다시 입력을 받을 수 있는 상태가 되도록, INPUT char 배열, INPUT_SPLIT[][] char 배열을 초기화*/
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void refresh_input() {
    INPUT[0] = '\0';
    for (int i = 0; i < MAXNUM_OF_TOKEN; i++) {
        strcpy(INPUT_SPLIT[i], "\0");
    }
    NUM_OF_TOKENS = 0;
}

/*------------------------------------------------------------------------------------*/
/*함수 : input_split_by_comma*/
/*목적 : 사용자로부터 입력 받은 명령어들을 ‘,’ 기호를 기준하여, parsing*/
/*리턴값 : SUCCESS - 성공인 경우, FAIL - 실패인 경*/
/*------------------------------------------------------------------------------------*/
int input_split_by_comma() {
    int cur_ind, dx;

    for (cur_ind = 0; INPUT[cur_ind] == ' ' || INPUT[cur_ind] == '\t'; cur_ind++); // s는 dump의 d를 가리키게 됨
    while (cur_ind < MAX_INPUT_LEN) {
        // utils.h에서 정의한 함수 get_dx_to_nxt_token()
        dx = get_dx_to_nxt_token(INPUT + cur_ind);

        // 종료 조건 : 1. next 토큰이 더이상 없다면, -> 정상 종료 2. TOKEN 개수가 5개 이상 -> error 3. TOKEN의 길이가 너무 길거나 -> error
        if (dx == -1) break;
        if (NUM_OF_TOKENS >= MAXNUM_OF_TOKEN - 1) return FAIL;
        if (strlen(INPUT + cur_ind) > MAX_TOKEN_LEN) return FAIL;

        if (IS_COMMA == YES_COMMA) {
            if (strcmp(INPUT + cur_ind, "") == 0) { // "[white space]," 인 경우
                strcat(INPUT_SPLIT[--NUM_OF_TOKENS], ",");
                NUM_OF_TOKENS++;
            } else { // "[operand]," 인 경우
                strcpy(INPUT_SPLIT[NUM_OF_TOKENS], INPUT + cur_ind);
                strcat(INPUT_SPLIT[NUM_OF_TOKENS++], ",");
            }
        } else {
            strcpy(INPUT_SPLIT[NUM_OF_TOKENS++], INPUT + cur_ind);
        }
        cur_ind += dx;
    }
    return SUCCESS;
}

/*------------------------------------------------------------------------------------*/
/*함수 : get_command*/
/*목적 : 사용자로부터 command를 입력받는다.*/
/*리턴값 : *_command - 성공인 경우 해당 command, wrong_command - 잘못된 명령어인 경우*/
/*------------------------------------------------------------------------------------*/
command get_command() {
    char *cmd = INPUT_SPLIT[0];
    // shell commands
    if ((strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0) && NUM_OF_TOKENS == 1) return help_command;
    else if ((strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) && NUM_OF_TOKENS == 1) return quit_command;
    else if ((strcmp(cmd, "d") == 0 || strcmp(cmd, "dir") == 0) && NUM_OF_TOKENS == 1) return dir_command;
    else if ((strcmp(cmd, "hi") == 0 || strcmp(cmd, "history") == 0) && NUM_OF_TOKENS == 1) return history_command;

        // memory commands
    else if ((strcmp(cmd, "du") == 0 || strcmp(cmd, "dump") == 0) && (NUM_OF_TOKENS >= 1 && NUM_OF_TOKENS <= 3))
        return dump_command;
    else if ((strcmp(cmd, "e") == 0 || strcmp(cmd, "edit") == 0) && NUM_OF_TOKENS == 3) return edit_command;
    else if ((strcmp(cmd, "f") == 0 || strcmp(cmd, "fill") == 0) && NUM_OF_TOKENS == 4) return fill_command;

        // opcode table commands
    else if (strcmp(cmd, "reset") == 0 && NUM_OF_TOKENS == 1) return reset_command;
    else if (strcmp(cmd, "opcode") == 0 && NUM_OF_TOKENS == 2) return opcode_mnemonic_command;
    else if (strcmp(cmd, "opcodelist") == 0 && NUM_OF_TOKENS == 1) return opcodelist_command;

        // assemble commands
    else if (strcmp(cmd, "assemble") == 0 && NUM_OF_TOKENS == 2) return opcodelist_command;
    else if (strcmp(cmd, "type") == 0 && NUM_OF_TOKENS == 2) return opcodelist_command;
    else if (strcmp(cmd, "symbol") == 0 && NUM_OF_TOKENS == 1) return opcodelist_command;

    return wrong_cmd;
}

/*------------------------------------------------------------------------------------*/
/*함수 : execute_cmd*/
/*목적 : 사용자로부터 입력받은 command를 수행한다.*/
/*리턴값 : SUCCESS - 성공인 경우, FAIL - 실패인 경우*/
/*------------------------------------------------------------------------------------*/
int execute_cmd(command cmd) {
    int RESULT = SUCCESS;
    switch (cmd) {
        // shell commands
        case help_command:
            help();
            break;
        case quit_command:
            break;
        case dir_command:
            dir();
            break;
        case history_command:
            history();
            break;

            // memory commands
        case dump_command:
            RESULT = dump(NUM_OF_TOKENS, INPUT_SPLIT[1], INPUT_SPLIT[2]);
            break;
        case edit_command:
            RESULT = edit(INPUT_SPLIT[1], INPUT_SPLIT[2]);
            break;
        case fill_command:
            RESULT = fill(INPUT_SPLIT[1], INPUT_SPLIT[2], INPUT_SPLIT[3]);
            break;
        case reset_command:
            reset();
            break;

            // opcode table commands
        case opcode_mnemonic_command:
            RESULT = get_opcode(INPUT_SPLIT[1]);
            break;
        case opcodelist_command:
            opcodelist();
            break;

            // assemble commands
        case assemble_command:
            break;
        case type_command:
            type(INPUT_SPLIT[1]);
        case symbol_command:
            break;
        default:// wrong_cmd
            printf("command err!\n");
            break;
    }
    if (RESULT <= 0) RESULT = FAIL;
    return RESULT;
}

/*------------------------------------------------------------------------------------*/
/*함수 : make_refined_input*/
/*목적 : 사용자로부터 입력 받은 입력어를 정제하여 INPUT_SPLIT[][] char 배열에 저장한다. */
/*리턴값 : 없음*/
/*------------------------------------------------------------------------------------*/
void make_refined_input() {
    for (int i = 0; i < NUM_OF_TOKENS; i++) {
        switch (i) {
            case 0:
                strcpy(REFINED_INPUT, INPUT_SPLIT[i]);
                break;
            case 1:
                strcat(REFINED_INPUT, " ");
                strcat(REFINED_INPUT, INPUT_SPLIT[i]);
                break;
            default:
                strcat(REFINED_INPUT, ", ");
                strcat(REFINED_INPUT, INPUT_SPLIT[i]);
                break;
        }
    }
}