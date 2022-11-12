/*
	tescape.exe [/E | /U] [INPUT-FILE [OUTPUT-FILE]]
*/

#include "C:\Factory\Common\all.h"

static FILE *RFp;
static FILE *WFp;
static int Backed;
static int BackedChr;

static int NextChar(void)
{
	if (Backed)
	{
		Backed = 0;
		return BackedChr;
	}
	return readChar(RFp);
}
static void BackChar(int chr)
{
	errorCase(Backed);
	Backed = 1;
	BackedChr = chr;
}
static void WriteChar(int chr)
{
	writeChar(WFp, chr);
}
static void DoEscape(char *rFile, char *wFile)
{
	RFp = fileOpen(rFile, "rb");
	WFp = fileOpen(wFile, "wb");

	for (; ; )
	{
		int chr = NextChar();
		int chr2;
		int mbChr;

		if (chr == EOF)
			break;

		chr2 = NextChar();
		mbChr = chr << 8 | chr2;

//		if (isJChar(mbChr))
		if (isSJISChar(mbChr))
		{
			WriteChar(chr);
			WriteChar(chr2);
		}
		else
		{
			BackChar(chr2);

			if (chr == '\\')
			{
				WriteChar('\\');
				WriteChar('\\');
			}
			else if (
				chr == 0x09 ||
				chr == 0x0a ||
				0x20 <= chr && chr <= 0x7e ||
				0xa1 <= chr && chr <= 0xdf
				)
			{
				WriteChar(chr);
			}
			else
			{
				WriteChar('\\');
				WriteChar(hexadecimal[(chr >> 4) & 0x0f]);
				WriteChar(hexadecimal[(chr >> 0) & 0x0f]);
			}
		}
	}
	fileClose(RFp);
	fileClose(WFp);
}
static void DoUnescape(char *rFile, char *wFile)
{
	RFp = fileOpen(rFile, "rb");
	WFp = fileOpen(wFile, "wb");

	for (; ; )
	{
		int chr = NextChar();

		if (chr == EOF)
			break;

		if (chr == '\\')
		{
			chr = NextChar();

			if (chr != '\\')
			{
				int chr2 = NextChar();

				chr = m_c2i(chr) << 8 | m_c2i(chr2);
			}
		}
		WriteChar(chr);
	}
	fileClose(RFp);
	fileClose(WFp);
}
static void DoEscapeMain(int doEscapeFlag, char *rFile, char *wFile)
{
	if (doEscapeFlag)
		DoEscape(rFile, wFile);
	else
		DoUnescape(rFile, wFile);
}
static void EscapeMain(int doEscapeFlag)
{
	if (hasArgs(2))
	{
		DoEscapeMain(doEscapeFlag, getArg(0), getArg(1));
		return;
	}

	if (hasArgs(1))
	{
		DoEscapeMain(doEscapeFlag, nextArg(), c_getOutFile("tescape_out.txt"));
		openOutDir();
		return;
	}

	{
		DoEscapeMain(doEscapeFlag, c_dropFile(), c_getOutFile("tescape_out.txt"));
		openOutDir();
	}
}
int main(int argc, char **argv)
{
	if (argIs("/E"))
	{
		EscapeMain(1);
		return;
	}
	if (argIs("/U"))
	{
		EscapeMain(0);
		return;
	}
	EscapeMain(1);
}
