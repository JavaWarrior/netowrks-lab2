#include "sender.h"
#include "utils.h"


#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


#define POST_HEAD_SIZE 50

/* return file size */
int getFileLength(FILE * fp){
	fseek(fp, 0, SEEK_END);
	int x= ftell(fp);
	rewind(fp);
	return x;
}

/* send post header over socket */
statusEnum sendPostHeader(char * filepath, int length, int socketfd){
	char * buf = (char *) malloc(POST_HEAD_SIZE + sizeof(filepath));
	length = sprintf(buf, "POST %s HTTP/1.1\r\nContent-Length: %d\r\n\r\n",filepath, length);
	if(length > 0){
		int status = sendChar(buf, length, socketfd);
		free(buf);
		return status;
	}
	free(buf);
	return GENERROR;
}

/* send not found response */
statusEnum sendGet404Resp(int socketfd){
	char * buf = "HTTP/1.0 404 Not Found\r\n\r\n";
	return sendChar(buf, sizeof(buf), socketfd);
}

/* send get found header */
statusEnum sendGetRespHeader(int length, int socketfd){
	char * buf = (char *) malloc(POST_HEAD_SIZE);
	length = sprintf(buf, "HTTP/1.1 200 OK\r\nContent-Length: %d\r\n\r\n",length);

	if(length > 0){
		int status = sendChar(buf, length, socketfd);
		free(buf);
		return status;
	}
	else{
		free(buf);
		return GENERROR;	
	} 
}

/* sends nuffer over socket */
statusEnum sendChar(char * buffer, int bufSize, int socketfd){
	int sent = 0;
	unsigned long long curTime = getCurrentSeconds();	/*get current time for timeout */
	do{
		sent += send(socketfd, (void *)(buffer+sent), bufSize-sent, 0);
		/*check number of sent bytes and continue from offset */
	}while(sent < bufSize && getCurrentSeconds() - curTime < TIMEOUT);
	/* check if all bytes are sent and check for time out */
	
	if(sent != bufSize) {
		return SEND_ERROR; /* if not all characters are sent report error */
	}
	return SUCCESS;
}

statusEnum sendFile(FILE * fp, int socketfd, int length){
	/* send file fp over socked fd */
	char * buf = (char *)malloc(CHUNK);
	while( length > 0 ){
		if(length > CHUNK){
			fread(buf, CHUNK, 1, fp);							/* read chunk of data in buffer */
			int status = sendChar(buf, CHUNK, socketfd);		/* if send successfully continue else report it */
			if(status != SUCCESS){
				perror("error happened during sending file");
				free(buf);
				return status;
			}
			length -= CHUNK;

		}else{
			int x  = fread(buf, length, 1, fp);
			printf("%d\n",x);
			int status = sendChar(buf, CHUNK, socketfd);
			if(status != SUCCESS){
				perror("error happened during sending file");
				free(buf);
				return status;
			}
			length = 0;											/* all the file is sent*/
		}
	}
	free(buf);
	return SUCCESS;
}


/* send any file over HTTP protocol */
statusEnum HTTPSendFile(char * filepath, int socketfd, requestType reqType){
	FILE *fp = fopen(filepath,"rb");
	if(!fp && reqType == POST) {
		//file not found
		return GENERROR;
	}else if(!fp) {
		return sendGet404Resp(socketfd);
	}
	int length = getFileLength(fp);
	if(reqType == GET){
		/* responding to a GET request */
		int status = sendGetRespHeader(length, socketfd);
		/* send header first `ok` */
		if(status != SUCCESS){
			perror("error sending get request response");
			return status;
		}
		return sendFile(fp, socketfd, length);				/*send file after that*/
	}else if(reqType == POST){
		/* post request */
		int status = sendPostHeader(filepath, length, socketfd);
		if(status != SUCCESS){
			perror("error sending post header");
			return status;
		}
		return sendFile(fp, socketfd, length);
	}

	//not reachable 

}

statusEnum sendHTTPGET(char * hostname, char * filename,char * port, int socketfd){
	char * buf = (char *) malloc(POST_HEAD_SIZE+sizeof(filename) + sizeof(hostname) + sizeof(port));
	int length = sprintf(buf,"GET /%s HTTP/1.1\r\nHOST: %s:%s\r\n\r\n",filename, hostname, port);
	if(length > 0){
		int status = sendChar(buf, length, socketfd);
		free(buf);
		return status;
	}
	else return GENERROR;
}