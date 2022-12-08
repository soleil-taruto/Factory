/*
	SabunPct.exe [TARGET-DIR-1 TARGET-DIR-2]
*/

#include "C:\Factory\Common\all.h"

static sint SWL_Comp(uint prm1, uint prm2)
{
	char *line1 = (char *)prm1;
	char *line2 = (char *)prm2;
	char *p1;
	char *p2;

	p1 = strchr(line1, '*');
	p2 = strchr(line2, '*');

	errorCase(!p1);
	errorCase(!p2);

	p1++;
	p2++;

	return _stricmp(p1, p2);
}
static void SortWLines(autoList_t *lines)
{
	rapidSort(lines, SWL_Comp);
}
static char *GetWLine_File_File(char *file1, char *file2, char *wPath)
{
	uint64 size1 = getFileSize(file1);
	uint64 size2 = getFileSize(file2);
	char *ret;

	if (size1 == size2)
	{
		FILE *fp1 = fileOpen(file1, "rb");
		FILE *fp2 = fileOpen(file2, "rb");
		uint64 count;
		uint64 diffCount = 0;

		for (count = 0; count < size1; count++)
		{
			int chr1 = readChar(fp1);
			int chr2 = readChar(fp2);

			if (chr1 != chr2)
				diffCount++;
		}
		fileClose(fp1);
		fileClose(fp2);

		if (diffCount == 0)
			ret = xcout("*%s", wPath);
		else
			ret = xcout("%I64u / %I64u = %f*%s", diffCount, size1, (double)diffCount / size1, wPath);
	}
	else
	{
		ret = xcout("Different Size*%s", wPath);
	}
	return ret;
}
static void SabunPct(char *dir1, char *dir2)
{
	autoList_t *paths1;
	autoList_t *paths2;
	autoList_t *only1Paths = newList();
	autoList_t *bothPaths  = newList();
	autoList_t *only2Paths = newList();
	autoList_t *lines = newList();
	char *line;
	uint index;

	dir1 = makeFullPath(dir1);
	dir2 = makeFullPath(dir2);
	paths1 = lss(dir1);
	paths2 = lss(dir2);

	changeRoots(paths1, dir1, NULL);
	changeRoots(paths2, dir2, NULL);

	sortJLinesICase(paths1);
	sortJLinesICase(paths2);
	mergeLines2ICase(paths1, paths2, only1Paths, bothPaths, only2Paths);

	foreach (only1Paths, line, index)
		addElement(lines, (uint)xcout("Left Only*%s", line));

	foreach (only2Paths, line, index)
		addElement(lines, (uint)xcout("Right Only*%s", line));

	foreach (bothPaths, line, index)
	{
		char *path1 = combine(dir1, line);
		char *path2 = combine(dir2, line);
		int dirFlag = 0;
		char *wLine;

		errorCase(!existPath(path1));
		errorCase(!existPath(path2));

		dirFlag |= existDir(path1) ? 1 : 0;
		dirFlag |= existDir(path2) ? 2 : 0;

		switch (dirFlag)
		{
		case 0:
			wLine = GetWLine_File_File(path1, path2, line);
			break;
		case 1:
			wLine = xcout("File - Dir*%s", line);
			break;
		case 2:
			wLine = xcout("Dir - File*%s", line);
			break;
		case 3:
			wLine = xcout("*%s\\", line);
			break;
		default:
			error();
		}
		addElement(lines, (uint)wLine);

		memFree(path1);
		memFree(path2);
	}
	SortWLines(lines);
	shootingStarLines(lines);

	foreach (lines, line, index)
		cout("%s\n", line);

	memFree(dir1);
	memFree(dir2);
	releaseDim(paths1, 1);
	releaseDim(paths2, 1);
	releaseAutoList(only1Paths);
	releaseAutoList(bothPaths);
	releaseAutoList(only2Paths);
	releaseDim(lines, 1);
}
int main(int argc, char **argv)
{
	if (hasArgs(2))
	{
		SabunPct(getArg(0), getArg(1));
		return;
	}

	{
		char *dir1;
		char *dir2;

		dir1 = dropDir();
		dir2 = dropDir();

		SabunPct(dir1, dir2);

		memFree(dir1);
		memFree(dir2);
	}
}
