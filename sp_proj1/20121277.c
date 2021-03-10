#include "20121277.h"

void input_split_by_comma();
command get_command();

int main() {
    command cmd;
    while(1){
        printf("sicsim> ");
        scanf("%[^\n]", input);

        cmd = get_command();

        if (cmd == command_err){
            printf("\ncommand err!\n");
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

void input_split_by_comma(){
    // input을 쪼개서 input_split[][]에 저장하는 함수 작성
    // dump AA, BB -> input_split = [dump, AA, BB] 이렇게 되도록
}

command get_command(){
    // input_split[][]에서 첫번째 string 주어와서, 유효성검
}

