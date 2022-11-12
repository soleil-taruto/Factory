/*
	linenum /M 開始行番号 終了行番号 ファイル名

		開始行番号 ... 1 ～ ファイルの行数
		終了行番号 ... 1 ～ ファイルの行数

		開始行から終了行までを表示する。

		ex. linenum 100 200 a.txt   ... a.txt の 100 行目 ～ 200 行目を表示する。

	linenum ファイル名

		指定されたファイルの行数を表示する。

		ex. linenum a.txt   ... a.txt の 行数を表示する。

	linenum

		標準入力の行数を表示する。

		ex. dir /b | linenum   ... カレントディレクトリのファイルとディレクトリの数を表示する。

	----

	HACK: .\Count.c と被ってる。
*/

#include "C:\Factory\Common\all.h"

static int SkipLine(FILE *fp)
{
	int chr = readChar(fp);

	if (chr == EOF) // ? 0文字のEOFで終わる行 <- 行と見なさない。
		return 0;

	do
	{
		chr = readChar(fp);

		if (chr == EOF) // ? 1文字以上のEOFで終わる行 <- 行と見なす。
			break;
	}
	while (chr != '\n'); // ? ! 改行

	return 1;
}
static void LineMid(char *file, uint64 minLineNo, uint64 maxLineNo)
{
	FILE *fp = fileOpen(file, "rb");
	uint64 lineNo = 1;

	errorCase(minLineNo < 1);
	errorCase(maxLineNo < minLineNo);

	while (lineNo < minLineNo)
	{
		errorCase(!SkipLine(fp)); // ? 開始行より前にファイルが終了した。
		lineNo++;
	}
	while (lineNo <= maxLineNo)
	{
		char *line = readLine(fp);

		errorCase(!line); // ? 終了行より前にファイルが終了した。

		cout("%s\n", line);
		memFree(line);
		lineNo++;
	}
	fileClose(fp);
}
static void LineNumMain(char *file)
{
	FILE *fp = file ? fileOpen(file, "rb") : (stdin_set_bin(), stdin);
	uint64 linenum = 0;

	while (SkipLine(fp))
	{
		linenum++;
	}
	cout("%I64u\n", linenum);

	if (file)
		fileClose(fp);
}
int main(int argc, char **argv)
{
	if (argIs("/M"))
	{
		uint64 minLineNo;
		uint64 maxLineNo;
		char *file;

		minLineNo = toValue64(nextArg());
		maxLineNo = toValue64(nextArg());
		file = nextArg();

		LineMid(file, minLineNo, maxLineNo);
		return;
	}
	if (hasArgs(1))
		LineNumMain(nextArg());
	else
		LineNumMain(NULL);
}
