/*
	rm.exe [/F] ...

		/F ... 確認を行わない。

	rm.exe ... /LSS

		最後のファイルリストの全てのファイルを削除する。(lssとの併用を想定)

	rm.exe ... /D [FILE-OR-DIR]

		ファイルまたはディレクトリを削除する。無ければ何もしない。

	rm.exe ... [DIR]

		ディレクトリの直下のファイル・ディレクトリを選択して削除する。
*/

#include "C:\Factory\Common\all.h"

static int ForceMode;

static void RemoveEx(autoList_t *paths)
{
	char *path;
	uint index;
	char *destDir;
	autoList_t *restoreCmds = newList();

	if (!ForceMode)
	{
		foreach (paths, path, index)
		{
			cout("* %s\n", path);
		}
		cout("削除？\n");

		if (clearGetKey() == 0x1b)
			termination(1);
	}
	destDir = makeFreeDir();

	foreach (paths, path, index)
	{
		char *destPath = combine(destDir, getLocal(path));

		destPath = toCreatablePath(destPath, index);

		cout("< %s\n", path);
		cout("> %s\n", destPath);

		if (existDir(path))
		{
			createDir(destPath);
			moveDir(path, destPath);
			removeDir(path);
		}
		else
		{
			moveFile(path, destPath);
		}
		addElement(restoreCmds, (uint)xcout("MOVE \"%s\" \"%s\"", destPath, c_makeFullPath(path)));
		memFree(destPath);
	}

	if (getCount(restoreCmds))
	{
		char *batch = combine(destDir, "_リストア.bat_");

		batch = toCreatablePath(batch, getCount(paths));
		cout("# %s\n", batch);
		writeLines(batch, restoreCmds);
		memFree(batch);
	}
	else
	{
		removeDir(destDir);
	}
	memFree(destDir);
	releaseDim(restoreCmds, 1);
}
static void RemoveExOne(char *path)
{
	uint geb;
	autoList_t gab;

	if (!existPath(path))
	{
		cout("指定されたパスが見つからないので、何もしません。\n");
		return;
	}
	RemoveEx(gndOneElementVar((uint)path, geb, gab));
}
static void SelectRemoveEx(char *dir)
{
	autoList_t *lpaths = ls(dir);

	eraseParents(lpaths);
	lpaths = selectLines_x(lpaths);

	addCwd(dir);
	RemoveEx(lpaths);
	unaddCwd();

	releaseDim(lpaths, 1);
}

int main(int argc, char **argv)
{
	if (argIs("/F"))
	{
		ForceMode = 1;
	}
	if (argIs("/LSS"))
	{
		autoList_t *paths = readLines(FOUNDLISTFILE);

		if (getCount(paths))
		{
			RemoveEx(paths);
		}
		releaseDim(paths, 1);
		return;
	}
	if (argIs("/D"))
	{
		RemoveExOne(hasArgs(1) ? nextArg() : c_dropDirFile());
		return;
	}
	SelectRemoveEx(hasArgs(1) ? nextArg() : c_dropDir());
}
