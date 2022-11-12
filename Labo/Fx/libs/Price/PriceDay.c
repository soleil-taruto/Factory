#include "all.h"

static PriceDayCache_t *FixedCache;
static PriceDayCache_t *SnapshotCache;

static autoList_t *DummyPriceDay;

autoList_t *GetPriceDay(uint date, char *pair)
{
	static int inited;
	static uint lastSnapshotTime;

	if (!inited) // init
	{
		FixedCache    = CreatePriceDayCache();
		SnapshotCache = CreatePriceDayCache();

		DummyPriceDay = LoadDummyPriceDay();

		ExecuteCollect();
		inited = 1;
	}

	{
		autoList_t *ret = PDC_GetPriceDay(FixedCache, date, pair);

		if (ret)
			return ret;

		if (HasFixedPriceDay(date, pair))
		{
			ret = GetFixedPriceDay(date, pair);
			PDC_AddPriceDay(FixedCache, date, pair, ret);
			return ret;
		}
	}

	if (lastSnapshotTime + 30 < now()) // ? snapshot time-out
	{
		ExecuteSnapshot();
		PDC_Clear(SnapshotCache);

		{
			void SnapshotReloaded(void); // ..\Common.h

			SnapshotReloaded();
		}

		lastSnapshotTime = now();
	}

	{
		autoList_t *ret = PDC_GetPriceDay(SnapshotCache, date, pair);

		if (ret)
			return ret;

		if (HasSnapshotPriceDay(date, pair))
		{
			ret = GetSnapshotPriceDay(date, pair);
			PDC_AddPriceDay(SnapshotCache, date, pair, ret);
			return ret;
		}
	}

	return DummyPriceDay;
}
void ReleasePriceDay(autoList_t *list)
{
	Price_t *i;
	uint index;

	foreach (list, i, index)
		ReleasePrice(i);

	releaseAutoList(list);
}

// ---- Tools ----

/*
	dir: データのディレクトリ
	date: YYYYMMDD 形式
	pair: 通貨ペア
*/
char *GetPriceDayFile(char *dir, uint date, char *pair)
{
	return combine_cx(dir, xcout("%08u_%s.csv", date, pair));
}

// ----
