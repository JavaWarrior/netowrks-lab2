#include "receiver.h"
#include "sender.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>

#define BUF_INIT_SIZE 1000

struct header {
	int contentLength;
	int fileStart;
};
void echo(char * buf, int st, int en){
	for(int i =st ; i <= en ; i++){
		printf("%c",buf[i]);
	}
	printf("\n");
}

int isnumeric(char c){
	if ( c <= '9' && c >= '0') return 1;
	return 0;
}
int ifEqual(char * b1, char * b2, int st, int en){
	for(int i =0  ; b2[i] != 0 && b1[i+st] != 0 ; i++){
		if(b1[i+st] != b2[i]) return 0;
	}
	return 1;
}
int getSize(char * buf, int start){
	int num =0 ;
	while(isnumeric(buf[start])){
		num *= 10;
		num += buf[start++] - '0';
	}
	return num;
}
struct header getContLength(char * buf, int length){
	int num, dataStart;
	struct header hd;
	for(int i =0 ; i  < length; i++){
		if(i > 2){
			if(buf[i] == '\n' && buf[i-1] =='\r' && buf[i-2] == '\n' && buf[i-3] == '\r'){
				dataStart = i+1;
			}
		}
	}
	int cur = -1, last = -1;
	for(int i =0 ; i < length ; i++){
		if(i > 1){
			if(buf[i] == '\n' && buf[i-1] == '\r'){
				if(cur == -1){
					cur = i-2;
				}else{
					last = cur+3;
					cur = i-2;
					if(ifEqual(buf, "Content-Length: ", last, cur)){
						// printf("%d\n",last);
						hd.fileStart = dataStart;
						hd.contentLength = getSize(buf,last+16);
						// printf("%d\n", hd.contentLength);
						return hd;
					}
				}
			}
		}
	}
	hd.contentLength = -1;
	return hd;
}



statusEnum receiveGETResponse(int socketfd, char * filename){
	/* receive response */
	char * buf = (char *) malloc(BUF_INIT_SIZE+1);
	int status = recv (socketfd, buf, BUF_INIT_SIZE, MSG_PEEK); /* peek on data size on the other end */
	if(status < 0){
		perror("error receiving packages");
		free(buf);
		return GENERROR;
	}
	if(status < 19){
		perror("invalid response ");
		free(buf);
		return GENERROR;
		/* response must be in form 'HTTP/1.0 xxx OK\r\n\r\n' 19 char at least  */
	}
	status = recv(socketfd, buf, BUF_INIT_SIZE, 0); /* make sure to get the exact response */
	int resp;
	sscanf(buf, "HTTP/1.1 %d",&resp);
	// printf("%d\n",resp);
	/* scanning response to learn its type */
	if(resp == 400){
		/*file not found */
		free(buf);
		return FILE_NOT_FOUND;
	}else if (resp != 200){
		free(buf);
		perror("INVALID RESPONSE");
		return GENERROR;
	}
	// puts(buf);

	struct header hd = getContLength(buf, status);
	int num = hd.contentLength;

	if(num <= 0){
		perror("Invalid RESPONSE");
		return GENERROR;
	}

	int dataStart = hd.fileStart;

	/*open new file */
	FILE * fp = fopen(filename, "wb");
	
	if(!fp){
		perror("could not write to file");
		return GENERROR;
	}

	printf("writing %d bytes to %s\n",num,  filename);
	
	char * buf2 = (char *)malloc(num);			/* create new file of the size of the new data */
	int indx = 0;
	for(; indx < status - dataStart; indx++){
		buf2[indx] = buf[indx + dataStart];
	}
	while(num - indx > 1){
		status = recv(socketfd, (void *)(buf2+indx), num - indx, 0);
		if(status <= 0) {
			return GENERROR;
		}
		indx += status;
	}
	/* receive all the file and write it*/
	fwrite(buf2, num, 1, fp);

	fclose(fp);

	if(buf != NULL)
		free(buf);
	if(buf2 != NULL)
		free(buf2);
	return SUCCESS;
}
/* to do later: live buffering */

statusEnum receivePOSTRequest(int socketfd, char * buf, int status){
	/* receive response */
	if(status < 55){
		perror("invalid Request ");
		return GENERROR;
		/* response must be in form 'POST /x.x HTTP/1.1\r\nhost:x.x\r\ncontent-length: x\r\n\r\n' 55 char at least  */
	}
	char * filename = (char *) malloc(400);
	sscanf(buf, "POST %s",filename);
	
	struct header hd = getContLength(buf, status);
	int num = hd.contentLength;

	if(num <= 0){
		perror("Invalid Request");
		return GENERROR;
	}

	int dataStart = hd.fileStart;


	/*open new file */
	FILE * fp = fopen(filename, "wb");
	
	if(!fp){
		perror("could not write to file");
		return GENERROR;
	}
	
	char * buf2 = (char *)malloc(num);			/* create new file of the size of the new data */
	int indx = 0;
	for(; indx < status - dataStart; indx++){
		buf2[indx] = buf[indx + dataStart];
	}
	while(num - indx > 1){
		status = recv(socketfd, (void *)(buf2+indx), num - indx, 0);
		if(status <= 0) {
			return GENERROR;
		}
		indx += status;
	}
	/* receive all the file and write it*/
	fwrite(buf2, num, 1, fp);

	fclose(fp);

	if(buf != NULL)
		free(buf);
	if(buf2 != NULL)
		free(buf2);


	return SUCCESS;

}

statusEnum receiveGETRequest(int socketfd, char * buf, int status){
	/* GET / HTTP/1.1\r\nhost:x.xx\r\n\r\n 32 character */
	if(status < 32){
		perror("invalid Request ");
		return GENERROR;
	}
	char * filename = (char *) malloc(400);
	sscanf(buf, "GET %s",filename);

	if(strcmp(filename,"/") ==0){
		filename = "/default.html";
	}
	
	
	/* we've known the file name, now we send it back */
	filename++; //advance pointer so we ignore the slash 
	HTTPSendFile("", filename, socketfd, GET);

}
