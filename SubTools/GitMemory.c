#include "C:\Factory\Common\all.h"

#define GIT_ROOTDIR "C:\\huge\\GitHub"
#define MEM_ROOTDIR "C:\\huge\\GitMem"
#define RUM_ROOTDIR "C:\\huge\\GitRum"

static uint EndCode = 0;

static int IsGitPath(char *path)
{
	char *localPath = getLocal(path);

	return localPath[0] == '.'; // '.' で始まっているファイルは全てgitのファイルと見なす。
}
static void MemoryMain_MD(char *memDir)
{
	autoList_t *repoDirs = lsDirs(GIT_ROOTDIR);
	char *repoDir;
	uint repoDir_index;

	sortJLinesICase(repoDirs);

	foreach (repoDirs, repoDir, repoDir_index)
	{
		char *memRepoDir = combine(memDir, getLocal(repoDir));
		autoList_t *paths;
		char *path;
		uint index;

		cout("< %s\n", repoDir);
		cout("> %s\n", memRepoDir);
		createDir(memRepoDir);

		paths = ls(repoDir);
		sortJLinesICase(paths);

		foreach (paths, path, index)
		{
			char *wPath = combine(memRepoDir, getLocal(path));

			cout("<# %s\n", path);
			cout("># %s\n", wPath);

			if (IsGitPath(path))
			{
				cout("ignore git file!\n");
			}
			else if (existDir(path))
			{
				createDir(wPath);
				copyDir(path, wPath);
			}
			else
			{
				copyFile(path, wPath);
			}
			LOGPOS();
			memFree(wPath);
			LOGPOS();
		}
		LOGPOS();
		releaseDim(paths, 1);
		memFree(memRepoDir);
		LOGPOS();
	}
	LOGPOS();
	releaseDim(repoDirs, 1);
	LOGPOS();
}
static void MemoryMain(void)
{
	char *memDir = combine_cx(MEM_ROOTDIR, addLine(makeCompactStamp(NULL), "00"));

	memDir = toCreatablePath(memDir, 99); // 秒間 100 件超えは想定しない。
	cout("memDir: %s\n", memDir);
	createDir(memDir);

	MemoryMain_MD(memDir);

	LOGPOS();
	memFree(memDir);
	LOGPOS();
}
static void RumMain(void)
{
	LOGPOS();
	recurClearDir(RUM_ROOTDIR);
	LOGPOS();
	MemoryMain_MD(RUM_ROOTDIR);
	LOGPOS();
}
static void FlushMain(void)
{
	autoList_t *memDirs = lsDirs(MEM_ROOTDIR);

	if (getCount(memDirs) == 0)
	{
		cout("no more memory!\n");
		EndCode = 1;
	}
	else
	{
		char *memDir;
		autoList_t *memRepoDirs;
		char *memRepoDir;
		uint memRepoDir_index;

		sortJLinesICase(memDirs);
		memDir = getLine(memDirs, 0); // 辞書順で最初のフォルダを処理する。
		cout("memDir: %s\n", memDir);

		memRepoDirs = lsDirs(memDir);
		sortJLinesICase(memRepoDirs);

		foreach (memRepoDirs, memRepoDir, memRepoDir_index)
		{
			char *repoDir = combine(GIT_ROOTDIR, getLocal(memRepoDir));
			autoList_t *paths;
			char *path;
			uint index;

			cout("< %s\n", memRepoDir);
			cout("> %s\n", repoDir);

			errorCase(!existDir(repoDir)); // HACK: リポジトリを削除することは無いという前提、、、

			paths = ls(repoDir);
			sortJLinesICase(paths);

			foreach (paths, path, index)
			{
				cout("! %s\n", path);

				if (IsGitPath(path))
				{
					cout("ignore git file!\n");
				}
				else
				{
					recurRemovePath(path);
				}
				LOGPOS();
			}
			LOGPOS();
			releaseDim(paths, 1);
			LOGPOS();

			paths = ls(memRepoDir);
			sortJLinesICase(paths);

			foreach (paths, path, index)
			{
				char *wPath = combine(repoDir, getLocal(path));

				cout("<# %s\n", path);
				cout("># %s\n", wPath);

				errorCase(IsGitPath(path)); // あるはずない。

				if (existDir(path))
				{
					createDir(wPath);
					copyDir(path, wPath);
				}
				else
				{
					copyFile(path, wPath);
				}
				LOGPOS();
				memFree(wPath);
				LOGPOS();
			}
			LOGPOS();
			releaseDim(paths, 1);
			memFree(repoDir);
			LOGPOS();
		}
		LOGPOS();
		releaseDim(memRepoDirs, 1);
		LOGPOS();

		recurRemoveDir(memDir);
//		semiRemovePath(memDir); // del @ 2020.1.19
		LOGPOS();
	}
	LOGPOS();
	releaseDim(memDirs, 1);
	LOGPOS();
}
static void TrimMain(void)
{
	autoList_t *memDirs = lsDirs(MEM_ROOTDIR);
	uint index;
	autoList_t *deletableDirs = newList();
	char *dir;

	LOGPOS();

	for (index = 1; index < getCount(memDirs); index++)
	{
		char *memDir1 = getLine(memDirs, index - 1);
		char *memDir2 = getLine(memDirs, index - 0);

		cout("1 %s\n", memDir1);
		cout("2 %s\n", memDir2);

		if (isSameDir(memDir1, memDir2, 1))
		{
			LOGPOS();
			addElement(deletableDirs, (uint)strx(memDir2));
		}
	}
	LOGPOS();
	foreach (deletableDirs, dir, index)
	{
		cout("* %s\n", dir);
		recurRemoveDir(dir);
//		semiRemovePath(dir); // del @ 2020.1.19
	}
	LOGPOS();
	releaseDim(memDirs, 1);
	releaseDim(deletableDirs, 1);
	LOGPOS();
}
int main(int argc, char **argv)
{
	errorCase(!existDir(GIT_ROOTDIR));
	errorCase(!existDir(MEM_ROOTDIR));
	errorCase(!existDir(RUM_ROOTDIR));

	if (argIs("MEMORY"))
	{
		MemoryMain();
	}
	else if (argIs("RUM"))
	{
		RumMain();
	}
	else if (argIs("FLUSH"))
	{
		FlushMain();
	}
	else if (argIs("TRIM"))
	{
		TrimMain();
	}
	else
	{
		error_m("不明なコマンド引数");
	}
	cout("EndCode: %u\n", EndCode);
	termination(EndCode);
}
