/*
	mkcr.exe [/F KB-OUT-FILE] [ROW [COLUMN]]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

static char *KBOutFile;

static void MakeCryptoRand(uint rowcnt, uint colcnt)
{
	char *file = makeTempPath("txt");
	uint rowidx;
	uint colidx;
	FILE *fp;
	autoBlock_t *block;
	uint i = 0;
	char *swrk;

	errorCase(!rowcnt);
	errorCase(!colcnt);
	errorCase(UINTMAX / rowcnt < colcnt);

	fp = fileOpen(file, "wt");

	block = makeCryptoRandBlock(rowcnt * colcnt);

	for (rowidx = 0; rowidx < rowcnt; rowidx++)
	{
		for (colidx = 0; colidx < colcnt; colidx++)
		{
			writeToken(fp, swrk = xcout("%02x", getByte(block, i++))); memFree(swrk);
		}
		writeLine(fp, "");
	}
	fileClose(fp);
	releaseAutoBlock(block);

	if (KBOutFile)
	{
		cout("< %s\n", file);
		cout("> %s\n", KBOutFile);

		copyFile(file, KBOutFile);
	}
	else
	{
		execute(file);
	}
	remove(file);
	memFree(file);
}
int main(int argc, char **argv)
{
	if (argIs("/F"))
	{
		KBOutFile = nextArg();
	}

	if (hasArgs(2))
	{
		MakeCryptoRand(toValue(getArg(0)), toValue(getArg(1))); // n x m x 8 bits
		return;
	}
	if (hasArgs(1))
	{
		MakeCryptoRand(toValue(getArg(0)), 32); // n x 256 bits
		return;
	}
	MakeCryptoRand(32, 32); // 32 x 256 bits == 8192 bits
}
