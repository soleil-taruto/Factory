/*
	ブックマークファイル (入力ファイル) の作り方
		IE -> ファイル(F) -> インポートおよびエクスポート(I)
		ファイルにエクスポートするを選んで次へ
		お気に入りを選んで次へ
		お気に入り (ルート) または任意のフォルダを選んで次へ
		任意の出力先を選んでエクスポート

	chrome 対応
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

#define INDENTSTR "　"

static char *TrySkipWord(char *p, char *word)
{
	if (strncmp(p, word, strlen(word)))
		return NULL;

	return p + strlen(word);
}
static char *SkipWord(char *p, char *word)
{
	char *ret = TrySkipWord(p, word);

	errorCase(!ret);
	return ret;
}
static char *GetWord(char **pp, int endChr)
{
	char *ep = strchr(*pp, endChr);
	char *word;

	errorCase(!ep);

	word = strxl(*pp, (uint)ep - (uint)*pp);
	*pp = ep + 1;

	return word;
}
static char *TitleFltr(char *title)
{
	while (startsWith(title, "？"))
		eraseLine(title, 2);

	trim(title, ' ');
	return title;
}

static void BookmarkToText(char *file)
{
	autoList_t *lines = readLines(file);
	char *line;
	uint index;
	autoList_t *textLines;
	char *wkfile;
	char *dlAddDates = strx("+");
	char *indent = strx("");
	uint treeIndex = 0;

	textLines = createAutoList(getCount(lines));

	foreach (lines, line, index)
	{
		char *p = strstr(line, "<DT><A");
		char *q;

		if (p)
		{
			char *href;
			char *addDate;
			char *title;

			/*
				... <DT><A HREF=" ... " ADD_DATE=" ... " ... > ... .url</A>
				                   |                |           |
				                  href           addDate      title
			*/
			p = SkipWord(p, "<DT><A HREF=\"");
			href = GetWord(&p, '"');
			p = SkipWord(p, " ADD_DATE=\"");
			addDate = GetWord(&p, '"');
			p = strchr(p, '>');
			errorCase(!p);
			p++;
			title = GetWord(&p, '<');
			p = SkipWord(p, "/A>");
			errorCase(*p);

			// .url をカットする。win7には無い！
			if (!_stricmp(getExt(title), "url"))
			{
				p = mbs_strrchr(title, '.');
				errorCase(!p);
				*p = '\0';
			}

			title = TitleFltr(title);

			addElement(textLines, (uint)xcout("%s%s\n%s%s %s", dlAddDates, addDate, indent, title, href));

			memFree(href);
			memFree(addDate);
			memFree(title);

			goto next_line;
		}
		p = strstr(line, "<DT><H");

		if (p)
		{
			char *addDate;
			char *title;
			char *swrk;

			/*
				... <DT><H3 FOLDED ADD_DATE=" ... "> ... </H3>
				                               |      |
				                            addDate title
			*/
			q = TrySkipWord(p, "<DT><H3 FOLDED ADD_DATE=\"");

			if (!q)
				q = SkipWord(p, "<DT><H3 ADD_DATE=\""); // chrome

			p = q;
			addDate = GetWord(&p, '"');

			while (*p && *p != '>') p++; // chrome

			errorCase(*p != '>'); p++;
			title = GetWord(&p, '<');
			p = SkipWord(p, "/H3>");
			errorCase(*p);

			dlAddDates = addLine(dlAddDates, addDate);
			dlAddDates = addLine(dlAddDates, swrk = xcout("%010u", treeIndex)); memFree(swrk);
			dlAddDates = addChar(dlAddDates, '+');
			treeIndex++;

			title = TitleFltr(title);

			addElement(textLines, (uint)xcout("%s\n%s%s", dlAddDates, indent, title));

			indent = addLine(indent, INDENTSTR);

			memFree(addDate);
			memFree(title);

			goto next_line;
		}
		p = strstr(line, " </DL>"); // 最終の </DL> を避けるため

		if (p)
		{
			errorCase(!*dlAddDates);
			p = strchr(dlAddDates, '\0') - 1;
			errorCase(*p != '+');
			*p = '\0';

			p = mbs_strrchr(dlAddDates, '+');
			errorCase(!p);
			p[1] = '\0';

			errorCase(strlen(indent) < strlen(INDENTSTR));
			indent[strlen(indent) - strlen(INDENTSTR)] = '\0';
		}
	next_line:;
	}
	rapidSortLines(textLines);

	foreach (textLines, line, index)
	{
		char *p = strchr(line, '\n');
		errorCase(!p);
		copyLine(line, p + 1);
	}

	wkfile = makeTempPath("txt");
	writeLines(wkfile, textLines);
	execute(wkfile);
	remove(wkfile);
	memFree(wkfile);

	releaseDim(lines, 1);
	releaseDim(textLines, 1);
	memFree(dlAddDates);
	memFree(indent);
}

int main(int argc, char **argv)
{
	char *file = dropFile();

	if (argIs("/SJIS"))
	{
		BookmarkToText(file);
	}
	else
	{
		char *wkfile = makeTempPath("sjis");

		UTF8ToSJISFile(file, wkfile);
		BookmarkToText(wkfile);
		removeFile(wkfile);
		memFree(wkfile);
	}
	memFree(file);
}
