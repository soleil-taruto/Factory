#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *newLine = "\r\n";
	FILE *rfp;
	FILE *wfp;

	if (argIs("/CR"))
	{
		newLine = "\r";
	}
	if (argIs("/LF"))
	{
		newLine = "\n";
	}
	if (argIs("/CRLF"))
	{
		newLine = "\r\n";
	}

	rfp = fileOpen(nextArg(), "rb");
	wfp = fileOpen(nextArg(), "wb");

	for (; ; )
	{
		int chr = readChar(rfp);

	charProc:
		if (chr == EOF)
			break;

		if (chr == '\r')
		{
			writeToken(wfp, newLine);
			chr = readChar(rfp);

			if (chr != '\n')
				goto charProc;
		}
		else if (chr == '\n')
		{
			writeToken(wfp, newLine);
		}
		else
		{
			writeChar(wfp, chr);
		}
	}

	fileClose(rfp);
	fileClose(wfp);
}
