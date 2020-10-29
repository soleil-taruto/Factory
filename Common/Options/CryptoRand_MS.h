#pragma once

#pragma comment(lib, "ADVAPI32")

#include "C:\Factory\Common\all.h"
#include <wincrypt.h>

int createKeyContainer(void);
int deleteKeyContainer(void);
void getCryptoBlock_MS(uchar *buffer, uint size);
autoBlock_t *makeCryptoBlock_MS(uint count);
