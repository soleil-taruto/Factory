/*
	Sabun.exe [/L] [/S] [/-D] [/B] [/-B] [/DL] [/-R] [左側のディレクトリ 右側のディレクトリ [出力先ディレクトリ]]

		/L  ... 左側にのみ存在するファイルを出力する。デフォルトではしない。
		/S  ... 両方に存在し、内容が同じファイルを出力する。デフォルトではしない。
		/-D ... 両方に存在し、内容が異なるファイルを出力しない。デフォルトではする。
		/B  ... 両方に存在するファイルを出力する。  デフォルトでは内容が異なるファイルのみ出力する。
		/-B ... 両方に存在するファイルを出力しない。デフォルトでは内容が異なるファイルのみ出力する。
		/DL ... 両方に存在し、内容が異なるファイルを出力する場合、左側のファイルを出力する。デフォルトでは右側のファイルを出力する。
		/-R ... 右側にのみ存在するファイルを出力しない。デフォルトではする。
*/

#include "C:\Factory\Common\all.h"

static int OutputLeftOnly  = 0; // 左にのみ存在する。
static int OutputSame      = 0; // 両方に存在し、内容が同じもの。
static int OutputDiff      = 1; // 両方に存在し、内容が異なるもの。
static int OutputDiffLeft  = 0; // 両方に存在し、内容が異なるとき左を出力する。(0 == 右を出力する)
static int OutputRightOnly = 1; // 右にのみ存在する。

static void DoOutput(char *outDir, char *srcDir, char *file)
{
	char *srcFile = combine(srcDir, file);
	char *outFile = combine(outDir, file);

	cout("< %s\n", srcFile);
	cout("> %s\n", outFile);

	errorCase(!existFile(srcFile));
	errorCase( existPath(outFile));

	createPath(outFile, 'X');
	copyFile(srcFile, outFile);

	memFree(srcFile);
	memFree(outFile);

	cout("COPY_OK\n");
}
static void Sabun(char *lDir, char *rDir, char *outDir)
{
	autoList_t *lFiles;
	autoList_t *rFiles;
	autoList_t *bothFiles;
	autoList_t *lOnlyFiles;
	autoList_t *rOnlyFiles;
	char *file;
	uint index;

	lDir   = makeFullPath(lDir);
	rDir   = makeFullPath(rDir);
	outDir = makeFullPath(outDir);

	cout("L.< %s\n", lDir);
	cout("R.< %s\n", rDir);
	cout("O.> %s\n", outDir);
	cout("LO: %d\n", OutputLeftOnly);
	cout("BS: %d\n", OutputSame);
	cout("BD: %d (%c)\n", OutputDiff, OutputDiffLeft ? 'L' : 'R');
	cout("RO: %d\n", OutputRightOnly);

	createPath(outDir, 'd');

	errorCase(!existDir(lDir));
	errorCase(!existDir(rDir));
	errorCase(!existDir(outDir)); // 2bs

	if (lsCount(outDir)) // ? 空ではない。
	{
#if 1
		error_m("出力先ディレクトリが空ではありません。"); // やっぱ危ないし、、
#else
		errorCase_m(isAbsRootDir(outDir), "出力先がルートディレクトリの場合、空のドライブでなければなりません。"); // C:\ とか全削除したらヤバいので、、

		cout("+---------------------------------------+\n");
		cout("| 出力先ディレクトリは空ではありません。|\n");
		cout("+---------------------------------------+\n");
		cout("空にする？\n");

		if (getKey() == 0x1b)
			termination(0);

		cout("空にします。\n");

		recurClearDir(outDir);
#endif
	}

	LOGPOS();
	lFiles = lssFiles(lDir);
	LOGPOS();
	rFiles = lssFiles(rDir);
	LOGPOS();

	changeRoots(lFiles, lDir, NULL);
	LOGPOS();
	changeRoots(rFiles, rDir, NULL);
	LOGPOS();

	sortJLinesICase(lFiles);
	LOGPOS();
	sortJLinesICase(rFiles);
	LOGPOS();

	bothFiles  = newList();
	lOnlyFiles = newList();
	rOnlyFiles = newList();

	mergeLines2ICase(lFiles, rFiles, lOnlyFiles, bothFiles, rOnlyFiles);
	LOGPOS();

	if (OutputLeftOnly)
		foreach (lOnlyFiles, file, index)
			DoOutput(outDir, lDir, file);

	LOGPOS();

	if (OutputSame || OutputDiff)
	{
		foreach (bothFiles, file, index)
		{
			if (OutputSame && OutputDiff)
			{
				DoOutput(outDir, OutputDiffLeft ? lDir : rDir, file);
			}
			else
			{
				char *lFile = combine(lDir, file);
				char *rFile = combine(rDir, file);

				cout("C %s\n", file);

				errorCase(!existFile(lFile)); // 2bs?
				errorCase(!existFile(rFile)); // 2bs?

				if (isSameFile(lFile, rFile)) // ? 同じ内容
				{
					if (OutputSame)
						DoOutput(outDir, lDir, file);
				}
				else // ? 異なる内容
				{
					if (OutputDiff)
						DoOutput(outDir, OutputDiffLeft ? lDir : rDir, file);
				}
				memFree(lFile);
				memFree(rFile);
			}
		}
	}
	LOGPOS();

	if (OutputRightOnly)
		foreach (rOnlyFiles, file, index)
			DoOutput(outDir, rDir, file);

	LOGPOS();

	memFree(lDir);
	memFree(rDir);
	memFree(outDir);
	releaseAutoList(bothFiles);
	releaseAutoList(lOnlyFiles);
	releaseAutoList(rOnlyFiles);

	cout("done!\n");
}
int main(int argc, char **argv)
{
readArgs:
	// basic opt

	if (argIs("/L"))
	{
		OutputLeftOnly = 1;
		goto readArgs;
	}
	if (argIs("/S"))
	{
		OutputSame = 1;
		goto readArgs;
	}
	if (argIs("/-D"))
	{
		OutputDiff = 0;
		goto readArgs;
	}
	if (argIs("/DL"))
	{
		OutputDiffLeft = 1;
		goto readArgs;
	}
	if (argIs("/-R"))
	{
		OutputRightOnly = 0;
		goto readArgs;
	}

	// ext opt

	if (argIs("/B"))
	{
		OutputSame = 1;
		OutputDiff = 1;
		goto readArgs;
	}
	if (argIs("/-B"))
	{
		OutputSame = 0;
		OutputDiff = 0;
		goto readArgs;
	}

	// - - -

	if (hasArgs(3))
	{
		char *lDir;
		char *rDir;
		char *outDir;

		lDir = nextArg();
		rDir = nextArg();
		outDir = nextArg();

		Sabun(lDir, rDir, outDir);
		return;
	}
	if (hasArgs(2))
	{
		char *lDir;
		char *rDir;
		char *outDir;
		char *outDir2;

		lDir = nextArg();
		rDir = nextArg();
		outDir = makeFreeDir();
		outDir2 = combine(outDir, "Sabun.out");

		Sabun(lDir, rDir, outDir2);
		coExecute_x(xcout("START %s", outDir));

		memFree(outDir);
		memFree(outDir2);
		return;
	}

	{
		char *lDir;
		char *rDir;
		char *outDir;
		char *outDir2;

		lDir = dropDir();
		rDir = dropDir();
		outDir = makeFreeDir();
		outDir2 = combine(outDir, "Sabun.out");

		Sabun(lDir, rDir, outDir2);
		coExecute_x(xcout("START %s", outDir));

		memFree(lDir);
		memFree(rDir);
		memFree(outDir);
		memFree(outDir2);
	}
}
