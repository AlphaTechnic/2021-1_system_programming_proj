/*
 * echoclient.c - An echo client
 */
/* $begin echoclientmain */
#include "csapp.h"

int main(int argc, char **argv) 
{
    int clientfd;
    char *host, *port, buf[MAXLINE];
    rio_t rio;

    if (argc != 3) {
	    fprintf(stderr, "usage: %s <host> <port>\n", argv[0]);
	    exit(0);
    }
    host = argv[1];
    port = argv[2];

    clientfd = Open_clientfd(host, port);
    Rio_readinitb(&rio, clientfd);

    while ((Fgets(buf, MAXLINE, stdin) != NULL)) {
        if (!strcmp(buf, "exit\n")) exit(0);
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
    }
    Close(clientfd); //line:netp:echoclient:close
    exit(0);
}

/* $end echoclientmain */
