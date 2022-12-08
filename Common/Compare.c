#include "all.h"

int isSameBlock(autoBlock_t *block1, autoBlock_t *block2)
{
	uint b1Size = getSize(block1);

	return b1Size == getSize(block2) &&
		!memcmp(directGetBuffer(block1), directGetBuffer(block2), b1Size);
}
int isSameList(autoList_t *list1, autoList_t *list2)
{
	uint count = getCount(list1);

	return count == getCount(list2) &&
		!memcmp(directGetList(list1), directGetList(list2), count * sizeof(uint));
}
int isSameFile(char *file1, char *file2)
{
	uint64 fsize1 = getFileSize(file1);
	uint64 bcnt;
	BUFF *bp1;
	BUFF *bp2;
	int result = 0;

	if (fsize1 != getFileSize(file2))
		goto endfunc;

	bp1 = buffOpen(file1, "rb", 32 * 1024 * 1024);
	bp2 = buffOpen(file2, "rb", 32 * 1024 * 1024);

	for (bcnt = 0; bcnt < fsize1; bcnt++)
	{
		int chr1 = buffReadChar(bp1);
		int chr2 = buffReadChar(bp2);

		if (chr1 != chr2)
		{
			goto notSame;
		}
	}
	result = 1;

notSame:
	buffClose(bp1);
	buffClose(bp2);

endfunc:
	return result;
}
int isSameDir(char *dir1, char *dir2, int ignoreCaseOfPath)
{
	autoList_t *paths1;
	autoList_t *paths2;
	uint dcnt1;
	uint dcnt2;
	uint index;
	int result = 0;

	dir1 = makeFullPath(dir1);
	dir2 = makeFullPath(dir2);

	paths1 = lss(dir1);
	dcnt1 = lastDirCount;
	paths2 = lss(dir2);
	dcnt2 = lastDirCount;

	if (dcnt1 != dcnt2)
		goto endfunc;

	if (getCount(paths1) != getCount(paths2))
		goto endfunc;

	dirFileSortDirCount(paths1, dcnt1);
	dirFileSortDirCount(paths2, dcnt2);

	// パス文字列の比較
	{
		int (*funcPathComp)(char *, char *) = ignoreCaseOfPath ? mbs_stricmp : strcmp;
		uint index;

		for (index = 0; index < getCount(paths1); index++)
		{
			char *rp1 = getLine(paths1, index);
			char *rp2 = getLine(paths2, index);

			rp1 = eraseRoot(rp1, dir1);
			rp2 = eraseRoot(rp2, dir2);

			if (funcPathComp(rp1, rp2))
			{
				goto endfunc;
			}
		}
	}

	for (index = dcnt1; index < getCount(paths1); index++) // ファイルサイズの比較
	{
		char *file1 = getLine(paths1, index);
		char *file2 = getLine(paths2, index);

		if (getFileSize(file1) != getFileSize(file2))
		{
			goto endfunc;
		}
	}
	for (index = dcnt1; index < getCount(paths1); index++) // ファイルの内容の比較
	{
		char *file1 = getLine(paths1, index);
		char *file2 = getLine(paths2, index);

		if (!isSameFile(file1, file2))
		{
			goto endfunc;
		}
	}
	result = 1;

endfunc:
	memFree(dir1);
	memFree(dir2);

	releaseDim(paths1, 1);
	releaseDim(paths2, 1);

	return result;
}
int isSameLines(autoList_t *lines1, autoList_t *lines2, int ignoreCase)
{
	int result = 0;

	if (getCount(lines1) == getCount(lines2))
	{
		int (*funcCompLine)(char *, char *) = ignoreCase ? mbs_stricmp : strcmp;
		uint index;

		for (index = 0; index < getCount(lines1); index++)
		{
			char *line1 = getLine(lines1, index);
			char *line2 = getLine(lines2, index);

			if (funcCompLine(line1, line2))
			{
				goto endfunc;
			}
		}
		result = 1;
	}

endfunc:
	return result;
}
int isSameLinesList(autoList_t *table1, autoList_t *table2, int ignoreCase)
{
	int result = 0;

	if (getCount(table1) == getCount(table2))
	{
		uint index;

		for (index = 0; index < getCount(table1); index++)
		{
			autoList_t *lines1 = (autoList_t *)getElement(table1, index);
			autoList_t *lines2 = (autoList_t *)getElement(table2, index);

			if (!isSameLines(lines1, lines2, ignoreCase))
			{
				goto endfunc;
			}
		}
		result = 1;
	}

endfunc:
	return result;
}

// ---- getDiffLinesReport ----

static struct
{
	autoList_t *Lines1;
	autoList_t *Lines2;
	sint (*LineComp)(char *, char *);
	uint Index1;
	uint Index2;
}
FD;

static struct
{
	autoList_t *SLines;
	autoList_t *LLines;
	uint SIndex;
	uint LIndex;
}
FDE;

