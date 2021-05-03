/*
	RemoveHiddenFiles.exe [/B] [対象DIR]
*/

#include "C:\Factory\Common\all.h"

static int BatchMode;

static void RemoveHiddenFiles(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;

	sortJLinesICase(files);

	foreach(files, file, index)
	{
		if(getFileAttr_Hidden(file)) // ? 隠しファイル -> 削除対象
		{
			cout("%s\n", file);
		}
		else
		{
			*file = '\0'; // 除外
		}
	}
	trimLines(files);

	if(BatchMode)
	{
		cout("続行します。(バッチモード)\n");
	}
	else
	{
		// Confirm
		{
			cout("これら隠しファイルを削除します。\n");
			cout("続行？\n");

			if(clearGetKey() == 0x1b)
				termination(0);

			cout("続行します。\n");
		}
	}

	foreach(files, file, index)
		semiRemovePath(file);

	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	BatchMode = argIs("/B");

	if(hasArgs(1))
	{
		RemoveHiddenFiles(nextArg());
	}
	else
	{
		RemoveHiddenFiles(dropDir()); // g
	}
}
