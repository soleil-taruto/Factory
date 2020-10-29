#pragma once

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "mutex.h"

autoList_t *mailRecv(char *popServer, uint portno, char *user, char *pass, uint recvMax, uint sizeMax, int recvAndDelete);
