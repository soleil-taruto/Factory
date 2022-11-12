#include "C:\Factory\Common\all.h"
#include "C:\Factory\DevTools\libs\RandData.h"

#define J_COUNT 123

static void MiddleJam(autoBlock_t *block)
{
	uint index;

	for (index = 0; index < getSize(block); index++)
	{
		setByte(block, index, getByte(block, index) ^ 0xff);
	}
}
static void Jammer3(autoBlock_t *block, uint mode)
{
	uint index;

	if (mode)
	{
		for (index = 1; index < getSize(block); index++)
		{
			setByte(block, index, (getByte(block, index) + 256 - getByte(block, index - 1)) % 256);
		}
	}
	else
	{
		for (index = getSize(block) - 1; index; index--)
		{
			setByte(block, index, (getByte(block, index) + getByte(block, index - 1)) % 256);
		}
	}
}
static void Jammer2(autoBlock_t *block, uint mode, uint j_num)
{
	uint count;

	for (count = 0; count < j_num; count++)
	{
		if (count)
			reverseBytes(block);

		Jammer3(block, mode);
	}
}
static void Jammer(autoBlock_t *block)
{
	Jammer2(block, 1, J_COUNT);
	MiddleJam(block);
	Jammer2(block, 0, J_COUNT);
}

static void DoTest(uint size) // size: 1-
{
	autoBlock_t *block1 = MakeRandBinaryBlock(size);
	autoBlock_t *block2;
	autoBlock_t *block3;

	cout("size: %u\n", getSize(block1));

	block2 = copyAutoBlock(block1);
	Jammer(block2);
	block3 = copyAutoBlock(block2);
	Jammer(block3);

	errorCase( isSameBlock(block1, block2));
	errorCase(!isSameBlock(block1, block3));

	releaseAutoBlock(block1);
	releaseAutoBlock(block2);
	releaseAutoBlock(block3);

	cout("ok!\n");
}
int main(int argc, char **argv)
{
	uint c;

	mt19937_init();

	DoTest(1);
	DoTest(1);
	DoTest(1);

	DoTest(2);
	DoTest(2);
	DoTest(2);

	DoTest(3);
	DoTest(3);
	DoTest(3);

	for (c = 1; c < 1000; c++)
	{
		DoTest(c);
	}
	while (!waitKey(0))
	{
		DoTest(1 + mt19937_rnd(1000));
	}
}
