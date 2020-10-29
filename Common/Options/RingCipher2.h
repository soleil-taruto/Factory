#pragma once

#include "C:\Factory\Common\all.h"
#include "Cipher.h"

autoList_t *rngcphrCreateKeyTableList(autoBlock_t *rawKey);

void rngcphrEncryptBlock(autoBlock_t *block, autoList_t *keyTableList);
void rngcphrDecryptBlock(autoBlock_t *block, autoList_t *keyTableList);
void rngcphrEncrypt(autoBlock_t *block, autoList_t *keyTableList);
int rngcphrDecrypt(autoBlock_t *block, autoList_t *keyTableList);

void rngcphrEncryptFile(char *file, autoList_t *keyTableList);
int rngcphrDecryptFile(char *file, autoList_t *keyTableList);
