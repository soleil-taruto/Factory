#include "all.h"

Sabun_t *CreateSabun(uint pos, uint length, uchar *data)
{
	Sabun_t *i = nb_(Sabun_t);

	i->Pos = pos;
	i->Length = length;
	i->Data = data; // bind, NULL ok

	return i;
}
void ReleaseSabun(Sabun_t *i)
{
	if (!i)
		return;

	memFree(i->Data);
	memFree(i);
}
void ReleaseSabunList(autoList_t *list)
{
	if (!list)
		return;

	callAllElement(list, (void (*)(uint))ReleaseSabun);
	releaseAutoList(list);
}

static autoBlock_t SBlock;
static autoBlock_t BBlock;
static uint SPos;
static uint BPos;

static void FindSameArea(void)
{
	uint bound = iRoot(getSize(&SBlock), 3);
	uint sndx;
	uint bndx;
	uint index;

	SPos = getSize(&SBlock);
	BPos = getSize(&BBlock);

	if (!bound)
		return;

	errorCase(UINTMAX - SPos < BPos); // 2bs

	for (sndx = 0; sndx + bound < getSize(&SBlock); sndx += bound)
	for (bndx = 0; bndx + bound < getSize(&BBlock); bndx++)
	{
		if (SPos + BPos <= sndx + bndx) // ? 既知の差分より大きいか同じになる。-> next sndx
			break;

		for (index = 0; index < bound; index++)
			if (getByte(&SBlock, sndx) != getByte(&BBlock, bndx)) // ? 不一致
				break;

		if (index == bound) // ? found
		{
			SPos = sndx;
			BPos = bndx;
		}
	}
}
static void FindSameBegin(void)
{
	while (SPos && BPos)
	{
		if (getByte(&SBlock, SPos - 1) != getByte(&BBlock, BPos - 1)) // ? 差分の終端
			break;

		SPos--;
		BPos--;
	}
}

static autoBlock_t *LBlock;
static autoBlock_t *RBlock;
static uint LIndex;
static uint RIndex;

static int FindDiffBegin(void)
{
	while (LIndex < getSize(LBlock) && RIndex < getSize(RBlock))
	{
		if (getByte(LBlock, LIndex) != getByte(RBlock, RIndex))
			break;

		LIndex++;
		RIndex++;
	}
	return LIndex < getSize(LBlock) || RIndex < getSize(RBlock);
}
static void FindDiffEnd(void)
{
	int swapped;

	SBlock = gndFollowBytes(LBlock, LIndex);
	BBlock = gndFollowBytes(RBlock, RIndex);

	if (getSize(&BBlock) < getSize(&SBlock))
	{
		swapBlock(&SBlock, &BBlock, sizeof(autoBlock_t));
		swapped = 1;
	}
	else
	{
		swapped = 0;
	}
	FindSameArea();
	FindSameBegin();

	if (swapped)
		m_swap(SPos, BPos, uint);

	errorCase(getSize(LBlock) - LIndex < SPos); // 2bs
	errorCase(getSize(RBlock) - RIndex < BPos); // 2bs

	LIndex += SPos;
	RIndex += BPos;
}

static void AddSabun(autoList_t *diff, autoBlock_t *block, uint begin, uint end, int withData)
{
	Sabun_t *i;
	uint length = end - begin;

	i = CreateSabun(begin, length, NULL);

	if (withData)
		i->Data = (uchar *)unbindBlock(ab_makeSubBytes(block, begin, length));

	addElement(diff, (uint)i);
}
void MakeSabun(autoBlock_t *lBlock, autoBlock_t *rBlock, autoList_t *lDiff, autoList_t *rDiff, int lWithData, int rWithData)
{
	errorCase(!lBlock);
	errorCase(!rBlock);
	errorCase(!lDiff);
	errorCase(!rDiff);

	LBlock = lBlock;
	RBlock = rBlock;
	LIndex = 0;
	RIndex = 0;

	while (FindDiffBegin())
	{
		uint lBegin = LIndex;
		uint rBegin = RIndex;
		uint lEnd;
		uint rEnd;

		FindDiffEnd();

		lEnd = LIndex;
		rEnd = RIndex;

		AddSabun(lDiff, lBlock, lBegin, lEnd, lWithData);
		AddSabun(rDiff, rBlock, rBegin, rEnd, rWithData);
	}
}
