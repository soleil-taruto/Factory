#include "Sequence.h"

void makeSq(autoList_t *list, uint start, sint step)
{
	uint index;

	for (index = 0; index < getCount(list); index++)
	{
		setElement(list, index, start);
		start += step;
	}
}
autoList_t *createSq(uint count, uint start, sint step)
{
	autoList_t *list = nobCreateList(count);

	makeSq(list, start, step);
	return list;
}
void shuffle(autoList_t *list)
{
	uint index;

	for (index = 0; index + 1 < getCount(list); index++)
	{
		swapElement(list, index, mt19937_range(index, getCount(list) - 1));
	}
}
void sortSq(autoList_t *list)
{
	rapidSort(list, simpleComp);
}
uint chooseOneSq(autoList_t *list, int (*isTarget)(uint))
{
#if 1
	uint count = 0;
	uint value;
	uint index;

	foreach (list, value, index)
		if (isTarget(value))
			count++;

	count = mt19937_rnd(count);

	foreach (list, value, index)
	{
		if (isTarget(value))
		{
			if (!count)
				goto foundChoseTarget;

			count--;
		}
	}
	error();
foundChoseTarget:
	return index;
#else
	autoList_t *indexes = newList();
	uint value;
	uint index;

	foreach (list, value, index)
		if (isTarget(value))
			addElement(indexes, index);

	index = getElement(indexes, mt19937_rnd(getCount(indexes)));
	releaseAutoList(indexes);
	return index;
#endif
}
