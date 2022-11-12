#include "CsvSort.h"

static uint SortColumnIndex;
static sint (*SortComp)(char *, char *);

static uint ReadRow(FILE *fp)
{
	return (uint)readCSVRow(fp);
}
static void WriteRow_x(FILE *fp, uint row)
{
	writeCSVRow_x(fp, (autoList_t *)row);
}
static sint DefCompRow(autoList_t *row1, autoList_t *row2)
{
	uint colcnt1 = getCount(row1);
	uint colcnt2 = getCount(row2);
	uint colcnt;
	uint colidx;

	colcnt = m_min(colcnt1, colcnt2);

	for (colidx = 0; colidx < colcnt; colidx++)
	{
		sint ret = strcmp(getLine(row1, colidx), getLine(row2, colidx));

		if (ret)
			return ret;
	}
	return (sint)colcnt1 - (sint)colcnt2;
}
static sint CompRow(uint row1, uint row2)
{
	char *cell1 = getLine((autoList_t *)row1, SortColumnIndex);
	char *cell2 = getLine((autoList_t *)row2, SortColumnIndex);
	sint ret;

	ret = SortComp(cell1, cell2);

	if (!ret)
		ret = DefCompRow((autoList_t *)row1, (autoList_t *)row2);

	return ret;
}
static uint GetRowWeightSize(uint row)
{
	return getCount((autoList_t *)row) * 10 + 20;
}
void CsvSortComp(char *srcFile, char *destFile, uint sortColumnIndex, sint (*funcComp)(char *, char *), uint partSize)
{
	SortColumnIndex = sortColumnIndex;
	SortComp = funcComp;

	MS_GetRecordWeightSize = GetRowWeightSize;
	MergeSort(srcFile, destFile, 1, ReadRow, WriteRow_x, CompRow, partSize, 0);
	MS_GetRecordWeightSize = NULL;
}

static sint (*CI_Comp)(char *, char *);

static sint CompInv(char *a, char *b)
{
	return CI_Comp(a, b) * -1;
}

/*
	sortDirection:
		'A' == Ascending
		'D' == Descending

	compareMode:
		'S' == case Sensitive
		'I' == Ignore case
		'9' == Numerically
*/
void CsvSort(char *srcFile, char *destFile, uint sortColumnIndex, int sortDirection, int compareMode, uint partSize)
{
	sint (*funcComp)(char *, char *);

	switch (compareMode)
	{
	case 'S': funcComp = strcmp; break;
	case 'I': funcComp = mbs_stricmp; break;
	case '9': funcComp = numstrcmp; break;

	default:
		error();
	}

	switch (sortDirection)
	{
	case 'A': break;
	case 'D':
		CI_Comp = funcComp;
		funcComp = CompInv;
		break;

	default:
		error();
	}

	CsvSortComp(srcFile, destFile, sortColumnIndex, funcComp, partSize);
}
