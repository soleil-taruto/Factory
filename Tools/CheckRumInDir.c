/*
	指定ディレクトリ配下の .rum ディレクトリを全てチェックします。
	デフォルトでは CheckRum.exe を実行しない。

	CheckRumInDir.exe [/S] [/CR [/-S]] ROOT-DIR

		/S  ... サブディレクトリも検索する。
		/CR ... ディレクトリ毎に CheckRum.exe を実行する。
		/-S ... files.txt, tree.txt の（中身の）パスを isFairRelPath() しない。
*/

#include "C:\Factory\Common\all.h"

static int IntoSubDir;
static int DoCheckRum;
static int NoStrict;

static void CollectRumDirs(char *rootDir, autoList_t *dest)
{
	autoList_t *dirs = lsDirs(rootDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		if (!_stricmp("rum", getExt(dir)))
		{
			addElement(dest, (uint)strx(dir));
		}
		else
		{
			if (IntoSubDir)
				CollectRumDirs(dir, dest);
		}
	}
	releaseDim(dirs, 1);
}
static void CheckRum(char *dir)
{
	char *mastDir = changeExt(dir, "");

	cout("%s\n", dir);

	errorCase_m(!existDir(mastDir), "孤立しています。");

	if (DoCheckRum)
	{
		cout("CheckRum.exe を実行します。\n");
		execute_x(xcout("C:\\Factory\\Tools\\CheckRum.exe %s\"%s\"", NoStrict ? "/-S " : "", dir));
		cout("cr_done\n");
	}
	memFree(mastDir);
}
static void CheckRumInDir(char *rootDir)
{
	autoList_t *dirs = newList();
	char *dir;
	uint index;

	rootDir = makeFullPath(rootDir);
	errorCase(!existDir(rootDir));
	CollectRumDirs(rootDir, dirs);

	foreach (dirs, dir, index)
		CheckRum(dir);

	memFree(rootDir);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/S"))
	{
		IntoSubDir = 1;
		goto readArgs;
	}
	if (argIs("/CR"))
	{
		DoCheckRum = 1;
		goto readArgs;
	}
	if (argIs("/-S"))
	{
		NoStrict = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		CheckRumInDir(nextArg());
	}
	else
	{
		char *rootDir = dropDir();

		CheckRumInDir(rootDir);
		memFree(rootDir);
	}

	cout("done\n");
}
