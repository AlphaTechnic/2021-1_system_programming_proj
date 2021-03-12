#include "20121277.h"

int input_split_by_comma();

void flush_input();

command get_command();

int main() {
    command cmd;
    while (1) {
        printf("sicsim> ");

        flush_input();
        scanf("%[^\n]", input);
        getchar();
        // 1. 긴 명령어를 입력하거나 2. token의 개수가 5개 이상이면 error 처리
        if (!input_split_by_comma()) printf("command err!\n");

        cmd = get_command();

        if (cmd == command_err) {
            printf("command err!\n");
            continue;
        }
        if (cmd == quit_command) {
            break;
        }
        // 이 line에서 명령을 execute
        // 이 line에서 명령을 history에 저장
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

// cur token의 첫번째 index부터 ~ nxt token의 첫번째 index까지의 차이를 구하는 함수
int get_dx_to_nxt_token(char *start_ptr) {
    int ind = 0;
    if (start_ptr[ind] == '\0') return -1;
    for (ind = 0;
         start_ptr[ind] != ' ' && start_ptr[ind] != '\t' && start_ptr[ind] != '\0' && start_ptr[ind] != ',';
         ind++);
    if (start_ptr[ind] == '\0') return ind;

    start_ptr[ind++] = '\0'; // token 끝에 NULL을 삽입
    for (; start_ptr[ind] == ' ' || start_ptr[ind] == '\t'; ind++); // 다음 토큰의 첫 ind까지 접근
    return ind;
}

int input_split_by_comma() {
    int cur_ind, dx;

    for (cur_ind = 0; input[cur_ind] == ' ' || input[cur_ind] == '\t'; cur_ind++); // s는 dump의 d를 가리키게 됨
    while (cur_ind < MAX_INPUT_LEN) {
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
    // input_split[][]에서 첫번째 string 주어와서, 유효성검
}

