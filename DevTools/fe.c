#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *files = readLines(FOUNDLISTFILE);
	autoList_t *selfiles;
	char *file;
	uint index;

	selfiles = selectLines(files);

	foreach (selfiles, file, index)
	{
		while (hasKey())
		{
			if (getKey() == 0x1b)
			{
				cout("中止しました。\n");
				termination(0);
			}
			cout("中止するにはエスケープキーを押す。\n");
		}
		cmdTitle_x(xcout("fe - %u (%u)", index, getCount(selfiles) - index));
		cout("%s\n", file);
		editTextFile(file);
	}
	releaseDim(files, 1);
	releaseDim(selfiles, 1);
}
