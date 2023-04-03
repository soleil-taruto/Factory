/*
	display Dev Utilities file Hash
*/

#include "C:\Factory\Common\all.h"

static autoList_t *UtilitiesFileNames;
static autoList_t *UtilitiesFilesList;

static void AddUtilitiesFiles_UtDir(char *utDir)
{
	autoList_t *files = slsFiles(utDir);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		char *name = getLocal(file);
		uint namePos;

		cout("< %s\n", file);
		cout("> %s\n", name);

		namePos = findLineCase(UtilitiesFileNames, name, 1);

		if (namePos == getCount(UtilitiesFileNames))
		{
			addElement(UtilitiesFileNames, (uint)strx(name));
			addElement(UtilitiesFilesList, (uint)newList());
		}
		addElement(getList(UtilitiesFilesList, namePos), (uint)strx(file));
	}
	releaseDim(files, 1);
}
static int IsUtilitiesDir(char *dir)
{
	return !_stricmp(getLocal(dir), "Utilities");
}
static void AddUtilitiesFiles_ProjDir(char *projDir)
{
	autoList_t *dirs = slsDirs(projDir);
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		char *name = getLocal(dir);

		if (IsUtilitiesDir(dir))
		{
			AddUtilitiesFiles_UtDir(dir);
		}
	}
	releaseDim(dirs, 1);
}
static char *GetFileMD5(char *file)
{
	char *rdrFile = makeTempPath("tmp");
	char *hash;

	execute_x(xcout("C:\\Factory\\Tools\\md5.exe \"%s\" > \"%s\"", file, rdrFile));

	hash = readFirstLine(rdrFile);

	errorCase(!lineExp("<32,09af>", hash));

	removeFile(rdrFile);
	memFree(rdrFile);

	return hash;
}
static void ShowUtilitiesFiles(char *name, autoList_t *utFiles)
{
	char *file;
	uint index;
	char *firstHash = NULL;
	int existDiff = 0;

	cout("\n");
	cout("----\n");
	cout("%s\n", name);

	foreach (utFiles, file, index)
	{
		char *hash = GetFileMD5(file);

		cout("%s %s\n", hash, file);

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
static void ShowAllUtilitiesFiles(void)
{
	char *name;
	uint index;

	foreach (UtilitiesFileNames, name, index)
	{
		ShowUtilitiesFiles(name, getList(UtilitiesFilesList, index));
	}
}
int main(int argc, char **argv)
{
	autoList_t *dirs = slsDirs("C:\\Dev");
	char *dir;
	uint index;

	UtilitiesFileNames = newList();
	UtilitiesFilesList = newList();

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
			AddUtilitiesFiles_ProjDir(dir);
		}
		else // ? not プロジェクトDIR
		{
			addElements_x(dirs, slsDirs(dir));
		}
		releaseDim(files, 1);
	}
	releaseDim(dirs, 1);

	ShowAllUtilitiesFiles();
}
