/*
	2 phrases (passwords) to 256-bits key
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\crc.h"
#include "C:\Factory\OpenSource\md5.h"

#define KEYFILESDIR "C:\\Factory\\tmp\\p2k"

static char *OutputFile;

static void Phrase2Key(char *phrase1, char *phrase2)
{
	autoBlock_t *hash1 = md5_makeHashLine(phrase1);
	autoBlock_t *hash2 = md5_makeHashLine(phrase2);
	uint crc = crc8Start();
	char *file;
	char *strHash;
	char *strw;

	if (!OutputFile)
	{
		file = xcout("[%s]+[%s].txt", phrase1, phrase2);
		errorCase(!isFairLocalPath(file, strlen(KEYFILESDIR)));
		file = combine_cx(KEYFILESDIR, file);
	}
	else
	{
		file = strx(OutputFile);
	}

	crc = crc8UpdateBlock(crc, directGetBuffer(hash1), 16);
	crc = crc8UpdateBlock(crc, directGetBuffer(hash2), 16);
	strHash = makeHexLine(hash1);
	strHash = addLine(strHash, strw = makeHexLine(hash2)); memFree(strw);

	cout("(%02x) %s\n", crc8Finish(crc), strHash);

	removeFileIfExist(file);
	writeOneLine(file, strHash);

	releaseAutoBlock(hash1);
	releaseAutoBlock(hash2);
	memFree(strHash);
	memFree(file);
}
int main(int argc, char **argv)
{
	mkdirEx(KEYFILESDIR);

	if (argIs("/O"))
	{
		OutputFile = nextArg();
	}
	errorCase(hasArgs(3));

	if (hasArgs(2))
	{
		Phrase2Key(getArg(0), getArg(1));
	}
	else
	{
		errorCase(hasArgs(1));
	}
	if (!OutputFile)
	{
		execute("START " KEYFILESDIR);
	}
}
