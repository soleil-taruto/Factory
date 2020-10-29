#pragma once

#include "C:\Factory\Common\all.h"
#include "mt19937\mt19937ar.h"

void mt19937_initByArray(autoBlock_t *initKey);
void mt19937_init32(uint seed);
void mt19937_init(void);
uint mt19937_rnd32(void);
uint mt19937_rnd(uint modulo);
uint mt19937_range(uint minval, uint maxval);
uint64 mt19937_rnd64(void);
uint64 mt19937_rnd64Mod(uint64 modulo);
uint64 mt19937_range64(uint64 minval, uint64 maxval);
void mt19937_rndBlock(void *block, uint size);
