/*
	Hugest.exe [/S 最小の下限] [/-S] [/LSS | 入力ディレクトリ]

		/-S  ... サブディレクトリを無視

		[/LSS | 入力ディレクトリ] を省略した場合 ⇒ D&D
*/

#include "C:\Factory\Common\all.h"

static uint64 SmallestMin = 0;

static void DispHugest_Files_x(autoList_t *files)
{
	char *file;
	uint index;
	uint64 minsz;
	uint64 maxsz;

	minsz = UINT64MAX;
	maxsz = 0;

	foreach (files, file, index)
	{
		uint64 csz = getFileSize(file);

		if (csz < minsz && SmallestMin <= csz)
		{
			minsz = csz;
			cout("<%15s:%s\n", c_thousandComma(xcout("%I64u", minsz)), file);
		}
		if (maxsz < csz)
		{
			maxsz = csz;
			cout(">%15s:%s\n", c_thousandComma(xcout("%I64u", maxsz)), file);
		}
	}
	releaseDim(files, 1);

	cout("====\n");
	cout("%s\n", c_thousandComma(xcout("%I64u", minsz)));
	cout("%s\n", c_thousandComma(xcout("%I64u", maxsz)));
}
static void DispHugest(char *dir, int ignoreSubDir)
{
	DispHugest_Files_x(ignoreSubDir ? lsFiles(dir) : lssFiles(dir));
}
static void DispHugest_LSS(void)
{
	DispHugest_Files_x(readLines(FOUNDLISTFILE));
}
int main(int argc, char **argv)
{
	int ignoreSubDir = 0;

readArgs:
	if (argIs("/S"))
	{
		SmallestMin = toValue64(nextArg());
		cout("Smallest-Min: %I64u\n", SmallestMin);
		goto readArgs;
	}
	if (argIs("/-S"))
	{
		ignoreSubDir = 1;
		goto readArgs;
	}

	if (argIs("/LSS"))
	{
		DispHugest_LSS();
		return;
	}
	if (hasArgs(1))
	{
		DispHugest(nextArg(), ignoreSubDir);
		return;
	}
	for (; ; )
	{
		DispHugest(c_dropDir(), ignoreSubDir);
		cout("\n");
	}
}
