#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"

static void *MakeBlock(uint size)
{
	uchar *block = memAlloc(size);
	uint index;

	for (index = 0; index < size; index += 100)
	{
		memset(block +index, mt19937_range(1, 255), 100);
	}
	return block;
}
int main(int argc, char **argv)
{
	autoList_t *blockList = newList();

	for (; ; )
	{
		cout("[%u]\n", getCount(blockList));

		switch (getKey())
		{
		case '0':
			releaseDim(blockList, 1);
			blockList = newList();
			break;

		case '+':
			cout("+10MB\n");
			addElement(blockList, (uint)MakeBlock(10000000)); // += 10MB
			break;

		case '*':
			cout("+0.1GB\n");
			addElement(blockList, (uint)MakeBlock(100000000)); // += 100MB
			break;

		case '5':
			cout("+0.5GB\n");
			addElement(blockList, (uint)MakeBlock(500000000)); // += 500MB
			break;

		case '-':
			memFree((void *)unaddElement(blockList));
			break;

		default:
			goto endProc;
		}
	}

endProc:
	releaseDim(blockList, 1);
}
