#include "C:\Factory\Common\all.h"

static int ExtFltrMode;

static void ExtFltr(char *path)
{
	char *ext = getExt(path);

	if (lineExp("<1,,09AZaz>_", ext))
	{
		char *newPath = strx(path);

		strchr(newPath, '\0')[-1] = '\0';

		cout("< %s\n", path);
		cout("> %s\n", newPath);

		moveFile(path, newPath);
		memFree(newPath);
	}
}
static void Main2(char *dir)
{
	autoList_t *paths = lss(dir);
	char *path;
	uint index;

	reverseElements(paths);

	foreach (paths, path, index)
	{
		if (ExtFltrMode)
		{
			ExtFltr(path);
		}
	}
	releaseDim(paths, 1);
}
int main(int argc, char **argv)
{
	char *dir;

readArgs:
	if (argIs("/EF"))
	{
		ExtFltrMode = 1;
		goto readArgs;
	}

	dir = nextArg();

	/*
		オプションを間違えた可能性 -> 念のため error
	*/
	errorCase(dir[0] == '/');
	errorCase(hasArgs(1));

	dir = makeFullPath(dir);
	Main2(dir);
	memFree(dir);
}
