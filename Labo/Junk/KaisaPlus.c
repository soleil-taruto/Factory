#include "C:\Factory\Common\all.h"

static void Kazoeru(autoList_t *kaisa)
{
	autoList_t *kosuu = newList();
	uint c;

	for (c = 0; c < getCount(kaisa); c++)
	{
		directRefPoint(kosuu, getElement(kaisa, c))[0]++;
	}
	setCount(kaisa, 0);
	addElements(kaisa, kosuu);
	releaseAutoList(kosuu);
}
static autoList_t *GetKaisaXor(uint num)
{
	autoList_t *kaisa = newList();
	uint c;

	errorCase(!num);

	for (c = 0; c < num; c++)
	{
		addElement(kaisa, (uint)createOneElement(c + 1));
	}

#if 0
	// XRx
	{
		for (c = 1; c < num; c++)
		{
			addElements(getList(kaisa, c), getList(kaisa, c - 1));
		}
		reverseElements(kaisa);
		for (c = num - 1; c; c--)
		{
			addElements(getList(kaisa, c), getList(kaisa, c - 1));
		}
	}
#endif

#if 1
	// XRX
	{
		for (c = 1; c < num; c++)
		{
			addElements(getList(kaisa, c), getList(kaisa, c - 1));
		}
		reverseElements(kaisa);
		for (c = 1; c < num; c++)
		{
			addElements(getList(kaisa, c), getList(kaisa, c - 1));
		}
	}
#endif

#if 0
	// xRx
	{
		for (c = num - 1; c; c--)
		{
			addElements(getList(kaisa, c), getList(kaisa, c - 1));
		}
		reverseElements(kaisa);
		for (c = num - 1; c; c--)
		{
			addElements(getList(kaisa, c), getList(kaisa, c - 1));
		}
	}
#endif

	for (c = 0; c < num; c++)
	{
		Kazoeru(getList(kaisa, c));
	}
	return kaisa;
}
int main(int argc, char **argv)
{
	uint c;

	for (c = 1; c <= 30; c++)
	{
		autoList_t *kaisa = GetKaisaXor(c);
		uint d;
		uint e;

		cout("[%u]\n", c);

		for (d = 0; d < c; d++)
		{
			autoList_t *idxlst = getList(kaisa, d);

			for (e = 0; e < getCount(idxlst); e++)
			{
				cout("%u,", getElement(idxlst, e));
			}
			cout("\n");
		}
		cout("\n");

		releaseAutoList(kaisa);
	}
}
