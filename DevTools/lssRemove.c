#include "C:\Factory\Common\all.h"

static int ForceMode;

static void RemoveEx(autoList_t *paths)
{
	char *path;
	uint index;
	char *destDir;

	if (!ForceMode)
	{
		foreach (paths, path, index)
		{
			cout("* %s\n", path);
		}
		cout("çÌèúÅH\n");

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
		memFree(destPath);
	}
	memFree(destDir);
}
int main(int argc, char **argv)
{
	autoList_t *files;

	if (argIs("/F"))
	{
		ForceMode = 1;
	}

	files = readLines(FOUNDLISTFILE);
	RemoveEx(files);
	releaseDim(files, 1);
}
