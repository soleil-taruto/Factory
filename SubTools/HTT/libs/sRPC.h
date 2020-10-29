#pragma once

#include "Service.h"
#include "C:\Factory\Common\Options\FileTools.h"

extern uint RPC_RecvSizeMax;
extern uint RPC_ConnectTimeoutSec;
extern uint RPC_RecvTimeoutSec;
extern uint RPC_SendTimeoutSec;

void RPC_SendCompleteAndDisconnect(void);

autoBlock_t *ServiceRPC(autoBlock_t *recvData, uint phase); // —vŽÀ‘•
