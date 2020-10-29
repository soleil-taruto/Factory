#pragma once

#include "C:\Factory\Common\all.h"
#include "CryptoRand.h"

char *MakeUUID(int bracket);
int GetRandIDChar(void);
char *MakeRandID(void);
char *MakeRandHexID(void);
autoBlock_t *GetConcreteRawKey(void);
