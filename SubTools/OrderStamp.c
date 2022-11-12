/*
	タイムスタンプのソート順がファイル名のソート順になるようにタイムスタンプを変更する。
	★更新日時のみ変更する。

	OrderStamp.exe [/R] [/-I] [対象ディレクトリ]

		/R  ... Reverse
		/-I ... case sensitive
*/

#include "C:\Factory\Common\all.h"

static int ReverseFlag;
static int CaseSensitiveFlag;

static void DoOrderStamp(void)
{
	autoList_t *files = lsFiles(".");
	char *file;
	uint index;
	time_t t;

	LOGPOS();

	eraseParents(files);

	if (CaseSensitiveFlag)
		rapidSortLines(files);
	else
		sortJLinesICase(files);

	if (ReverseFlag)
		reverseElements(files);

	t = time(NULL) - getCount(files);

	foreach (files, file, index)
	{
		uint64 stamp = getFileStampByTime(t++);

		cout("%I64u -> %s\n", stamp, file);

		setFileStamp(file, 0, 0, stamp); // 更新日時のみ
//		setFileStamp(file, stamp, stamp, stamp); // 全ての日時
	}
	releaseDim(files, 1);

	LOGPOS();
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/R"))
	{
		ReverseFlag = 1;
		goto readArgs;
	}
	if (argIs("/-I"))
	{
		CaseSensitiveFlag =1;
		goto readArgs;
	}

	addCwd(hasArgs(1) ? nextArg() : c_dropDir());
	DoOrderStamp();
	unaddCwd();
}
