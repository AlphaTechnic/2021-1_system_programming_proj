
/*
 * echoserveri.c - An iterative echo server 
 */
/* $begin echoserverimain */
#include "csapp.h"

#define MAXARGS 5
#define MAXSTOCK 1024


// void echo(int connfd);
typedef struct { /* Represents a pool of connected descriptors */
    int maxfd;          /* Largest descriptor in read_set */
    fd_set read_set;    /* Set of all active descriptors */
    fd_set ready_set;    /* Subset of descriptors ready for reading */
    int nready;         /* Number of ready descriptors from select */
    int maxi;           /* High water index into client array */
    int clientfd[FD_SETSIZE];   /* Set of active descriptors */
    rio_t clientrio[FD_SETSIZE];/* Set of active read buffers */
}pool;

typedef struct node{
    int ID;
    int left_stock;
    int price;

    struct node *left, *right, *parent;
}node;
node* BINARY_TREE = NULL;

typedef struct stack{
    node* arr[MAXSTOCK];
    int top;
}stack;

void init_pool(int listenfd, pool *p);
void add_client(int connfd, pool *p);
void check_clients(pool *p, node* tree);
void parse_command(char* buf, char** argv);

node* search_node(node *tree, int id_to_search);
void push_node(node** tree, int id_to_push, int left_stock_to_push, int price_to_push);
void buy(int connfd, node** tree, int id_to_buy, int num);
void sell(int connfd, node** tree, int id_to_sell, int num);
void show_node(int connfd, node *tree);

int is_empty(stack* st);
int is_full(stack* st);
void push(stack* st, node* node_ptr);
node* pop(stack* st);


//int byte_cnt = 0; /* Counts total bytes received by server */
int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;  /* Enough space for any address */  //line:netp:echoserveri:sockaddrstorage
    //char client_hostname[MAXLINE], client_port[MAXLINE];
    static pool pool;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // init BINARY_TREE
    push_node(&BINARY_TREE, 1, 7, 1000);
    push_node(&BINARY_TREE, 5, 3, 3700);
    push_node(&BINARY_TREE, 3, 10, 1200);
    push_node(&BINARY_TREE, 4, 8, 5000);
    push_node(&BINARY_TREE, 2, 6, 20000);


    listenfd = Open_listenfd(argv[1]);
    init_pool(listenfd, &pool);
    while (1) {
        /* Wait for listening/connected descriptor(s) to become ready */
        pool.ready_set = pool.read_set;
        pool.nready = Select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

        /* If listening descriptor ready, add new client to pool */
        if (FD_ISSET(listenfd, &pool.ready_set)){
            clientlen = sizeof(struct sockaddr_storage);
            connfd = Accept(listenfd, (SA*)&clientaddr, &clientlen);
            add_client(connfd, &pool);
        }

        /* Echo a text line from each ready connected descriptor */
        check_clients(&pool, BINARY_TREE);

//        clientlen = sizeof(struct sockaddr_storage);
//        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
//        Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
//                    client_port, MAXLINE, 0);
//        printf("Connected to (%s, %s)\n", client_hostname, client_port);
//        echo(connfd);
//        Close(connfd);
    }
    //exit(0);
}
/* $end echoserverimain */


void init_pool(int listenfd, pool *p){
    /* Initially, there are no connected descriptors */
    p->maxi = -1;
    for (int i = 0; i <FD_SETSIZE; i++) p->clientfd[i] = -1;

    /* Initially, listenfd is only member of select read set */
    p->maxfd = listenfd;
    FD_ZERO(&p->read_set);
    FD_SET(listenfd, &p->read_set);
}

void add_client(int connfd, pool *p){
    int i;
    p->nready--;
    for(i = 0; i < FD_SETSIZE; i++) /* Find an available slot */
        if (p->clientfd[i] < 0){
            /* Add connected descriptor to the pool */
            p->clientfd[i] = connfd;
            Rio_readinitb(&p->clientrio[i], connfd);

            /* Add the descriptor to descriptor set */
            FD_SET(connfd, &p->read_set);

            /* Update max descriptor and pool high water mark */
            if (connfd > p->maxfd) p->maxfd = connfd;
            if (i > p->maxi) p->maxi = i; /* maxi는 클라이언트 배열의 최대 인덱스값*/
            break;
        }

    /* Couldn't find an empty slot */
    if (i == FD_SETSIZE) app_error("add_client error: Too many clients");
}

void check_clients(pool *p, node* tree){
    int i, connfd, n;
    char buf[MAXLINE];
    char *argv[MAXARGS];
    rio_t rio;

    for (i = 0; (i <= p->maxi) && (p->nready > 0); i++){
        connfd = p->clientfd[i];
        rio = p->clientrio[i];

        /* If the descriptor is ready, echo a text line from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &p->ready_set))){
            p->nready--;
            if((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0){
                printf("server received %d bytes\n", n);
                parse_command(buf, argv);
                if (!strcmp(argv[0], "exit")){
                    printf("exit 블럭 진입!\n");
                    strcpy(buf, "exit");
                    Rio_writen(connfd, buf, strlen(buf));

                    Close(connfd);
                    FD_CLR(connfd, &p->read_set);
                    p->clientfd[i] = -1;
                }
                if (!strcmp(argv[0], "show")){
                    printf("show 블럭 진입!\n");
                    show_node(connfd, tree);
                }
                else if (!strcmp(argv[0], "buy")){
                    printf("buy 블럭 진입!\n");
                    buy(connfd, &tree, atoi(argv[1]), atoi(argv[2]));
                }
                else if (!strcmp(argv[0], "sell")){
                    printf("sell 블럭 진입!\n");
                    sell(connfd, &tree, atoi(argv[1]), atoi(argv[2]));
                }
                //Rio_writen(connfd, buf, n);
            }

            /* EOF detected, remove descriptor from pool */
            else{
                printf("close close close close close close\n");
                Close(connfd);
                FD_CLR(connfd, &p->read_set);
                p->clientfd[i] = -1;
            }
        }
    }
}


