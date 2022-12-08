/*
	crlf.exe [(/CR | /CRLF | /LF) [/K]] (/LSS | 対象ファイル名)

		改行コードを指定しなかった場合 ... チェックのみ
		改行コードを指定した場合       ... 指定された改行コードに置き換える。
*/

#include "C:\Factory\Common\all.h"

static void CheckFile(char *file)
{
	FILE *fp = fileOpen(file, "rb");
	int backed = 0;
	int backedChr;
	int chr;
	uint num_cr = 0;
	uint num_crlf = 0;
	uint num_lf = 0;

	cout("%s\n", file);

	for (; ; )
	{
		if (backed)
		{
			backed = 0;
			chr = backedChr;
		}
		else
		{
			chr = readChar(fp);
		}

		if (chr == EOF)
			break;

		if (chr == '\r')
		{
			chr = readChar(fp);

			if (chr != '\n')
			{
				backed = 1;
				backedChr = chr;

				num_cr++;
			}
			else
			{
				num_crlf++;
			}
		}
		else if (chr == '\n')
		{
			num_lf++;
		}
	}
	fileClose(fp);

	cout("CR    %u\n", num_cr);
	cout("CR-LF %u\n", num_crlf);
	cout("LF    %u\n", num_lf);
}

static char *Conv_NewLine;
static int Conv_KeepTimeStamp;

static void ConvFile(char *file)
{
	char *midFile = makeTempFile(NULL);
	FILE *rfp;
	FILE *wfp;
	int backed = 0;
	int backedChr;
	int chr;
	uint64 cTm;
	uint64 aTm;
	uint64 uTm;

	cout("Conv: %s\n", file);

	if (Conv_KeepTimeStamp)
		getFileStamp(file, &cTm, &aTm, &uTm);

	rfp = fileOpen(file, "rb");
	wfp = fileOpen(midFile, "wb");

	for (; ; )
	{
		if (backed)
		{
			backed = 0;
			chr = backedChr;
		}
		else
		{
			chr = readChar(rfp);
		}

		if (chr == EOF)
			break;

		if (chr == '\r')
		{
			chr = readChar(rfp);

			if (chr != '\n')
			{
				backed = 1;
				backedChr = chr;
			}
			writeToken(wfp, Conv_NewLine);
		}
		else if (chr == '\n')
		{
			writeToken(wfp, Conv_NewLine);
		}
		else
		{
			writeChar(wfp, chr);
		}
	}
	fileClose(rfp);
	fileClose(wfp);

	semiRemovePath(file);
	moveFile(midFile, file);
	memFree(midFile);

	if (Conv_KeepTimeStamp)
		setFileStamp(file, cTm, aTm, uTm);
}

static void FileAction(char *file)
{
	if (Conv_NewLine)
		ConvFile(file);
	else
		CheckFile(file);
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/CR"))
	{
		Conv_NewLine = "\r";
		goto readArgs;
	}
	if (argIs("/CRLF"))
	{
		Conv_NewLine = "\r\n";
		goto readArgs;
	}
	if (argIs("/LF"))
	{
		Conv_NewLine = "\n";
		goto readArgs;
	}
	if (argIs("/K"))
	{
		Conv_KeepTimeStamp = 1;
		goto readArgs;
	}

	if (argIs("/LSS"))
	{
		autoList_t *files = readLines(FOUNDLISTFILE);
		char *file;
		uint index;

		foreach (files, file, index)
			FileAction(file);

		releaseDim(files, 1);
		return;
	}

	FileAction(nextArg());
}
