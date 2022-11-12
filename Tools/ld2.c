/*
	> ld

		�ۑ�����Ă���f�B���N�g���ꗗ��\������B

	> ld NAME

		�ۑ�����Ă���f�B���N�g���ֈړ�����B

	> ld /c

		�ۑ�����Ă���f�B���N�g�����N���A����B

	> ld /c NAME

		�ۑ�����Ă��� NAME ���N���A����B

	> od [NAME]

		�ۑ�����Ă���f�B���N�g�����J���B�ȗ����F�J�����g

	> ldzz
	> ld /cz

		�K���Ȕԍ��ŕۑ����ꂽ�f�B���N�g�� 0 �` 999 ���N���A����B

	----

	�ۑ��� sd [NAME] [DIR] �ōs���B
*/

#include "C:\Factory\Common\all.h"
#include "libs\SaveDir.h"

#define BATCH_FILE "C:\\Factory\\tmp\\LoadDir.bat"

static void ShowList(void)
{
	autoList_t *lines = readLines(SAVE_FILE);
	char *line;
	uint index;

	for (index = 0; index < getCount(lines); index += 2)
		cout("%s %s\n", getLine(lines, index), getLine(lines, index + 1));

	releaseDim(lines, 1);
}
static void LoadDir(char *name)
{
	autoList_t *lines = readLines(SAVE_FILE);
	uint index;

	for (index = 0; index < getCount(lines); index += 2)
		if (!_stricmp(name, getLine(lines, index)))
			break;

	if (index < getCount(lines))
	{
		writeOneLine_cx(BATCH_FILE, xcout("CD /D \"%s\"", getLine(lines, index + 1)));
	}
	else
	{
		ShowList();
	}
	releaseDim(lines, 1);
}
static void OpenDir(char *name)
{
	autoList_t *lines = readLines(SAVE_FILE);
	uint index;

	for (index = 0; index < getCount(lines); index += 2)
		if (!_stricmp(name, getLine(lines, index)))
			break;

	if (index < getCount(lines))
	{
		coExecute_x(xcout("START \"\" \"%s\"", getLine(lines, index + 1)));
	}
	else
	{
		ShowList();
	}
	releaseDim(lines, 1);
}
static void ForgetDir(char *name)
{
	autoList_t *lines = readLines(SAVE_FILE);
	uint index;

	for (index = 0; index < getCount(lines); index += 2)
		if (!_stricmp(name, getLine(lines, index)))
			break;

	if (index < getCount(lines))
	{
		memFree((char *)desertElement(lines, index)); // NAME
		memFree((char *)desertElement(lines, index)); // �f�B���N�g��

		writeLines(SAVE_FILE, lines);

		cout("�폜���܂����B\n");
	}
	releaseDim(lines, 1);
}
int main(int argc, char **argv)
{
	mkAppDataDir();
	createFileIfNotExist(SAVE_FILE);
	removeFileIfExist(BATCH_FILE);

	if (argIs("/C")) // Clear
	{
		if (hasArgs(1))
		{
			ForgetDir(nextArg());
			return;
		}

		// confirm
		{
			cout("�S�ĖY��܂��B\n"
				"���s�H\n"
				);

			if (getKey() == 0x1b)
				termination(0);

			cout("���s�I\n");
		}

		removeFile(SAVE_FILE);
		return;
	}
	if (argIs("/O")) // Open
	{
		if (hasArgs(1))
		{
			OpenDir(nextArg());
		}
		else
		{
			coExecute("START .");
		}
		return;
	}
	if (argIs("/CZ")) // Clear 0 �` 999
	{
		uint n;

		for (n = 0; n <= 999; n++)
		{
			char sn[4];

			sprintf(sn, "%u", n);

			ForgetDir(sn);
		}
		return;
	}

	if (hasArgs(1))
	{
		LoadDir(nextArg());
	}
	else
	{
		ShowList();
	}
}
