/*
	                       order                                        用途
	------------------------------------------------------------------------
	rapidSortLines                        strcmp (-> simpleComp 廃止)
	rapidSortJLinesICase   mbs_stricmp -> strcmp (-> simpleComp 廃止)
	sortJLinesICase        mbs_stricmp                                  ls系の戻り値
*/

#include "all.h"

void gnomeSort(autoList_t *list, sint (*funcComp)(uint, uint)) // 安定ソート
{
	uint nearidx;
	uint faridx;

	for (faridx = 1; faridx < getCount(list); faridx++)
	{
		for (nearidx = faridx; nearidx && 0 < funcComp(getElement(list, nearidx - 1), getElement(list, nearidx)); nearidx--)
		{
			swapElement(list, nearidx - 1, nearidx);
		}
	}
}
void combSort(autoList_t *list, sint (*funcComp)(uint, uint))
{
	uint span = getCount(list);
	uint nearidx;
	uint faridx;
//cout("* %u\n", getCount(list)); // test test test

	for (; ; )
	{
		span = (uint)(((uint64)span * 10) / 13); //(uint)(span / 1.3);

		if (span < 2)
			break;

		if (span == 9 || span == 10)
			span = 11;

		for (nearidx = 0, faridx = span; faridx < getCount(list); nearidx++, faridx++)
		{
			if (0 < funcComp(getElement(list, nearidx), getElement(list, faridx)))
			{
				swapElement(list, nearidx, faridx);
			}
		}
	}
	gnomeSort(list, funcComp);
}
void selectionSort(autoList_t *list, sint (*funcComp)(uint, uint)) // 安定ソート
{
	uint nearidx;
	uint faridx;
	uint minidx;

	for (nearidx = 0; nearidx + 1 < getCount(list); nearidx++)
	{
		minidx = nearidx;

		for (faridx = nearidx + 1; faridx < getCount(list); faridx++)
		{
			if (0 < funcComp(getElement(list, minidx), getElement(list, faridx)))
			{
				minidx = faridx;
			}
		}
		if (minidx != nearidx)
		{
			swapElement(list, nearidx, minidx);
		}
	}
}

#define CUTOVER 9

/*
	2分割時の大きい方の大きさの期待値は 0.75 (たぶん..
	0.75 ^ 80 = 0.0000000001*
	0.75 ^ 40 = 0.00001*
	0.75 ^ 30 = 0.00017858209*
*/
#define ABANDON 30

