// smpl
/*
	2つのテキストファイルの行単位の相違を表示する。

	SabunTextFile.exe [/I] [/L 出力制限] [比較対象ファイル_1 比較対象ファイル_2]
*/

#include "C:\Factory\Common\all.h"

static int IgnoreCaseFlag;
static uint Limit;

static void Main2(char *file1, char *file2)
{
	autoList_t *lines1 = readLines(file1);
	autoList_t *lines2 = readLines(file2);
	autoList_t *report;
	char *line;
	uint index;

	if (Limit)
		report = getDiffLinesReportLim(lines1, lines2, IgnoreCaseFlag, Limit);
	else
		report = getDiffLinesReport(lines1, lines2, IgnoreCaseFlag);

	foreach (report, line, index)
		cout("%s\n", line);

	releaseDim(report, 1);
}
int main(int argc, char **argv)
{
	if (argIs("/I"))
	{
		IgnoreCaseFlag = 1;
	}
	if (argIs("/L"))
	{
		Limit = toValue(nextArg());
	}

	if (hasArgs(2))
	{
		Main2(getArg(0), getArg(1));
		return;
	}

	{
		char *file1;
		char *file2;

		file1 = dropFile();
		file2 = dropFile();

		Main2(file1, file2);

		memFree(file1);
		memFree(file2);
	}
}
