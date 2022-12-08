/*
	rlss.exe [/D | /F] [SEARCH-PTN | TARGET-DIR SEARCH-PTN...]
*/

#include "C:\Factory\Common\all.h"

#define RUM_DIR_EXT "rum"
#define DIR_REVISIONS "revisions"
#define FILE_FILES "files.txt"
#define FILE_TREE "tree.txt"

static int DirFileMode;
static autoList_t *FoundRumDirs;
static int FoundFlag;

static int IsMatch(char *relPath, autoList_t *searchPtns)
{
	char *path_a = xcout("%s*", relPath);
	char *searchPtn;
	uint index;
	int found = 0;

	if (getCount(searchPtns))
	{
		foreach (searchPtns, searchPtn, index)
		{
			if (mbs_stristr(path_a, searchPtn))
			{
				found = 1;
				break;
			}
		}
	}
	else
	{
		found = 1;
	}
	memFree(path_a);
	return found;
}
static void DispList_RelPath(char *rumDir, char *revision, char *relPath, autoList_t *searchPtns)
{
	if (IsMatch(relPath, searchPtns))
	{
		cout("%s<%s>%s\n", rumDir, revision, relPath);
		FoundFlag = 1;
	}
}
static void DispList_RumDir(char *rumDir, autoList_t *searchPtns)
{
	char *revsDir = combine(rumDir, DIR_REVISIONS);
	autoList_t *revDirs;
	char *revDir;
	uint index;

	FoundFlag = 0;

	revDirs = lsDirs(revsDir);
	sortJLinesICase(revDirs);

	foreach (revDirs, revDir, index)
	{
		char *revision = getLocal(revDir);

		if (DirFileMode != 'D') // ディレクトリを表示
		{
			char *treeFile = combine(revDir, FILE_TREE);
			autoList_t *lines;
			char *line;
			uint index;

			lines = readLines(treeFile);

			foreach (lines, line, index)
				DispList_RelPath(rumDir, revision, line, searchPtns);

			releaseDim(lines, 1);
			memFree(treeFile);
		}
		if (DirFileMode != 'F') // ファイルを表示
		{
			char *filesFile = combine(revDir, FILE_FILES);
			autoList_t *lines;
			char *line;
			uint index;

			lines = readLines(filesFile);

			foreach (lines, line, index)
			{
				errorCase(!lineExp("<32,09AFaf> <>", line));
				DispList_RelPath(rumDir, revision, line + 33, searchPtns);
			}
			releaseDim(lines, 1);
			memFree(filesFile);
		}
	}
	memFree(revsDir);
	releaseDim(revDirs, 1);

	if (FoundFlag)
		addElement(FoundRumDirs, (uint)strx(rumDir));
}
static void DispList(char *targetDir, autoList_t *searchPtns)
{
	targetDir = makeFullPath(targetDir);

	if (!_stricmp(RUM_DIR_EXT, getExt(targetDir)))
	{
		DispList_RumDir(targetDir, searchPtns);
	}
	else
	{
		autoList_t *dirs = lssDirs(targetDir);
		char *dir;
		uint index;

		foreach (dirs, dir, index)
			if (!_stricmp(RUM_DIR_EXT, getExt(dir)))
				DispList_RumDir(dir, searchPtns);

		releaseDim(dirs, 1);
	}
	memFree(targetDir);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/D"))
	{
		DirFileMode = 'D';
		goto readArgs;
	}
	if (argIs("/F"))
	{
		DirFileMode = 'F';
		goto readArgs;
	}

	FoundRumDirs = newList();

	if (hasArgs(2))
	{
		DispList(getArg(0), getFollowArgs(1));
	}
	else
	{
		DispList(".", allArgs());
	}

	writeLines(FOUNDLISTFILE, FoundRumDirs);
}
