/*
	rept.exe [/F] [/I] [/P 置き換え前パターン 置き換え後パターン]... 入力ファイル 出力ファイル
*/

#include "C:\Factory\Common\all.h"

static autoList_t *FindPtns;
static autoList_t *DestPtns;
static int ForceMode;
static int IgnoreCaseFlag;

static void RepText_Main(char *rFile, char *wFile)
{
	char *text = readText_b(rFile);
	char *findPtn;
	uint index;

	foreach (FindPtns, findPtn, index)
	{
		char *destPtn = getLine(DestPtns, index);

		text = replaceLine(text, findPtn, destPtn, IgnoreCaseFlag);
	}
	writeOneLineNoRet_b(wFile, text);
	memFree(text);
}
static void RepText(char *rFile, char *wFile)
{
	if (ForceMode || !existFile(wFile))
	{
		RepText_Main(rFile, wFile);
	}
	else
	{
		char *midFile = makeTempPath(NULL);

		RepText_Main(rFile, midFile);

		if (isSameFile(midFile, wFile)) // ? 同じ -> 適用不要
		{
			cout("SKIP!\n");
			removeFile(midFile);
		}
		else
		{
			removeFile(wFile);
			moveFile(midFile, wFile);
		}
		memFree(midFile);
	}
}
int main(int argc, char **argv)
{
	FindPtns = newList();
	DestPtns = newList();

readArgs:
	if (argIs("/F")) // Force mode
	{
		ForceMode = 1;
		goto readArgs;
	}
	if (argIs("/I")) // Ignore case
	{
		IgnoreCaseFlag = 1;
		goto readArgs;
	}
	if (argIs("/P")) // Pair
	{
		addElement(FindPtns, (uint)nextArg());
		addElement(DestPtns, (uint)nextArg());

		goto readArgs;
	}

	{
		char *rFile;
		char *wFile;

		rFile = nextArg();
		wFile = nextArg();

		errorCase(hasArgs(1)); // 不明なオプション

		RepText(rFile, wFile);
	}
}
