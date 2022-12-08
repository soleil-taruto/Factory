/*
	RegExpToText.exe 入力ファイル 出力ファイル

	入力ファイル：

		reg export レジストリのパス regファイル

		...のregファイル

	出力ファイルの内容：

		Loop
		{
			パス (HKxx \ 場所 \ キー) <改行>
			型 <改行>
			値 <改行>
		}
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\UTF.h"

static FILE *RFp;
static FILE *WFp;
static char *LastLine;
static char *CurrPath;

static char *ReadLine_N(void)
{
	memFree(LastLine);
	LastLine = readLine(RFp);
	return LastLine;
}
static char *ReadLine(void)
{
	errorCase(!ReadLine_N());
	return LastLine;
}
static char *ReadJoin(void)
{
	char *buff = strx("");
	char *p;

	while ((p = mbs_strrchr(LastLine, '\\')) && !p[1])
	{
		*p = '\0';
		buff = addLine(buff, LastLine);
		ReadLine();
	}
	return addLine(buff, LastLine);
}
static void WriteLine(char *line)
{
	writeLine(WFp, line);
}
static void WriteLine_x(char *line)
{
	WriteLine(line);
	memFree(line);
}
static char *HKEY_xxxToHKxx(char *src)
{
	if (!strcmp(src, "HKEY_CLASSES_ROOT"))
		return "HKCR";

	if (!strcmp(src, "HKEY_CURRENT_USER"))
		return "HKCU";

	if (!strcmp(src, "HKEY_LOCAL_MACHINE"))
		return "HKLM";

	if (!strcmp(src, "HKEY_USERS"))
		return "HKU";

	if (!strcmp(src, "HKEY_CURRENT_CONFIG"))
		return "HKCC";

	error();
	return NULL; // never
}

static char *PV_Type;
static char *PV_Value;

static void ParseValue(char *value)
{
	memFree(PV_Type);
	memFree(PV_Value);

	if (lineExp("\"<>\"", value))
	{
		char *r;
		char *w;

		w = value;

		for (r = value + 1; *r != '"'; r++)
		{
			errorCase(!*r);

			if (*r == '\\')
			{
				r++;
				errorCase(!*r);
			}
			*w++ = *r;
		}
		errorCase(r[1]);

		*w = '\0';

		line2JLine(value, 1, 0, 1, 1);

		PV_Type = strx("text");
		PV_Value = strx(value);

		return;
	}
	if (lineExp("dword:<8,09af>", value))
	{
		value += 6;

		PV_Type = strx("dword");
		PV_Value = xcout("%s %u", value, toValueDigits(value, hexadecimal));

		return;
	}
	if (lineExp("hex:<>", value))
	{
		char *r;
		char *w;

		value += 4;

		w = value;

		for (r = value; *r; r++)
			if (m_ishexadecimal(*r))
				*w++ = *r;

		*w = '\0';

		errorCase(strlen(value) % 2 != 0);

		PV_Type = strx("hex");
		PV_Value = strx(value);

		return;
	}
	error();
}
static void Main3(char *rFile, char *wFile)
{
	RFp = fileOpen(rFile, "rt");
	WFp = fileOpen(wFile, "wt");

	errorCase(strcmp(ReadLine(), "Windows Registry Editor Version 5.00")); // バージョンとか変わるんじゃね？
	errorCase(strcmp(ReadLine(), ""));

	while (ReadLine_N())
	{
		char *line = LastLine;

		if (line[0] == '[') // ex. [HKEY_CURRENT_USER\xxx\xxx\xxx]
		{
			autoList_t *ptkns;
			char *ptkn;
			char *ptkn_new;
			uint ptkn_index;

			errorCase(CurrPath);
			errorCase(!lineExp("[<>]", line));

			line++;
			strchr(line, '\0')[-1] = '\0';
			ptkns = tokenizeYen_heap(line);

			errorCase(getCount(ptkns) < 2); // HKxx だけってことは無いだろう。

			foreach (ptkns, ptkn, ptkn_index)
				errorCase(!lineExp("<1,,09AZaz__%%..>", ptkn)); // パストークンの書式チェック

			ptkn = getLine(ptkns, 0);
			ptkn_new = strx(HKEY_xxxToHKxx(ptkn));
			memFree(ptkn);
			setElement(ptkns, 0, (uint)ptkn_new);

			memFree(CurrPath);
			CurrPath = untokenize(ptkns, "\\");
		}
		else if (line[0] == '@') // ex. @="xxx"
		{
			errorCase(!CurrPath);
			errorCase(!lineExp("@=\"<>\"", line));

			ParseValue(line + 2);

			WriteLine_x(xcout("%s\\@", CurrPath));
			WriteLine(PV_Type);
			WriteLine(PV_Value);
		}
		else if (line[0] == '"') // ex. "xxx"=dword:00000000 , "yyy"="zzz"
		{
			char *buff;
			char *key;
			char *value;

			errorCase(!CurrPath);

			buff = ReadJoin();

			errorCase(!lineExp("\"<1,,09AZaz__>\"=<>", buff)); // キーの書式チェック

			key   = toknext(buff + 1, "\"");
			value = toknext(NULL, "") + 1;

			ParseValue(value);

			WriteLine_x(xcout("%s\\%s", CurrPath, key));
			WriteLine(PV_Type);
			WriteLine(PV_Value);

			memFree(buff);
		}
		else if (line[0] == '\0')
		{
			errorCase(!CurrPath);

			memFree(CurrPath);
			CurrPath = NULL;
		}
		else
		{
			error();
		}
	}
	errorCase(CurrPath);

	fileClose(RFp);
	fileClose(WFp);
}
static void Main2(char *rFile, char *wFile)
{
	if (existFile(rFile))
	{
		char *midFile = makeTempPath(NULL);

		LOGPOS();
		UTF16ToSJISFile(rFile, midFile);
		LOGPOS();
		Main3(midFile, wFile);
		LOGPOS();

		removeFile(midFile);
		memFree(midFile);
	}
	else
	{
		cout("regファイルがありません。\n");

		createFile(wFile);
	}
}
int main(int argc, char **argv)
{
	Main2(getArg(0), getArg(1));
}
