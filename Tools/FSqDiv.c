/*
	FSqDiv.exe [/T] ディレクトリ 分割数

		/T ... タイムスタンプを名前順に設定する。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Progress.h"

static int T_Mode;

static void Do_T(char *dir)
{
	autoList_t *files;
	char *file;
	uint index;
	time_t tCount = time(NULL);

	LOGPOS();
	files = lssFiles(dir);
	LOGPOS();
	reverseElements(files);

	foreach (files, file, index)
	{
		uint64 fTime = toValue64(c_makeCompactStamp(getStampDataTime(tCount))) * 1000;

		cout("< %I64u\n", fTime);
		cout("> %s\n", file);

		setFileStamp(file, fTime, fTime, fTime);
		tCount--;
	}
	LOGPOS();
	releaseDim(files, 1);
}
static void FSqDiv(char *srcdir, uint divnum)
{
	char *destRootDir = makeFreeDir();
	autoList_t *destDirs = newList();
	autoList_t *files = lsFiles(srcdir);
	char *file;
	uint index;
	uint progCyc;

	for (index = 0; index < divnum; index++)
	{
		char *wDir = combine_cx(destRootDir, xcout("%05u", index + 1));

		createDir(wDir);
		addElement(destDirs, (uint)wDir);
	}

	ProgressBegin();
	progCyc = getCount(files) / 79;
	m_maxim(progCyc, 1);

	foreach (files ,file, index)
	{
		char *wDir = getLine(destDirs, index % getCount(destDirs));
		char *wFile;

		if (index % progCyc == 0)
			Progress();

		wFile = combine(wDir, getLocal(file));
		copyFile(file, wFile);
		memFree(wFile);
	}
	ProgressEnd(0);

	if (T_Mode)
		Do_T(destRootDir);

	coExecute_x(xcout("START %s", destRootDir));

	memFree(destRootDir);
	releaseDim(destDirs, 1);
	releaseDim(files, 1);
}
int main(int argc, char **argv)
{
	char *dir;
	uint divnum;

readArgs:
	if (argIs("/T"))
	{
		T_Mode = 1;
		goto readArgs;
	}

	dir = makeFullPath(nextArg());
	divnum = toValue(nextArg());

	errorCase(!existDir(dir));
	errorCase(!m_isRange(divnum, 2, 99999));

	FSqDiv(dir, divnum);
}
