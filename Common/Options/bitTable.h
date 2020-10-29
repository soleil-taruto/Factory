#pragma once

#include "C:\Factory\Common\all.h"
#include "bitList.h"

typedef struct bitTable_st // static member
{
	bitList_t *Buffer;
	uint W;
	uint H;
}
bitTable_t;

bitTable_t *newBitTable(uint w, uint h);
void releaseBitTable(bitTable_t *i);

// <-- cdtor

void resizeBitTable(bitTable_t *i, uint w, uint h);
uint getBitTableWidth(bitTable_t *i);
uint getBitTableHeight(bitTable_t *i);

uint getTableBit(bitTable_t *i, uint x, uint y);
void setTableBit(bitTable_t *i, uint x, uint y, uint value);
void setTableBits(bitTable_t *i, uint l, uint t, uint w, uint h, uint value);

void invTableBit(bitTable_t *i, uint x, uint y);
void invTableBits(bitTable_t *i, uint l, uint t, uint w, uint h);
