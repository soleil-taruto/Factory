/*
	ダブりを解消する。

	デフォルトの動作
	・階層の浅い方を残す。
	・辞書順で前の方を残す。

	- - -

	GazoSlim.exe [/D] [/R] [対象ディレクトリ]

		/D ... 階層の深い方を残す。
		/R ... 辞書順で後の方を残す。

	- - -
	例

	GazoSlim.exe /D C:\home\画像
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\OpenSource\md5.h"

#define MD5_BGN 1
#define MD5_LEN 32

static int KeepPriorDeep; // ? 階層の深い方を残す。
static int KeepPriorRear; // ? 辞書順で後の方を残す。

static uint GetYenCount(char *str)
{
	uint count = 0;
	char *p;

	for (p = str; *p; p = mbsNext(p))
		if (*p == '\\')
			count++;

	return count;
}
static sint LineComp(uint a, uint b)
{
	char *line1 = (char *)a + MD5_BGN;
	char *line2 = (char *)b + MD5_BGN;
	sint ret;

	ret = memcmp(line1, line2, MD5_LEN);

	if (ret)
		return ret;

	ret = (sint)GetYenCount(line1 + MD5_LEN) - (sint)GetYenCount(line2 + MD5_LEN);

	if (ret)
		return ret * (KeepPriorDeep ? -1 : 1);

	ret = mbs_stricmp(line1 + MD5_LEN, line2 + MD5_LEN);

	if (ret)
		return ret * (KeepPriorRear ? -1 : 1);

	return 0;
}
static sint LineComp_File(uint a, uint b)
{
	char *file1 = (char *)a + MD5_BGN + MD5_LEN;
	char *file2 = (char *)b + MD5_BGN + MD5_LEN;

	return mbs_stricmp(file1, file2);
}
static void DoDeleteFile(char *file)
{
	cout("* %s\n", file);
	semiRemovePath(file);
}
static void GazoSlim(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;
	autoList_t *lines = newList();
	char *line;

	ProgressBegin();

	foreach (files, file, index)
	{
		ProgressRate((double)index / getCount(files));

		addElement(lines, (uint)xcout("_%s%s", c_md5_makeHexHashFile(file), file));
	}
	ProgressEnd(0);

	rapidSort(lines, LineComp);

	foreach (lines, line, index)
	{
		if (!index || _strnicmp(getLine(lines, index - 1) + MD5_BGN, line + MD5_BGN, MD5_LEN))
			line[0] = 'K'; // Keep
		else
			line[0] = 'D'; // Delete
	}
	cout("---- file list ----\n");

	foreach (lines, line, index)
		cout("%s\n", line);

	rapidSort(lines, LineComp_File);

	cout("---- order by file ----\n");

	foreach (lines, line, index)
		cout("%s\n", line);

	cout("---- delete only ----\n");

	foreach (lines, line, index)
		if (line[0] == 'D')
			cout("%s\n", line);

	cout("続行？\n");

	if (getKey() == 0x1b)
		termination(0);

	cout("続行します。\n");

	foreach (lines, line, index)
		if (line[0] == 'D')
			DoDeleteFile(line + MD5_BGN + MD5_LEN);

	releaseDim(files, 1);
	releaseDim(lines, 1);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/D"))
	{
		KeepPriorDeep = 1;
		goto readArgs;
	}
	if (argIs("/R"))
	{
		KeepPriorRear = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		GazoSlim(nextArg());
		return;
	}

	for (; ; )
	{
		GazoSlim(c_dropDir());
		cout("\n");
	}
}
