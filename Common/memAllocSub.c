#include "all.h"

void *memCalloc(uint size)
{
	void *block = memAlloc(size);

	memset(block, 0x00, size);
	return block;
}
void *memCloneWithBuff(void *block, uint size, uint buffsize)
{
	void *newBlock = memAlloc(size + buffsize);

	memcpy(newBlock, block, size);
	return newBlock;
}
void *memClone(void *block, uint size)
{
	return memCloneWithBuff(block, size, 0);
}
