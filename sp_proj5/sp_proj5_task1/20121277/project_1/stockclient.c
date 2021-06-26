/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

int main(int argc, char **argv) 
{
    int clientfd;
    char *host, *port;
    rio_t rio;

    if (argc != 3) {
	    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
	    exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    while (1) {
        Rio_readinitb(&rio, clientfd);
        char buf[MAXLINE];
        if (Fgets(buf, MAXLINE, stdin) == NULL) break;
        if (!strcmp(buf, "exit\n")) exit(0);

        else if (!strcmp(buf, "show\n")){
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
        Rio_writen(clientfd, buf, strlen(buf));
        Rio_readlineb(&rio, buf, MAXLINE);
        Fputs(buf, stdout);
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}

/* $end echoclientmain */
