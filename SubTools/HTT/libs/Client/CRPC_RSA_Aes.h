#pragma once

#include "sRPC.h"
#include "..\Aes.h"
#include "C:\Factory\Meteor\RSA.h"

void ClientCRPC_PreConnect(void);

void ClientCRPC_Begin(SockStream_t *ss);
void ClientCRPC_End(void);

autoBlock_t *ClientCRPC(SockStream_t *ss, autoBlock_t *sendData);
