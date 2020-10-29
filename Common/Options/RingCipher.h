#pragma once

#include "C:\Factory\Common\all.h"
#include "Cipher.h"

void rcphrEncryptor(autoBlock_t *block, autoList_t *keyTableList, int doEncrypt);

void rcphrEncryptorBlock(autoBlock_t *block, autoList_t *keyTableList, uint64 counter2[2]);
int rcphrDecryptorBlock(autoBlock_t *block, autoList_t *keyTableList, uint64 counter2[2]);
