#pragma once

#include "sRPC.h"
#include "Aes.h"

autoBlock_t *CRPC_GetRawKey(void); // �v����
void CRPC_Init(void); // �v����
autoBlock_t *ServiceCRPC(autoBlock_t *recvData, uint phase); // �v����
