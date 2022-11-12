#include "all.h"

autoList_t *createAutoList(uint allocCount)
{
	autoList_t *i = (autoList_t *)memAlloc(sizeof(autoList_t));

	errorCase(UINTMAX / sizeof(uint) < allocCount); // ? Overflow

	i->Elements = (uint *)memAlloc(allocCount * sizeof(uint));
	i->Count = 0;
	i->AllocCount = allocCount;
	i->Unresizable = 0;
	i->Reserved_01 = 0;

	return i;
}
autoList_t *copyAutoList(autoList_t *i)
{
	autoList_t *j = (autoList_t *)memAlloc(sizeof(autoList_t));

	j->Elements = (uint *)memClone(i->Elements, i->Count * sizeof(uint));
	j->Count = i->Count;
	j->AllocCount = i->Count;
	j->Unresizable = 0;
	j->Reserved_01 = 0;

	return j;
}
void releaseAutoList(autoList_t *i)
{
	errorCase(!i);
	errorCase(i->Unresizable);

	memFree(i->Elements);
	memFree(i);
}
void releaseList(autoList_t *i)
{
	errorCase(!i);
	errorCase(i->Unresizable);

	memFree(i);
}

autoList_t *bindAutoList(uint *list, uint count)
{
	autoList_t *i = (autoList_t *)memAlloc(sizeof(autoList_t));

	errorCase(!list);
	errorCase(UINTMAX / sizeof(uint) < count); // ? Overflow

	i->Elements = list;
	i->Count = count;
	i->AllocCount = count;
	i->Unresizable = 0;
	i->Reserved_01 = 0;

	return i;
}
uint *unbindAutoList(autoList_t *i)
{
	uint *list = i->Elements;

	memFree(i);
	return list;
}
autoList_t *recreateAutoList(uint *list, uint count)
{
	return bindAutoList((uint *)memClone(list, count * sizeof(uint)), count);
}
autoList_t *createOneElement(uint element)
{
	uint *elements = memAlloc(sizeof(uint));

	elements[0] = element;
	return bindAutoList(elements, 1);
}
autoList_t *newList(void)
{
	return createAutoList(16);
}

// NO-Bulldoze
autoList_t *nobCreateList(uint count)
{
	errorCase(UINTMAX / sizeof(uint) < count); // ? Overflow
	return bindAutoList(memAlloc(count * sizeof(uint)), count);
}
void nobSetCount(autoList_t *i, uint count)
{
	errorCase(!i);
	errorCase(i->Unresizable);
	errorCase(UINTMAX / sizeof(uint) < count); // ? Overflow

	i->Elements = (uint *)memRealloc(i->Elements, count * sizeof(uint));
	i->Count = count;
	i->AllocCount = count;
}

// Bulldoze-NEW
autoList_t *bnewList(uint count)
{
	autoList_t *i = newList();

	setCount(i, count);
	return i;
}
void clearList(autoList_t *i)
{
	memset(i->Elements, 0x00, i->Count * sizeof(uint));
}
void resetCount(autoList_t *i, uint count)
{
	setCount(i, 0);
	setCount(i, count);
}

/*
	使い方
		autoList_t gal = gndAutoList(statically_declared_block, num);
		rapidSort(&gal, simpleComp);
		開放しなくて良い。
*/
autoList_t gndAutoList(uint *list, uint count)
{
	autoList_t i;

	i.Elements = list;
	i.Count = count;
	i.AllocCount = count;
	i.Unresizable = 1;
	i.Reserved_01 = 0;

	return i;
}
autoList_t gndSubElements(autoList_t *i, uint start, uint count)
{
	errorCase(!i);
	errorCase(i->Count < start);
	errorCase(i->Count - start < count);

	return gndAutoList(i->Elements + start, count);
}
autoList_t gndFollowElements(autoList_t *i, uint start)
{
	return gndSubElements(i, start, i->Count - start);
}
autoList_t gndOneElement(uint element, uint *elementBox)
{
	*elementBox = element;
	return gndAutoList(elementBox, 1);
}
autoList_t *gndAutoListVarPtr(uint *list, uint count, autoList_t *varPtr)
{
	*varPtr = gndAutoList(list, count);
	return varPtr;
}
autoList_t *gndSubElementsVarPtr(autoList_t *i, uint start, uint count, autoList_t *varPtr)
{
	*varPtr = gndSubElements(i, start, count);
	return varPtr;
}
autoList_t *gndFollowElementsVarPtr(autoList_t *i, uint start, autoList_t *varPtr)
{
	*varPtr = gndFollowElements(i, start);
	return varPtr;
}
autoList_t *gndOneElementVarPtr(uint element, uint *elementBox, autoList_t *varPtr)
{
	*varPtr = gndOneElement(element, elementBox);
	return varPtr;
}

