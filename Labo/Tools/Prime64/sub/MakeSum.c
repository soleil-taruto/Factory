#include "C:\Factory\Common\all.h"

static void Summary(char *rDir, char *outFile)
{
	autoList_t *files = lsFiles(rDir);
	char *file;
	uint index;
	uint64 total = 0;

	foreach (files, file, index)
		total += toValue64_x(readFirstLine(file));

	writeOneLine_cx(outFile, xcout("%I64u", total));
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	Summary(getArg(0), getArg(1));
}
