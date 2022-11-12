#include "all.h"

/*
	�p�X�̔�r -> mbs_stricmp

	���d���b�N�\

	�p�X�̐���
		�󕶎���͕s��
*/
#define LOCKED_FILE "C:\\Factory\\tmp\\Hemachi_FileShare_LockedPathList.txt"
#define LOCKED_NUMMAX 10000
#define PATH_LENMAX 300

int HFS_NM_LockPath(char *path, int checkOnlyMode) // ? ���b�N�\ || ���b�N����
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
	else if (LOCKED_NUMMAX <= getCount(lines)) // ? try lock mode + overflow (���b�N�s��)
	{
		cout("����ȏネ�b�N�ł��܂���B\n");
		retval = 0;
	}
	else // ? unlocked -> lock
	{
		cout("���b�N���܂��B\n");
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
			cout("�A�����b�N���܂��B\n");
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
