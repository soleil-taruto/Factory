/*
	display Dev *Commons dir Hash
*/

#include "C:\Factory\Common\all.h"

static autoList_t *CommonsDirNames;
static autoList_t *CommonsDirsList;

static void AddCommonsDirsIfExist(char *projDir)
{
	autoList_t *dirs = slsDirs(projDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		if (endsWithICase(dir, "Commons")) // ? *Commons dir
		{
			char *name = getLocal(dir);
			uint namePos;

			cout("< %s\n", dir);
			cout("> %s\n", name);

			namePos = findLineCase(CommonsDirNames, name, 1);

			if (namePos == getCount(CommonsDirNames))
			{
				addElement(CommonsDirNames, (uint)strx(name));
				addElement(CommonsDirsList, (uint)newList());
			}
			addElement(getList(CommonsDirsList, namePos), (uint)strx(dir));
		}
	}
	releaseDim(dirs, 1);
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
static void ShowCommonsDirs(char *name, autoList_t *commonsDirs)
{
	char *dir;
	uint index;
	char *firstHash = NULL;
	int existDiff = 0;

	cout("\n");
	cout("----\n");
	cout("%s\n", name);

	foreach (commonsDirs, dir, index)
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
static void ShowAllCommonsDirs(void)
{
	char *name;
	uint index;

	foreach (CommonsDirNames, name, index)
	{
		ShowCommonsDirs(name, getList(CommonsDirsList, index));
	}
}
int main(int argc, char **argv)
{
	autoList_t *dirs = slsDirs("C:\\Dev");
	char *dir;
	uint index;

	CommonsDirNames = newList();
	CommonsDirsList = newList();

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
			AddCommonsDirsIfExist(dir);
		}
		else // ? not プロジェクトDIR
		{
			addElements_x(dirs, slsDirs(dir));
		}
		releaseDim(files, 1);
	}
	releaseDim(dirs, 1);

	ShowAllCommonsDirs();
}
