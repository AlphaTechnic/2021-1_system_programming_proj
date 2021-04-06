#include "20121277.h"

/* 프로그램 시작 */
int main() {
    COMMAND cmd;
    init();
    while (1) {
        printf("sicsim> ");

        refresh_input();
        scanf("%[^\n]", INPUT);
        getchar();

        // 1. 긴 명령어를 입력하거나 2. token의 개수가 5개 이상이면 error 처리
        if (!input_split_by_comma()) {
            printf("COMMAND err!\n");
            continue;
        }
        // 3. 예약되어있지 않은 명령어를 입력하거
        cmd = get_command();
        if (cmd == WRONG_CMD) {
            printf("err : wrong cmd or wrong number of parameters!\n");
            continue;
        } else if (cmd == HISTORY_CMD) {
            make_refined_input();
            save_instructions(REFINED_INPUT);
            execute_cmd(cmd);
            continue;
        } else if (cmd == QUIT_CMD) {
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
SUCCESS_or_FAIL input_split_by_comma() {
    int cur_ind, dx;

    for (cur_ind = 0; INPUT[cur_ind] == ' ' || INPUT[cur_ind] == '\t'; cur_ind++);
    while (cur_ind < MAX_INPUT_LEN) {
        dx = get_dx_to_nxt_token(INPUT + cur_ind);

        if (dx == -1) break;
        if (NUM_OF_TOKENS >= MAXNUM_OF_TOKEN - 1) return FAIL;
        if (strlen(INPUT + cur_ind) > MAX_TOKEN_LEN) return FAIL;

        if (IS_COMMA == YES_COMMA) {
            if (strcmp(INPUT + cur_ind, "") == 0) {
                strcat(INPUT_SPLIT[--NUM_OF_TOKENS], ",");
                NUM_OF_TOKENS++;
            }
            else {
                strcpy(INPUT_SPLIT[NUM_OF_TOKENS], INPUT + cur_ind);
                strcat(INPUT_SPLIT[NUM_OF_TOKENS++], ",");
            }
        }
        else {
            strcpy(INPUT_SPLIT[NUM_OF_TOKENS++], INPUT + cur_ind);
        }
        cur_ind += dx;
    }
    return SUCCESS;
}

/*------------------------------------------------------------------------------------*/
/*함수 : get_command*/
/*목적 : 사용자로부터 command를 입력받는다.*/
/*리턴값 : *_command - 성공인 경우 해당 COMMAND, wrong_command - 잘못된 명령어인 경우*/
/*------------------------------------------------------------------------------------*/
COMMAND get_command() {
    char *cmd = INPUT_SPLIT[0];
    // shell commands
    if ((strcmp(cmd, "h") == 0 || strcmp(cmd, "help") == 0) && NUM_OF_TOKENS == 1) return HELP_CMD;
    else if ((strcmp(cmd, "q") == 0 || strcmp(cmd, "quit") == 0) && NUM_OF_TOKENS == 1) return QUIT_CMD;
    else if ((strcmp(cmd, "d") == 0 || strcmp(cmd, "dir") == 0) && NUM_OF_TOKENS == 1) return DIR_CMD;
    else if ((strcmp(cmd, "hi") == 0 || strcmp(cmd, "history") == 0) && NUM_OF_TOKENS == 1) return HISTORY_CMD;
    else if (strcmp(cmd, "type") == 0 && NUM_OF_TOKENS == 2) return OPCODELIST_CMD;

        // memory commands
    else if ((strcmp(cmd, "du") == 0 || strcmp(cmd, "dump") == 0) && (NUM_OF_TOKENS >= 1 && NUM_OF_TOKENS <= 3))
        return DUMP_CMD;
    else if ((strcmp(cmd, "e") == 0 || strcmp(cmd, "edit") == 0) && NUM_OF_TOKENS == 3) return EDIT_CMD;
    else if ((strcmp(cmd, "f") == 0 || strcmp(cmd, "fill") == 0) && NUM_OF_TOKENS == 4) return FILL_CMD;

        // opcode table commands
    else if (strcmp(cmd, "reset") == 0 && NUM_OF_TOKENS == 1) return RESET_CMD;
    else if (strcmp(cmd, "opcode") == 0 && NUM_OF_TOKENS == 2) return OPCODE_MNEMONIC_CMD;
    else if (strcmp(cmd, "opcodelist") == 0 && NUM_OF_TOKENS == 1) return OPCODELIST_CMD;

        // assembler commands
    else if (strcmp(cmd, "assemble") == 0 && NUM_OF_TOKENS == 2) return ASSEMBLE_CMD;
    else if (strcmp(cmd, "symbol") == 0 && NUM_OF_TOKENS == 1) return SYMBOL_CMD;

    return WRONG_CMD;
}

/*------------------------------------------------------------------------------------*/
/*함수 : execute_cmd*/
/*목적 : 사용자로부터 입력받은 command를 수행한다.*/
/*리턴값 : SUCCESS - 성공인 경우, FAIL - 실패인 경우*/
/*------------------------------------------------------------------------------------*/
SUCCESS_or_FAIL execute_cmd(COMMAND cmd) {
    int RESULT = SUCCESS;
    switch (cmd) {
        // shell commands
        case HELP_CMD:
            help();
            break;
        case QUIT_CMD:
            break;
        case DIR_CMD:
            dir();
            break;
        case HISTORY_CMD:
            history();
            break;
        case TYPE_CMD:
            type(INPUT_SPLIT[1]);

            // memory commands
        case DUMP_CMD:
            RESULT = dump(NUM_OF_TOKENS, INPUT_SPLIT[1], INPUT_SPLIT[2]);
            break;
        case EDIT_CMD:
            RESULT = edit(INPUT_SPLIT[1], INPUT_SPLIT[2]);
            break;
        case FILL_CMD:
            RESULT = fill(INPUT_SPLIT[1], INPUT_SPLIT[2], INPUT_SPLIT[3]);
            break;
        case RESET_CMD:
            reset();
            break;

            // opcode table commands
        case OPCODE_MNEMONIC_CMD:
            RESULT = get_opcode(INPUT_SPLIT[1]);
            break;
        case OPCODELIST_CMD:
            opcodelist();
            break;

            // assembler commands
        case ASSEMBLE_CMD:
            assemble(INPUT_SPLIT[1]);
            break;
        case SYMBOL_CMD:
            print_symbols();
            break;
        default:// WRONG_CMD
            printf("COMMAND err!\n");
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