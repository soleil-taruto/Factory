#pragma once

#include "C:\Factory\Common\all.h"
#include "camellia\camellia.h"

typedef struct camellia_keyTable_st // static member
{
	camellia_KeyTableType KeyTable;
	uint KeyBitLength;
}
camellia_keyTable_t;
camellia_keyTable_t *camellia_createKeyTable(autoBlock_t *rawKey);
void camellia_releaseKeyTable(camellia_keyTable_t *i);

void camellia_encrypt(camellia_keyTable_t *i, void *in, void *out, int doEncrypt);
void camellia_encryptBlock(camellia_keyTable_t *i, autoBlock_t *in, autoBlock_t *out, int doEncrypt);

void camellia_ecb(camellia_keyTable_t *i, autoBlock_t *in, autoBlock_t *out, int doEncrypt);
void camellia_cbc(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out, int doEncrypt);
void camellia_cbcRing(camellia_keyTable_t *i, autoBlock_t *in, autoBlock_t *out, int doEncrypt);
void camellia_ctr(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out);
void camellia_ofb(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out);
void camellia_cfb(camellia_keyTable_t *i, autoBlock_t *iv, autoBlock_t *in, autoBlock_t *out, int doEncrypt);
