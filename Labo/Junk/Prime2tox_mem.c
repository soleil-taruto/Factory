/*
	Prime2tox_mem.exe 最大値

		最大値 ... 23～

	最大値以下の素数を表示する。
	メモリが十分にあれば Prime2tox_mem.exe 1700000000 くらいまでいけるはず。
*/

#include "C:\Factory\Common\all.h"

#define SOSUMAP_SIZE (2 * 3 * 5 * 7 * 11 * 13 * 17 * 19 * 23)

static uint SosuList[] =
{
	2,
	3,
	5,
	7,
	11,
	13,
	17,
	19,
	23,
};

static uint CountMax;
static uchar *SosuMap;

static uint Root(uint value)
{
	uint root = 0;
	uint bit;

	for (bit = 1u << 15; bit; bit >>= 1)
	{
		uint r = root | bit;

		if (r * r <= value)
		{
			root = r;
		}
	}
	return root;
}
static void MakeSosuMap(void)
{
	uint index;
	uint ndx;

	SosuMap = (uchar *)memAlloc(SOSUMAP_SIZE);
	SosuMap[0] = 0;
	memset(SosuMap + 1, 1, SOSUMAP_SIZE - 1);

	for (index = 0; index < lengthof(SosuList); index++)
	{
		for (ndx = SosuList[index]; ndx < SOSUMAP_SIZE; ndx += SosuList[index])
		{
			SosuMap[ndx] = 0;
		}
	}
}
static void WriteSosuMap(void)
{
	uint index;
	uint size = CountMax + 1;

	if (SOSUMAP_SIZE < size)
	{
		for (index = SOSUMAP_SIZE; index + SOSUMAP_SIZE < size; index += SOSUMAP_SIZE)
		{
			memcpy(SosuMap + index, SosuMap, SOSUMAP_SIZE);
		}
		memcpy(SosuMap + index, SosuMap, size - index);
	}
}
static void DispSosu(void)
{
	uint index;

	for (index = 0; index < lengthof(SosuList); index++)
	{
		cout("%u\n", SosuList[index]);
	}
	for (index = 29; index <= CountMax; index++)
	{
		if (index % 10000 == 0)
		{
			while (hasKey())
			{
				if (getKey() == 0x1b)
				{
					cout("表示を中止しました。\n");
					return;
				}
			}
		}
		if (SosuMap[index])
		{
			cout("%u\n", index);
		}
	}
}
int main(int argc, char **argv)
{
	uint count;
	uint rootCntMax;
	uint wcnt;

	CountMax = toValue(nextArg());

	errorCase(CountMax < 23);
	errorCase(CountMax == UINTMAX);

	MakeSosuMap();
	SosuMap = (uchar *)memRealloc(SosuMap, CountMax + 1);
	WriteSosuMap();

	rootCntMax = Root(CountMax);

	for (count = 29; count <= rootCntMax; count++)
	{
		if (count < 100 || count % 100 == 0)
			if (pulseSec(1, NULL))
				cmdTitle_x(xcout("Prime2tox - %u あたりまで計算済み...", count * count));

		if (SosuMap[count])
		{
			for (wcnt = (uint64)count * 3; wcnt <= CountMax; wcnt += (uint64)count * 2)
			{
				SosuMap[wcnt] = 0;
			}
		}
	}
	cmdTitle("Prime2tox - 計算完了 出力中...");
	DispSosu();

	memFree(SosuMap);
}
