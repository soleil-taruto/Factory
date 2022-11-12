/*
	> lss
	> lssExt

	> lss
	> lssExt | TypeGroupBy
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *files = readLines(FOUNDLISTFILE);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		cout("%s\n", getExt(file));
	}
	releaseDim(files, 1);
}
