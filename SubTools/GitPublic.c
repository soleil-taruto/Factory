#include "C:\Factory\Common\all.h"

#define GIT_ROOTDIR "C:\\huge\\GitHub"
#define MEM_ROOTDIR "C:\\huge\\GitMem"
#define PUB_ROOTDIR "C:\\huge\\GitPub"

static void NormalizeMemRepoDir(char *memRepoDir)
{
	autoList_t *paths = ls(memRepoDir);
	char *path;
	uint index;

	LOGPOS();

	foreach (paths, path, index)
	{
		char *wPath = strx(path);
		char *wLocalPath;

		wLocalPath = getLocal(wPath);

		if (*wLocalPath == '.') // GitMemory の IsGitPath() 対策 -- 通常は無いはずだけど、念の為
		{
			*wLocalPath = '_';
			wPath = toCreatableTildaPath(wPath, IMAX);

			cout("< %s\n", path);
			cout("> %s\n", wPath);

			movePath(path, wPath);
		}
		memFree(wPath);
	}
	releaseDim(paths, 1);

	LOGPOS();
}
static void NormalizeMemDir(char *memDir)
{
	autoList_t *paths = ls(memDir);
	char *path;
	uint index;

	LOGPOS();

	foreach (paths, path, index)
	{
		if (existFile(path))
		{
			cout("!F %s", path); // ここにファイルを置いてはならない。
			semiRemovePath(path);
			continue;
		}

		{
			char *gitDir = combine(GIT_ROOTDIR, getLocal(path));

			cout("G %s\n", gitDir);

			if (!existDir(gitDir))
			{
				cout("!R %s\n", path); // そんなリポジトリありません。
				semiRemovePath(path);

				memFree(gitDir);
				continue;
			}
			memFree(gitDir);
		}

		NormalizeMemRepoDir(path);
	}
	releaseDim(paths, 1);

	LOGPOS();
}
static void MemoryToPublic(void)
{
	autoList_t *memDirs = lsDirs(MEM_ROOTDIR);
	char *memDir;
	uint memDir_index;

	sortJLinesICase(memDirs);

	foreach (memDirs, memDir, memDir_index)
	{
		char *pubFile = combine(PUB_ROOTDIR, getLocal(memDir));

		pubFile = toCreatablePath(pubFile, 1000); // tkt

		cout("< %s\n", memDir);
		cout("> %s\n", pubFile);

		coExecute_x(xcout("C:\\Factory\\Tools\\Cluster.exe /OAD /M \"%s\" \"%s\"", pubFile, memDir));

		LOGPOS();
		memFree(pubFile);
		LOGPOS();
	}
	LOGPOS();
	releaseDim(memDirs, 1);
	LOGPOS();
}
static void PublicToMemory(void)
{
	autoList_t *pubFiles = lsFiles(PUB_ROOTDIR);
	char *pubFile;
	uint pubFile_index;

	sortJLinesICase(pubFiles);

	foreach (pubFiles, pubFile, pubFile_index)
	{
		char *memDir = combine(MEM_ROOTDIR, getLocal(pubFile));

		memDir = toCreatablePath(memDir, 1000); // tkt

		cout("< %s\n", pubFile);
		cout("> %s\n", memDir);

		coExecute_x(xcout("C:\\Factory\\Tools\\Cluster.exe /OAD /R \"%s\" \"%s\"", pubFile, memDir));

		NormalizeMemDir(memDir);

		LOGPOS();
		memFree(memDir);
		LOGPOS();
	}
	LOGPOS();
	releaseDim(pubFiles, 1);
	LOGPOS();
}
int main(int argc, char **argv)
{
	errorCase(!existDir(MEM_ROOTDIR));
	errorCase(!existDir(PUB_ROOTDIR));

	if (argIs("PUSH"))
	{
		MemoryToPublic();
	}
	else if (argIs("PULL"))
	{
		PublicToMemory();
	}
	else
	{
		error_m("不明なコマンド引数");
	}
	LOGPOS();
}
