#include "C:\Factory\Common\all.h"

static void SemiRelease(char *resFile, char *destResFile, char *outPrefix, char *resPrefix)
{
	autoList_t *files = readResourcePaths(resFile);
	char *file;
	uint index;
	autoList_t *outFiles = newList();

	foreach (files, file, index)
	{
		char *outFile = xcout("%08u.%s", index, getExt(file));

		copyFile_cx(file, xcout("%s%s", outPrefix, outFile));
		addElement(outFiles, (uint)xcout("%s%s", resPrefix, outFile));
		memFree(outFile);
	}
	writeLines(destResFile, outFiles);

	releaseDim(files, 1);
	releaseDim(outFiles, 1);
}
int main(int argc, char **argv)
{
	char *resFile;
	char *destResFile;
	char *outPrefix;
	char *resPrefix;

	resFile = nextArg();
	destResFile = nextArg();
	outPrefix = nextArg();
	resPrefix = nextArg();

	if (existDir(destResFile))
		destResFile = combine(destResFile, getLocal(resFile)); // g

	SemiRelease(resFile, destResFile, outPrefix, resPrefix);
}
