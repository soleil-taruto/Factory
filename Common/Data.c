#include "all.h"

void swapBlock(void *block1, void *block2, uint size)
{
	void *swap = memAlloc(size);

	copyBlock(swap, block1, size);
	copyBlock(block1, block2, size);
	copyBlock(block2, swap, size);

	memFree(swap);
}
