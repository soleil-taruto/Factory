#pragma once

#include "Client.h"

extern uint RPC_RecvSizeMax;

autoBlock_t *ClientRPC(SockStream_t *ss, autoBlock_t *sendData);
