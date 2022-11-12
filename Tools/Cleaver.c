#include "C:\Factory\Common\all.h"

#define BUFFSIZE (256 * 1024 * 1024)

static uint64 DivSize = 128 * 1024 * 1024;
static uint DeleteMode;

static char *GetDivFile(char *file, uint dividx)
{
	char *swrk;

	errorCase(99999 < dividx);

	file = addExt(strx(file), swrk = xcout("%05u", dividx)); memFree(swrk);
	return file;
}
static void DMDeleteFile(char *file)
{
	if (DeleteMode)
	{
		cout("DELETE: %s\n", file);
		removeFile(file);
	}
}
static void Divide(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	uint dividx;

	cout("•ªŠ„‚µ‚Ü‚·B\n");
	cout("file: %s\n", file);
	cout("divsz: %I64u\n", DivSize);

	for (dividx = 0; ; dividx++)
	{
		char *divfile = GetDivFile(file, dividx);
		FILE *divfp;
		uint64 szcnt;

		cout("divfile: %s\n", divfile);
		divfp = fileOpen(divfile, "wb");

		for (szcnt = 0; szcnt < DivSize; )
		{
			uint readsize = (uint)m_min((uint64)BUFFSIZE, DivSize - szcnt);
			autoBlock_t *divblock;

			cout("readsize: %u\n", readsize);

			divblock = readBinaryStream(fp, readsize);

			cout("divblock: %p (%u)", divblock, divblock ? getSize(divblock) : 0);

			if (!divblock)
				break;

			writeBinaryBlock(divfp, divblock);

			szcnt += getSize(divblock);
			releaseAutoBlock(divblock);
		}
		fileClose(divfp);
		cout("szcnt: %I64u\n", szcnt);

		if (!szcnt)
		{
			removeFile(divfile);
			memFree(divfile);
			break;
		}
		memFree(divfile);
	}
	fileClose(fp);
	DMDeleteFile(file);
	cout("\\e\n");
}
static void Restore(char *file)
{
	FILE *fp = fileOpen(file, "wb");
	uint dividx;

	cout("•œŒ³‚µ‚Ü‚·B\n");
	cout("file: %s\n", file);

	for (dividx = 0; ; dividx++)
	{
		char *divfile = GetDivFile(file, dividx);
		FILE *divfp;

		cout("divfile: %s\n", divfile);

		if (!existFile(divfile))
		{
			memFree(divfile);
			break;
		}
		divfp = fileOpen(divfile, "rb");

		for (; ; )
		{
			autoBlock_t *divblock = readBinaryStream(divfp, BUFFSIZE);

			cout("divblock: %p (%u)\n", divblock, divblock ? getSize(divblock) : 0);

			if (!divblock)
				break;

			writeBinaryBlock(fp, divblock);
			releaseAutoBlock(divblock);
		}
		fileClose(divfp);
		DMDeleteFile(divfile);
		memFree(divfile);
	}
	fileClose(fp);
	cout("\\e\n");
}
static void Cleaver(char *file)
{
	char *ext = getExt(file);

	if (lineExp("<5,09>", ext))
	{
		file = changeExt(file, "");
		Restore(file);
		memFree(file);
	}
	else
	{
		Divide(file);
	}
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/S")) // Size
	{
		DivSize = toValue64(nextArg());
		errorCase(!DivSize);
		goto readArgs;
	}
	if (argIs("/D") || argIs("/OAD")) // read-end and Delete
	{
		DeleteMode = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		Cleaver(nextArg());
	}
	else
	{
		char *file = dropFile();

		Cleaver(file);
		memFree(file);
	}
}
