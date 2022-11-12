#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Performance.h"
#include "C:\Factory\OpenSource\sha512.h"

static void LongHash(int chr, uint blockSize, uint blockNum)
{
	uint64 startTime = GetPerformanceCounter();
	uint64 endTime;

	{
		sha512_t *i = sha512_create();
		uchar *block = memAlloc(blockSize);
		uint count;
		autoBlock_t gBlock;

		memset(block, chr, blockSize);
		gBlock = gndBlock(block, blockSize);

		for (count = blockNum; count; count--)
		{
			sha512_update(i, &gBlock);
		}
		memFree(block);
		sha512_makeHash(i);
		sha512_release(i);
		sha512_makeHexHash();
	}
	endTime = GetPerformanceCounter();

	cout("%.3f\n", (endTime - startTime) / (double)GetPerformanceFrequency());
	cout("%s\n", sha512_hexHash);
}
int main(int argc, char **argv)
{
	LongHash(getArg(0)[0], toValue(getArg(1)), toValue(getArg(2)));
}
