/*
	VwPw.exe [/T] [/-C] [PASSWORD]

		/T   ... コンソールに表示
		/-C  ... 色無し
*/

#include "C:\Factory\Common\all.h"

static int DispTextMode;
static int NoColorMode;

static char *PCLAddOpenSpan(char *buff, uint color)
{
	char *foreColors[] =
	{
		"000000",
		"880000",
		"004400",
		"000088",
		"444400",
		"440044",
		"006666",
	};

	char *backColors[] =
	{
		"eeeeee",
		"ffdddd",
		"eeffee",
		"ddddff",
		"ffffee",
		"ffeeff",
		"ddffff",
	};

	buff = addLine_x(buff, xcout(
		"<span style=\"color: #%s; background-color: #%s;\">"
		, foreColors[color % lengthof(foreColors)]
		, backColors[color % lengthof(backColors)]
		));

	return buff;
}
static char *PCLAddCloseSpan(char *buff)
{
	return addLine(buff, "</span>");
}
static char *PutColorLine_x(char *line)
{
	char *buff = strx("");
	uint i;
	uint color = 0;

	buff = PCLAddOpenSpan(buff, color++);

	for (i = 0; line[i]; i++)
	{
		if (i && i % 5 == 0)
		{
			buff = PCLAddCloseSpan(buff);
			buff = PCLAddOpenSpan(buff, color++);
		}
		buff = addChar(buff, line[i]);
	}
	buff = PCLAddCloseSpan(buff);
	memFree(line);
	return buff;
}
static void PutColorFile(char *file)
{
	autoList_t *lines = readLines(file);
	char *line1;
	char *line2;
	char *line3;

	line1 = getLine(lines, 1);
	line2 = getLine(lines, 2);
	line3 = getLine(lines, 3);

	line1 = PutColorLine_x(line1);
	line2 = PutColorLine_x(line2);
	line3 = PutColorLine_x(line3);

	setElement(lines, 1, (uint)line1);
	setElement(lines, 2, (uint)line2);
	setElement(lines, 3, (uint)line3);

	writeLines_cx(file, lines);
}
static void ViewPasswordBrowser(char *pw, uint pwLen)
{
	char *htmlFile = makeTempPath("html");
	FILE *htmlFp;
	uint index;

	htmlFp = fileOpen(htmlFile, "wt");

	writeLine(htmlFp, "<pre style=\"font: 60px 'Consolas';\">");

	for (index = 0; index < pwLen; index++)
	{
		if (index && index % 4 == 0)
			writeToken(htmlFp, " ");

		writeToken_x(htmlFp, xcout("%c", pw[index]));
	}
	writeLine(htmlFp, "");

	for (index = 0; index < pwLen; index++)
	{
		uint c = index % 8 / 4;
		uint d = index / 8 % 2;

		if (index && index % 4 == 0)
			writeToken(htmlFp, " ");

		writeToken_x(htmlFp, xcout("%c", "=- "[c ? d : 2]));
	}
	writeLine(htmlFp, ";");

	for (index = 0; index < pwLen; index++)
	{
		uint c = index % 16 / 12;

		if (index && index % 4 == 0)
			writeToken(htmlFp, " ");

		writeToken_x(htmlFp, xcout("%c", " #"[c]));
	}
	writeLine(htmlFp, ";");
	writeLine(htmlFp, "</pre>");

	fileClose(htmlFp);

	if (!NoColorMode)
		PutColorFile(htmlFile);

	execute_x(xcout("START \"\" \"%s\"", htmlFile));

	memFree(htmlFile);
}
static void ViewPassword(char *pw)
{
	uint pwLen = strlen(pw);
	uint index;

	if (!pwLen)
		return;

	errorCase(!isAsciiLine(pw, 0, 0, 0));

	if (!DispTextMode)
	{
		ViewPasswordBrowser(pw, pwLen);
		return;
	}

	cout("\n");

	for (index = 0; index < pwLen; index++)
	{
		if (index && index % 4 == 0)
			cout(" ");

		cout("%c", pw[index]);
	}
	cout("\n");

	for (index = 0; index < pwLen; index++)
	{
		uint c = index % 8 / 4;
		uint d = index / 8 % 2;

		if (index && index % 4 == 0)
			cout(" ");

		cout("%c", "=- "[c ? d : 2]);
	}
	cout("\n");

	for (index = 0; index < pwLen; index++)
	{
		uint c = index % 16 / 12;

		if (index && index % 4 == 0)
			cout(" ");

		cout("%c", " #"[c]);
	}
	cout("\n");
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/T"))
	{
		DispTextMode = 1;
		goto readArgs;
	}
	if (argIs("/-C"))
	{
		NoColorMode = 1;
		goto readArgs;
	}

	if (hasArgs(1))
	{
		ViewPassword(nextArg());
	}
	else
	{
		ViewPassword(inputLine());
	}
}