void setElement(autoList_t *i, uint index, uint element)
{
	errorCase(!i);
	errorCase(i->Count <= index);

	i->Elements[index] = element;
}
uint getElement(autoList_t *i, uint index)
{
	errorCase(!i);
	errorCase(i->Count <= index);

	return i->Elements[index];
}
void putElement(autoList_t *i, uint index, uint element)
{
	errorCase(!i);

	if (i->Count <= index)
	{
		uint newCount;

		errorCase(UINTMAX / sizeof(uint) <= index); // ? Overflow
		newCount = index + 1;

		if (i->AllocCount < newCount)
		{
			uint allocCount = newCount;

			errorCase(i->Unresizable);

			if (allocCount < 16)
			{
				// noop
			}
			else
			{
				allocCount += allocCount / 2; // allocCount *= 1.5
			}
			errorCase(allocCount < newCount); // 2bs
			errorCase(UINTMAX / sizeof(uint) < allocCount); // ? Overflow
			i->Elements = memRealloc(i->Elements, allocCount * sizeof(uint));
			i->AllocCount = allocCount;
		}
		// この時点で i->Count < newCount であることは確実
		do
		{
			i->Elements[i->Count] = 0; // 未定義値
			i->Count++;
		}
		while (i->Count < newCount);
//		memset(i->Elements + i->Count, 0x00, (newCount - i->Count) * sizeof(uint)); // 殆どの場合 addElement によって拡張される。addElement の場合１要素しか拡張しないので、
	}
	i->Elements[index] = element;
}
uint refElement(autoList_t *i, uint index)
{
	errorCase(!i);

	if (i->Count <= index)
		return 0; // 未定義値

	return i->Elements[index];
}
uint getLastElement(autoList_t *i)
{
	errorCase(!i);
	errorCase(!i->Count);

	return i->Elements[i->Count - 1];
}
void swapElement(autoList_t *i, uint index1, uint index2)
{
	uint swap;

	errorCase(!i);
	errorCase(i->Count <= index1);
	errorCase(i->Count <= index2);

	swap = i->Elements[index1];
	i->Elements[index1] = i->Elements[index2];
	i->Elements[index2] = swap;
}

void addElement(autoList_t *i, uint element)
{
	errorCase(!i);
	putElement(i, i->Count, element);
}
uint unaddElement(autoList_t *i)
{
	errorCase(!i);
	errorCase(!i->Count);

	i->Count--;
	return i->Elements[i->Count];
}

void insertElement(autoList_t *i, uint index, uint element)
{
	uint n;

	errorCase(!i);
	errorCase(i->Count < index);

	addElement(i, 0); // Dummy

	for (n = i->Count - 1; index < n; n--)
	{
		i->Elements[n] = i->Elements[n - 1];
	}
	i->Elements[index] = element;
}
uint desertElement(autoList_t *i, uint index)
{
	uint element;
	uint n;

	errorCase(!i);
	errorCase(i->Count <= index);

	element = i->Elements[index];

	i->Count--;

	for (n = index; n < i->Count; n++)
	{
		i->Elements[n] = i->Elements[n + 1];
	}
	return element;
}
uint fastDesertElement(autoList_t *i, uint index)
{
	uint element;

	errorCase(!i);
	errorCase(i->Count <= index);

	element = i->Elements[index];

	i->Count--;
	i->Elements[index] = i->Elements[i->Count];

	return element;
}
void removeElement(autoList_t *i, uint target)
{
	uint count;
	uint index;
	uint element;

	foreach (i, element, index)
	{
		if (element == target)
		{
			i->Count--;
			i->Elements[index] = i->Elements[i->Count];
			break;
		}
	}
}

/*
	バッファの長さを要素数に合わせる。
*/
void fixElements(autoList_t *i)
{
	errorCase(!i);
	errorCase(i->Unresizable);

	setAllocCount(i, 0);
}
/*
	バッファの長さを少なくとも allocCount 個にする。
*/
void setAllocCount(autoList_t *i, uint allocCount)
{
	errorCase(!i);
	errorCase(i->Unresizable);
	errorCase(UINTMAX / sizeof(uint) < allocCount); // ? Overflow

	allocCount = m_max(allocCount, i->Count);

	i->Elements = (uint *)memRealloc(i->Elements, allocCount * sizeof(uint));
	i->AllocCount = allocCount;
}
void setCount(autoList_t *i, uint count)
{
	errorCase(!i);
	errorCase(UINTMAX / sizeof(uint) < count); // ? Overflow

	if (i->Count < count)
	{
		if (i->AllocCount < count)
		{
			errorCase(i->Unresizable);

			i->Elements = (uint *)memRealloc(i->Elements, count * sizeof(uint));
			i->AllocCount = count;
		}
		memset(i->Elements + i->Count, 0x00, (count - i->Count) * sizeof(uint));
	}
	i->Count = count;
}
void setCountFloor(autoList_t *i, uint count)
{
	if (getCount(i) < count)
	{
		setCount(i, count);
	}
}
void setCountRoof(autoList_t *i, uint count)
{
	if (count < getCount(i))
	{
		setCount(i, count);
	}
}
uint getCount(autoList_t *i)
{
	errorCase(!i);
	return i->Count;
}

