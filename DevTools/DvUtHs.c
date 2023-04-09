/*
	display Dev Utilities file Hash

	相違を発見した場合、最初に発見した相違のあるファイルセットを FOUNDLISTFILE へ書き出す。
*/

#include "C:\Factory\Common\all.h"

static autoList_t *UtilitiesFileNames;
static autoList_t *UtilitiesFilesList;
static int ExistDiffOverall;

static void AddUtilitiesFile(char *file)
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
static void AddUtilitiesFile_IfExist(char *dir, char *name)
{
	char *file = combine(dir, name);

	if (existFile(file))
	{
		AddUtilitiesFile(file);
	}
	memFree(file);
}
static void AddUtilitiesFiles_UtDir(char *utDir)
{
	autoList_t *files = slsFiles(utDir);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		AddUtilitiesFile(file);
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

	AddUtilitiesFile_IfExist(projDir, "Extensions.cs");
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
static void DiffFileSetToFoundFileList(autoList_t *utFiles)
{
	writeLines(FOUNDLISTFILE, utFiles);
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

		if (!ExistDiffOverall)
		{
			DiffFileSetToFoundFileList(utFiles);
			ExistDiffOverall = 1;
		}
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

	if (ExistDiffOverall)
	{
		cout("\n");
		cout("#####################\n");
		cout("## 1件以上相違あり ##\n");
		cout("#####################\n");
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
