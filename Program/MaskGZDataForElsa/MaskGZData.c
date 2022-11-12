#include "C:\Factory\SubTools\libs\MaskGZData.h"
#include "C:\Factory\Common\Options\Sequence.h"

static uint MGZE_X;

static void MGZE_AvoidXIsZero(void)
{
	MGZE_X = MGZE_X % UINTMAX + 1;
}
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
static uint MGZE_GetMaskSize(uint size)
{
	size /= 2;
	return m_min(13, size);
}
static void MGZE_Mask(autoBlock_t *data)
{
	uint size = MGZE_GetMaskSize(getSize(data));
	uint index;

	for (index = 0; index < size; index++)
	{
		b_(data)[index] ^= 0xf5;
	}
}
static void MGZE_Swap(autoBlock_t *data, autoList_t *swapIdxLst)
{
	uint swapIdx;
	uint index;

	foreach (swapIdxLst, swapIdx, index)
	{
		swapByte(data, index, getSize(data) - swapIdx);
	}
}
static uint MGZE_GetTransposeSize(uint size)
{
	size /= 2;
	return m_min(307, size);
}
static void MGZE_Transpose_seed(autoBlock_t *data, uint seed)
{
	autoList_t *swapIdxLst = createSq(MGZE_GetTransposeSize(getSize(data)), 1, 1);

	MGZE_X = getSize(data);
	MGZE_Rand();
	MGZE_X ^= seed;
	MGZE_AvoidXIsZero();
	MGZE_Shuffle(swapIdxLst);

	MGZE_Mask(data);
	MGZE_Swap(data, swapIdxLst);
	MGZE_Mask(data);

	releaseAutoList(swapIdxLst);
}
void MaskGZData(autoBlock_t *fileData)
{
	MGZE_Transpose_seed(fileData, 2022021621);
}
