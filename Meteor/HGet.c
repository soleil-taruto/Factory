#include "HGet.h"

static char *GetHGetFile(void)
{
	static char *file;

	if (!file)
//		file = GetCollaboFile("C:\\app\\Kit\\HGet\\HGet.exe"); // 廃止 @ 2023.4.13
		file = GetCollaboFile("C:\\Factory\\tmp\\Kit_Dummy\\HGet\\HGet.exe");

	return file;
}

static int ProxyMode; // "DIS"
static char *ProxyHost;
static uint ProxyPortNo;
static uint ConnectionTimeoutMillis; // 接続開始から、応答ヘッダを受信し終えるまでのタイムアウト
static uint TimeoutMillis;           // 接続開始から、全て通信し終えるまでのタイムアウト
static uint NoTrafficTimeoutMillis;  // 応答ボディ受信中の無通信タイムアウト
static char *Url;
static uint HTTPVersion; // 10 or 11
static autoList_t *Header;    // { char *name, char *value } ...
static autoList_t *ResHeader; // { char *name, char *value } ...
static autoBlock_t *Body1; // NULL == no body-1
static char *Body2File;    // NULL == no body-2, 呼び出し側が指定したファイルなので、こちら側で変更・削除しない。
static autoBlock_t *Body3; // NULL == no body-3
static char *Body1TmpFile;
static char *Body3TmpFile;
static char *SuccessfulFile;
static char *ResHeaderFile;
static char *ResBodyFile;
static char *ParamFile;
static uint64 ResBodySizeMax;
static int Resetted;

