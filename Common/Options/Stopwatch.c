/*
	GetTickCount() を使用する。PC 起動から 49.7 日後のカンストは考慮しない。
*/

#include "Stopwatch.h"

void SW_Clear(autoList_t *lapTimes)
{
	errorCase(!lapTimes);

	setCount(lapTimes, 0);
}
void SW_Lap(autoList_t *lapTimes)
{
	errorCase(!lapTimes);

	addElement(lapTimes, GetTickCount());
}
uint SW_GetTotal(autoList_t *lapTimes)
{
	errorCase(!lapTimes);
	errorCase(!getCount(lapTimes));

	return getLastElement(lapTimes) - getElement(lapTimes, 0);
}
void SW_ShowLaps(autoList_t *lapTimes)
{
	uint index;

	errorCase(!lapTimes);
	errorCase(!getCount(lapTimes));

	for (index = 0; index + 1 < getCount(lapTimes); index++)
	{
		cout("Lap %u to %u = %u\n", index, index + 1, getElement(lapTimes, index + 1) - getElement(lapTimes, index));
	}
	cout("Lap total = %u\n", getLastElement(lapTimes) - getElement(lapTimes, 0));
}
