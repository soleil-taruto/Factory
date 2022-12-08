#include "C:\Factory\Common\all.h"

#define FILE_LASTREPLACELIST "C:\\Factory\\tmp\\LastReplaceList.txt"

static int AntiInvalidPaths(autoList_t *paths)
{
	char *path;
	uint index;
	uint inccnt = 0;

	foreach (paths, path, index)
	{
		autoList_t *ptkns;
		char *ptkn;
		uint tndx;
		char *newPath;

		escapeYen(path);
		ptkns = tokenize(path, '/');
		restoreYen(path);

		foreach (ptkns, ptkn, tndx)
		{
			ptkn = lineToFairLocalPath_x(ptkn, 0);
			setElement(ptkns, tndx, (uint)ptkn);
		}
		newPath = untokenize(ptkns, "\\");

		if (!*newPath)
		{
			newPath = addChar(newPath, '_');
		}
		if (strcmp(path, newPath))
		{
			cout("< %s\n", path);
			cout("> %s\n", newPath);
			memFree(path);
			setElement(paths, index, (uint)newPath);
			inccnt++;
		}
		releaseDim(ptkns, 1);
	}
	return inccnt;
}
static int AntiSamePaths(autoList_t *paths)
{
	char *path;
	uint index;
	uint inccnt = 0;

	while (index = findPair(paths, (sint (*)(uint, uint))mbs_stricmp))
	{
		path = getLine(paths, index);
		cout("< %s\n", path);
		path = incrementPath(path);
		cout("> %s\n", path);
		setElement(paths, index, (uint)path);
		inccnt++;
	}
	return inccnt;
}
static void CleanupDirEx(char *dir)
{
	cout("clean-up dir...\n");
	mutex();
	execute_x(xcout("RD /S /Q \"%s\"", dir));
	cout("rm dir ok.\n");
	createDir(dir);
	cout("create dir ok.\n");
	unmutex();
	cout("clean-up dir ok!\n");
}
static int FindTooLongPaths(autoList_t *paths, uint rootLen)
{
	char *path;
	uint index;
	uint fndcnt = 0;

	foreach (paths, path, index)
	{
		if (PATH_SIZE < rootLen + 1 + strlen(path))
		{
			cout("* %s\n", path);
			fndcnt++;
		}
	}
	return fndcnt;
}

static void RenameTestEx(char *dir)
{
	char *escDir = makeTempDir(NULL);
	char *tmpDir;

	cout("escDir: %s\n", escDir);

	tmpDir = strx(dir);
	tmpDir = toCreatablePath(tmpDir, 10000);
	cout("tmpDir: %s\n", tmpDir);

	addCwd(escDir);

	cout("rename start...\n");
	moveFile(dir, tmpDir);
	cout("rename 1 ok!\n");
	moveFile(tmpDir, dir);
	cout("rename 2 ok!\n");

	unaddCwd();
	removeDir(escDir);

	memFree(escDir);
	memFree(tmpDir);
}
static void CreatePathCascade(char *path)
{
	autoList_t *ptkns;
	char *ptkn;
	uint index;

	escapeYen(path);
	ptkns = tokenize(path, '/');
	restoreYen(path);

	memFree((void *)unaddElement(ptkns));

	addCwd(".");
	foreach (ptkns, ptkn, index)
	{
		mkdirEx(ptkn);
		changeCwd(ptkn);
	}
	unaddCwd();

	releaseDim(ptkns, 1);
}
static int Confirm(void)
{
	cout("Press P to continue.\n");

	for (; ; )
	{
		int key = getKey();

		if (key == 0x1b) return 0;
		if (key == 'P') return 1;
	}
}

static void ReplaceEx(void)
{
	char *rootDir = getCwd();
	char *midDir = makeFreeDir();
	autoList_t *paths = lss(".");
	autoList_t *newPaths;
	autoList_t *tmpPaths;
	char *path;
	char *newPath;
	uint index;

	changeRoots(paths, rootDir, NULL);
	sortJLinesICase(paths);
	newPaths = copyLines(paths);

	for (; ; )
	{
		tmpPaths = editTextLines(newPaths);
		releaseDim(newPaths, 1);
		newPaths = tmpPaths;

		if (getCount(newPaths) == 0)
		{
			goto endfunc;
		}
		errorCase(getCount(paths) != getCount(newPaths));

		if (AntiInvalidPaths(newPaths) == 0 && AntiSamePaths(newPaths) == 0 && FindTooLongPaths(newPaths, strlen(rootDir)) == 0)
		{
			break;
		}
	}
	writeLines(FILE_LASTREPLACELIST, newPaths);
	cout("save path-list ok!\n");

	foreach (paths, path, index)
	{
		newPath = getLine(newPaths, index);

		if (strcmp(path, newPath))
		{
			cout("< %s\n", path);
			cout("> %s\n", newPath);
		}
	}
	if (!Confirm())
		goto endfunc;

	addCwd(midDir);
	foreach (paths, path, index) // ˆÚ“®ƒeƒXƒg
	{
		newPath = getLine(newPaths, index);

		cout("< %s\n", path);
		cout("? %s\n", newPath);

		CreatePathCascade(newPath);
		if (existFile(path))
		{
			errorCase(existFile(newPath));
			createFile(newPath);
		}
		else
		{
			createDir(newPath);
		}
	}
	unaddCwd();
	cout("move test ok.\n");

	RenameTestEx(rootDir);
	cout("rename test ok.\n");

	CleanupDirEx(midDir);
	cout("clean-up dir ok.\n");

	addCwd(midDir);
	foreach (paths, path, index)
	{
		newPath = getLine(newPaths, index);

		cout("< %s\n", path);
		cout("> %s\n", newPath);

		path = combine(rootDir, path);

		CreatePathCascade(newPath);
		if (existFile(path))
			moveFile(path, newPath);
		else
			createDir(newPath);

		memFree(path);
	}
	cout("move files ok.\n");

	CleanupDirEx(rootDir);
	unaddCwd();
	moveDir(midDir, ".");
	cout("move dir ok.\n");

endfunc:
	removeDir(midDir);

	releaseDim(paths, 1);
	releaseDim(newPaths, 1);
	memFree(rootDir);
	memFree(midDir);
}
static void ReplaceExDir(char *dir)
{
	addCwd(dir);
	ReplaceEx();
	unaddCwd();
}

int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		ReplaceExDir(nextArg());
	}
	else
	{
		ReplaceExDir(c_dropDir());
	}
}
