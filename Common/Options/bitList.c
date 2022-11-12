#include "BitList.h"

bitList_t *newBitList(void)
{
	return newBitList_A(0);
}
bitList_t *newBitList_A(uint64 allocBitSize)
{
	bitList_t *i = memAlloc(sizeof(bitList_t));

	i->Buffer = createAutoList((uint)((allocBitSize + 31) >> 5));

	return i;
}
void releaseBitList(bitList_t *i)
{
	releaseAutoList(i->Buffer);
	memFree(i);
}

// <-- cdtor

uint refBit(bitList_t *i, uint64 index)
{
	return refElement(i->Buffer, (uint)(index >> 5)) >> (uint)(index & 31) & 1;
}
void putBit(bitList_t *i, uint64 index, uint value)
{
	uint c = refElement(i->Buffer, (uint)(index >> 5));

	if (value)
		c |= 1u << (uint)(index & 31);
	else
		c &= ~(1u << (uint)(index & 31));

	putElement(i->Buffer, (uint)(index >> 5), c);
}
void putBits(bitList_t *i, uint64 index, uint64 size, uint value) // size: 0 ok
{
	uint64 bgn = index;
	uint64 end = index + size;

	while (bgn < end && bgn & 31)
	{
		putBit(i, bgn, value);
		bgn++;
	}
	while (bgn < end && end & 31)
	{
		end--;
		putBit(i, end, value);
	}
	if (bgn < end)
	{
		uint index    = (uint)(bgn >> 5);
		uint indexEnd = (uint)(end >> 5);

		value = value ? 0xffffffff : 0;

		for (; index < indexEnd; index++)
			putElement(i->Buffer, index, value);
	}
}

void invBit(bitList_t *i, uint64 index)
{
	uint c = refElement(i->Buffer, (uint)(index >> 5));

	c ^= 1u << (uint)(index & 31);

	putElement(i->Buffer, (uint)(index >> 5), c);
}
void invBits(bitList_t *i, uint64 index, uint64 size) // size: 0 ok
{
	uint64 bgn = index;
	uint64 end = index + size;

	while (bgn < end && bgn & 31)
	{
		invBit(i, bgn);
		bgn++;
	}
	while (bgn < end && end & 31)
	{
		end--;
		invBit(i, end);
	}
	if (bgn < end)
	{
		uint index    = (uint)(bgn >> 5);
		uint indexEnd = (uint)(end >> 5);

		for (; index < indexEnd; index++)
			putElement(i->Buffer, index, refElement(i->Buffer, index) ^ 0xffffffff);
	}
}

// <-- accessor
