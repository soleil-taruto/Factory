/*
	テキストファイルの文字コードを SJIS から UTF-16 または UTF-8 に変換する。またその逆を行う。
	でたらめな内容のファイルを入力しても正常に動作するが、出力ファイルの内容は保障しない。

	[使い方]

	* 直前の UTF_BE, UTF_NoWriteBOM の値は不定とする。

	SJIS -> UTF-16_LE
		UTF_BE = 0;
		UTF_NoWriteBOM = 0;
		SJISToUTF16File(R_FILE, W_FILE);

	SJIS -> UTF-16_BE
		UTF_BE = 1;
		UTF_NoWriteBOM = 0;
		SJISToUTF16File(R_FILE, W_FILE);

	SJIS -> UTF-16_LE (NO_BOM)
		UTF_BE = 0;
		UTF_NoWriteBOM = 1;
		SJISToUTF16File(R_FILE, W_FILE);

	SJIS -> UTF-16_BE (NO_BOM)
		UTF_BE = 1;
		UTF_NoWriteBOM = 1;
		SJISToUTF16File(R_FILE, W_FILE);

	SJIS -> UTF-8
		SJISToUTF8File(R_FILE, W_FILE);

	UTF-16 -> SJIS
		UTF16ToSJISFile(R_FILE, W_FILE);

	UTF-16_LE (NO_BOM) -> SJIS
		UTF_BE = 0;
		UTF16ToSJISFile(R_FILE, W_FILE);

	UTF-16_BE (NO_BOM) -> SJIS
		UTF_BE = 1;
		UTF16ToSJISFile(R_FILE, W_FILE);

	UTF-8 -> SJIS
		UTF8ToSJISFile(R_FILE, W_FILE);
*/

#include "UTF.h"

int UTF_BE;
int UTF_NoWriteBOM;
int UTF_UseJIS0208;

static int ReadUTF16Char(FILE *fp)
{
	int chr1;
	int chr2;

restart:
	chr1 = readChar(fp);

	if (chr1 == EOF)
		return EOF;

	chr2 = readChar(fp);

	if (chr1 == 0xff && chr2 == 0xfe) // LE
	{
		UTF_BE = 0;
		goto restart;
	}
	if (chr1 == 0xfe && chr2 == 0xff) // BE
	{
		UTF_BE = 1;
		goto restart;
	}
	if (UTF_BE)
		return chr1 << 8 | chr2;

	return chr1 | chr2 << 8;
}
void UTF16ToSJISFile(char *rFile, char *wFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = fileOpen(wFile, "wb");

	for (; ; )
	{
		int chr = ReadUTF16Char(rfp);

		if (chr == EOF)
			break;

		if (UTF_UseJIS0208)
			chr = convCharUTF16ToSJIS(chr);
		else
			chr = convCharUTF16ToCP932(chr);

		if (chr == -1)
		{
			cout("Warning: UTF-16 処理できない文字\n");
			chr = 0x8148; // "？"
		}
		if (chr & 0xff00)
			writeChar(wfp, chr >> 8);

		writeChar(wfp, chr & 0xff);
	}
	fileClose(rfp);
	fileClose(wfp);
}
static void WriteUTF16Char(FILE *fp, int chr1, int chr2)
{
	if (UTF_BE)
	{
		writeChar(fp, chr2);
		writeChar(fp, chr1);
	}
	else
	{
		writeChar(fp, chr1);
		writeChar(fp, chr2);
	}
}
void SJISToUTF16File(char *rFile, char *wFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = fileOpen(wFile, "wb");

	if (!UTF_NoWriteBOM)
	{
		WriteUTF16Char(wfp, 0xff, 0xfe);
	}
	for (; ; )
	{
		int chr = readChar(rfp);

		if (chr == EOF)
			break;

		if (_ismbblead(chr))
			chr = chr << 8 | readChar(rfp);

		if (UTF_UseJIS0208)
			chr = convCharSJISToUTF16(chr);
		else
			chr = convCharCP932ToUTF16(chr);

		if (chr == -1)
		{
			cout("Warning: SJIS 処理できない文字\n");
			chr = 0xff1f; // "？"
		}
		WriteUTF16Char(wfp, chr & 0xff, chr >> 8);
	}
	fileClose(rfp);
	fileClose(wfp);
}