/*
	HGet_ の中で最初に呼び出すこと。
	可能な限りプロセス終了前にも呼び出すこと。
*/
void HGet_Reset(void)
{
	ProxyMode = 'D';
	strzp(&ProxyHost, "localhost");
	ProxyPortNo = 8080;
	ConnectionTimeoutMillis = 20000;
	TimeoutMillis           = 30000;
	NoTrafficTimeoutMillis  = 15000;
	strzp(&Url, "http://localhost/");
	HTTPVersion = 11;
	if (Header)    { releaseDim(Header, 2);    }
	if (ResHeader) { releaseDim(ResHeader, 2); }
	Header = newList();
	ResHeader = newList();
	if (Body1) { releaseAutoBlock(Body1); }
	memFree(Body2File);
	if (Body3) { releaseAutoBlock(Body3); }
	Body1 = NULL;
	Body2File = NULL;
	Body3 = NULL;
	if (!Body1TmpFile)   { Body1TmpFile   = makeTempPath("HGet_Body1.tmp");          }
	if (!Body3TmpFile)   { Body3TmpFile   = makeTempPath("HGet_Body3.tmp");          }
	if (!SuccessfulFile) { SuccessfulFile = makeTempPath("HGet_Successful.flg.tmp"); }
	if (!ResHeaderFile)  { ResHeaderFile  = makeTempPath("HGet_ResHeader.tmp");      }
	if (!ResBodyFile)    { ResBodyFile    = makeTempPath("HGet_ResBody.tmp");        }
	if (!ParamFile)      { ParamFile      = makeTempPath("HGet_Param.tmp");          }
	ResBodySizeMax = IMAX_64;
	Resetted = 1;

	// clear
	{
		removeFileIfExist(Body1TmpFile);
		removeFileIfExist(Body3TmpFile);
		removeFileIfExist(SuccessfulFile);
		removeFileIfExist(ResHeaderFile);
		removeFileIfExist(ResBodyFile);
		removeFileIfExist(ParamFile);
	}
}
void HGet_SetProxy_IE(void)
{
	errorCase(!Resetted);

	ProxyMode = 'I';
}
void HGet_SetProxy(char *host, uint portNo)
{
	errorCase(!Resetted);
	errorCase(m_isEmpty(host));
	errorCase(!isLine(host));
	errorCase(!m_isRange(portNo, 1, 0xffff));

	ProxyMode = 'S';
	strzp(&ProxyHost, host);
	ProxyPortNo = portNo;
}
void HGet_SetConnectionTimeoutMillis(uint millis)
{
	errorCase(!Resetted);

	ConnectionTimeoutMillis = millis;
}
void HGet_SetTimeoutMillis(uint millis)
{
	errorCase(!Resetted);

	TimeoutMillis = millis;
}
void HGet_SetNoTrafficTimeoutMillis(uint millis)
{
	errorCase(!Resetted);

	NoTrafficTimeoutMillis = millis;
}
void HGet_SetUrl(char *url)
{
	errorCase(!Resetted);
	errorCase(m_isEmpty(url));
	errorCase(!isLine(url));

	strzp(&Url, url);
}
void HGet_SetHTTPVersion_10(void)
{
	errorCase(!Resetted);

	HTTPVersion = 10;
}
void HGet_AddHeaderField(char *name, char *value)
{
	autoList_t *pair;

	errorCase(!Resetted);
	errorCase(m_isEmpty(name));
	errorCase(m_isEmpty(value));
	errorCase(!isLine(name));
	errorCase(!isLine(value));

	pair = newList();

	addElement(pair, (uint)strx(name));
	addElement(pair, (uint)strx(value));

	addElement(Header, (uint)pair);
}
static void SetBody1(autoBlock_t *body)
{
	errorCase(!Resetted);
	errorCase(!body);

	if (Body1)
		releaseAutoBlock(Body1);

	Body1 = copyAutoBlock(body);
}
static void SetBody2File(char *file)
{
	errorCase(!Resetted);
	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));

	strzp(&Body2File, file);
}
static void SetBody3(autoBlock_t *body)
{
	errorCase(!Resetted);
	errorCase(!body);

	if (Body3)
		releaseAutoBlock(Body3);

	Body3 = copyAutoBlock(body);
}
void HGet_SetBody(autoBlock_t *body)
{
	SetBody1(body);
}
void HGet_SetBody_BB(autoBlock_t *body1, autoBlock_t *body2)
{
	SetBody1(body1);
	SetBody3(body2);
}
void HGet_SetBody_BF(autoBlock_t *body1, char *body2File)
{
	SetBody1(body1);
	SetBody2File(body2File);
}
void HGet_SetBody_BFB(autoBlock_t *body1, char *body2File, autoBlock_t *body3)
{
	SetBody1(body1);
	SetBody2File(body2File);
	SetBody3(body3);
}
void HGet_SetBody_F(char *bodyFile)
{
	SetBody2File(bodyFile);
}
void HGet_SetBody_FB(char *body1File, autoBlock_t *body2)
{
	SetBody2File(body1File);
	SetBody3(body2);
}
void HGet_SetResBodySizeMax(uint64 resBodySizeMax)
{
	errorCase(!Resetted);

	ResBodySizeMax = resBodySizeMax;
}
static int Perform(int method) // method: "HGP"
{
	FILE *fp = fileOpen(ParamFile, "wt");
	char *p;
	autoList_t *pair;
	uint index;
	char *line;

	writeLine(fp, "/P");

	switch (ProxyMode)
	{
	case 'D':
		writeLine(fp, "DIRECT");
		break;

	case 'I':
		writeLine(fp, "IE");
		break;

	case 'S':
		writeLine(fp, "SPECIAL");
		writeLine(fp, asLine(ProxyHost));
		writeLine_x(fp, xcout("%u", ProxyPortNo));
		break;

	default:
		error();
	}
	writeLine(fp, "/CT");
	writeLine_x(fp, xcout("%u", ConnectionTimeoutMillis));
	writeLine(fp, "/To");
	writeLine_x(fp, xcout("%u", TimeoutMillis));
	writeLine(fp, "/NTT");
	writeLine_x(fp, xcout("%u", NoTrafficTimeoutMillis));
	writeLine(fp, "/M");

	switch (method)
	{
	case 'H': p = "HEAD"; break;
	case 'G': p = "GET";  break;
	case 'P': p = "POST"; break;

	default:
		error();
	}
	writeLine(fp, p);
	writeLine(fp, "/U");
	writeLine(fp, asLine(Url));
	writeLine(fp, "/V");
	writeLine_x(fp, xcout("%u", HTTPVersion));

	foreach (Header, pair, index)
	{
		writeLine(fp, "/H");
		writeLine(fp, asLine(getLine(pair, 0)));
		writeLine(fp, asLine(getLine(pair, 1)));
	}
	if (Body1)
	{
		writeBinary(Body1TmpFile, Body1);

		writeLine(fp, "/B");
		writeLine(fp, Body1TmpFile);
	}
	if (Body2File)
	{
		writeLine(fp, "/F");
		writeLine(fp, Body2File);
	}
	if (Body3)
	{
		writeBinary(Body3TmpFile, Body3);

		writeLine(fp, "/T");
		writeLine(fp, Body3TmpFile);
	}
	writeLine(fp, "/RSF");
	writeLine(fp, SuccessfulFile);
	writeLine(fp, "/RHF");
	writeLine(fp, ResHeaderFile);
	writeLine(fp, "/RBF");
	writeLine(fp, ResBodyFile);
	writeLine(fp, "/RBFX");
	writeLine_x(fp, xcout("%I64u", ResBodySizeMax));

	fileClose(fp);

	// clear
	{
		removeFileIfExist(SuccessfulFile);
		removeFileIfExist(ResHeaderFile);
		removeFileIfExist(ResBodyFile);

		releaseDim(ResHeader, 2);
		ResHeader = newList();
	}

	coExecute_x(xcout("START \"\" /B /WAIT \"%s\" //R \"%s\"", GetHGetFile(), ParamFile));

	if (!existFile(SuccessfulFile)) // ? 失敗した。
	{
		removeFileIfExist(ResBodyFile); // 失敗したら res-body は必ず空。
		return 0;
	}
	fp = fileOpen(ResHeaderFile, "rt");

	for (; ; )
	{
		line = readLine(fp);

		if (!line)
			break;

		p = strstr(line, ": ");
		errorCase(!p);
		*p = '\0';

		pair = newList();

		addElement(pair, (uint)strx(line));
		addElement(pair, (uint)strx(p + 2));

		// 必要無いかもしれないけど、いちおう整形しておく。
		{
			ucTrimEdge(getLine(pair, 0));
			ucTrimEdge(getLine(pair, 1));
		}

		addElement(ResHeader, (uint)pair);

		memFree(line);
	}
	fileClose(fp);

//	createFileIfNotExist(ResBodyFile);

	return 1;
}
int HGet_Head(void) // ret: ? 成功
{
	return Perform('H');
}
int HGet_GetOrPost(void) // ret: ? 成功
{
	return Perform(Body1 || Body2File || Body3 ? 'P' : 'G');
}
uint HGet_GetResHeaderFieldCount(void)
{
	errorCase(!Resetted);

	return getCount(ResHeader);
}
char *HGet_GetResHeaderFieldName(uint index)
{
	errorCase(!Resetted);
	errorCase(getCount(ResHeader) <= index);

	return getLine(getList(ResHeader, index), 0);
}
char *HGet_GetResHeaderFieldValue(uint index)
{
	errorCase(!Resetted);
	errorCase(getCount(ResHeader) <= index);

	return getLine(getList(ResHeader, index), 1);
}
void HGet_MvResBodyFile(char *destFile)
{
	errorCase(!Resetted);

	if (!existFile(ResBodyFile))
		createFile(destFile);

	moveFile(ResBodyFile, destFile);
}
autoBlock_t *HGet_GetResBody(void)
{
	errorCase(!Resetted);

	if (!existFile(ResBodyFile))
		return newBlock();

	return readBinary(ResBodyFile);
}
