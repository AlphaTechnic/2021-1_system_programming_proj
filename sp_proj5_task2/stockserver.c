#include "csapp.h"

#define MAXARGS 5
#define MAXSTOCK 1024
#define NTHREADS  4
#define SBUFSIZE  16

// connfd들을 관리하기 위한 구조체 선언
typedef struct {
    int *buf;          /* Buffer array */
    int n;             /* Maximum number of slots */
    int front;         /* buf[(front+1)%n] is first item */
    int rear;          /* buf[rear%n] is last item */
    sem_t mutex;       /* Protects accesses to buf */
    sem_t slots;       /* Counts available slots */
    sem_t items;       /* Counts available items */
} sbuf_t;

// sbuf를 관리하는 함수들
void sbuf_init(sbuf_t *sp, int n);
void sbuf_deinit(sbuf_t *sp);
void sbuf_insert(sbuf_t *sp, int item);
int sbuf_remove(sbuf_t *sp);


// 주식 종목의 정보를 가지는 node 정의
typedef struct node{
    int ID;
    int left_stock;
    int price;

    int readcnt;
    sem_t mutex2;

    struct node *left, *right, *parent;
}node;
node* BINARY_TREE = NULL;

// 이진 트리 탐색을 위한 stack 자료구조
typedef struct stack{
    node* arr[MAXSTOCK];
    int top;
}stack;

// 이진 트리를 탐색하여 client의 query를 수행하는 함수들
node* search_node(node *tree, int id_to_search);
void push_node(node** tree, int id_to_push, int left_stock_to_push, int price_to_push);
void buy(int connfd, node** tree, int id_to_buy, int num);
void sell(int connfd, node** tree, int id_to_sell, int num);
void parse_command(char* buf, char** argv);
void show_nodes(int connfd, node *tree);
void store_nodes(node *tree);

// functions for stack
int is_empty(stack* st);
int is_full(stack* st);
void push(stack* st, node* node_ptr);
node* pop(stack* st);

// client의 connection관리, client의 query 수행
void response(int connfd, node* tree);
void *thread(void *vargp);

// 전역변수들
sbuf_t sbuf; /* shared buffer of connected descriptors */
static sem_t mutex;   /* and the mutex that protects it */
static sem_t w;    /* Initially = 1 */
static sem_t t;     // thread의 개수를 업데이트할 때 다른 thread에 의해 방해받지 않게 하기 위한 semaphore
int CNT = 0; // connfd의 개수를 추적하기 위한 전역변수


int main(int argc, char **argv)
{
    int i, listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];
    pthread_t tid;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }

    // init sbuf and semaphores
    sbuf_init(&sbuf, SBUFSIZE); //line:conc:pre:initsbuf
    Sem_init(&w, 0, 1);
    Sem_init(&mutex, 0, 1);
    Sem_init(&t, 0, 1);

    // init BINARY_TREE
    FILE* fp = fopen("stock.txt", "r");
    char line[MAXLINE];
    char *line_split[MAXARGS];
    while (1){
        if (fgets(line, MAXLINE, fp) == NULL) break;
        parse_command(line, line_split);
        push_node(&BINARY_TREE, atoi(line_split[0]), atoi(line_split[1]), atoi(line_split[2]));
        if (feof(fp)) {
            fclose(fp);
            break;
        }
    }

    listenfd = Open_listenfd(argv[1]);

    // thread들을 미리 띄워둔다. thread pulling
    for (i = 0; i < NTHREADS; i++)  /* Create worker threads */ //line:conc:pre:begincreate
        Pthread_create(&tid, NULL, thread, NULL);               //line:conc:pre:endcreate

    while (1) {
        clientlen = sizeof(struct sockaddr_storage);
        connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);

        Getnameinfo((SA*)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to (%s, %s)\n", client_hostname, client_port);
        sbuf_insert(&sbuf, connfd); /* Insert connfd in buffer */
    }
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


// thread pulling
void *thread(void *vargp)
{
    Pthread_detach(pthread_self());
    while (1) {
        int connfd = sbuf_remove(&sbuf); /* Remove connfd from buffer */ //line:conc:pre:removeconnfd

        // connection 개수 1 증가
        P(&t);
        CNT++;
        V(&t);

        response(connfd, BINARY_TREE);                /* Service client */
        Close(connfd);

        // Close(connfd)하면서 connection 개수 1 감
        P(&t);
        CNT--;
        V(&t);

        // client와 연결된 connection이 하나도 없을 때,
        // 이진 트리 구조로 프로그램에 저장된 주식 정보들을 디스크에 백업
        if (CNT == 0){
            //printf("저장저장저장\n");
            store_nodes(BINARY_TREE);
        }
    }
}


/* Create an empty, bounded, shared FIFO buffer with n slots */
void sbuf_init(sbuf_t *sp, int n)
{
    sp->buf = Calloc(n, sizeof(int));
    sp->n = n;                       /* Buffer holds max of n items */
    sp->front = sp->rear = 0;        /* Empty buffer iff front == rear */
    Sem_init(&sp->mutex, 0, 1);      /* Binary semaphore for locking */
    Sem_init(&sp->slots, 0, n);      /* Initially, buf has n empty slots */
    Sem_init(&sp->items, 0, 0);      /* Initially, buf has zero data items */
}


/* Clean up buffer sp */
/* $begin sbuf_deinit */
void sbuf_deinit(sbuf_t *sp)
{
    Free(sp->buf);
}
/* $end sbuf_deinit */

