#pragma once

#include "sRPC.h"
#include "Aes.h"
#include "C:\Factory\Meteor\RSA.h"

void CRPC_Init(void); // �v����
autoBlock_t *ServiceCRPC(autoBlock_t *recvData, uint phase); // �v����
