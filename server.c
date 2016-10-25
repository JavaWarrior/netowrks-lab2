#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h> //getprotobyname()


#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <pthread.h>
#include <unistd.h>

#include "server.h"

#define PORTNUM "2000"  /*server port*/
#define BACKLOG 10      /*server connection queue size */

//#define CONNECTION_MAX 100

#define BUF_SIZE 10000



int                 server_socketfd = 0;
struct addrinfo     server_addrinfo;

struct con_handler_struct_args{
    int socketfd;
};

pthread_mutex_t printmutex;


void * handle_connection(void * args){
    int socketfd = ((struct con_handler_struct_args *)args)->socketfd;
    char * buf = (char *) malloc(BUF_SIZE);
    int status = recv(socketfd, buf, BUF_SIZE, 0);
    if(status == -1){
        fprintf(stderr, "error receiving from client !\n");
        fprintf(stderr, "%s\n", gai_strerror(status));
       pthread_exit(NULL);
    }
    if(buf[0] == 'G'){
        /* get request */
        puts("GET");
        receiveGETRequest(socketfd, buf, status);

    }else if (buf[0] == 'P'){
        /* post request */
        puts("POST");
        receivePOSTRequest(socketfd, buf, status);
    }else{
        perror("parsing error");
    }
    close(socketfd);

    // puts(buf);

    free(buf);
    pthread_exit(NULL);
    return NULL;
}

void init_threads(){
    pthread_mutex_init(&printmutex, NULL);
}
statusEnum init_server(){

    init_threads();

    int status, yes = 1;                             /* server addressinfo status */
    struct addrinfo hints, *res;     /* hints to be used in getaddrinfo */
    memset(&hints,0,sizeof(hints));         /* make sure hints is clear */
    hints.ai_family = AF_INET;              /* server will work on ipv4 */
    hints.ai_socktype = SOCK_STREAM;        /* TCP for SOCK_STREAM and UDP for SOCK_DGRAM*/
    hints.ai_flags = AI_PASSIVE;            /* let getaddrinfo get our ip for us */
    status = getaddrinfo(NULL, PORTNUM, &hints, &res);
                                            /* fill addrinfo for us */
    /* if error happened print it*/
    if(status || res == NULL){
        fprintf(stderr, "getaddrsinfo() error: %s\n", gai_strerror(status));
        return GENERROR;
    }

    for(; res != NULL; res = res->ai_next){
        /* assign server addrinfo from the first result we can use */
        server_addrinfo = *res;
        /* assign a socket for the server */
        server_socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(server_socketfd == -1){
            fprintf(stderr, "socket() error: %s\n", gai_strerror(status));
            continue;
        }
        /* check if socket is used and try to re-use it */
        if (setsockopt(server_socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            return GENERROR;
        }
        /*bind socket to local machine */
        status = bind(server_socketfd, res->ai_addr, res->ai_addrlen);
        /* if error happened print it*/
        if(status){
            fprintf(stderr, "bind() error: %s\n", gai_strerror(status));
            continue;
        }

        break;
        /* we found our beloved connection */
    }

    /* if loop reached its end, it means that we didn't succeed at opening socket */
    if(res == NULL){
        return GENERROR;
    }

    freeaddrinfo(res);               /* free the linked list after been used */
    return SUCCESS;
}



void start_serverloop(){
    struct sockaddr_storage client_addr;    /* client address info */
    socklen_t sock_size;
    int client_fd;                          /* client socket descriptor */
    while(1){
    /* server main loop */
        sock_size = sizeof(client_addr);
        client_fd = accept(server_socketfd, (struct sockaddr *)&client_addr, &sock_size); /* accept connection */
        if(client_fd == -1){
            perror("accept error ");
            continue;
        }
        pthread_t * worker_thread = (pthread_t *) malloc(sizeof(pthread_t));
        struct con_handler_struct_args * args = (struct con_handler_struct_args *)malloc(sizeof(struct con_handler_struct_args));
        args->socketfd = client_fd;
        pthread_create(worker_thread, NULL, handle_connection, (void *)args);
    }
}

statusEnum startServer(){
    int status;
    /* start listening on this port */
    status = listen(server_socketfd, BACKLOG);
    /* if error happened print it*/
    if(status){
        fprintf(stderr, "listen() error: %s\n", gai_strerror(status));
        return GENERROR;
    }


    printf("server: waiting for connections !\n");

    start_serverloop();
    return SUCCESS;
}
