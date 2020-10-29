#include "all.h"

double MvAvg_GetMid(uint fxTime, uint span, char *pair)
{
	return MA_GetMid(MAC_GetMvAvg(fxTime, span, pair), fxTime);
}
