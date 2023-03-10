/*
	zzEz.exe
*/

#include "C:\Factory\Common\all.h"

#define DATA_FILE "C:\\Factory\\tmp_data\\zzEz.txt"

static autoList_t *NNDirs;
static autoList_t *NNDirDates;
static uint Today;

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
		char *p = ne_strchr(line, '\t');

		*p++ = '\0';

		addElement(NNDirs, (uint)strx(p));
		addElement(NNDirDates, toValue(line));
	}
	releaseDim(lines, 1);

	LOGPOS();
}
static void SaveData(void)
{
	autoList_t *lines = newList();
	char *nnDir;
	uint index;

	LOGPOS();

	errorCase(getCount(NNDirs) != getCount(NNDirDates)); // 2bs

	foreach (NNDirs, nnDir, index)
		addElement(lines, (uint)xcout("%u\t%s", getElement(NNDirDates, index), nnDir));

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
			addElement(NNDirDates, Today);
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
			setElement(NNDirDates, index, 0);
		}
	}
	removeZero(NNDirs);
	removeZero(NNDirDates);

	errorCase(getCount(NNDirs) != getCount(NNDirDates)); // 2bs

	LOGPOS();
}
static void DeleteExpiredNNDirs(void)
{
	char *nnDir;
	uint index;

	LOGPOS();

	foreach (NNDirs, nnDir, index)
	{
		uint nnDirDate = getElement(NNDirDates, index);

		if (nnDirDate + 3 <= Today) // ? ŽžŠÔØ‚ê
		{
			coExecute_x(xcout("RD /S /Q %s", nnDir));

			setElement(NNDirs,     index, 0);
			setElement(NNDirDates, index, 0);
		}
	}
	removeZero(NNDirs);
	removeZero(NNDirDates);

	errorCase(getCount(NNDirs) != getCount(NNDirDates)); // 2bs

	LOGPOS();
}

int main(int argc, char **argv)
{
	LOGPOS();

	NNDirs     = newList();
	NNDirDates = newList();
	Today = (uint)(time(NULL) / 86400);

	errorCase(!Today); // 2bs

	LoadData();
	AddNewNNDirs();
	RemoveOldNNDirs();
	DeleteExpiredNNDirs();
	SaveData();

	LOGPOS();
}
