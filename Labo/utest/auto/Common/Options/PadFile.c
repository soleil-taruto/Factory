#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\PadFile.h"
#include "C:\Factory\DevTools\libs\RandData.h"

static void DoTest2(uint64 size, uint breakMode)
{
	char *file = makeTempPath("pad-file-test");
	char *eFile = makeTempPath("pad-file-test-escape");
	uint retval;

	cout("file: %s\n", file);
	cout("eFile: %s\n", eFile);
	cout("size: %I64u\n", size);
	cout("breakMode: %u\n", breakMode);

	MakeRandBinaryFile(file, size);
	copyFile(file, eFile);

	PadFileLine(file, "PAD-TEST-FOOTER");

	if (breakMode)
		RandXorOneBitFile(file);

	retval = UnpadFileLine(file, "PAD-TEST-FOOTER");
	cout("retval: %u\n", retval);

	if (!breakMode)
	{
		errorCase(!retval);
		errorCase(!isSameFile(file, eFile));
	}
	else
	{
		errorCase(retval);
	}

	removeFile(file);
	removeFile(eFile);
	memFree(file);
	memFree(eFile);

	cout("OK\n");
}
static void DoTest(uint64 size)
{
	DoTest2(size, 0);
	DoTest2(size, 1);
}
int main(int argc, char **argv)
{
	uint c;

	mt19937_init();

	for (c = 0; c < 100; c++)
	{
		DoTest((uint64)c);
	}
	for (c = 0; c < 100; c++)
	{
		cout("[%u]\n", c);
		DoTest(mt19937_rnd64Mod(1000000ui64));
	}
}
