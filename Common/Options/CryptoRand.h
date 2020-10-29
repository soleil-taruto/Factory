#pragma once

#include "C:\Factory\Common\all.h"
#include "CryptoRand_MS.h"

uint getCryptoByte(void);
void getCryptoBytes(uchar *buff, uint size);
uint getCryptoRand16(void);
uint getCryptoRand24(void);
uint getCryptoRand(void);
uint64 getCryptoRand64(void);
uint64 getCryptoRand64Mod(uint64 modulo);
uint64 getCryptoRand64Range(uint64 minval, uint64 maxval);
autoBlock_t *makeCryptoRandBlock(uint count);
