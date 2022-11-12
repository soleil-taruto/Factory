#include "C:\Factory\SubTools\libs\MaskGZData.h"

// sync > @ MaskGZDataEng_c

#include "C:\Factory\Common\Options\Sequence.h"

static void MGZE_MaskSignature(autoBlock_t *data)
{
	if (2 <= getSize(data))
	{
		setByte(data, 0, getByte(data, 0) ^ 0x1f ^ 'D');
		setByte(data, 1, getByte(data, 1) ^ 0x8b ^ '3');
	}
}

static uint MGZE_X;

static uint MGZE_Rand(void)
{
	// Xorshift-32

	MGZE_X ^= MGZE_X << 13;
	MGZE_X ^= MGZE_X >> 17;
	MGZE_X ^= MGZE_X << 5;

	return MGZE_X;
}
static void MGZE_Shuffle(autoList_t *values)
{
	uint index;

	for (index = getCount(values); 2 <= index; index--)
	{
		swapElement(values, index - 1, MGZE_Rand() % index);
	}
}
static void MGZE_Transpose_seed(autoBlock_t *data, uint seed)
{
	autoList_t *swapIdxLst = createSq(getSize(data) / 2, 1, 1);
	uint swapIdx;
	uint index;

	MGZE_MaskSignature(data);

	MGZE_X = seed;
	MGZE_Shuffle(swapIdxLst);

	foreach (swapIdxLst, swapIdx, index)
	{
		swapByte(data, index, getSize(data) - swapIdx);
	}
	releaseAutoList(swapIdxLst);

	MGZE_MaskSignature(data);
}
static void MGZE_Transpose(autoBlock_t *data)
{
	MGZE_Transpose_seed(data, 1999123123);
}

// < sync

void MaskGZData(autoBlock_t *fileData)
{
	MGZE_Transpose_seed(fileData, 2020081102);
}
