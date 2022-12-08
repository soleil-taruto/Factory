/*
	使い方

		lss [...]
		lssColl [/K]

	例) C:\Dev 配下の *.cs のみを階層を維持して抽出する。

		CD /D C:\Dev
		lss /e .cs*
		lssColl /K
*/

#include "C:\Factory\Common\all.h"

static void Collect(int keepHierarchy)
{
	autoList_t *paths = readLines(FOUNDLISTFILE);
	char *path;
	uint index;
	char *outDir = makeFreeDir();

	foreach (paths, path, index)
	{
		char *outPath;

		if (keepHierarchy)
		{
			outPath = combine_cx(outDir, lineToFairRelPath(path, strlen(outDir)));
			createPath(outPath, 'X');
		}
		else
		{
			outPath = toCreatablePath(combine(outDir, getLocal(path)), index + 10); // index + margin
		}

		cout("< %s\n", path);
		cout("> %s\n", outPath);

		copyPath(path, outPath);
		memFree(outPath);
	}
	execute_x(xcout("START %s", outDir));
	memFree(outDir);
}
int main(int argc, char **argv)
{
	Collect(argIs("/K"));
}
