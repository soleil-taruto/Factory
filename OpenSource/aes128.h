#pragma once

#include "C:\Factory\Common\all.h"
#include "aes128\aes128.h"

typedef struct AES128_KeyTable_st // static member
{
	uint Ctx[44];
	uint DoEncrypt;
}
AES128_KeyTable_t;
AES128_KeyTable_t *AES128_CreateKeyTable(autoBlock_t *rawKey, uint doEncrypt);
void AES128_ReleaseKeyTable(AES128_KeyTable_t *i);

void AES128_Encrypt(AES128_KeyTable_t *i, void *in, void *out);
void AES128_EncryptBlock(AES128_KeyTable_t *i, autoBlock_t *in, autoBlock_t *out);
