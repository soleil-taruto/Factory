#include "C:\Factory\Common\all.h"

#define DIRS_FILE "dirs.tmp"
#define FILES_FILE "files.tmp"

static autoList_t *GetPaths1(char *dir)
{
	return lss(dir);
}
static autoList_t *GetPaths2(char *dir)
{
	autoList_t *paths;

	lss2File(dir, DIRS_FILE, FILES_FILE);

	paths = readLines(DIRS_FILE);
	addLines_x(paths, readLines(FILES_FILE));

	removeFile(DIRS_FILE);
	removeFile(FILES_FILE);

	return paths;
}
static void Test_lss_lss2File(char *dir)
{
	autoList_t *paths1 = GetPaths1(dir);
	autoList_t *paths2 = GetPaths2(dir);

	rapidSortLines(paths1);
	rapidSortLines(paths2);

	errorCase(!isSameLines(paths1, paths2, 0));

	releaseDim(paths1, 1);
	releaseDim(paths2, 1);

	cout("OK\n");
}

int main(int argc, char **argv)
{
	for (; ; )
	{
		Test_lss_lss2File(dropDir());
	}
}
