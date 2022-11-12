/*
	tcmp.exe [/I] [INPUT-FILE OUTPUT-FILE]
*/

#include "C:\Factory\Common\all.h"

static void CompareFiles(char *file1, char *file2, int ignoreCase)
{
	autoList_t *lines1 = readLines(file1);
	autoList_t *lines2 = readLines(file2);
	autoList_t *report;
	char *line;
	uint index;

	report = getDiffLinesReport(lines1, lines2, ignoreCase);

	foreach (report, line, index)
	{
		cout("%s\n", line);
	}
	releaseDim(lines1, 1);
	releaseDim(lines2, 1);
	releaseDim(report, 1);
}
int main(int argc, char **argv)
{
	char *file1;
	char *file2;
	int ignoreCase = 0;

	if (argIs("/I")) // Ignore case
	{
		ignoreCase = 1;
	}

	if (hasArgs(2))
	{
		file1 = nextArg();
		file2 = nextArg();

		CompareFiles(file1, file2, ignoreCase);
	}
	else
	{
		for (; ; )
		{
			file1 = dropFile();
			file2 = dropFile();

			CompareFiles(file1, file2, ignoreCase);
			cout("\n");
		}
	}
}
