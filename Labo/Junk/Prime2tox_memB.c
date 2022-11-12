/*
	Prime2tox_memB.exe �ő�l

		�ő�l ... 3�`

	�ő�l�ȉ��̑f����\������B
	���������\���ɂ���� Prime2tox_memB.exe 31100000000 ���炢�܂ł�����͂��B
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\bitList.h"

static uint64 CountMax;
static bitList_t *SosuMap;

static uint Root(uint64 value)
{
	uint root = 0;
	uint bit;

	for (bit = 1u << 31; bit; bit >>= 1)
	{
		uint r = root | bit;

		if ((uint64)r * r <= value)
		{
			root = r;
		}
	}
	return root;
}
static void DispSosu(void)
{
	uint64 index;

	cout("2\n");

	for (index = 3; index <= CountMax; index += 2)
	{
		if (index % 10000 == 1)
		{
			while (hasKey())
			{
				if (getKey() == 0x1b)
				{
					cout("�\���𒆎~���܂����B\n");
					return;
				}
			}
		}
		if (!refBit(SosuMap, index / 2))
		{
			cout("%I64u\n", index);
		}
	}
}
int main(int argc, char **argv)
{
	uint64 count;
	uint64 rootCntMax;
	uint64 wcnt;

	CountMax = toValue64(nextArg());

	errorCase(CountMax < 3);
	errorCase(68000000000 < CountMax); // HACK

	SosuMap = newBitList_A(CountMax / 2 + 100); // HACK

	rootCntMax = Root(CountMax);

	for (count = 3; count <= rootCntMax; count += 2)
	{
		if (pulseSec(1, NULL))
			cmdTitle_x(xcout("Prime2tox_memB - %I64u ������܂Ōv�Z�ς�...", count * count));

		if (!refBit(SosuMap, count / 2))
		{
			for (wcnt = count * 3; wcnt <= CountMax; wcnt += count * 2)
			{
				putBit(SosuMap, wcnt / 2, 1);
			}
		}
	}
	cmdTitle("Prime2tox_memB - �v�Z���� �o�͒�...");
	DispSosu();

	releaseBitList(SosuMap);
}
