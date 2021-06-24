#include "csapp.h"
#include <time.h>

#define MAX_CLIENT 200
#define ORDER_PER_CLIENT 10
#define STOCK_NUM 10
#define BUY_SELL_MAX 10

int main(int argc, char **argv) 
{
    clock_t start, end;
    start = clock();

	pid_t pids[MAX_CLIENT];
	int runprocess = 0, status, i;

	int clientfd, num_client;
	char *host, *port, buf[MAXLINE], tmp[3];
	rio_t rio;

	if (argc != 4) {
		fprintf(stderr, "usage: %s <host> <port> <client#>\n", argv[0]);
		exit(0);
	}

	host = argv[1];
	port = argv[2];
	num_client = atoi(argv[3]);

/*	fork for each client process	*/
	while(runprocess < num_client){
		//wait(&state);
		pids[runprocess] = fork();

		if(pids[runprocess] < 0)
			return -1;
		/*	child process		*/
		else if(pids[runprocess] == 0){
			printf("child %ld\n", (long)getpid());

			clientfd = Open_clientfd(host, port);
			srand((unsigned int) getpid());

			for(i=0;i<ORDER_PER_CLIENT;i++){
                Rio_readinitb(&rio, clientfd);
				int option = rand() % 3;
				
				if(option == 0){//show
					strcpy(buf, "show\n");
					printf("%s", buf);
                    Rio_writen(clientfd, buf, strlen(buf));
                    while(1){
                        Rio_readlineb(&rio, buf, MAXLINE);
                        if (buf[strlen(buf)-2] == 'E'){
                            buf[strlen(buf)-2] = '\0';
                            Fputs(buf, stdout);
                            printf("\n");
                            break;
                        }
                        Fputs(buf, stdout);
                    }
                    continue;
				}
				else if(option == 1){//buy
					int list_num = rand() % STOCK_NUM + 1;
					int num_to_buy = rand() % BUY_SELL_MAX + 1;//1~10

					strcpy(buf, "buy ");
					sprintf(tmp, "%d", list_num);
					strcat(buf, tmp);
					strcat(buf, " ");
					sprintf(tmp, "%d", num_to_buy);
					strcat(buf, tmp);
					strcat(buf, "\n");
                    printf("%s", buf);
				}
				else if(option == 2){//sell
					int list_num = rand() % STOCK_NUM + 1; 
					int num_to_sell = rand() % BUY_SELL_MAX + 1;//1~10
					
					strcpy(buf, "sell ");
					sprintf(tmp, "%d", list_num);
					strcat(buf, tmp);
					strcat(buf, " ");
					sprintf(tmp, "%d", num_to_sell);
					strcat(buf, tmp);
					strcat(buf, "\n");
                    printf("%s", buf);
				}
				//strcpy(buf, "buy 1 2\n");
			
				Rio_writen(clientfd, buf, strlen(buf));
				Rio_readlineb(&rio, buf, MAXLINE);
				Fputs(buf, stdout);

				//usleep(1000000);
			}
			// 사용자가 마지막에 exit을 입력했다는 가정
			Close(clientfd);
			exit(0);
		}
		/*	parten process		*/
		/*else{
			for(i=0;i<num_client;i++){
				waitpid(pids[i], &status, 0);
			}
		}*/
		runprocess++;
	}
	for(i=0;i<num_client;i++){
		waitpid(pids[i], &status, 0);
	}

    end = clock();
	printf("수행 시간 : %f\n", (float)(end - start)/CLOCKS_PER_SEC);

	return 0;
}