/*
	クイックソートもどき
*/
void rapidSort(autoList_t *list, sint (*funcComp)(uint, uint))
{
	autoList_t *rangeStack = createAutoList((ABANDON + 3) * 3);
	uint depth;
	uint startidx;
	uint endnextidx;
	uint nearidx;
	uint pivotidx;
	uint pivot;
	uint faridx;

	/*
	if (getCount(list) < 2) // ? ソート不要
	{
		goto endsort;
	}
	*/

	addElement(rangeStack, 0);
	addElement(rangeStack, 0);
	addElement(rangeStack, getCount(list));

	while (getCount(rangeStack))
	{
		endnextidx = unaddElement(rangeStack);
		startidx = unaddElement(rangeStack);
		depth = unaddElement(rangeStack);

		/*
		if (endnextidx < startidx + 2) // ? ソート不要
		{
			continue;
		}
		*/
		if (endnextidx - startidx < CUTOVER)
		{
			autoList_t sublist = gndSubElements(list, startidx, endnextidx - startidx);

			selectionSort(&sublist, funcComp);
			continue;
		}
		if (ABANDON < depth)
		{
#if 1
			autoList_t sublist = gndSubElements(list, startidx, endnextidx - startidx);

			combSort(&sublist, funcComp);
			continue;
#else
			combSort(list, funcComp);
			goto endsort;
#endif
		}

		nearidx = startidx;
		pivotidx = (startidx + endnextidx) / 2;
		faridx = endnextidx - 1;

		for (; ; )
		{
			pivot = getElement(list, pivotidx);

#if 1
			while (nearidx < pivotidx && funcComp(getElement(list, nearidx), pivot) <= 0)
			{
				nearidx++;
			}
			while (pivotidx < faridx && funcComp(pivot, getElement(list, faridx)) <= 0)
			{
				faridx--;
			}
#elif 1 // 同値が多いとき多少速い。<--- pivotの最終位置が中央に寄りやすいみたい。
			while (nearidx < pivotidx && funcComp(getElement(list, nearidx), pivot) < 0)
			{
				nearidx++;
			}
			while (pivotidx < faridx && funcComp(pivot, getElement(list, faridx)) < 0)
			{
				faridx--;
			}
#else // 同じ要素を比較することがある。この動作は想定されない場合がある気がする。
			while (funcComp(getElement(list, nearidx), pivot) < 0)
			{
				nearidx++;
			}
			while (funcComp(pivot, getElement(list, faridx)) < 0)
			{
				faridx--;
			}
#endif

			if (nearidx == faridx)
			{
				break;
			}

			swapElement(list, nearidx, faridx);

			if (nearidx == pivotidx)
			{
				pivotidx = faridx;
				nearidx++;
			}
			else if (faridx == pivotidx)
			{
				pivotidx = nearidx;
				faridx--;
			}
			else
			{
				nearidx++;
				faridx--;
			}
		}

		addElement(rangeStack, depth + 1);
		addElement(rangeStack, startidx);
		addElement(rangeStack, pivotidx);

		addElement(rangeStack, depth + 1);
		addElement(rangeStack, pivotidx + 1);
		addElement(rangeStack, endnextidx);
errorCase(rangeStack->AllocCount != (ABANDON + 3) * 3); // test test test
	}

endsort:
	releaseAutoList(rangeStack);
}
void rapidSortLines(autoList_t *lines)
{
	rapidSort(lines, (sint (*)(uint, uint))strcmp2);
}
void rapidSortJLinesICase(autoList_t *lines)
{
	rapidSort(lines, (sint (*)(uint, uint))strcmp3);
}

static sint (*RSWSC_FuncComp)(uint, uint);

static sint RSWSC_Comp(uint a, uint b)
{
	return RSWSC_FuncComp(*(uint *)a, *(uint *)b);
}

/*
	subColumn:  null ok, lines と同じ長さのリストであること。
	subColumns: null ok, 各要素は lines と同じ長さのリストであること。各要素は null 不可
*/
void rapidSortSubColumns(autoList_t *list, sint (*funcComp)(uint, uint), autoList_t *subColumn, autoList_t *subColumns)
{
	autoList_t *wList = newList();
	uint element;
	uint index;
	uint *we;
	uint wei;
	uint weCount = 1 + (subColumn ? 1 : 0) + (subColumns ? getCount(subColumns) : 0);
	autoList_t *sc;
	uint sc_index;

	foreach (list, element, index)
	{
		we = (uint *)memAlloc(weCount * sizeof(uint));
		wei = 0;

		we[wei++] = element;

		if (subColumn)
			we[wei++] = getElement(subColumn, index);

		if (subColumns)
			foreach (subColumns, sc, sc_index)
				we[wei++] = getElement(sc, index);

		addElement(wList, (uint)we);
	}
	RSWSC_FuncComp = funcComp;
	rapidSort(wList, RSWSC_Comp);
	RSWSC_FuncComp = NULL;

	foreach (wList, we, index)
	{
		wei = 0;

		setElement(list, index, we[wei++]);

		if (subColumn)
			setElement(subColumn, index, we[wei++]);

		if (subColumns)
			foreach (subColumns, sc, sc_index)
				setElement(sc, index, we[wei++]);

		memFree(we);
	}
	releaseAutoList(wList);
}
void rapidSortLinesSubColumns(autoList_t *lines, autoList_t *subColumn, autoList_t *subColumns)
{
	rapidSortSubColumns(lines, (sint (*)(uint, uint))strcmp2, subColumn, subColumns);
}
void rapidSortLinesICaseSubColumns(autoList_t *lines, autoList_t *subColumn, autoList_t *subColumns)
{
	rapidSortSubColumns(lines, (sint (*)(uint, uint))strcmp3, subColumn, subColumns);
}

