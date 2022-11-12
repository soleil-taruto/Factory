#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"

// GetBound >

static autoList_t *SubList;
static sint SLStart;
static sint SLEnd;
static sint SLStep;

static uint SLGetCount(void)
{
	return (SLEnd - SLStart) / SLStep + 1;
}
static uint SLGetRealIndex(uint index)
{
	return SLStart + (sint)index * SLStep;
}
static uint SLGetLowestIndex(uint value)
{
	uint p = 0;
	uint q = SLGetCount() - 1;

	while (p < q)
	{
		uint c = (p + q) / 2;
		sint r = m_simpleComp(getElement(SubList, SLGetRealIndex(c)), value);

		if (r) // ? [c] != value
		{
			p = c + 1;
		}
		else // ? [c] == value
		{
			q = c;
		}
	}
	return SLGetRealIndex(p);
}
static uint GetIndex(autoList_t *list, uint value)
{
	uint p = 0;
	uint q = getCount(list);

	while (p < q)
	{
		uint c = (p + q) / 2;
		sint r = m_simpleComp(getElement(list, c), value);

		if (r < 0) // ? [c] < value
		{
			p = c + 1;
		}
		else if (0 < r) // ? value < [c]
		{
			q = c;
		}
		else // ? [c] == value
		{
			return c;
		}
	}
	return UINTMAX; // not found
}
static void GetBound(autoList_t *list, uint value, uint *p_ndx1, uint *p_ndx2)
{
	uint index = GetIndex(list, value);

	if (index == UINTMAX)
	{
		*p_ndx1 = UINTMAX;
		*p_ndx2 = UINTMAX;
		return;
	}
	errorCase(getElement(list, index) != value); // 2bs

	SubList = list;
	SLStart = 0;
	SLEnd = index;
	SLStep = 1;
	*p_ndx1 = SLGetLowestIndex(value);

	SubList = list;
	SLStart = getCount(list) - 1;
	SLEnd = index;
	SLStep = -1;
	*p_ndx2 = SLGetLowestIndex(value);
}

// < GetBound

static void AddValueNum(autoList_t *list, uint value, uint num)
{
	while (num)
	{
		addElement(list, value);
		num--;
	}
}
static void DoTest(uint maxnum)
{
	autoList_t *list = newList();
	uint num1 = mt19937_range(0, maxnum);
	uint num2 = mt19937_range(0, maxnum);
	uint num3 = mt19937_range(0, maxnum);
	uint exp1;
	uint exp2;
	uint ret1;
	uint ret2;

	cout("%u %u %u\n", num1, num2, num3);

	AddValueNum(list, 1, num1);
	AddValueNum(list, 2, num2);
	AddValueNum(list, 3, num3);

	if (num2)
	{
		exp1 = num1;
		exp2 = num1 + num2 - 1;
	}
	else
	{
		exp1 = UINTMAX;
		exp2 = UINTMAX;
	}

	GetBound(list, 2, &ret1, &ret2);

	errorCase(exp1 != ret1);
	errorCase(exp2 != ret2);

	releaseAutoList(list);
	cout("ok!\n");
}
static void DoTest2(uint num, uint valmax)
{
	autoList_t *list = newList();
	uint count;
	uint targetVal;
	uint exp1;
	uint exp2;
	uint ret1;
	uint ret2;

	cout("%u %u\n", num, valmax);

	for (count = num; count; count--)
		addElement(list, mt19937_range(0, valmax));

	rapidSort(list, simpleComp);
	targetVal = mt19937_range(0, valmax);

	{
		uint value;
		uint index;

		foreach (list, value, index)
			if (value == targetVal)
				break;

		if (index < getCount(list))
		{
			exp1 = index;

			while (++index < getCount(list))
				if (getElement(list, index) != targetVal)
					break;

			exp2 = index - 1;
		}
		else
		{
			exp1 = UINTMAX;
			exp2 = UINTMAX;
		}
	}

	GetBound(list, targetVal, &ret1, &ret2);

	errorCase(exp1 != ret1);
	errorCase(exp2 != ret2);

	releaseAutoList(list);
	cout("ok!\n");
}
int main(int argc, char **argv)
{
	mt19937_init();

	while (!checkKey(0x1b))
	{
		DoTest(10);
		DoTest(100);
		DoTest(1000);

		DoTest2(10, 1000);
		DoTest2(100, 100);
		DoTest2(1000, 10);

		DoTest2(mt19937_rnd(10), mt19937_rnd(10));
		DoTest2(mt19937_rnd(100), mt19937_rnd(100));
		DoTest2(mt19937_rnd(1000), mt19937_rnd(1000));
	}
}
