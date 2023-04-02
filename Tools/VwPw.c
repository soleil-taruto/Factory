/*
	VwPw.exe [/T] [PASSWORD]

		/T ... コンソールに表示
*/

#include "C:\Factory\Common\all.h"

static int DispTextMode;

static void ViewPasswordBrowser(char *pw, uint pwLen)
{
	char *htmlFile = makeTempPath("html");
	FILE *htmlFp;
	uint index;

	htmlFp = fileOpen(htmlFile, "wt");

	writeLine(htmlFp, "<pre style=\"font: 60px 'Courier New';\">");

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
	writeLine(htmlFp, "");

	for (index = 0; index < pwLen; index++)
	{
		uint c = index % 16 / 12;

		if (index && index % 4 == 0)
			writeToken(htmlFp, " ");

		writeToken_x(htmlFp, xcout("%c", " #"[c]));
	}
	writeLine(htmlFp, "");
	writeLine(htmlFp, "</pre>");

	fileClose(htmlFp);

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

	if (hasArgs(1))
	{
		ViewPassword(nextArg());
	}
	else
	{
		ViewPassword(inputLine());
	}
}
