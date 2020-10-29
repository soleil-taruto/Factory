#include "all.h"

Price_t *GetPrice_TSec(uint64 tSec, char *pair)
{
	TimeData_t td = TSec2TimeData(tSec);
	uint date;
	uint index;

	date = td.Y * 10000 + td.M * 100 + td.D;
	index = td.H * 3600 + td.I * 60 + td.S;
	index /= 2;

	return (Price_t *)getElement(GetPriceDay(date, pair), index);
}
Price_t *GetPrice(uint fxTime, char *pair)
{
	return GetPrice_TSec(FxTime2TSec(fxTime), pair);
}