static void FDE_FindSameArea(void)
{
	uint bound = iRoot(getCount(FDE.SLines), 3);
	uint ndx1;
	uint ndx2;

	FDE.SIndex = getCount(FDE.SLines);
	FDE.LIndex = getCount(FDE.LLines);

	if (!bound)
		return;

	for (ndx1 = 0; ndx1 + bound <= getCount(FDE.SLines); ndx1 += bound)
	for (ndx2 = 0; ndx2 + bound <= getCount(FDE.LLines); ndx2++)
	{
		uint index;

		if (FDE.SIndex + FDE.LIndex <= ndx1 + ndx2) // ? 既知の差分より大きいか同じになる。-> next ndx1
			break;

		for (index = 0; index < bound; index++)
			if (FD.LineComp(getLine(FDE.SLines, ndx1 + index), getLine(FDE.LLines, ndx2 + index))) // ? not same
				break;

		if (index == bound) // ? found
		{
			FDE.SIndex = ndx1;
			FDE.LIndex = ndx2;
		}
	}
}
static void FDE_FindSameBegin(void)
{
	while (FDE.SIndex && FDE.LIndex)
	{
		if (FD.LineComp(getLine(FDE.SLines, FDE.SIndex - 1), getLine(FDE.LLines, FDE.LIndex - 1))) // ? not same
			break;

		FDE.SIndex--;
		FDE.LIndex--;
	}
}

static int FindDiffBegin(void) // ret: ? found
{
	while (FD.Index1 < getCount(FD.Lines1) && FD.Index2 < getCount(FD.Lines2))
	{
		if (FD.LineComp(getLine(FD.Lines1, FD.Index1), getLine(FD.Lines2, FD.Index2))) // ? not same
			return 1;

		FD.Index1++;
		FD.Index2++;
	}
	return FD.Index1 < getCount(FD.Lines1) || FD.Index2 < getCount(FD.Lines2);
}
static void FindDiffEnd(void)
{
	int swapped;
	autoList_t gal1;
	autoList_t gal2;

	if (getCount(FD.Lines2) < getCount(FD.Lines1))
	{
		m_swap(FD.Lines1, FD.Lines2, autoList_t *);
		m_swap(FD.Index1, FD.Index2, uint);
		swapped = 1;
	}
	else
	{
		swapped = 0;
	}
	FDE.SLines = gndFollowElementsVar(FD.Lines1, FD.Index1, gal1);
	FDE.LLines = gndFollowElementsVar(FD.Lines2, FD.Index2, gal2);

	FDE_FindSameArea();
	FDE_FindSameBegin();

	FD.Index1 += FDE.SIndex;
	FD.Index2 += FDE.LIndex;

	if (swapped)
	{
		m_swap(FD.Lines1, FD.Lines2, autoList_t *);
		m_swap(FD.Index1, FD.Index2, uint);
	}
}

#define DIFFLINESOUT_DIR "C:\\Factory\\tmp\\LastDiffText"
#define DIFFLINESOUT_FILE1 DIFFLINESOUT_DIR "\\Text1.txt"
#define DIFFLINESOUT_FILE2 DIFFLINESOUT_DIR "\\Text2.txt"

autoList_t *getDiffLinesReport(autoList_t *lines1, autoList_t *lines2, int ignoreCase)
{
	autoList_t *report = newList();
	uint begin1;
	uint begin2;
	uint end1;
	uint end2;
	uint index;

	FD.Lines1 = lines1;
	FD.Lines2 = lines2;
	FD.LineComp = ignoreCase ? mbs_stricmp : strcmp;
	FD.Index1 = 0;
	FD.Index2 = 0;

	while (FindDiffBegin())
	{
		begin1 = FD.Index1;
		begin2 = FD.Index2;

		FindDiffEnd();

		end1 = FD.Index1;
		end2 = FD.Index2;

		for (index = 0; begin1 + index < end1 && begin2 + index < end2; index++)
		{
			if (FD.LineComp(getLine(lines1, begin1 + index), getLine(lines2, begin2 + index)))
			{
				addElement(report, (uint)xcout("%u < %s", begin1 + index + 1, getElement(lines1, begin1 + index)));
				addElement(report, (uint)xcout("%u > %s", begin2 + index + 1, getElement(lines2, begin2 + index)));
			}
		}
		for (; begin1 + index < end1; index++) addElement(report, (uint)xcout("%u < %s", begin1 + index + 1, getElement(lines1, begin1 + index)));
		for (; begin2 + index < end2; index++) addElement(report, (uint)xcout("%u > %s", begin2 + index + 1, getElement(lines2, begin2 + index)));
	}
	// output
	{
		LOGPOS();
		createDirIfNotExist(DIFFLINESOUT_DIR);
		writeLines(DIFFLINESOUT_FILE1, lines1);
		writeLines(DIFFLINESOUT_FILE2, lines2);
	}
	return report;
}
autoList_t *getDiffLinesReportLim(autoList_t *lines1, autoList_t *lines2, int ignoreCase, uint lineNumLimit) // lineNumLimit: 0 == 無制限
{
	autoList_t *report = getDiffLinesReport(lines1, lines2, ignoreCase);

	if (lineNumLimit)
	{
		uint lineNum = getCount(report);

		if (lineNumLimit < lineNum)
		{
			while (lineNumLimit < getCount(report))
				memFree((char *)unaddElement(report));

			addElement(report, (uint)xcout("全部で %u 行ありました...", lineNum));
		}
	}
	return report;
}

// ----
