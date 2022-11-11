#include "all.h"

#define FIXED_DIR "C:\\temp\\Fx"

// ---- Collect -----

void ExecuteCollect(void)
{
	coExecute("C:\\Dev\\Fx\\YahooFinanceMonitor\\Tools\\collcsv.exe");
}

// ----

autoList_t *GetFixedPriceDay(uint date, char *pair)
{
	char *file = GetPriceDayFile(FIXED_DIR, date, pair);
	autoList_t *list;

	list = LoadPriceDay(file);
	memFree(file);
	return list;
}
int HasFixedPriceDay(uint date, char *pair)
{
	char *file = GetPriceDayFile(FIXED_DIR, date, pair);
	int ret;

	ret = existFile(file);
	memFree(file);
	return ret;
}
