/*
	UTF2SJIS.exe [/B] [/S] [/SF] R-ENC W-ENC [/LSS | 対象ファイル | 対象ディレクトリ]

		/B    ... バッチモード
		/S    ... ディレクトリを指定する場合、サブディレクトリも対象とする。
		/SF   ... セーフモード == 読み込み文字コードが違うまたは内容が変化しない場合、何もしない。
		/LSS  ... FOUNDLISTFILE を対象とする。
		R-ENC ... 読み込み文字コード
		W-ENC ... 書き出し文字コード

		★ R-ENC, W-ENC の少なくともどちらかを SJIS にしなければならない。

		- - -
		読み込み文字コード

		SJIS    ... Shift_JIS
		UTF16   ... UTF-16(LE) | UTF-16(BE) | UTF-16(LE)NoBOM
		UTF16BE ... UTF-16(LE) | UTF-16(BE) | UTF-16(BE)NoBOM
		UTF8    ... UTF-8

		- - -
		書き出し文字コード

		SJIS          ... Shift_JIS
		UTF16         ... UTF-16(LE)
		UTF16-NOBOM   ... UTF-16(LE)NoBOM
		UTF16BE       ... UTF-16(BE)
		UTF16BE-NOBOM ... UTF-16(BE)NoBOM
		UTF8          ... UTF-8
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

typedef enum Enc_et
{
	ENC_SJIS = 1,
	ENC_UTF16,
	ENC_UTF16_NOBOM,
	ENC_UTF16BE,
	ENC_UTF16BE_NOBOM,
	ENC_UTF8,
}
Enc_t;

static int BatchMode;
static int IntoSubDir;
static int SafeMode;
static Enc_t REnc;
static Enc_t WEnc;

static Enc_t GetEnc(char *str)
{
	if (!_stricmp(str, "SJIS"))          return ENC_SJIS;
	if (!_stricmp(str, "UTF16"))         return ENC_UTF16;
	if (!_stricmp(str, "UTF16-NOBOM"))   return ENC_UTF16_NOBOM;
	if (!_stricmp(str, "UTF16BE"))       return ENC_UTF16BE;
	if (!_stricmp(str, "UTF16BE-NOBOM")) return ENC_UTF16BE_NOBOM;
	if (!_stricmp(str, "UTF8"))          return ENC_UTF8;

	error();
	return -1;
}
static char *GetSEnc(Enc_t enc)
{
	switch (enc)
	{
	case ENC_SJIS:          return "SJIS";
	case ENC_UTF16:         return "UTF16";
	case ENC_UTF16_NOBOM:   return "UTF16-NOBOM";
	case ENC_UTF16BE:       return "UTF16BE";
	case ENC_UTF16BE_NOBOM: return "UTF16BE-NOBOM";
	case ENC_UTF8:          return "UTF8";
	}
	error();
	return NULL;
}
static void DoConv3(char *rFile, char *wFile)
{
	UTF_BE = 0;
	UTF_NoWriteBOM = 0;

	if (REnc == ENC_SJIS)
	{
		switch (WEnc)
		{
		case ENC_SJIS:
			copyFile(rFile, wFile);
			return;

		case ENC_UTF16:
			SJISToUTF16File(rFile, wFile);
			return;

		case ENC_UTF16_NOBOM:
			UTF_NoWriteBOM = 1;
			SJISToUTF16File(rFile, wFile);
			return;

		case ENC_UTF16BE:
			UTF_BE = 1;
			SJISToUTF16File(rFile, wFile);
			return;

		case ENC_UTF16BE_NOBOM:
			UTF_BE = 1;
			UTF_NoWriteBOM = 1;
			SJISToUTF16File(rFile, wFile);
			return;

		case ENC_UTF8:
			SJISToUTF8File(rFile, wFile);
			return;
		}
	}
	else
	{
		switch (REnc)
		{
		case ENC_UTF16:
			UTF16ToSJISFile(rFile, wFile);
			return;

		case ENC_UTF16BE:
			UTF_BE = 1;
			UTF16ToSJISFile(rFile, wFile);
			return;

		case ENC_UTF8:
			UTF8ToSJISFile(rFile, wFile);
			return;
		}
	}
	error();
}
static int IsNeedConv(char *rFile, char *wFile, char *rFile2)
{
	if (!isSameFile(rFile, rFile2))
	{
		cout("読み込み文字コードが違うようです。\n");
		return 0;
	}
	if (isSameFile(rFile, wFile))
	{
		cout("変化無し。\n");
		return 0;
	}
	return 1;
}
static int DoConv2(char *rFile, char *wFile)
{
	int ret = 1;

	if (SafeMode)
	{
		char *midFile = makeTempPath(NULL);
		char *retFile = makeTempPath(NULL);

		DoConv3(rFile, midFile);
		m_swap(REnc, WEnc, uint);
		DoConv3(midFile, retFile);
		m_swap(REnc, WEnc, uint); // 復元

		if (!IsNeedConv(rFile, midFile, retFile))
		{
			removeFile(midFile);
			ret = 0;
		}
		else
		{
			moveFile(midFile, wFile);
		}
		removeFile(retFile);

		memFree(midFile);
		memFree(retFile);
	}
	else
	{
		DoConv3(rFile, wFile);
	}
	return ret;
}
static void DoConv(autoList_t *files)
{
	char *file;
	uint index;

	foreach (files, file, index)
		cout("(%u) %s\n", index, file);

	cout("文字コード変換: %s -> %s\n", GetSEnc(REnc), GetSEnc(WEnc));

	if (!BatchMode)
	{
		cout("続行？\n");

		if (clearGetKey() == 0x1b)
			termination(0);

		cout("続行します。\n");
	}

	foreach (files, file, index)
	{
		cout("[%u] %s\n", index, file);

		errorCase(!existFile(file));
		writeBinary_cx(file, readBinary(file)); // I/O Test
	}
	foreach (files, file, index)
	{
		char *midFile = makeTempPath("UTF2SJIS.tmp");

		cout("<%u> %s\n", index, file);

		if (DoConv2(file, midFile))
		{
			LOGPOS();
			removeFile(file);
			LOGPOS();
			moveFile(midFile, file);
			LOGPOS();
		}
		memFree(midFile);
	}
}
static void DoConv_File(char *file)
{
	autoList_t *files = newList();

	addElement(files, (uint)file);
	DoConv(files);
	releaseAutoList(files);
}
static void DoConv_Path(char *path)
{
	if (existDir(path))
	{
		autoList_t *files = (IntoSubDir ? lssFiles : lsFiles)(path);

		DoConv(files);
		releaseDim(files, 1);
	}
	else
	{
		DoConv_File(path);
	}
}
int main(int argc, char **argv)
{
readArgs:
	if (argIs("/B"))
	{
		BatchMode = 1;
		goto readArgs;
	}
	if (argIs("/S"))
	{
		IntoSubDir = 1;
		goto readArgs;
	}
	if (argIs("/SF"))
	{
		LOGPOS();
		SafeMode = 1;
		goto readArgs;
	}

	REnc = GetEnc(nextArg());
	WEnc = GetEnc(nextArg());

	errorCase(REnc == ENC_UTF16_NOBOM);
	errorCase(REnc == ENC_UTF16BE_NOBOM);
	errorCase(REnc != ENC_SJIS && WEnc != ENC_SJIS);

	if (argIs("/LSS"))
	{
		autoList_t *files = readLines(FOUNDLISTFILE);

		DoConv(files);
		releaseDim(files, 1);
		return;
	}

	if (hasArgs(1))
	{
		DoConv_Path(nextArg());
		return;
	}

	for (; ; )
	{
		DoConv_Path(c_dropDirFile());
		cout("\n");
	}
}
