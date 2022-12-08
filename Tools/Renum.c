/*
	Renum.exe [/R] [/N] [/K 桁数] 開始番号 ステップ [対象DIR]
*/

#include "C:\Factory\Common\all.h"

static int ReverseMode;
static int ToNumOnly;
static uint Ketasuu = 4;

static uint StartNum;
static uint NumStep;

static char *ZeroPad(uint value)
{
	static char *ret;

	memFree(ret);
	ret = xcout("%u", value);

	while (strlen(ret) < Ketasuu)
		ret = insertChar(ret, 0, '0');

	return ret;
}

static char *IN_NamePtr;

static int IsNumbered(char *file)
{
	char *p;

	for (p = file; *p; p++)
	{
		if (*p == '.' || *p == '_')
			break;

		if (!m_isdecimal(*p))
			return 0;
	}
	IN_NamePtr = p;
	return 1;
}
static void DoFRenum(void)
{
	autoList_t *files = lsFiles(".");
	autoList_t *midFiles = newList();
	autoList_t *destFiles = newList();
	char *file;
	uint index;
	uint no = StartNum;
	char *unqptn = xcout("_$u=%I64x_", time(NULL)); // 適当..

	eraseParents(files);
	sortJLinesICase(files);

	if (ReverseMode)
		reverseElements(files);

	foreach (files, file, index)
	{
		char *dest;

		errorCase(!*file); // 2bs

		if (ToNumOnly)
			dest = xcout("%s%s", ZeroPad(no), getExtWithDot(file));
		else if (IsNumbered(file))
			dest = xcout("%s%s", ZeroPad(no), IN_NamePtr);
		else
			dest = xcout("%s_%s", ZeroPad(no), file);

		if (strcmp(file, dest)) // ? file != dest
		{
			cout("< %s\n", file);
			cout("> %s\n", dest);

			addElement(midFiles, (uint)xcout("%s%s", unqptn, dest));
			addElement(destFiles, (uint)dest);
		}
		else
		{
			file[0] = '\0';
		}
		no += NumStep;
	}
	trimLines(files);
	errorCase(getCount(files) != getCount(midFiles)); // 2bs
	errorCase(getCount(files) != getCount(destFiles)); // 2bs

	cout("Press R to renumber\n");

	if (getKey() != 'R')
		termination(0);

	LOGPOS();

	for (index = 0; index < getCount(files); index++) // 移動テスト(1)
	{
		char *rFile = getLine(files, index);
		char *wFile = getLine(midFiles, index);

		errorCase(!existFile(rFile));
		errorCase( existFile(wFile));

		moveFile(rFile, wFile); // 移動トライ

		errorCase( existFile(rFile));
		errorCase(!existFile(wFile));

		moveFile(wFile, rFile); // 復元

		errorCase(!existFile(rFile));
		errorCase( existFile(wFile));
	}

	LOGPOS();

	for (index = 0; index < getCount(files); index++)
		moveFile(
			getLine(files, index),
			getLine(midFiles, index)
			);

	LOGPOS();

	// 移動テスト(2)
	{
		int errorFlag = 0;

		for (index = 0; index < getCount(files); index++)
		{
			char *wFile = getLine(destFiles, index);

			if (existPath(wFile))
			{
				cout("%s <既に存在する>\n", wFile);
				errorFlag = 1;
				break;
			}
			if (!creatable(wFile))
			{
				cout("%s <作成不可>\n", wFile);
				errorFlag = 1;
				break;
			}
		}

		if (errorFlag)
		{
			for (index = 0; index < getCount(files); index++) // 復元
				moveFile(
					getLine(midFiles, index),
					getLine(files, index)
					);

			error();
		}
	}

	LOGPOS();

	for (index = 0; index < getCount(files); index++)
		moveFile(
			getLine(midFiles, index),
			getLine(destFiles, index)
			);

	cout("Renumbered!\n");

	releaseDim(files, 1);
	releaseDim(midFiles, 1);
	releaseDim(destFiles, 1);
	memFree(unqptn);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/R"))
	{
		ReverseMode = 1;
		goto readArgs;
	}
	if (argIs("/N"))
	{
		ToNumOnly = 1;
		goto readArgs;
	}
	if (argIs("/K"))
	{
		Ketasuu = toValue(nextArg());
		goto readArgs;
	}

	StartNum = toValue(nextArg());
	NumStep  = toValue(nextArg());

	errorCase(!m_isRange(Ketasuu, 1, 10));
	errorCase(!NumStep);

	addCwd(hasArgs(1) ? nextArg() : c_dropDir());
	DoFRenum();
	unaddCwd();
}
