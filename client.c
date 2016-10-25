
#include "client.h"

#include <stdio.h>
#include <stdlib.h>


#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


#include <string.h>


#include "errorStruct.h"
#include "sender.h"
#include "receiver.h"

#include <unistd.h>

#define BUF_SIZE 1000


int client_socketfd = 0;

statusEnum make_connection(char * hostip, char * portnum);

/* tokenize line 'buf' to fill hostaname, port, filename */
void readCommand(char * buf,char * command,  char * hostname, char* filename, char * port){
    char * next ;
    next = strtok(buf, " \n");
    strcpy(command, next);
    next = strtok(NULL, " \n");
    strcpy(filename, next);
    next = strtok(NULL, " \n");
    strcpy(hostname, next);
    next = strtok(NULL, " \n");
    if(next != NULL){
        strcpy(port, next);
    }else{
        strcpy(port, "80");
    }
}

void closeconnection(int socketfd){
     close(socketfd);
}

/* start main client function, reads from input file and process requests */
void startClient(){
    FILE * fp = fopen("client.in","r");
    /* read instructions for client */

    if( fp == NULL){
        return;
    }
    char * command = (char *) malloc(10), * filename = (char *) malloc(200), *hostname = (char *) malloc(200), * port = (char *) malloc(10);
    char  *buf;
    size_t sz;
    while(1){
        int ret = getline(&buf, &sz, fp);
        if(!buf || ret <= 0){
            break;
        }
        readCommand(buf, command, hostname, filename, port);
        printf("connecting to: %s on port %s requesting page %s\n",hostname, port, filename);
        int status = make_connection(hostname, port);
        if(status != SUCCESS){
            return ;
        }

        puts("connection established ");
        if(strcmp(command, "GET") == 0){
            sendHTTPGET(hostname, filename, port, client_socketfd);
            // recv(client_socketfd, buf, 1000, 0);
            // puts(buf);
            receiveGETResponse(client_socketfd, filename);
        }else if(strcmp(command, "POST" == 0)){
            HTTPSendFile(filename, client_socketfd, POST);
            char * response = (char *) malloc(BUF_SIZE);
            recv(client_socketfd, response, BUF_SIZE, 0);
            puts(response);
        }else{
            perror("invalid command");
        }
        closeconnection(client_socketfd);
    }
    free(command);
    free(filename);
    free(hostname);
    free(port);
    fclose(fp);
}

statusEnum make_connection(char * hostname, char * portnum){
    
    int status;                             /* server addressinfo status */
    struct addrinfo hints, *res;            /* hints to be used in getaddrinfo */
    
    memset(&hints,0,sizeof(hints));         /* make sure hints is clear */
    hints.ai_family = AF_INET;              /* server will work on ipv4 */
    hints.ai_socktype = SOCK_STREAM;        /* TCP for SOCK_STREAM and UDP for SOCK_DGRAM*/
    
    printf("%s\n",hostname);
    status = getaddrinfo(hostname, portnum, &hints, &res);
                                            /* fill addrinfo for us */
    /* if error happened print it*/
    if(status != 0){
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
