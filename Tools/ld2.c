/*
	> ld

		保存されているディレクトリ一覧を表示する。

	> ld NAME

		保存されているディレクトリへ移動する。

	> ld /c

		保存されているディレクトリをクリアする。

	> ld /c NAME

		保存されている NAME をクリアする。

	> od [NAME]

		保存されているディレクトリを開く。省略時：カレント

	> ldzz
	> ld /cz

		適当な番号で保存されたディレクトリ 0 ～ 999 をクリアする。

	----

	保存は sd [NAME] [DIR] で行う。
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
		memFree((char *)desertElement(lines, index)); // ディレクトリ

		writeLines(SAVE_FILE, lines);

		cout("削除しました。\n");
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
			cout("全て忘れます。\n"
				"続行？\n"
				);

			if (getKey() == 0x1b)
				termination(0);

			cout("続行！\n");
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
	if (argIs("/CZ")) // Clear 0 ～ 999
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
