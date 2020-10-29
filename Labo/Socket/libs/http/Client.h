#pragma once

#include "C:\Factory\Common\Options\SockClient.h"
#include "GetPost.h"
#include "MultiPart.h" // httpRecvRequestFile() for httpSendRequestFile()

extern autoList_t *httpExtraHeader;
extern uint httpBlockTimeout;

autoBlock_t *httpSendRequest(char *domain, uint portno, char *proxyDomain, uint proxyPortno, char *path, autoBlock_t *content, uint timeout, uint sizemax);
int httpSendRequestFile(char *domain, uint portno, char *proxyDomain, uint proxyPortno, char *path, autoBlock_t *content, uint timeout, char *retContFile);
