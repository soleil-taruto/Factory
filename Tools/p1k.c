/*
	1 phrases (passwords) to 512-bits key
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\crc.h"
#include "C:\Factory\OpenSource\sha512.h"

#define KEYFILESDIR "C:\\Factory\\tmp\\p1k"

static char *OutputFile;

static void Phrase2Key(char *phrase)
{
	char *file;
	FILE *fp;

	if (!OutputFile)
	{
		file = xcout("[%s].txt", phrase);
		errorCase(!isFairLocalPath(file, strlen(KEYFILESDIR)));
		file = combine_cx(KEYFILESDIR, file);
	}
	else
	{
		file = strx(OutputFile);
	}

	sha512_makeHashLine(phrase);
	sha512_makeHexHash();

	cout("(%02x) %s\n", crc8CheckBlock(sha512_hash, 64), sha512_hexHash);

	removeFileIfExist(file);
	fp = fileOpen(file, "wt");
	writeLine_x(fp, strxl(sha512_hexHash, 64));
	writeLine(fp, sha512_hexHash + 64);
	fileClose(fp);

	memFree(file);
}
int main(int argc, char **argv)
{
	mkdirEx(KEYFILESDIR);

	if (argIs("/O")) // Output file
	{
		OutputFile = nextArg();
	}
	errorCase(hasArgs(2));

	if (hasArgs(1))
	{
		Phrase2Key(nextArg());
	}
	if (!OutputFile)
	{
		execute("START " KEYFILESDIR);
	}
}
