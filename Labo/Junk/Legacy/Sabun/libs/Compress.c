#include "Compress.h"

static autoList_t *SqList;

static void AddSqList(autoBlock_t *block)
{
	uint index;

	for (index = getCount(SqList); index; index--)
	{
		ab_addValue(block, getElement(SqList, index - 1));
	}
	ab_addValue(block, getCount(SqList));
}
static void UnaddSqList(autoBlock_t *block)
{
	uint count = ab_unaddValue(block);
	uint index;

	for (index = 0; index < count; index++)
	{
		addElement(SqList, ab_unaddValue(block));
	}
}

void SBN_CompressSabun(autoBlock_t *sabun)
{
	uint index;

	SqList = newList();

	for (index = 0; index < getSize(sabun); index++)
	{
		uint zcnt = 0;

		for (zcnt = 0; index + zcnt < getSize(sabun); zcnt++)
		{
			if (getByte(sabun, index + zcnt) != 0x00)
			{
				break;
			}
		}
		if (9 <= zcnt)
		{
			addElement(SqList, index);
			addElement(SqList, zcnt);
			removeBytes(sabun, index, zcnt);
		}
	}
	AddSqList(sabun);
	releaseAutoList(SqList);
}
void SBN_DecompressSabun(autoBlock_t *sabun)
{
	uint zcnt = 0;
	uint index;

	SqList = newList();
	UnaddSqList(sabun);

	for (index = 0; index < getCount(SqList); index += 2)
	{
		uint zIndex = getElement(SqList, index);
		uint zCount = getElement(SqList, index + 1);

		insertByteRepeat(sabun, zIndex + zcnt, 0x00, zCount);
		zcnt += zCount;
	}
	releaseAutoList(SqList);
}

void SBN_KaisaDown(autoBlock_t *block)
{
	if (2 <= getSize(block))
	{
		uint index;
		uint sa;

		for (index = getSize(block) - 1; index; index--)
		{
			sa = getByte(block, index) + 256 - getByte(block, index - 1);
			sa &= 0xff;
			setByte(block, index, sa);
		}
	}
}
void SBN_KaisaUp(autoBlock_t *block)
{
	uint index;
	uint sa;

	for (index = 1; index < getSize(block); index++)
	{
		sa = getByte(block, index) + getByte(block, index - 1);
		sa &= 0xff;
		setByte(block, index, sa);
	}
}

void SBN_CompressBlock(autoBlock_t *block)
{
	SBN_CompressSabun(block); // 0, 0, 0, 0 ... ‚ðˆ³k
	SBN_KaisaDown(block);
	SBN_CompressSabun(block); // x, x, x, x ... ‚ðˆ³k
	SBN_KaisaDown(block);
	SBN_CompressSabun(block); // x, x+1c, x+2c, x+3c ... ‚ðˆ³k
}
void SBN_DecompressBlock(autoBlock_t *block)
{
	SBN_DecompressSabun(block);
	SBN_KaisaUp(block);
	SBN_DecompressSabun(block);
	SBN_KaisaUp(block);
	SBN_DecompressSabun(block);
}
