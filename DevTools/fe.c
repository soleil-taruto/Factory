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
				cout("���~���܂����B\n");
				termination(0);
			}
			cout("���~����ɂ̓G�X�P�[�v�L�[�������B\n");
		}
		cmdTitle_x(xcout("fe - %u (%u)", index, getCount(selfiles) - index));
		cout("%s\n", file);
		editTextFile(file);
	}
	releaseDim(files, 1);
	releaseDim(selfiles, 1);
}
