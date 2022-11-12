#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"

typedef struct Block_st
{
	int a;
	int b;
	int c;
}
Block_t;

// Stock >

#define BLOCK_MAX 500

static Block_t BlockSequence[BLOCK_MAX];
static Block_t *BlockStock[BLOCK_MAX];
static uint BlockStockIndex;

static Block_t *TakeBlock(void)
{
	Block_t **pp;

	errorCase(BLOCK_MAX <= BlockStockIndex);
	pp = BlockStock + BlockStockIndex;

	if (!*pp)
		*pp = BlockSequence + BlockStockIndex;
	else
		memset(*pp, 0x00, sizeof(Block_t)); // as memCalloc()

	BlockStockIndex++;
	return *pp;
}
static void GiveBlock(Block_t *p)
{
	errorCase(!BlockStockIndex);

	BlockStockIndex--;
	BlockStock[BlockStockIndex] = p;
}

static uint GetBlockCount(void)
{
	return BlockStockIndex;
}
static uint GetFreeBlockCount(void)
{
	return BLOCK_MAX - BlockStockIndex;
}

// < Stock

int main(int argc, char **argv)
{
	Block_t **blocks = (Block_t **)memCalloc(BLOCK_MAX * sizeof(Block_t *));
	Block_t *block;
	uint bindex;

	mt19937_init();

	while (!hasKey())
	{
		bindex = mt19937_rnd(BLOCK_MAX);
		block = blocks[bindex];

		cout("%3u: %p (%3u)\n", bindex, block, GetBlockCount());

		if (block == NULL) // add
		{
			block = TakeBlock();

			errorCase(block->a);
			errorCase(block->b);
			errorCase(block->c);

			block->a = bindex + 1;
			block->b = bindex + 2;
			block->c = bindex + 3;

			blocks[bindex] = block;
		}
		else // remove
		{
			blocks[bindex] = NULL;

			errorCase(block->a != bindex + 1);
			errorCase(block->b != bindex + 2);
			errorCase(block->c != bindex + 3);

			GiveBlock(block);
		}
	}
	memFree(blocks);
}