/*--------------------------------------------------------------------------------------*/
node* search_node(node *tree, int id_to_search){
    while (tree != NULL && tree->ID != id_to_search){
        if (tree->ID > id_to_search) tree = tree->left;
        else tree = tree->right;
    }
    return tree;
}


void push_node(node** tree, int id_to_push, int left_stock_to_push, int price_to_push){
    node* new_node = (node*)malloc(sizeof(node));
    node* cur_node = *tree;

    // create
    new_node->ID = id_to_push;
    new_node->left_stock = left_stock_to_push;
    new_node->price = price_to_push;
    new_node->left = new_node->right = new_node->parent = NULL;

    // if BINARY_TREE is empty
    if (cur_node == NULL){
        *tree = new_node;
        return;
    }

    // search_node position to insert
    while (cur_node != NULL){
        new_node->parent = cur_node;

        if (cur_node->ID > id_to_push) cur_node = cur_node -> left;
        else cur_node = cur_node->right;
    }

    // insert!
    if ((new_node->parent)->ID > id_to_push) (new_node->parent)->left = new_node;
    else (new_node->parent)->right = new_node;
}


void buy(int connfd, node** tree, int id_to_buy, int num){
    node* cur_node = NULL;
    char buf[MAXLINE];
    char tmp[MAXLINE];
    strcpy(buf, "");

    // search the node to buy
    cur_node = search_node(*tree, id_to_buy);

    // if there is no id to delete
    if(cur_node == NULL){
        //printf("There is no such ID!\n");
        sprintf(tmp, "There is no such ID!\n");
        strcat(buf, tmp);
        Rio_writen(connfd, buf, strlen(buf));
        return;
    }

    if ((cur_node->left_stock - num) < 0 ){
        //printf("Not enough left stocks!\n");
        sprintf(tmp, "Not enough left stocks!\n");
        strcat(buf, tmp);
        Rio_writen(connfd, buf, strlen(buf));
        return;
    }

    cur_node->left_stock = cur_node->left_stock - num;
}


void sell(int connfd, node** tree, int id_to_sell, int num){
    node* cur_node = NULL;
    char buf[MAXLINE];
    char tmp[MAXLINE];
    strcpy(buf, "");

    // search the node to buy
    cur_node = search_node(*tree, id_to_sell);

    // if there is no id to delete
    if(cur_node == NULL){
        sprintf(tmp, "There is no such ID!\n");
        strcat(buf, tmp);
        Rio_writen(connfd, buf, strlen(buf));
        return;
    }

    cur_node->left_stock = cur_node->left_stock + num;
}


void parse_command(char* buf, char** argv){
    char *delim;                // points to first space delimiter
    int ind = 0;

    buf[strlen(buf)-1] = ' ';   // replace triling '\n' with space
    while (*buf && (*buf == ' ')) buf++;

    // build the argv list
    while ((delim = strchr(buf, ' '))){
        argv[ind++] = buf;
        *delim = '\0';
        buf = delim + 1;
        while(*buf && (*buf == ' ')) // ignore space
            buf++;
    }
    argv[ind] = NULL;
}


void show_node(int connfd, node *tree){
    if (tree == NULL) return;

    char buf[MAXLINE];
    char tmp[MAXLINE];
    strcpy(buf, "");

    stack* st = malloc(sizeof(stack));
    st->top = -1;

    push(st, tree);
    while(st->top != -1){
        tree = pop(st);
        //printf("%d %d %d\n", tree->ID, tree->left_stock, tree->price);
        sprintf(tmp, "%d %d %d\n", tree->ID, tree->left_stock, tree->price);
        strcat(buf, tmp);
        if (tree->right != NULL) push(st, tree->right);
        if (tree->left != NULL) push(st, tree->left);
    }

    // 마지막 line이라는 표식
    buf[strlen(buf)-1] = 'E';
    buf[strlen(buf)] = '\n';
    printf("%s", buf);

    Rio_writen(connfd, buf, strlen(buf));
}

int is_empty(stack* st){
    if (st->top < 0) return 1;
    return 0;
}

int is_full(stack* st){
    if(st->top >= MAXSTOCK-1) return 1;
    return 0;
}

void push(stack* st, node* node_ptr){
    if (is_full(st)) printf("STACK FULL!\n");
    (st->arr)[++(st->top)] = node_ptr;
}

node* pop(stack* st){
    if (is_empty(st)) printf("STACK EMPTY!\n");
    return (st->arr)[(st->top)--];
}