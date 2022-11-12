/*
	EditConfig.exe 設定ファイル 編集対象行の直前の行 編集対象行に上書きする文字列
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *file;
	char *mLine;
	char *owLine;
	autoList_t *lines;
	char *line;
	uint index;

	LOGPOS();

	file = nextArg();
	mLine = nextArg();
	owLine = nextArg();

	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));
	// mLine
	// owLine

	lines = readLines(file);

	foreach (lines, line, index)
	{
		if (!strcmp(line, mLine))
		{
			index++; // 編集対象は次の行
			break;
		}
	}
	errorCase(getCount(lines) <= index); // ? 編集対象の行が存在しない。

	strzp((char **)directGetPoint(lines, index), owLine);

	writeLines_cx(file, lines);

	LOGPOS();
}
