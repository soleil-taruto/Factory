/*
	GitFactory.exe [/OW] [[入力ディレクトリ] 出力ディレクトリ]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"
#include "libs\GitCommon.h"
#include "libs\GitResourceMask.h"
#include "libs\GitSourceFilter.h"
#include "libs\GitSourceMask.h"
#include "libs\GitJapanesePath.h"

#define IGNORE_FILE "_gitignore"

static void CleanupGitDir(char *dir)
{
	autoList_t *paths;
	char *path;
	uint index;

	addCwd(dir);
	paths = ls(".");
	eraseParents(paths);

	foreach (paths, path, index)
	{
		if (
			!_stricmp(path, ".git") ||
			!_stricmp(path, ".gitattributes") ||
			!_stricmp(path, ".gitignore")
			)
			continue;

		errorCase(path[0] == '.'); // ? 新しい git のファイルかしら？

		recurRemovePath(path);
	}
	releaseDim(paths, 1);
	unaddCwd();
}

static void FoundIgnoreFile(autoList_t *files, char *ignFile)
{
	char *ignPtn;
	char *ignPtnYen;
	char *file;
	uint index;

	ignPtn = changeLocal(ignFile, "");
	ignPtnYen = addChar(strx(ignPtn), '\\');

	foreach (files, file, index)
	{
		if (
#if 0 // フォルダごと消す。
			!_stricmp(file, ignPtn) ||
			startsWithICase(file, ignPtnYen)
#else // _gitignore だけ残す。
			startsWithICase(file, ignPtnYen) &&
			_stricmp(file, ignFile)
#endif
			)
		{
			cout("IGNORE: %s\n", file);

			file[0] = '\0';
		}
	}
	memFree(ignPtn);
	memFree(ignPtnYen);
}
static void RemoveIgnoreFiles(autoList_t *files)
{
	char *file;
	uint index;

	foreach (files, file, index)
		if (!_stricmp(getLocal(file), IGNORE_FILE))
			FoundIgnoreFile(files, file);

	trimLines(files);
}

static int GF_SE_IsSJISFile(char *file)
{
	char *text = readText(file);
	char *text2;
	int ret;

	text2 = strx(text);
	line2JLine(text2, 1, 1, 1, 1);

	ret = !strcmp(text, text2);

	memFree(text);
	memFree(text2);

	cout("GF_SE_IsSJISFile_ret: %d\n", ret);

	return ret;
}
static void SolveEncoding(char *rootDir)
{
	autoList_t *files = lssFiles(rootDir);
	char *file;
	uint index;

	LOGPOS();

	RemoveGitPaths(files);

	foreach (files, file, index)
	{
		char *localFile = getLocal(file);

		if (!_stricmp("txt", getExt(localFile)))
//		if (!_stricmp("Readme.txt", localFile) || !_stricmp("_source.txt", localFile)) // old
		{
			cout("SE_file: %s\n", file);

			if (GF_SE_IsSJISFile(file))
			{
				SJISToUTF8File(file, file);
			}
		}
	}
	releaseDim(files, 1);

	LOGPOS();
}

static void SolveEmptyDir(char *rootDir)
{
	autoList_t *dirs = lssDirs(rootDir);
	char *dir;
	uint index;

	LOGPOS();

	RemoveGitPaths(dirs);

	foreach (dirs, dir, index)
	{
		if (!lsCount(dir)) // ? 空のDIR
		{
			char *dmyfile = combine(dir, "_this-folder-is-empty");

			cout("SED_dmyfile: %s\n", dmyfile);

			createFile(dmyfile);
			memFree(dmyfile);
		}
	}
	releaseDim(dirs, 1);

	LOGPOS();
}

static void GitFactory(char *rDir, char *wDir, int allowOverwrite)
{
	autoList_t *files;
	char *file;
	uint index;

	errorCase(m_isEmpty(rDir));
	errorCase(m_isEmpty(wDir));

	rDir = makeFullPath(rDir);
	wDir = makeFullPath(wDir);

	errorCase(!existDir(rDir));

	if (existPath(wDir))
	{
		errorCase(!allowOverwrite);
		CleanupGitDir(wDir);
	}
	else
	{
		error(); // zantei
//		createPath(wDir, 'D');
	}

	antiSubversion = 1;
	files = lss(rDir);
	changeRoots(files, rDir, NULL);

	foreach (files, file, index)
		errorCase_m(startsWithICase(file, ".git"), ".gitで始まる何かがあります。");

	RemoveIgnoreFiles(files);
	sortJLinesICase(files);

	foreach (files, file, index)
	{
		char *rFile = combine(rDir, file);
		char *wFile = combine(wDir, file);

		if (existDir(rFile))
		{
			createDir(wFile);
		}
		else
		{
			if (startsWithICase(file, "tmp\\"))
				goto skipfile;

			if (!_stricmp("obj", getExt(file)))
				goto skipfile;

			if (!_stricmp("exe", getExt(file)))
				goto skipfile;

			copyFile(rFile, wFile);
		}

		if (!_stricmp(getLocal(wFile), IGNORE_FILE))
		{
			char *dmyfile = changeLocal(wFile, "*");

			PostGitIgnoreFile(dmyfile);
			memFree(dmyfile);
		}

	skipfile:
		memFree(rFile);
		memFree(wFile);
	}

#if 0 // SolveEmptyDir があるから要らない。
	{
		char *dir = combine(wDir, "tmp");

		if (existDir(dir))
		{
			file = combine(dir, "1.tmp");
			createFile(file);
			memFree(file);
		}
		memFree(dir);
	}
#endif

	GitResourceMask(wDir);
	GitSourceFilter(wDir);
	GitSourceMask(wDir);
	SolveEncoding(wDir);
	SolveJapanesePath(wDir);
	SolveEmptyDir(wDir);

	memFree(rDir);
	memFree(wDir);
	releaseDim(files, 1);
}

int main(int argc, char **argv)
{
	int allowOverwrite = 0;

readArgs:
	if (argIs("/OW"))
	{
		allowOverwrite = 1;
		goto readArgs;
	}

	if (hasArgs(2))
	{
		GitFactory(getArg(0), getArg(1), allowOverwrite);
		return;
	}
	if (hasArgs(1))
	{
		GitFactory("C:\\Factory", nextArg(), allowOverwrite);
		return;
	}
	GitFactory("C:\\Factory", "C:\\huge\\GitHub\\Factory", 1);
}
