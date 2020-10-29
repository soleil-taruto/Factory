#pragma once

#include "sRPC.h"
#include "..\Aes.h"

void ClientCRPC_Begin(SockStream_t *ss);
void ClientCRPC_End(void);

autoBlock_t *ClientCRPC(SockStream_t *ss, autoBlock_t *sendData, autoBlock_t *rawKey);
