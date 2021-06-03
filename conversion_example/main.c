#include "csapp.h"

int main(int argc, char **argv){
    struct addrinfo *p, *listp, hints;
    char buf[MAXLINE];
    int rc, flags;

    /* Get a list of addrinfo records */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET; // IPv4 only
    hints.ai_socktype = SOCK_STREAM; // connection only

    if((rc = getaddrinfo(argv[1], NULL, &hints, &listp)) != 0){
        fprintf(stderr, "getaddrinfo error : %s\n", gai_strerror(rc));
        exit(1);
    }

    // WALK the list and display each IP ADDRESS
    flags = NI_NUMERICHOST;
    for (p=listp; p; p = p->ai_next){
        Getnameinfo(p->ai_addr, p->ai_addrlen, buf, MAXLINE, NULL, 0, flags);
        printf("%s\n", buf);
    }

    // Clear up
    Freeaddrinfo(listp);
}