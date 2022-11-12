#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\Common\Options\Sequence.h"

static void ST_DispList(autoList_t *list)
{
	uint index;

	for (index = 0; index < getCount(list); index++)
	{
		cout("[%u]: %u\n", index, getElement(list, index));
	}
}
static autoList_t *ST_MakeList(uint start, uint count, uint step)
{
	autoList_t *list = newList();
	uint index;

	for (index = 0; index < count; index++)
		addElement(list, start + index * step);

	shuffle(list);
	return list;
}
static void ST_DoTest(void (*funcSort)(autoList_t *, sint (*)(uint, uint)), uint start, uint count, uint step)
{
	autoList_t *list = ST_MakeList(start, count, step);
	uint index;

	errorCase(getCount(list) != count); // 2bs

	cout("sort start...\n");
//ST_DispList(list);
	funcSort(list, simpleComp);
//ST_DispList(list);
	cout("sort end\n");

	errorCase(getCount(list) != count);

	for (index = 0; index < count; index++)
	{
		errorCase(getElement(list, index) != start + index * step);
	}
}
static void SortTest_MMMC(void (*funcSort)(autoList_t *, sint (*)(uint, uint)), uint startMax, uint countMax, uint stepMax, uint testCount)
{
	cout("startMax: %u\n", startMax);
	cout("countMax: %u\n", countMax);
	cout("stepMax: %u\n", stepMax);

	while (testCount)
	{
		uint start = mt19937_range(0, startMax);
		uint count = mt19937_range(0, countMax);
		uint step  = mt19937_range(0, stepMax);

		cout("testCount: %u\n", testCount);
		cout("start: %u\n", start);
		cout("count: %u\n", count);
		cout("step: %u\n", step);

		ST_DoTest(funcSort, start, count, step);

		testCount--;
	}
	cout("OK\n");
}
static autoList_t *ST2_GetSource(uint grp, uint grpSz)
{
	autoList_t *src = newList();
	uint no;
	uint c;

	for (no = 1; no <= grp; no++)
	for (c = mt19937_range(0, grpSz); c; c--)
		addElement(src, no);

	return src;
}
static void ST2_DoTest(void (*funcSort)(autoList_t *, sint (*)(uint, uint)), uint grp, uint grpSz)
{
	autoList_t *src = ST2_GetSource(grp, grpSz);
	autoList_t *list;
	uint index;
	uint count;

	count = getCount(src);
	list = copyAutoList(src);
	shuffle(list);

	cout("sort start...\n");
//ST_DispList(list);
	funcSort(list, simpleComp);
//ST_DispList(list);
	cout("sort end\n");

	errorCase(getCount(list) != count);

	for (index = 0; index < count; index++)
	{
		errorCase(getElement(src, index) != getElement(list, index));
	}
}
static void SortTest_GGC(void (*funcSort)(autoList_t *, sint (*)(uint, uint)), uint grpMax, uint grpSzMax, uint testCount)
{
	cout("grpMax: %u\n", grpMax);
	cout("grpSzMax: %u\n", grpSzMax);

	while (testCount)
	{
		uint grp   = mt19937_range(0, grpMax);
		uint grpSz = mt19937_range(0, grpSzMax);

		cout("testCount: %u\n", testCount);
		cout("grp: %u\n", grp);
		cout("grpSz: %u\n", grpSz);

		ST2_DoTest(funcSort, grp, grpSz);

		testCount--;
	}
	cout("OK\n");
}
static void SortTest(void (*funcSort)(autoList_t *, sint (*)(uint, uint)))
{
	SortTest_MMMC(funcSort, 4000000000, 100, 100, 10000);
	SortTest_MMMC(funcSort, 2000000000, 20000, 100000, 100);
//	SortTest_MMMC(funcSort, 2000000000, 200000, 10000, 100);

	SortTest_GGC(funcSort, 3000,  3, 300);
	SortTest_GGC(funcSort, 1000, 10, 300);
	SortTest_GGC(funcSort, 500,  20, 300);
	SortTest_GGC(funcSort, 300,  30, 300);
	SortTest_GGC(funcSort, 100, 100, 300);
	SortTest_GGC(funcSort, 30,  300, 300);
	SortTest_GGC(funcSort, 20,  500, 300);
	SortTest_GGC(funcSort, 10, 1000, 300);
	SortTest_GGC(funcSort, 3,  3000, 300);
}
static void Test_gnomeSort(void)
{
	SortTest(gnomeSort);
}
static void Test_combSort(void)
{
	SortTest(combSort);
}
static void Test_selectionSort(void)
{
	SortTest(selectionSort);
}
static void Test_rapidSort(void)
{
	SortTest(rapidSort);
}

// ---- findBound tests ----

