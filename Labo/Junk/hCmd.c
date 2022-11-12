/*
	hCmd.exe [<START-DIR> [<PORT>]?]?
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Labo\Socket\libs\http\MultiPart.h"

static autoList_t *TextBuffer;

static char *HtmlFltr(char *line)
{
	line2JLine(line, 1, 0, 1, 1);

	line = replaceLine(line, "&", "&amp;", 0);
	line = replaceLine(line, "<", "&lt;", 0);
	line = replaceLine(line, ">", "&gt;", 0);

	return line;
}
static void WritePrompt(char *file, char *cmdln)
{
	writeOneLine_cx(file, xcout("\n%s>%s", c_getCwd(), cmdln));
}
static int TryExecuteInnerCommand(char *cmdln, char *stdoutFile, char *stderrFile)
{
	if (startsWithICase(cmdln, "CD "))
	{
		char *dir = cmdln + 3;

		if (existDir(dir))
		{
			WritePrompt(stdoutFile, cmdln);
			changeCwd(dir);
			return 1;
		}
	}
	if (!_stricmp(cmdln, "CLS"))
	{
		releaseDim(TextBuffer, 1);
		TextBuffer = newList();
		return 1;
	}
	if (startsWithICase(cmdln, "START "))
	{
		WritePrompt(stdoutFile, cmdln);
		coExecute(cmdln);
		return 1;
	}
	return 0;
}
static int Perform(int sock, uint userInfo)
{
	SockStream_t *i = CreateSockStream(sock, 3600);
	char *header;
	autoList_t *parts;

	parts = httpRecvRequestMultiPart(i, &header);

	if (!_stricmp(c_httpGetPartLine(parts, "hCmdRequest"), "hCmdExecuteCommandLine"))
	{
		char *batchFile = makeTempPath("bat");
		char *stdoutFile = makeTempPath("stdout.txt");
		char *stderrFile = makeTempPath("stderr.txt");
		char *cmdln = httpGetPartLine(parts, "hCmdCommandLine");

		line2JLine(cmdln, 1, 0, 0, 1);
		cout("cmdln: %s\n", cmdln);
		writeOneLine(batchFile, cmdln);

		if (!TryExecuteInnerCommand(cmdln, stdoutFile, stderrFile))
			coExecute_x(xcout("> %s 2> %s %s", stdoutFile, stderrFile, batchFile));

		cout("cmded\n");

		if (!existFile(stdoutFile)) createFile(stdoutFile);
		if (!existFile(stderrFile)) createFile(stderrFile);

		addElements_x(TextBuffer, readLines(stdoutFile));
		addElements_x(TextBuffer, readLines(stderrFile));

		while (9999 < getCount(TextBuffer))
			memFree((char *)desertElement(TextBuffer, 0));

		removeFile(batchFile);
		removeFile(stdoutFile);
		removeFile(stderrFile);

		memFree(batchFile);
		memFree(stdoutFile);
		memFree(stderrFile);
		memFree(cmdln);
	}
	memFree(header);
	httpReleaseParts(parts);

	{
		char *resHtml = strx(
			"<html>"
			"<head>"
			"<meta http-equiv=\"Content-Type\" content=\"text/html; charset=Shift_JIS\"/>"
			"<title>hCmd</title>"
			"</head>"
			"<body onload=\"document.hCmdForm.hCmdCommandLine.focus();\">"
			"<pre>"
			"<code>"
			);

		{
			char *line;
			uint index;

			foreach (TextBuffer, line, index)
			{
				line = HtmlFltr(strx(line));

				if (!*line)
					line = addChar(line, ' ');

				line = addLine(line, "<br/>");
				resHtml = addLine_x(resHtml, line);
			}
		}

		resHtml = addLine(resHtml, " <br/>");
		resHtml = addLine_x(resHtml, HtmlFltr(xcout("%s>", c_getCwd())));
		resHtml = addLine(resHtml,
			"</code>"
			"</pre>"
			"<form name=\"hCmdForm\" method=\"post\" action=\"/\" enctype=\"multipart/form-data\" accept-charset=\"Shift_JIS\">"
			"<input type=\"hidden\" name=\"hCmdRequest\" value=\"hCmdExecuteCommandLine\"/>"
			"<input type=\"text\" name=\"hCmdCommandLine\" size=\"80\"/>"
			"<input type=\"submit\" value=\"ŽÀs\"/>"
			"</form>"
			"<a name=\"pBottom\"/>"
			"</body>"
			"</html>"
			);

		httpSendResponseText(i, resHtml, httpExtToContentType("html"));
		memFree(resHtml);
	}
	ReleaseSockStream(i);
	return 0;
}

static uint CreateInfo(void)
{
	return 0;
}
static void ReleaseInfo(uint userInfo)
{
	// noop
}
static int Idle(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("End hCmd\n");
			return 0;
		}
		cout("Press ESCAPE to end hCmd\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	char *startDir = ".";
	uint portno = 80;

	if (hasArgs(1))
		startDir = nextArg();

	if (hasArgs(1))
		portno = toValue(nextArg());

	TextBuffer = newList();

	addCwd(startDir);
	sockServerUserTransmit(Perform, CreateInfo, ReleaseInfo, portno, 1, Idle);
	unaddCwd();

	releaseDim(TextBuffer, 1);
}
