/*
	CsvRowColCount.exe [入力CSVファイル]
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csvStream.h"

#define COLCOUNT_MAX 999999

static uint64 RowCount;
static uint64 *ColCounts; // 列数ゼロは有り得ないので [列数 - 1] にカウントする。

static void CsvRowColCount(char *csvFile)
{
	FILE *fp = fileOpen(csvFile, "rt");
	uint index;

	ColCounts = na_(uint64, COLCOUNT_MAX);

	for (; ; )
	{
		autoList_t *row = readCSVRow(fp);

		if (!row)
			break;

		index = getCount(row);

		errorCase(!m_isRange(index, 1, COLCOUNT_MAX));

		RowCount++;
		ColCounts[index - 1]++;

		releaseDim(row, 1);
	}
	fileClose(fp);

	for (index = 0; index < COLCOUNT_MAX; index++)
		if (0 < ColCounts[index])
			cout("%06u %I64u\n", index + 1, ColCounts[index]);

	cout("------\n");
	cout("%I64u\n", RowCount);

	memFree(ColCounts);
	ColCounts = NULL;
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		CsvRowColCount(nextArg());
	}
	else
	{
		for (; ; )
		{
			CsvRowColCount(c_dropFile());
			cout("\n");
		}
	}
}
