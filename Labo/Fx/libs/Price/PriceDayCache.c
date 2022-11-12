/*
	多分１通貨１年分くらいメモリに入る。-> クリアしなくて良い？
*/

#include "all.h"

static void CreateTree(PriceDayCache_t *i)
{
	i->Tree = rbCreateTree((uint (*)(uint))strx, (sint (*)(uint, uint))strcmp, (void (*)(uint))memFree);
}
static void ReleaseTree(PriceDayCache_t *i)
{
	rbtCallAllValue(i->Tree, (void (*)(uint))ReleasePriceDay);
	rbReleaseTree(i->Tree);
}

PriceDayCache_t *CreatePriceDayCache(void)
{
	PriceDayCache_t *i = nb_(PriceDayCache_t);

	CreateTree(i);

	return i;
}
void ReleasePriceDayCache(PriceDayCache_t *i)
{
	if (!i)
		return;

	ReleaseTree(i);
	memFree(i);
}

// <-- cdtor

static uint GetKey(uint date, char *pair)
{
	static char *key;

	memFree(key);
	key = xcout("%08u_%s", date, pair);
	return (uint)key;
}

autoList_t *PDC_GetPriceDay(PriceDayCache_t *i, uint date, char *pair) // ret: NULL == not found
{
	if (!rbtHasKey(i->Tree, GetKey(date, pair)))
		return NULL;

	return (autoList_t *)rbtGetLastAccessValue(i->Tree);
}
void PDC_AddPriceDay(PriceDayCache_t *i, uint date, char *pair, autoList_t *list)
{
	rbtAddValue(i->Tree, GetKey(date, pair), (uint)list);
}
void PDC_Clear(PriceDayCache_t *i)
{
	ReleaseTree(i);
	CreateTree(i);
}

// <-- accessor
