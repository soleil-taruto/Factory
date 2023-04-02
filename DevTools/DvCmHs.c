/*
	display Dev *Commons-s Hash
*/

#include "C:\Factory\Common\all.h"

static autoList_t *CommonsDirs;
static autoList_t *GUICommonsDirs;

static void AddToCommonDirsIfExist(autoList_t *commonDirs, char *name)
{
	char *dir = makeFullPath(name);

	if (existDir(dir))
	{
		cout("+ %s\n", dir);

		addElement(commonDirs, (uint)strx(dir));
	}
	memFree(dir);
}
static char *GetDirMD5(char *dir)
{
	char *rdrFile = makeTempPath("tmp");
	char *hash;

	execute_x(xcout("C:\\Factory\\Tools\\dmd5.exe /S \"%s\" > \"%s\"", dir, rdrFile));

	hash = readFirstLine(rdrFile);

	errorCase(!lineExp("<32,09af>", hash));

	removeFile(rdrFile);
	memFree(rdrFile);

	return hash;
}
static void ShowCommonDirs(autoList_t *commonDirs, char *name)
{
	char *dir;
	uint index;
	char *firstHash = NULL;
	int existDiff = 0;

	cout("\n");
	cout("----\n");
	cout("%s\n", name);

	foreach (commonDirs, dir, index)
	{
		char *hash = GetDirMD5(dir);

		cout("%s %s\n", hash, dir);

		if (firstHash)
			existDiff |= _stricmp(firstHash, hash);
		else
			firstHash = strx(hash);

		memFree(hash);
	}
	memFree(firstHash);

	if (existDiff)
	{
		cout("+----------+\n");
		cout("| 相違あり |\n");
		cout("+----------+\n");
	}
}
int main(int argc, char **argv)
{
	autoList_t *dirs = slsDirs("C:\\Dev");
	char *dir;
	uint index;

	CommonsDirs     = newList();
	GUICommonsDirs  = newList();

	foreach (dirs, dir, index)
	{
		autoList_t *files = lsFiles(dir);
		char *file;
		uint index;

		foreach (files, file, index)
			if (!_stricmp(getExt(file), "csproj"))
				break;

		if (index < getCount(files)) // ? プロジェクトDIR
		{
			addCwd(dir);
			{
				AddToCommonDirsIfExist(CommonsDirs, "Commons");
				AddToCommonDirsIfExist(GUICommonsDirs, "GUICommons");
			}
			unaddCwd();
		}
		else // ? not プロジェクトDIR
		{
			addElements_x(dirs, slsDirs(dir));
		}
		releaseDim(files, 1);
	}
	releaseDim(dirs, 1);

	ShowCommonDirs(CommonsDirs, "Commons");
	ShowCommonDirs(GUICommonsDirs, "GUICommons");
}
