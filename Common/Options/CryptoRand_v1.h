#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\sha512.h"
#include "C:\Factory\OpenSource\camellia.h"
#include "CryptoRand_MS.h"

uint getCryptoByte(void);
uint getCryptoRand16(void);
uint getCryptoRand24(void);
uint getCryptoRand(void);
uint64 getCryptoRand64(void);
autoBlock_t *makeCryptoRandBlock(uint count);
