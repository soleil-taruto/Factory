#include "all.h"

/*
	パスの比較 -> mbs_stricmp

	多重ロック可能

	パスの制限
		空文字列は不可
*/
#define LOCKED_FILE "C:\\Factory\\tmp\\Hemachi_FileShare_LockedPathList.txt"
#define LOCKED_NUMMAX 10000
#define PATH_LENMAX 300

int HFS_NM_LockPath(char *path, int checkOnlyMode) // ? ロック可能 || ロックした
{
	autoList_t *lines;
	char *line;
	uint index;
	int retval;
	int alreadyLocked;

	cout("HFS_NM_LockPath() Start\n");
	cout("path: %s\n", path);
	cout("checkOnlyMode: %d\n", checkOnlyMode);

	errorCase(PATH_LENMAX < strlen(path));

	createFileIfNotExist(LOCKED_FILE);
	lines = readLines(LOCKED_FILE);
	cout("lines_num: %u\n", getCount(lines));

	foreach (lines, line, index)
		if (!mbs_stricmp(line, path))
			break;

	releaseDim(lines, 1);
	alreadyLocked = line ? 1 : 0;
	cout("alreadyLocked: %d\n", alreadyLocked);

	if (checkOnlyMode)
	{
		retval = !alreadyLocked;
	}
	else if (LOCKED_NUMMAX <= getCount(lines)) // ? try lock mode + overflow (ロック不可)
	{
		cout("これ以上ロックできません。\n");
		retval = 0;
	}
	else // ? unlocked -> lock
	{
		cout("ロックします。\n");
		addLine2File(LOCKED_FILE, path);
		retval = 1;
	}
	cout("HFS_NM_LockPath() End %d\n", retval);
	return retval;
}
void HFS_NM_UnlockPath(char *path)
{
	autoList_t *lines;
	char *line;
	uint index;

	cout("HFS_NM_UnlockPath() Start\n");
	cout("path: %s\n", path);

	createFileIfNotExist(LOCKED_FILE);
	lines = readLines(LOCKED_FILE);
	cout("lines_num: %u\n", getCount(lines));

	foreach (lines, line, index)
	{
		if (!mbs_stricmp(line, path))
		{
			cout("アンロックします。\n");
			line[0] = '\0';
			break;
		}
	}
	trimLines(lines);
	writeLines(LOCKED_FILE, lines);
	releaseDim(lines, 1);

	cout("HFS_NM_UnlockPath() End\n");
}

int HFS_LockPath(char *path, int checkOnlyMode) // ? locked
{
	int retval;

	HFS_MutexEnter();
	{
		retval = HFS_NM_LockPath(path, checkOnlyMode);
	}
	HFS_MutexLeave();

	return retval;
}
void HFS_UnlockPath(char *path)
{
	HFS_MutexEnter();
	{
		HFS_NM_UnlockPath(path);
	}
	HFS_MutexLeave();
}
void HFS_UnlockAllPath(void)
{
	cout("HFS_UnlockAllPath() Start\n");

	HFS_MutexEnter();
	{
		removeFileIfExist(LOCKED_FILE);
	}
	HFS_MutexLeave();

	cout("HFS_UnlockAllPath() End\n");
}
