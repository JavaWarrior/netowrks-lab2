
#include "client.h"

#include <stdio.h>


#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


#include <string.h>


#include "server.h"


#define BUF_SIZE 1000


int client_socketfd = 0;

statusEnum init_client(char * hostip, char * portnum);



void startsend(){
    FILE * fp = fopen("client.in","r");
    if(!fp){
        return;
    }
    char buf[BUF_SIZE];
    getline(&buf, BUF_SIZE, fp);

    char command[5], filename[200], hostname[200], port[7];

    strcpy(command, strtok(buf, " "));
    strcpy(filename, strtok(NULL, " "));
    strcpy(hostname, strtok(NULL, " "));
    char * next = strtok(NULL, " ");
    if(next != NULL){
        strcpy(filename, next);
    }else{
        strcpy(port, "80");
    }

    int status = init_client(hostname, port);
    if(status != SUCCESS){
        perror("error in connecting to server ");
        return ;
    }
    if(strcmp(command, "GET") == 0){
        char full_cmd[1000];
        full_cmd[0] = 0;
        sprintf(full_cmd, "GET /%s HTTP/1.1\r\nHOST: %s:%s\r\n\r\n",filename, hostname, port);
        send(client_socketfd, full_cmd, sizeof(full_cmd), 0);
        recv(client_socketfd, buf, 1000, 0);
        puts(buf);
    }

}

statusEnum init_client(char * hostip, char * portnum){
    int status;                             /* server addressinfo status */
    struct addrinfo hints, *res;            /* hints to be used in getaddrinfo */
    memset(&hints,0,sizeof(hints));         /* make sure hints is clear */
    hints.ai_family = AF_INET;              /* server will work on ipv4 */
    hints.ai_socktype = SOCK_STREAM;        /* TCP for SOCK_STREAM and UDP for SOCK_DGRAM*/
    status = getaddrinfo(hostip, portnum, &hints, &res);
                                            /* fill addrinfo for us */
    /* if error happened print it*/
    if(status || res == NULL){
        fprintf(stderr, "getaddrsinfo() error: %s\n", gai_strerror(status));
        return GENERROR;
    }

    for(; res != NULL; res = res->ai_next){
        /* assign a socket for the server */
        client_socketfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if(client_socketfd == -1){
            fprintf(stderr, "socket() error: %s\n", gai_strerror(status));
            continue;
        }
        /*bind socket to local machine */
        status = connect(client_socketfd, res->ai_addr, res->ai_addrlen);
        /* if error happened print it*/
        if(status){
            fprintf(stderr, "connect() error: %s\n", gai_strerror(status));
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
