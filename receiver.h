#ifndef _____RECEIVER____H
#define _____RECEIVER____H

#include "errorStruct.h"

statusEnum receiveGETResponse(int socketfd, char * filename);

statusEnum receivePOSTRequest(int socketfd, char * buf, int status);


statusEnum receiveGETRequest(int socketfd, char * buf, int status);

#endif //_____RECEIVER____H