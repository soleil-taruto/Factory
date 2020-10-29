typedef struct PriceDayCache_st // static member
{
	rbTree_t *Tree;
}
PriceDayCache_t;

PriceDayCache_t *CreatePriceDayCache(void);
void ReleasePriceDayCache(PriceDayCache_t *i);

// <-- cdtor

autoList_t *PDC_GetPriceDay(PriceDayCache_t *i, uint date, char *pair);
void PDC_AddPriceDay(PriceDayCache_t *i, uint date, char *pair, autoList_t *list);
void PDC_Clear(PriceDayCache_t *i);

// <-- accessor
