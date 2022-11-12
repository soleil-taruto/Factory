#include "all.h"

static void *REAL_memAlloc(uint size)
{
	void *block = malloc(size);

	if (block == NULL)
	{
		error();
	}
	return block;
}
static void *REAL_memRealloc(void *block, uint size)
{
	block = realloc(block, size);

	if (block == NULL)
	{
		error();
	}
	return block;
}
static void REAL_memFree(void *block)
{
	free(block);
}

#define GetAroundChar(addr) \
	(100 + ((uint)addr) % 100)

#define CheckAroundChar(addr) \
	(*(uchar *)(addr) == GetAroundChar(addr))

#define SetAroundChar(addr) \
	(*(uchar *)(addr) = GetAroundChar(addr))

#define AROUNDSIZE 8

static uchar **Blocks;
static uint *BlockSizes;
static uint BlockCount;

static uint SearchBlock(uchar *block)
{
	uint index;

	for (index = BlockCount; index; )
	{
		index--;

		if (Blocks[index] == block)
		{
			return index;
		}
	}
	error();
	return 0; // Dummy
}
static void CheckAround(void)
{
	uint index;
	uint bPos;

	for (index = 0; index < BlockCount; index++)
	{
		for (bPos = 0; bPos < AROUNDSIZE; bPos++)
		{
			errorCase(!CheckAroundChar(Blocks[index] + bPos - AROUNDSIZE));
			errorCase(!CheckAroundChar(Blocks[index] + bPos + BlockSizes[index]));
		}
	}
}

#define MSIZEMAX SINTMAX

static void *Inner_memAlloc(uint size)
{
	uchar *block;
	uint index;

	errorCase(MSIZEMAX < size);

	block = (uchar *)REAL_memAlloc(AROUNDSIZE + size + AROUNDSIZE);

	for (index = 0; index < AROUNDSIZE; index++)
	{
		SetAroundChar(block + index);
		SetAroundChar(block + index + size + AROUNDSIZE);
	}
	block += AROUNDSIZE;

	Blocks = (uchar **)REAL_memRealloc(Blocks, (BlockCount + 1) * sizeof(uchar *));
	Blocks[BlockCount] = block;
	BlockSizes = (uint *)REAL_memRealloc(BlockSizes, (BlockCount + 1) * sizeof(uint));
	BlockSizes[BlockCount] = size;
	BlockCount++;

	return block;
}
static void *Inner_memRealloc(void *block, uint size)
{
	uchar *retBlock;
	uint index;

	if (!block)
		return memAlloc(size);

	index = SearchBlock((uchar *)block);

	retBlock = (uchar *)memAlloc(size);
	memcpy(retBlock, (uchar *)block, m_min(size, BlockSizes[index]));
	memFree(block);

	return retBlock;
}
static void Inner_memFree(void *block)
{
	uint index;

	if (!block)
		return;

	index = SearchBlock((uchar *)block);

	BlockCount--;
	Blocks[index] = Blocks[BlockCount];
	BlockSizes[index] = BlockSizes[BlockCount];

	REAL_memFree((uchar *)block - AROUNDSIZE);
}

static void DebugOutputBlocks(void) // •W€ŠÖ”‚Ì‚Ý‚ÅAA
{
	FILE *fp;
	char *line;
	uint index;
	uint bPos;

	printf("DebugOutputBlocks_Bgn\n");

	fp = fopen("C:\\temp\\memory.txt", "wt");

	if (!fp)
		return;

	for (index = 0; index < BlockCount; index++)
	{
		fprintf(fp, "[%u] %p(%u): ", index, Blocks[index], BlockSizes[index]);

		for (bPos = 0; bPos < BlockSizes[index]; bPos++)
			fprintf(fp, "%02x", Blocks[index][bPos]);

		fprintf(fp, "\n");
	}
	fclose(fp);

	printf("DebugOutputBlocks_End\n");
	system("start C:\\temp");
}
static void FNLZ(void)
{
	DebugOutputBlocks();
}
static void INIT(void)
{
	{
		static int inited;

		if (inited)
			return;

		inited = 1;
	}

	addFinalizer(FNLZ);
}

void *memAlloc(uint size)
{
	INIT();
	CheckAround();
	return Inner_memAlloc(size);
}
void *memRealloc(void *block, uint size)
{
	INIT();
	CheckAround();
	return Inner_memRealloc(block, size);
}
void memFree(void *block)
{
	INIT();
	CheckAround();
	Inner_memFree(block);
}
