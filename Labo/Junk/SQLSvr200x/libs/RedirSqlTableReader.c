/*
	改行とタブは処理できないので sqlcmd に -k オプションを付けてね。
*/

#include "RedirSqlTableReader.h"

int RSTR_NoHeaderMode;

autoList_t *RSTR_SecondLineToColSpans(char *line)
{
	autoList_t *colSpans = newList();
	char *p;
	uint count = 0;

	for (p = line; ; p++)
	{
		if (*p == '-')
		{
			count++;
		}
		else if (*p == ' ' || *p == '\0')
		{
			errorCase(!count); // 幅ゼロは考えられない。
			addElement(colSpans, count);

			if (*p == '\0')
			{
				break;
			}
			count = 0;
		}
		else
		{
			error(); // ? '-', ' ' 以外の文字
		}
	}
	return colSpans; // ここで (長さ 1 以上 && 各要素 1 以上) は保障される。
}
autoList_t *RSTR_LineToValues(char *line, autoList_t *colSpans, int asColName) // line を破壊する。
{
	uint cs;
	uint index;
	char *p = line;
	autoList_t *values = newList();

	if (!isJLine(line, 1, 0, 0, 1)) // タブの無い CP-932 であること。
	{
		line2JLine(line, 1, 0, 0, 1);
		cout("文字コードの補正を行いました。\n");
	}
	foreach (colSpans, cs, index)
	{
		char *q = p;

		while (cs)
		{
			errorCase(!*p); // 行が途切れた。
			p = mbsNext(p);
			cs--;
		}
		if (index + 1 < getCount(colSpans)) // ? 非最終カラム
		{
			errorCase(*p != ' '); // カラムの間が空白ではない。
			*p = '\0';
			p++;
		}
		else
		{
			errorCase(*p != '\0'); // 行がまだ終わらない。
		}
		trimEdge(q, ' ');

		if (asColName)
		{
			errorCase(asColName && *q == '\0'); // カラム名に空文字列は無いはず。
			errorCase(findLine(values, q) < index); // カラム名の重複を認めない。
		}
		addElement(values, (uint)strx(q));
	}
	return values;
}
void RedirSqlTableReader(char *file, FILE *outStrm, void (*writeColNames_x)(FILE *outStrm, autoList_t *colNames), void (*writeRow_x)(FILE *outStrm, autoList_t *row))
{
	FILE *fp = fileOpen(file, "rt");
	char *line;
	char *line2;
	autoList_t *colSpans;

	// 冒頭のゴミを読み飛ばす。
	{
		uint irow;

		for (irow = 0; ; irow++)
		{
			line = readLine(fp);
			errorCase(!line);

			if (lineExp("< ->", line) &&
				lineExp("-<>", line) &&
				lineExp("<>-", line) &&
				!lineExp("<>  <>", line)
				)
				break;

			memFree(line);
		}
		fileSeek(fp, SEEK_SET, 0);

		for (; 1 < irow; irow--) // (irow - 1) が不要な行数
		{
			line = readLine(fp);
			errorCase(!line);
			memFree(line);
		}
	}

	line = readLine(fp);
	errorCase(!line);
	line2 = readLine(fp);
	errorCase(!line2);

	colSpans = RSTR_SecondLineToColSpans(line2);
	writeColNames_x(outStrm, RSTR_LineToValues(line, colSpans, 1));

	memFree(line);
	memFree(line2);

	for (; ; )
	{
		line = readLine(fp);
		errorCase(!line);

		if (!*line)
		{
			break;
		}
		writeRow_x(outStrm, RSTR_LineToValues(line, colSpans, 0));
		memFree(line);
	}
	releaseAutoList(colSpans);

	line = readLine(fp); // '(999 行処理されました)' 行
	errorCase(!line);
	errorCase(!lineExp("(<>)", line)); // 英語とかあるかもしれないので "()" だけチェック
	memFree(line);

	// 末尾のゴミを無視する。
	/*
	line = readLine(fp); // EOF
	errorCase(line);
	*/

	fileClose(fp);
}
static void TLF_WriteColNames_x(FILE *fp, autoList_t *row)
{
	writeLines2Stream_x(fp, row);
	writeChar(fp, '\n'); // カラム名リストの終端を示す空行 (カラム名に空文字列は無い)
}
static void WriteHeaderNoop(FILE *fp, autoList_t *row)
{
	noop();
}
void RSTR_ToLinearFile(char *rfile, char *wfile)
{
	FILE *fp = fileOpen(wfile, "wt");

	RedirSqlTableReader(rfile, fp, RSTR_NoHeaderMode ? WriteHeaderNoop : TLF_WriteColNames_x, writeLines2Stream_x);
	fileClose(fp);
}
void RSTR_ToCSVFile(char *rfile, char *wfile)
{
	FILE *fp = fileOpen(wfile, "wt");

	RedirSqlTableReader(rfile, fp, RSTR_NoHeaderMode ? WriteHeaderNoop : RowToCSVStream_x, RowToCSVStream_x);
	fileClose(fp);
}
