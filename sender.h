#ifndef ______SENDER___H
#define ______SENDER___H

#include "errorStruct.h"

#define CHUNK 1000 /*chunk size*/
#define TIMEOUT 5LL

statusEnum HTTPSendFile(char * filepath, int socketfd, requestType reqType);
statusEnum sendChar(char * buffer, int bufSize, int socketfd);
statusEnum sendHTTPGET(char * hostname, char* filename, char * port, int socketfd);
#endif