uint *directGetList(autoList_t *i)
{
	return i->Elements;
}
uint *directGetPoint(autoList_t *i, uint index)
{
	errorCase(!i);
	errorCase(i->Count <= index);

	return i->Elements + index;
}
uint *directRefPoint(autoList_t *i, uint index)
{
	setCountFloor(i, index + 1);
	return directGetPoint(i, index);
}
void reverseElements(autoList_t *i)
{
	uint n, f;
	uint swap;

	errorCase(!i);

	if (i->Count)
	{
		for (n = 0, f = i->Count - 1; n < f; n++, f--)
		{
			swap = i->Elements[n];
			i->Elements[n] = i->Elements[f];
			i->Elements[f] = swap;
		}
	}
}
void removeZero(autoList_t *i)
{
	uint n, f;

	errorCase(!i);

	for (n = 0; ; n++)
	{
		if (i->Count <= n)
		{
			return;
		}
		if (i->Elements[n] == 0)
		{
			break;
		}
	}
	for (f = n + 1; f < i->Count; f++)
	{
		if (i->Elements[f])
		{
			i->Elements[n] = i->Elements[f];
			n++;
		}
	}
	i->Count = n;
}
void removeTrailZero(autoList_t *i)
{
	uint index;

	for (index = getCount(i); index; index--)
	{
		if (1 <= getElement(i, index - 1))
		{
			break;
		}
	}
	setCount(i, index);
}
void releaseDim_BR(void *block, uint depth, void (*blockReleaser)(void *))
{
	if (depth)
	{
		autoList_t *i = (autoList_t *)block;
		uint index;

		foreach (i, block, index)
		{
			releaseDim_BR(block, depth - 1, blockReleaser);
		}
		releaseAutoList(i);
	}
	else if (blockReleaser)
	{
		blockReleaser(block);
	}
}
void releaseDim(void *block, uint depth)
{
	releaseDim_BR(block, depth, memFree);
}
void addElements(autoList_t *i, autoList_t *j)
{
	uint element;
	uint index;

	foreach (j, element, index)
	{
		addElement(i, element);
	}
}
uint findElement(autoList_t *i, uint target, sint (*funcComp)(uint, uint))
{
	uint element;
	uint index;

	foreach (i, element, index)
	{
		if (!funcComp(element, target))
		{
			break;
		}
	}
	return index;
}
uint getCountElement(autoList_t *i, uint target, sint (*funcComp)(uint, uint))
{
	uint element;
	uint index;
	uint count = 0;

	foreach (i, element, index)
	{
		if (!funcComp(element, target))
		{
			count++;
		}
	}
	return count;
}

uint foundPairIndexes[2];

uint findPair(autoList_t *list, sint (*funcComp)(uint, uint))
{
	uint element;
	uint index;
	uint faridx;

	foundPairIndexes[0] = 0;
	foundPairIndexes[1] = 0;

	foreach (list, element, index)
	{
		for (faridx = index + 1; faridx < getCount(list); faridx++)
		{
			if (funcComp(element, getElement(list, faridx)) == 0)
			{
				goto found;
			}
		}
	}
	return 0;

found:
	foundPairIndexes[0] = index;
	foundPairIndexes[1] = faridx;

	return faridx; // 0 <
}

autoList_t *getList(autoList_t *table, uint index)
{
	autoList_t *list = (autoList_t *)getElement(table, index);

	errorCase(!list);
	return list;
}
autoList_t *refList(autoList_t *table, uint index)
{
	while (getCount(table) <= index)
	{
		addElement(table, (uint)createAutoList(0));
	}
	return getList(table, index);
}
autoList_t *makeTable(uint rowcnt, uint colcnt, uint initValue)
{
	autoList_t *table = newList();
	uint rowidx;
	uint colidx;

	for (rowidx = 0; rowidx < rowcnt; rowidx++)
	{
		autoList_t *row = newList();

		for (colidx = 0; colidx < colcnt; colidx++)
		{
			addElement(row, initValue);
		}
		addElement(table, (uint)row);
	}
	return table;
}
uint *getTablePoint(autoList_t *table, uint rowidx, uint colidx)
{
	return directGetPoint(getList(table, rowidx), colidx);
}

void callAllElement(autoList_t *i, void (*callFunc)(uint))
{
	uint element;
	uint index;

	foreach (i, element, index)
	{
		callFunc(element);
	}
}
uint zSetElement(autoList_t *i, uint index, uint element)
{
	uint oldElement = getElement(i, index);

	setElement(i, index, element);
	return oldElement;
}

// _x
void addElements_x(autoList_t *i, autoList_t *j)
{
	addElements(i, j);
	releaseAutoList(j);
}
void callAllElement_x(autoList_t *i, void (*callFunc)(uint))
{
	callAllElement(i, callFunc);
	releaseAutoList(i);
}