/* Insert item onto the rear of shared buffer sp */
// Producer-Consumer problem 해결
void sbuf_insert(sbuf_t *sp, int item)
{
    P(&sp->slots);                          /* Wait for available slot */
    P(&sp->mutex);                          /* Lock the buffer */
    sp->buf[(++sp->rear)%(sp->n)] = item;   /* Insert the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->items);                          /* Announce available item */
}

/* Remove and return the first item from buffer sp */
// Producer-Consumer problem 해결
int sbuf_remove(sbuf_t *sp)
{
    int item;
    P(&sp->items);                          /* Wait for available item */
    P(&sp->mutex);                          /* Lock the buffer */
    item = sp->buf[(++sp->front)%(sp->n)];  /* Remove the item */
    V(&sp->mutex);                          /* Unlock the buffer */
    V(&sp->slots);                          /* Announce available slot */
    return item;
}

static void init_response(void)
{
    Sem_init(&mutex, 0, 1);
}

void response(int connfd, node* tree)
{
    int n;
    char buf[MAXLINE];
    char *argv[MAXARGS];
    rio_t rio;
    static pthread_once_t once = PTHREAD_ONCE_INIT;

    Pthread_once(&once, init_response); //line:conc:pre:pthreadonce
    Rio_readinitb(&rio, connfd);        //line:conc:pre:rioinitb
    while((n = Rio_readlineb(&rio, buf, MAXLINE)) != 0) {
        printf("server received %d bytes\n", n);

        // parse query of client
        parse_command(buf, argv);
        if (!strcmp(argv[0], "exit")){
            //printf("exit 블럭 진입!\n");
            strcpy(buf, "exit");
            Rio_writen(connfd, buf, strlen(buf));

            Close(connfd);
        }
        if (!strcmp(argv[0], "show")){
            //printf("show 블럭 진입!\n");
            show_nodes(connfd, tree);
        }
        else if (!strcmp(argv[0], "buy")){
            //printf("buy 블럭 진입!\n");
            buy(connfd, &tree, atoi(argv[1]), atoi(argv[2]));
        }
        else if (!strcmp(argv[0], "sell")){
            //printf("sell 블럭 진입!\n");
            sell(connfd, &tree, atoi(argv[1]), atoi(argv[2]));
        }
    }
}

/*--------------------------------------------------------------------------------------*/
// 아래는 이진 트리를 탐색하여 client의 query를 수행하는 함수들
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
    new_node->readcnt = 0;
    Sem_init(&(new_node->mutex2), 0, 1);

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

    // if there's not enough left stocks
    // 잔여 주식이 없다는 메세지를 client에게 보냄
    if ((cur_node->left_stock - num) < 0 ){
        //printf("Not enough left stocks!\n");
        sprintf(tmp, "Not enough left stocks!\n");
        strcat(buf, tmp);
        Rio_writen(connfd, buf, strlen(buf));
        return;
    }

    // Reader-Writer problem 해결을 위한 semaphore 수행
    // 프로그램 내의 자료구조를 업데이트(write) 하는 동안 다른 read thread가 방해할 수 없다.
    P(&w);
    cur_node->left_stock = cur_node->left_stock - num;
    V(&w);

    // 매수 성공 메세지 전송
    sprintf(tmp, "[buy] success\n");
    strcat(buf, tmp);
    Rio_writen(connfd, buf, strlen(buf));
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

    // Reader-Writer problem 해결을 위한 semaphore 수행
    // 프로그램 내의 자료구조를 업데이트(write) 하는 동안 다른 read thread가 방해할 수 없다.
    P(&w);
    cur_node->left_stock = cur_node->left_stock + num;
    V(&w);

    // 매수 성공 메세지를 보냄
    sprintf(tmp, "[buy] success\n");
    strcat(buf, tmp);
    Rio_writen(connfd, buf, strlen(buf));
}


void show_nodes(int connfd, node *tree){
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

        ///////////////////////////////////////////////
        P(&(tree->mutex2));
        tree->readcnt++;
        if (tree->readcnt == 1) P(&w);    /* First in */
        V(&(tree->mutex2));
        ///////////////////////////////////////////////

        /* critical section : reader thread의 작업 */
        // write thread가 이를 방해할 수 없다.
        sprintf(tmp, "%d %d %d\n", tree->ID, tree->left_stock, tree->price);
        strcat(buf, tmp);

        ///////////////////////////////////////////////
        P(&(tree->mutex2));
        tree->readcnt--;
        if (tree->readcnt == 0) V(&w);    /* Last out */
        V(&(tree->mutex2));
        ///////////////////////////////////////////////

        if (tree->right != NULL) push(st, tree->right);
        if (tree->left != NULL) push(st, tree->left);
    }

    // 마지막 line이라는 표식
    buf[strlen(buf)-1] = 'E';
    buf[strlen(buf)] = '\n';
    //printf("%s", buf);

    Rio_writen(connfd, buf, strlen(buf));
}

// 프로그램 내의 주식 정보를 (비교적) 안전한 디스크에 백업
void store_nodes(node *tree){
    FILE* fp = fopen("stock.txt", "w");

    if (tree == NULL) {
        fclose(fp);
        return;
    }

    char buf[MAXLINE];
    char tmp[MAXLINE];
    strcpy(buf, "");

    // stack을 이용하여 이진 트리를 탐색
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
    //printf("%s\n", buf);
    fputs(buf, fp);
    fclose(fp);
}

// functions for stack
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
