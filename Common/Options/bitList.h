#pragma once

#include "C:\Factory\Common\all.h"

typedef struct bitList_st // static member
{
	autoList_t *Buffer;
}
bitList_t;

bitList_t *newBitList(void);
bitList_t *newBitList_A(uint64 allocBitSize);
void releaseBitList(bitList_t *i);

// <-- cdtor

uint refBit(bitList_t *i, uint64 index);
void putBit(bitList_t *i, uint64 index, uint value);
void putBits(bitList_t *i, uint64 index, uint64 size, uint value);

void invBit(bitList_t *i, uint64 index);
void invBits(bitList_t *i, uint64 index, uint64 size);

// <-- accessor