sint strcmp3(char *line1, char *line2) // mbs_
{
	sint retval = mbs_stricmp(line1, line2);

	if (retval)
		return retval;

	return strcmp2(line1, line2);
}
sint strcmp2(char *line1, char *line2)
{
#if 1 // @ 2016.3.1
	return strcmp(line1, line2);
#else
	sint retval = strcmp(line1, line2);

	if (retval)
		return retval;

	return m_simpleComp(line1, line2);
#endif
}
sint simpleComp(uint v1, uint v2)
{
	return m_simpleComp(v1, v2);
/*
	if (v1 < v2)
		return -1;

	if (v2 < v1)
		return 1;

	return 0;
*/
}
sint simpleComp64(uint64 v1, uint64 v2)
{
	if (v1 < v2)
		return -1;

	if (v2 < v1)
		return 1;

	return 0;
}
sint pSimpleComp(uint v1, uint v2)
{
	return m_simpleComp(*(uint *)v1, *(uint *)v2);
}
sint pSimpleComp2(uint v1, uint v2)
{
	sint retval = pSimpleComp(v1, v2);

	if (retval)
		return retval;

	return m_simpleComp(v1, v2);
}

// list はソート済みであること。
uint binSearch(autoList_t *list, uint target, sint (*funcComp)(uint, uint))
{
	uint p = 0;
	uint q = getCount(list);

	while (p < q)
	{
		uint mid = (p + q) / 2;
		sint comp;

		comp = funcComp(target, getElement(list, mid));

		if (0 < comp)
		{
			p = mid + 1;
		}
		else if (comp < 0)
		{
			q = mid;
		}
		else
		{
			return mid;
		}
	}
	return getCount(list); // not found
}
uint binSearchLines(autoList_t *lines, char *lineFind)
{
	return binSearch(lines, (uint)lineFind, (sint (*)(uint, uint))strcmp);
}

// list はソート済みであること。
uint findBoundNearestMatch(autoList_t *list, uint target, sint (*funcComp)(uint, uint)) // ? ret: targetより小さい最後の位置の次, targetより小さい要素が無ければ 0
{
	uint l = 0;
	uint r = getCount(list);

	while (l < r)
	{
		int m = (l + r) / 2;
		sint comp;

		comp = funcComp(target, getElement(list, m));

		if (0 < comp) // list[m] < target
		{
			l = m + 1;
		}
		else // target <= list[m]
		{
			r = m;
		}
	}
	return l;
}
// list はソート済みであること。
uint findBoundNearestRight(autoList_t *list, uint target, sint (*funcComp)(uint, uint)) // ? ret: targetより大きい最初の位置, targetより大きい要素が無ければlistの長さ
{
	uint l = 0;
	uint r = getCount(list);

	while (l < r)
	{
		int m = (l + r) / 2;
		sint comp;

		comp = funcComp(target, getElement(list, m));

		if (0 <= comp) // list[m] <= target
		{
			l = m + 1;
		}
		else // target < list[m]
		{
			r = m;
		}
	}
	return l;
}
// list はソート済みであること。
void findBound(autoList_t *list, uint target, sint (*funcComp)(uint, uint), uint bound[2])
{
	bound[0] = findBoundNearestMatch(list, target, funcComp);
	bound[1] = findBoundNearestRight(list, target, funcComp);
}

// list はソート済みであること。
int getBound(autoList_t *list, uint target, sint (*funcComp)(uint, uint), uint bound[2]) // ret: ? 見つかった。-- 見つからないとき bound を変更しない。
{
	uint wkBound[2];

	findBound(list, target, funcComp, wkBound);

	if (wkBound[0] < wkBound[1])
	{
		bound[0] = wkBound[0];
		bound[1] = wkBound[1] - 1;
		return 1;
	}
	return 0;
}
int getBoundLines(autoList_t *lines, char *lineFind, uint bound[2])
{
	return getBound(lines, (uint)lineFind, (sint (*)(uint, uint))strcmp, bound);
}
