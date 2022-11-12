#include "C:\Factory\Common\all.h"

static void ListType(autoList_t *files, char *wFile)
{
	FILE *fp;
	char *file;
	uint index;

	if (existFile(wFile))
	{
		cout("è„èëÇ´ÅH\n");

		if (getKey() == 0x1b)
			termination(0);

		cout("è„èëÇ´Ç∑ÇÈÅB\n");
	}
	fp = fileOpen(wFile, "wt");

	foreach (files, file, index)
	{
		writeLine(fp, "================================================================================");
		writeLine_x(fp, xcout("Å°%s", file));
		writeLine(fp, "================================================================================");
		writeLines2Stream(fp, readLines(file));
	}
	fileClose(fp);
}
int main(int argc, char **argv)
{
	if (argIs("/L"))
	{
		autoList_t *files = lsFiles(nextArg());
		sortJLinesICase(files);
		ListType(files, nextArg());
		return;
	}
	ListType(readLines(getArg(0)), getArg(1));
}
