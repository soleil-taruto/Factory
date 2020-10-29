#pragma once

#include "C:\Factory\Common\Options\SClient.h" // no using
#include "C:\Factory\Common\Options\SockStream.h"

extern uint Client_SockTimeoutSec;

SockStream_t *ClientBegin(int sock, char *serviceName);
void ClientEnd(SockStream_t *ss);
