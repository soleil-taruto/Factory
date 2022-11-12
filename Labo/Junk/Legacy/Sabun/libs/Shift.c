#include "Shift.h"

static uint GetDiffStart(autoBlock_t *block1, autoBlock_t *block2)
{
	uint size = m_min(getSize(block1), getSize(block2));
	uint index;

	for (index = 0; index < size; index++)
	{
		if (getByte(block1, index) != getByte(block2, index))
		{
			break;
		}
	}
	return index;
}
void SBN_ShiftBlock(autoBlock_t *block, autoBlock_t *subBlock)
{
	uint diffstart = GetDiffStart(block, subBlock);
	uint size;
	uint sbsize;
	uint mvsize;
	autoBlock_t *mvblock;

	size   = getSize(block)    - diffstart;
	sbsize = getSize(subBlock) - diffstart;

	if (size * 2 <= sbsize || size == sbsize)
	{
		mvsize = 0;
	}
	else if (size < sbsize)
	{
		mvsize = sbsize - size;
	}
	else
	{
		mvsize = sbsize;
	}
	mvblock = desertBytes(block, getSize(block) - mvsize, mvsize);
	insertBytes(block, diffstart, mvblock);
	releaseAutoBlock(mvblock);

	if (mvsize)
	{
		ab_addValue(block, diffstart);
	}
	ab_addValue(block, mvsize);
}
void SBN_UnshiftBlock(autoBlock_t *block)
{
	uint mvsize = ab_unaddValue(block);
	uint fromidx;
	autoBlock_t *mvblock;

	if (!mvsize)
		return;

	fromidx = ab_unaddValue(block);

	errorCase(getSize(block) < mvsize);
	errorCase(getSize(block) - mvsize < fromidx);

	mvblock = desertBytes(block, fromidx, mvsize);
	addBytes(block, mvblock);
	releaseAutoBlock(mvblock);
}