static autoList_t *FB_MakeList(uint count, uint minval, uint maxval)
{
	autoList_t *list = newList();

	while (count)
	{
		addElement(list, mt19937_range(minval, maxval));
		count--;
	}
	rapidSort(list, simpleComp);
	return list;
}
static void FB_Test(autoList_t *list, uint target, uint bound[2])
{
	uint element;
	uint index;

#if 0
	foreach (list, element, index)
		if (target <= element)
			break;

	bound[0] = index;
#else
	bound[0] = 0;

	foreach (list, element, index)
		if (element < target)
			bound[0] = index + 1;
#endif

	foreach (list, element, index)
		if (target < element)
			break;

	bound[1] = index;
}
static void FB_DoTest(uint count, uint minval, uint maxval) // 1 <= minval <= maxval < UINTMAX
{
	autoList_t *list = FB_MakeList(count, minval, maxval);
	uint target;
	uint bound1[2];
	uint bound2[2];

	cout("FB_DoTest: %u %u %u\n", count, minval, maxval);

	for (target = minval - 1; target <= maxval + 1; target++)
	{
		cout("target: %u\n", target);

		FB_Test(list, target, bound1);
		findBound(list, target, simpleComp, bound2);

		cout("bounds: %u %u %u %u\n", bound1[0], bound1[1], bound2[0], bound2[1]);

		errorCase(bound1[0] != bound2[0]);
		errorCase(bound1[1] != bound2[1]);
	}
	releaseAutoList(list);
}
static void Test_findBound(void)
{
	uint testcnt;

	for (testcnt = 0; testcnt < 1000; testcnt++)
	{
		{
			uint c = mt19937_rnd(100);
			uint n = mt19937_rnd(100);
			uint x = mt19937_rnd(100);

			FB_DoTest(c, n, n + x);
		}

		{
			uint c = mt19937_rnd(1000);
			uint n = mt19937_rnd(1000);
			uint x = mt19937_rnd(1000);

			FB_DoTest(c, n, n + x);
		}
	}
	cout("OK\n");
}

// ---- getBound tests ----

static autoList_t *GB_MakeList(uint count, uint minval, uint maxval)
{
	autoList_t *list = newList();

	while (count)
	{
		addElement(list, mt19937_range(minval, maxval));
		count--;
	}
	rapidSort(list, simpleComp);
	return list;
}
static int GB_Test(autoList_t *list, uint target, uint bound[2])
{
	uint index;

	for (index = 0; index < getCount(list); index++)
		if (getElement(list, index) == target)
			break;

	if (index == getCount(list))
		return 0;

	bound[0] = index;

	while (++index < getCount(list))
		if (getElement(list, index) != target)
			break;

	bound[1] = index - 1;
	return 1;
}
static void GB_DoTest(uint count, uint minval, uint maxval) // 1 <= minval <= maxval < UINTMAX
{
	autoList_t *list = GB_MakeList(count, minval, maxval);
	uint target;
	uint bound1[2];
	uint bound2[2];
	int ret1;
	int ret2;

	cout("GB_DoTest: %u %u %u\n", count, minval, maxval);

	for (target = minval - 1; target <= maxval + 1; target++)
	{
		uint bnd2Dmy1 = mt19937_rnd(256);
		uint bnd2Dmy2 = mt19937_rnd(256);

		cout("target: %u, bnd2Dmys: %u %u\n", target, bnd2Dmy1, bnd2Dmy2);

		bound2[0] = bnd2Dmy1;
		bound2[1] = bnd2Dmy2;

		ret1 = GB_Test(list, target, bound1);
		ret2 = getBound(list, target, simpleComp, bound2);

		ret1 = m_01(ret1);
		ret2 = m_01(ret2);

		errorCase(ret1 != ret2);

		if (ret1)
		{
			cout("bounds: %u %u %u %u\n", bound1[0], bound1[1], bound2[0], bound2[1]);

			errorCase(bound1[0] != bound2[0]);
			errorCase(bound1[1] != bound2[1]);
		}
		else
		{
			cout("bound2: %u %u\n", bound2[0], bound2[1]);

			errorCase(bnd2Dmy1 != bound2[0]);
			errorCase(bnd2Dmy2 != bound2[1]);
		}
	}
	releaseAutoList(list);
}
static void Test_getBound(void)
{
	uint testcnt;

	for (testcnt = 0; testcnt < 1000; testcnt++)
	{
		{
			uint c = mt19937_rnd(100);
			uint n = mt19937_rnd(100);
			uint x = mt19937_rnd(100);

			GB_DoTest(c, n, n + x);
		}

		{
			uint c = mt19937_rnd(1000);
			uint n = mt19937_rnd(1000);
			uint x = mt19937_rnd(1000);

			GB_DoTest(c, n, n + x);
		}
	}
	cout("OK\n");
}

// ----

int main(int argc, char **argv)
{
	mt19937_initRnd((uint)time(NULL));

	Test_gnomeSort();
	Test_combSort();
	Test_selectionSort();
	Test_rapidSort();
	Test_findBound();
	Test_getBound();
}
