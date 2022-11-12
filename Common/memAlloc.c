#include "all.h"

void *memAlloc(uint size)
{
	void *block;

	size = m_max(size, 1);
	block = malloc(size);

	if (block == NULL)
	{
		error();
	}
	return block;
}
void *memRealloc(void *block, uint size)
{
	size = m_max(size, 1);
	block = realloc(block, size);

	if (block == NULL)
	{
		error();
	}
	return block;
}
void memFree(void *block) // block: NULL ok
{
	free(block);
}
