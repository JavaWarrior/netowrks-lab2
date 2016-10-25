#ifndef _____RECEIVER____H
#define _____RECEIVER____H

#include "errorStruct.h"

statusEnum receiveGETResponse(int socketfd, char * filename);
statusEnum receivePOSTResponse(int socketfd, char * filename);

#endif //_____RECEIVER____H