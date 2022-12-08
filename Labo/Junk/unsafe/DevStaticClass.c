#include "C:\Factory\Common\all.h"

static char *GetSlnFile_Dir(char *dir)
{
	autoList_t *files = lsFiles(dir);
	char *file;
	uint index;
	char *slnFile;

	foreach (files, file, index)
		if (!_stricmp("sln", getExt(file)))
			break;

	if (file)
		slnFile = strx(file);
	else
		slnFile = NULL;

	releaseDim(files, 1);
	return slnFile;
}
static char *GetSlnFile(char *csFile)
{
	char *dir = getParent(csFile);

	for (; ; )
	{
		char *slnFile = GetSlnFile_Dir(dir);

		if (slnFile)
		{
			memFree(dir);
			return slnFile;
		}
		errorCase(isAbsRootDir(dir)); // .sln が見つからない。

		dir = getParent_x(dir);
	}
	error(); // never
	return NULL; // dummy
}
static int IsCS2010_SlnFile(char *slnFile)
{
	autoList_t *lines = readLines(slnFile);
	char *line;
	uint index;

	foreach (lines, line, index)
		if (!strcmp(line, "# Visual C# Express 2010"))
			break;

	releaseDim(lines, 1);
	return line != NULL;
}
static int TryBuild(char *csFile) // ret: ? ビルド成功
{
	char *slnFile = GetSlnFile(csFile);
	char *slnDir;
	int ret;

	slnDir = getParent(slnFile);

	addCwd(slnDir);
	{
		coExecute("qq");
		coExecute_x(xcout("MSBuild \"%s\"", getLocal(slnFile)));
		ret = !lastSystemRet;
	}
	unaddCwd();

	memFree(slnFile);
	memFree(slnDir);
	return ret;
}
static void ProcCSFile_Main(char *csFile)
{
	autoList_t *lines = readLines(csFile);
	char *line;
	uint index;
	char *bkFile = addExt(strx(csFile), "bk");
//	char *bkFile = addExt(strx(csFile), "bk.tmp"); // qq で消される。
	int modified = 0;

	errorCase(existPath(bkFile)); // バックアップ・ファイルが既に存在する。
	errorCase(!TryBuild(csFile)); // ビルドできない。

	moveFile(csFile, bkFile);

	foreach (lines, line, index)
	{
		if (
			lineExp("\tpublic class <1,,09AZaz__>", line) ||
			lineExp("\tpublic class <1,,09AZaz__> //<>", line)
			)
		{
			char *lineNew = strx(line);

			lineNew = insertLine(lineNew, 8, "static ");
			setElement(lines, index, (uint)lineNew);
			writeLines(csFile, lines);

			if (TryBuild(csFile))
			{
				memFree(line);
				modified = 1;
			}
			else
			{
				setElement(lines, index, (uint)line);
				memFree(lineNew);
			}
			removeFile(csFile);
		}
	}
	if (modified)
	{
		writeLines(csFile, lines);
		removeFile(bkFile);
	}
	else
	{
		moveFile(bkFile, csFile);
	}
	releaseDim(lines, 1);
	memFree(bkFile);
}
static void ProcCSFile(char *csFile)
{
	char *slnFile = GetSlnFile(csFile);

	if (IsCS2010_SlnFile(slnFile))
	{
		// .dll 必要なやつもあるよ。
		{
			coExecute("newcs /d");

			addCwd("C:\\Dev\\Annex\\ProjectTemplate");
			{
				coExecute("cx **");
			}
			unaddCwd();

			addCwd("C:\\Dev\\Main2\\Satellite");
			{
				coExecute("cx **");
			}
			unaddCwd();

			addCwd("C:\\Dev\\Riot\\Lunarwalk\\StarBeat\\StarBeat");
			{
				coExecute("cx **");
			}
			unaddCwd();
		}

		ProcCSFile_Main(csFile);
	}
	memFree(slnFile);
}
static void Main2(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		if (
			!_stricmp("cs", getExt(file)) &&
			!mbs_stristr(file, "\\Properties\\") &&
			!endsWithICase(file, ".Designer.cs")
			)
			ProcCSFile(file);
	}
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		Main2(nextArg());
	}
	else
	{
		Main2("C:\\Dev");
	}
}
