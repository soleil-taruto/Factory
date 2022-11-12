#include "bitTable.h"

bitTable_t *newBitTable(uint w, uint h)
{
	bitTable_t *i = (bitTable_t *)memAlloc(sizeof(bitTable_t));

	errorCase(w < 1);
	errorCase(h < 1);

	i->Buffer = newBitList_A((uint64)w * h);
	i->W = w;
	i->H = h;

	return i;
}
void releaseBitTable(bitTable_t *i)
{
	releaseBitList(i->Buffer);
	memFree(i);
}

#define GET_BIT(buff, w, h, x, y) \
	(refBit((buff), (x) + (uint64)(y) * (w)))

#define SET_BIT(buff, w, h, x, y, value) \
	(putBit((buff), (x) + (uint64)(y) * (w), (value)))

void resizeBitTable(bitTable_t *i, uint w, uint h)
{
	bitList_t *buffNew;
	uint mir_w;
	uint mir_h;
	uint x;
	uint y;

	errorCase(w < 1);
	errorCase(h < 1);

	buffNew = newBitList_A((uint64)w * h);

	mir_w = m_min(i->W, w);
	mir_h = m_min(i->H, h);

	for (x = 0; x < mir_w; x++)
	for (y = 0; y < mir_h; y++)
	{
		SET_BIT(buffNew, w, h, x, y, GET_BIT(i->Buffer, i->W, i->H, x, y));
	}
	releaseBitList(i->Buffer);
	i->Buffer = buffNew;
	i->W = w;
	i->H = h;
}
uint getBitTableWidth(bitTable_t *i)
{
	return i->W;
}
uint getBitTableHeight(bitTable_t *i)
{
	return i->H;
}

uint getTableBit(bitTable_t *i, uint x, uint y)
{
	errorCase(i->W <= x);
	errorCase(i->H <= y);

	return GET_BIT(i->Buffer, i->W, i->H, x, y);
}
void setTableBit(bitTable_t *i, uint x, uint y, uint value)
{
	errorCase(i->W <= x);
	errorCase(i->H <= y);

	SET_BIT(i->Buffer, i->W, i->H, x, y, value);
}
void setTableBits(bitTable_t *i, uint l, uint t, uint w, uint h, uint value) // w, h: 0 ok
{
	uint y;

	errorCase(i->W < l);
	errorCase(i->H < t);
	errorCase(i->W - l < w);
	errorCase(i->H - t < h);

	for (y = 0; y < h; y++)
	{
		putBits(i->Buffer, l + (uint64)(t + y) * i->W, (uint64)w, value);
	}
}

void invTableBit(bitTable_t *i, uint x, uint y)
{
	errorCase(i->W <= x);
	errorCase(i->H <= y);

	invBit(i->Buffer, x + (uint64)y * i->W);
}
void invTableBits(bitTable_t *i, uint l, uint t, uint w, uint h) // w, h: 0 ok
{
	uint y;

	errorCase(i->W < l);
	errorCase(i->H < t);
	errorCase(i->W - l < w);
	errorCase(i->H - t < h);

	for (y = 0; y < h; y++)
	{
		invBits(i->Buffer, l + (uint64)(t + y) * i->W, (uint64)w);
	}
}
