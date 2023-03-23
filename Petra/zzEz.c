/*
	zzEz.exe
*/

#include "C:\Factory\Common\all.h"

#define DATA_FILE "C:\\Factory\\tmp_data\\zzEz.txt"

static autoList_t *NNDirs;
static autoList_t *NNDirTimes;
static uint TimeNow;

static void LoadData(void)
{
	autoList_t *lines;
	char *line;
	uint index;

	LOGPOS();

	if (!existFile(DATA_FILE))
	{
		LOGPOS();
		return;
	}

	lines = readLines(DATA_FILE);

	foreach (lines, line, index)
	{
		char *p = ne_strchr(line, '*');

		*p++ = '\0';

		addElement(NNDirs, (uint)strx(p));
		addElement(NNDirTimes, toValue(line));
	}
	releaseDim(lines, 1);

	LOGPOS();
}
static void SaveData(void)
{
	autoList_t *lines;
	char *nnDir;
	uint index;

	LOGPOS();

	errorCase(getCount(NNDirs) != getCount(NNDirTimes)); // 2bs

	if (!getCount(NNDirs))
	{
		removeFileIfExist(DATA_FILE);

		LOGPOS();
		return;
	}

	lines = newList();

	foreach (NNDirs, nnDir, index)
		addElement(lines, (uint)xcout("%u*%s", getElement(NNDirTimes, index), nnDir));

	writeLines_cx(DATA_FILE, lines);

	LOGPOS();
}

static void AddNewNNDirs(void)
{
	uint nn;

	LOGPOS();

	for (nn = 1; nn <= 999; nn++)
	{
		char *nnDir = xcout("C:\\%u", nn);

		if (existDir(nnDir))
		if (findLine(NNDirs, nnDir) == getCount(NNDirs)) // ? not found
		{
			addElement(NNDirs, (uint)strx(nnDir));
			addElement(NNDirTimes, TimeNow);
		}
		memFree(nnDir);
	}
	LOGPOS();
}
static void RemoveOldNNDirs(void)
{
	char *nnDir;
	uint index;

	LOGPOS();

	foreach (NNDirs, nnDir, index)
	{
		if (!existDir(nnDir)) // ? ŽžŠÔØ‚ê‘O‚ÉÁ–Å
		{
			setElement(NNDirs,     index, 0);
			setElement(NNDirTimes, index, 0);
		}
	}
	removeZero(NNDirs);
	removeZero(NNDirTimes);

	errorCase(getCount(NNDirs) != getCount(NNDirTimes)); // 2bs

	LOGPOS();
}
static void DeleteExpiredNNDirs(void)
{
	char *nnDir;
	uint index;

	LOGPOS();

	foreach (NNDirs, nnDir, index)
	{
		uint nnDirTime = getElement(NNDirTimes, index);

		// ? ŽžŠÔØ‚ê
//		if (nnDirTime + 24 <= TimeNow) // 1“úŒo‰ß
		if (nnDirTime + 23 <= TimeNow)
		{
			coExecute_x(xcout("RD /S /Q %s", nnDir));

			memFree(nnDir);

			setElement(NNDirs,     index, 0);
			setElement(NNDirTimes, index, 0);
		}
	}
	removeZero(NNDirs);
	removeZero(NNDirTimes);

	errorCase(getCount(NNDirs) != getCount(NNDirTimes)); // 2bs

	LOGPOS();
}

int main(int argc, char **argv)
{
	LOGPOS();

	NNDirs     = newList();
	NNDirTimes = newList();
	TimeNow = (uint)(time(NULL) / 3600);

	errorCase(!TimeNow); // 2bs

	LoadData();
	AddNewNNDirs();
	RemoveOldNNDirs();
	DeleteExpiredNNDirs();
	SaveData();

	LOGPOS();
}
