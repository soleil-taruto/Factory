#include "C:\Factory\Common\all.h"
#include "C:\Factory\DevTools\libs\RandData.h"

#define MULTIVAL 157
#define POWERVAL 31

static int Jammer(autoBlock_t *block, int doEncode)
{
	uint tznum = ab_unaddTrailZero(block);
	uint index;
	uint value = 0;

	if (!getSize(block))
	{
		if (doEncode)
		{
			tznum++;
		}
		else
		{
			if (!tznum)
				return 0;

			tznum--;
		}
	}
	else if (doEncode)
	{
		for (index = 0; index < getSize(block) || value; index++)
		{
			value += refByte(block, index) * MULTIVAL;
			putByte(block, index, value & 0xff);
			value >>= 8;
		}
	}
	else
	{
		for (index = getSize(block); index; )
		{
			index--;
			value |= getByte(block, index);
			setByte(block, index, value / MULTIVAL);
			value %= MULTIVAL;
			value <<= 8;
		}
		if (value)
			return 0;

		ab_unaddTrailZero(block);
	}
	ab_addTrailZero(block, tznum);
	return 1;
}
static int PowerJammer(autoBlock_t *block, int doEncode)
{
	uint count;

	for (count = POWERVAL; count; count--)
		if (!Jammer(block, doEncode))
			return 0;

	return 1;
}
static void DoTest(uint size)
{
	autoBlock_t *block1 = MakeRandBinaryBlock(size);
	autoBlock_t *block2;
	autoBlock_t *block3;

	cout("size: %u\n", getSize(block1));

	block2 = copyAutoBlock(block1);

//	errorCase(!Jammer(block2, 1));
	errorCase(!PowerJammer(block2, 1));

	block3 = copyAutoBlock(block2);

//	errorCase(!Jammer(block3, 0));
	errorCase(!PowerJammer(block3, 0));

	errorCase( isSameBlock(block1, block2));
	errorCase(!isSameBlock(block1, block3));

	releaseAutoBlock(block1);
	releaseAutoBlock(block2);
	releaseAutoBlock(block3);

	cout("ok!\n");
}
static void ProcFile(int doEncode)
{
	char *rFile;
	char *wFile;
	autoBlock_t *fileImage;

	rFile = nextArg();
	wFile = nextArg();

	errorCase(existFile(wFile)); // ã‘‚«–hŽ~

	fileImage = readBinary(rFile);

	if (PowerJammer(fileImage, doEncode))
	{
		writeBinary(wFile, fileImage);
		cout("Successful\n");
	}
	else
	{
		cout("Fault\n");
	}
	releaseAutoBlock(fileImage);
	termination(0);
}
int main(int argc, char **argv)
{
	if (argIs("/E")) // encode
	{
		ProcFile(1);
	}
	if (argIs("/D")) // decode
	{
		ProcFile(0);
	}

	// test
	{
		uint c;

		mt19937_init();

		for (c = 0; c < 1000; c++)
		{
			DoTest(c / 100);
			DoTest(c / 10);
			DoTest(c);
		}
		while (!waitKey(0))
		{
			DoTest(1 + mt19937_rnd(1000));
		}
	}
}