static uint GetUTF8Size(int chr)
{
	if (!(chr & 0x80)) return 1;
	if (!(chr & 0x20)) return 2;
	if (!(chr & 0x10)) return 3;
	if (!(chr & 0x08)) return 4;
	if (!(chr & 0x04)) return 5;

	return 6;
}
void UTF8ToUTF16File(char *rFile, char *wFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = fileOpen(wFile, "wb");

	if (!UTF_NoWriteBOM)
	{
		WriteUTF16Char(wfp, 0xff, 0xfe);
	}
	for (; ; )
	{
		int chr = readChar(rfp);
		uint size;
		uint c;

		if (chr == EOF)
			break;

		size = GetUTF8Size(chr);

		for (c = 1; c < size; c++)
		{
			chr <<= 8;
			chr |= readChar(rfp);
		}
		switch (size)
		{
		case 3:
			chr = (chr & 0xf0000) >> 4 | (chr & 0x3f00) >> 2 | chr & 0x3f;
			break;

		case 2:
			chr = (chr & 0x1f00) >> 2 | chr & 0x3f;
			break;

		case 1:
			break;

		default:
			cout("Warning: UTF-8 処理できない有効ビット数\n");
			chr = 0xff1f; // "？"
			break;
		}
		WriteUTF16Char(wfp, chr & 0xff, chr >> 8);
	}
	fileClose(rfp);
	fileClose(wfp);
}
void UTF16ToUTF8File(char *rFile, char *wFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = fileOpen(wFile, "wb");

	for (; ; )
	{
		int chr = ReadUTF16Char(rfp);

		if (chr == EOF)
			break;

		if (chr & 0xf800)
		{
			chr = 0xe08080 | (chr & 0xf000) << 4 | (chr & 0xfc0) << 2 | chr & 0x3f;

			writeChar(wfp, chr >> 16);
			writeChar(wfp, chr >> 8 & 0xff);
			writeChar(wfp, chr & 0xff);
		}
		else if (chr & 0xff80)
		{
			chr = 0xc080 | (chr & 0x7c0) << 2 | chr & 0x3f;

			writeChar(wfp, chr >> 8);
			writeChar(wfp, chr & 0xff);
		}
		else
		{
			writeChar(wfp, chr);
		}
	}
	fileClose(rfp);
	fileClose(wfp);
}

static void DblConv(char *rFile, char *wFile, void (*conv1)(char *, char *), void (*conv2)(char *, char *))
{
	char *midFile = makeTempPath("utf-mid");

	conv1(rFile, midFile);
	conv2(midFile, wFile);

	removeFile(midFile);
	memFree(midFile);
}
void UTF8ToSJISFile(char *rFile, char *wFile) // rFile == wFile ok
{
	DblConv(rFile, wFile, UTF8ToUTF16File, UTF16ToSJISFile);
}
void SJISToUTF8File(char *rFile, char *wFile) // rFile == wFile ok
{
	DblConv(rFile, wFile, SJISToUTF16File, UTF16ToUTF8File);
}

static char *DblConvText(char *text, void (*conv)(char *, char *))
{
	char *tmpFile = makeTempPath("u2s-mid");

	writeOneLineNoRet_b(tmpFile, text);
	conv(tmpFile, tmpFile);
	text = readText_b(tmpFile);

	removeFile(tmpFile);
	memFree(tmpFile);

	return text;
}
char *UTF8ToSJISText(char *text)
{
	return DblConvText(text, UTF8ToSJISFile);
}
char *SJISToUTF8Text(char *text)
{
	return DblConvText(text, SJISToUTF8File);
}
char *UTF8ToSJISText_x(char *text)
{
	char *ret = UTF8ToSJISText(text);
	memFree(text);
	return ret;
}
char *SJISToUTF8Text_x(char *text)
{
	char *ret = SJISToUTF8Text(text);
	memFree(text);
	return ret;
}
