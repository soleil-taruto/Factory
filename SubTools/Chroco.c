/*
	Chroco.exe /R コピー元 /D コピー先 /P パスリストファイル [/-C] [/-N]

	コピー元
		ローカル上に存在するディレクトリへの相対パスまたは絶対パス
		相対パスの場合、実行時のカレントディレクトリを基準とする。
	コピー先
		ローカル上に存在する又は作成可能なディレクトリへの相対パスまたは絶対パス
		相対パスの場合、実行時のカレントディレクトリを基準とする。
	パスリストファイル
		対象パスのリスト(パスリスト)を記述したファイル
		文字コード CP932 改行コード CR-LF or LF のテキストファイルであること。
		対象パスを改行区切りで記述する。
		対象パス前後に不要な空白を入れてはならない。
		空行を入れてはならない。最終行の終端には改行が有っても無くても良い。
		１件以上あること。
	対象パス
		ローカル上に存在するディレクトリまたはファイルへの相対パスまたは絶対パス
		相対パスの場合、コピー元を基準とする。
		コピー元の配下のパスでなければならない。また、コピー元自身を指定してはならない。

	/-C
		コピー前にコピー先をクリアしない。デフォルトではクリアする。
	/-N
		パスの大文字・小文字はパスリストに合わせる。デフォルトではコピー元のパスに合わせる。
*/

#include "C:\Factory\Common\all.h"

static void ClearDir(char *dir)
{
	autoList_t *paths;
	char *path;
	uint index;

	cout("rmdir: %s\n", dir);

	paths = lss(dir);
	reverseElements(paths);

	foreach (paths, path, index)
	{
		cout("rm: %s\n", path);
		( existDir(path) ? removeDir : removeFile )(path);
	}
	releaseDim(paths, 1);

	cout("rmdir done.\n");

	removeDir(dir);
	createDir(dir);

	cout("re-mkdir ok.\n");
}
static char *NormalizePathCase(char *path, char *dir)
{
	autoList_t *ptkns;
	char *ptkn;
	uint index;

	escapeYen(path);
	ptkns = tokenize(path, '/');
	restoreYen(path);

	addCwd(dir);
	foreach (ptkns, ptkn, index)
	{
		char *newPTkn;

		updateFindData(ptkn);
		newPTkn = strx(lastFindData.name);

		memFree(ptkn);
		setElement(ptkns, index, (uint)newPTkn);

		if (index < getCount(ptkns) - 1)
		{
			changeCwd(newPTkn);
		}
	}
	unaddCwd();

	path = untokenize(ptkns, "\\");
	releaseDim(ptkns, 1);
	return path;
}

static char *RootDir;
static char *DestDir;
static char *PathListFile;
static int NoClearDestDir;
static int NoNormalizePathCase;
static autoList_t *PathList;

static void ExecCopy(char *path, char *dir1, char *dir2) // (dir1, dir2) == absDir
{
	autoList_t *ptkns;
	char *ptkn;
	uint index;
	char *path1;
	char *path2;

	cout("# %s\n", path);
	cout("< %s\n", dir1);
	cout("> %s\n", dir2);

	escapeYen(path);
	ptkns = tokenize(path, '/');
	restoreYen(path);

	memFree((void *)unaddElement(ptkns));

	addCwd(dir2);
	foreach (ptkns, ptkn, index)
	{
		mkdirEx(ptkn);
		changeCwd(ptkn);
	}
	unaddCwd();
	releaseDim(ptkns, 1);

	path1 = combine(dir1, path);
	path2 = combine(dir2, path);

	if (existFile(path1))
		copyFile(path1, path2);
	else
		mkdirEx(path2);

	memFree(path1);
	memFree(path2);
}
static void ProcCopy(void)
{
	char *path;
	uint index;

	RootDir = makeFullPath(RootDir);
	DestDir = makeFullPath(DestDir);
	PathList = readLines(PathListFile);

	mkdirEx(DestDir);

	errorCase_m(!existDir(RootDir), "コピー元のディレクトリにアクセスできません。");
	errorCase_m(!existDir(DestDir), "コピー先のディレクトリにアクセスできません。");
	errorCase_m(!getCount(PathList), "パスリストが空です。");

	cout("[R] %s\n", RootDir);
	cout("[D] %s\n", DestDir);

	foreach (PathList, path, index)
	{
		addCwd(RootDir);
		path = makeFullPath_x(path);
		unaddCwd();

		cout("[%u] %s\n", index + 1, path);

		errorCase_m(!existPath(path), xcout("コピーしようとしたパスにアクセスできません。\n%s", path));
		changeRoot(path, RootDir, NULL);
		errorCase(!*path);

		setElement(PathList, index, (uint)path);
	}
	if (!NoNormalizePathCase)
	{
		foreach (PathList, path, index)
		{
			char *newPath = NormalizePathCase(path, RootDir);

			if (strcmp(path, newPath))
			{
				cout("< %s\n", path);
				cout("> %s\n", newPath);
			}
			memFree(path);
			setElement(PathList, index, (uint)newPath);
		}
	}
	if (!NoClearDestDir)
	{
		ClearDir(DestDir);
	}
	foreach (PathList, path, index)
	{
		ExecCopy(path, RootDir, DestDir);
	}

	memFree(RootDir);
	memFree(DestDir);
	releaseDim(PathList, 1);

	cout("\\e\n");
}

int main(int argc, char **argv)
{
readArgs:
	if (argIs("/R")) // Root dir
	{
		RootDir = nextArg();
		goto readArgs;
	}
	if (argIs("/D")) // Destination dir
	{
		DestDir = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // Path list file
	{
		PathListFile = nextArg();
		goto readArgs;
	}

	if (argIs("/-C")) // no Clear destination dir
	{
		NoClearDestDir = 1;
		goto readArgs;
	}
	if (argIs("/-N")) // no Normalize path case
	{
		NoNormalizePathCase = 1;
		goto readArgs;
	}

	errorCase_m(hasArgs(1), "不明なコマンド引数があります。");

	errorCase_m(m_isEmpty(RootDir), "コピー元のディレクトリを指定して下さい。");
	errorCase_m(m_isEmpty(DestDir), "コピー先のディレクトリを指定して下さい。");
	errorCase_m(m_isEmpty(PathListFile), "パスリストファイルを指定して下さい。");

	ProcCopy();
}
