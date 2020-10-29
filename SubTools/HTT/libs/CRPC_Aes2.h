#pragma once

#include "sRPC.h"
#include "Aes.h"

autoBlock_t *CRPC_GetRawKey(void); // —vŽÀ‘•
void CRPC_Init(void); // —vŽÀ‘•
autoBlock_t *ServiceCRPC(autoBlock_t *recvData, uint phase); // —vŽÀ‘•
