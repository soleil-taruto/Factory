/*
	InstagramDL.exe [/DLE ダウンロード時に実行するコマンド] アカウント名 出力先DIR
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"
#include "C:\Factory\Common\Options\CryptoRand.h"
#include "C:\Factory\Common\Options\TimeData.h"
#include "C:\Factory\OpenSource\md5.h"

static char *HGetOption_03;

#define HGET_OPTION_01 "/H"
#define HGET_OPTION_02 "cookie"
#define HGET_OPTION_03 HGetOption_03

static char *DownloadedEventCommand;

// ---- known url ----

#define KNOWN_URL_FILE "C:\\Factory\\tmp\\instagram-dl\\known-url.txt"
//#define KNOWN_URL_FILE "C:\\appdata\\instagram-dl\\known-url.txt" // appdata 抑止 @ 2021.3.19
#define KNOWN_URL_MAX 20

static autoList_t *GetAllKnownUrl(void)
{
	autoList_t *ret;

	createPath(KNOWN_URL_FILE, 'f');
	ret = readLines(KNOWN_URL_FILE);
	trimLines(ret);
	return ret;
}
static int IsKnownUrl(char *url)
{
	autoList_t *knownUrls = GetAllKnownUrl();
	char *knownUrl;
	uint index;

	foreach (knownUrls, knownUrl, index)
		if (!strcmp(url, knownUrl))
			break;

	releaseDim(knownUrls, 1);
	return (int)knownUrl;
}
static void AddKnownUrl(char *url)
{
	autoList_t *knownUrls = GetAllKnownUrl();
	char *knownUrl;

	insertElement(knownUrls, 0, (uint)strx(url));

	while (KNOWN_URL_MAX < getCount(knownUrls))
		memFree((char *)unaddElement(knownUrls));

	writeLines(KNOWN_URL_FILE, knownUrls);
	releaseDim(knownUrls, 1);
}

// ---- known hash ----

#define KNOWN_HASH_FILE "C:\\Factory\\tmp\\instagram-dl\\known-hash.txt"
//#define KNOWN_HASH_FILE "C:\\appdata\\instagram-dl\\known-hash.txt" // appdata 抑止 @ 2021.3.19
#define KNOWN_HASH_MAX 20

static autoList_t *GetAllKnownHash(void)
{
	autoList_t *ret;

	createPath(KNOWN_HASH_FILE, 'f');
	ret = readLines(KNOWN_HASH_FILE);
	trimLines(ret);
	return ret;
}
static int IsKnownHash(char *hash)
{
	autoList_t *knownHashes = GetAllKnownHash();
	char *knownHash;
	uint index;

	foreach (knownHashes, knownHash, index)
		if (!strcmp(hash, knownHash))
			break;

	releaseDim(knownHashes, 1);
	return (int)knownHash;
}
static void AddKnownHash(char *hash)
{
	autoList_t *knownHashes = GetAllKnownHash();
	char *knownHash;

	insertElement(knownHashes, 0, (uint)strx(hash));

	while (KNOWN_HASH_MAX < getCount(knownHashes))
		memFree((char *)unaddElement(knownHashes));

	writeLines(KNOWN_HASH_FILE, knownHashes);
	releaseDim(knownHashes, 1);
}

// ----

static char *GetCurrImgLocal(void)
{
	uint64 stamp = GetNowStamp();

	return xcout("IMG_%08u_%06u.jpg", (uint)(stamp / 1000000ui64), (uint)(stamp % 1000000ui64));
}

static char *Account;
static char *DestDir;

static char *HGetExeFile(void)
{
	static char *file;

	if (!file)
//		file = GetCollaboFile("C:\\app\\Kit\\HGet\\HGet.exe"); // 廃止 @ 2023.4.13
		file = GetCollaboFile("C:\\Factory\\tmp\\Kit_Dummy\\HGet\\HGet.exe");

	return file;
}
static uint GetMinNotZero(uint p, uint q)
{
	cout("GMNZ < %08x, %08x\n", p, q);

	if (!p)
		p = UINTMAX;

	if (!q)
		q = UINTMAX;

	p = m_min(p, q);

	if (p == UINTMAX)
		p = 0;

	cout("GMNZ > %08x\n", p);
	return p;
}
static autoList_t *ParseUrls(char *resBodyFile)
{
	char *resBody = readText_b(resBodyFile);
	autoList_t *urls = newList();
	char *url;
	uint index;
	char *p;

	p = resBody;

	for (; ; )
	{
		char *q;
		char *q2;
		char *r;
		char *s;

		p = strstr(p, "GraphImage"); // __typename == GraphImage, GraphVideo がある。GraphVideo を除外する。
		cout("GI: %p\n", p);

		if (!p)
			break;

		q  = strstr(p, "display_src");
		q2 = strstr(p, "display_url");

		q = (char *)GetMinNotZero((uint)q, (uint)q2);

		if (!q)
			break;

		r = strstr(q + 13, "http");

		if (!r)
			break;

		s = strchr(r + 4, '"');

		if (!s)
			break;

		*s = '\0';
		r = strx(r);
		r = replaceLine(r, "\\u0026", "&", 0);
		addElement(urls, (uint)r);
		p = s + 1;
	}
	memFree(resBody);
	return urls;
}
static void Downloaded(autoBlock_t *imageData)
{
	char *imgLocal = GetCurrImgLocal();
	char *imgFile;

	imgFile = combine(DestDir, imgLocal);
	imgFile = toCreatablePath(imgFile, IMAX);
	memFree(imgLocal);
	imgLocal = strx(getLocal(imgFile));

	cout("< %u BYTES\n", getSize(imageData));
	cout("> %s\n", imgFile);

	writeBinary(imgFile, imageData);

	memFree(imgLocal);
	memFree(imgFile);

	if (DownloadedEventCommand)
		coExecute(DownloadedEventCommand);
}
static int CheckDownloaded(autoBlock_t *imageData) // ret: ? 保存する。
{
	char *hash = md5_makeHexHashBlock(imageData);
	int ret = 0;

	cout("hash: %s\n", hash);

	if (IsKnownHash(hash))
	{
		cout("■■■■■■■■■■■■■■■■■■■■■\n");
		cout("■既知のファイルであるため保存しません！■\n");
		cout("■■■■■■■■■■■■■■■■■■■■■\n");
	}
	else
	{
		AddKnownHash(hash);
		ret = 1;
	}
	memFree(hash);
	cout("ret: %d\n", ret);
	return ret;
}
static int Download(char *url) // ret: ? successful
{
	char *successfulFlag = makeTempPath(NULL);
	char *resHeaderFile = makeTempPath(NULL);
	char *resBodyFile = makeTempPath(NULL);
	char *prmFile = makeTempPath(NULL);
	int ret = 0;

	url = strx(url);
//	url = replaceLine(url, "/s640x640/", "/", 0); // del @ 2018.1.21

	line2JLine(url, 0, 0, 0, 0); // 表示するため。
	cout("url: %s\n", url);

	writeOneLineNoRet_b_cx(prmFile, xcout(
		"%s\n"
		"%s\n"
		"%s\n"
		"/RSF\n"
		"%s\n"
		"/RHF\n"
		"%s\n"
		"/RBF\n"
		"%s\n"
		"/L\n" // /RBFX より先に
		"/RBFX\n"
		"5000000\n" // 5 MB
		"/-\n"
		"%s"
		,HGET_OPTION_01
		,HGET_OPTION_02
		,HGET_OPTION_03
		,successfulFlag
		,resHeaderFile
		,resBodyFile
		,url
		));

	coExecute_x(xcout("START \"\" /B /WAIT \"%s\" //R \"%s\"", HGetExeFile(), prmFile));

	if (existFile(successfulFlag))
	{
		autoBlock_t *imageData = readBinary(resBodyFile);

		if (CheckDownloaded(imageData))
			Downloaded(imageData);

		releaseAutoBlock(imageData);
		ret = 1;
	}
	removeFileIfExist(successfulFlag);
	removeFileIfExist(resHeaderFile);
	removeFileIfExist(resBodyFile);
	removeFileIfExist(prmFile);

	memFree(successfulFlag);
	memFree(resHeaderFile);
	memFree(resBodyFile);
	memFree(prmFile);
	memFree(url);

	return ret;
}
static void Main2(void)
{
	char *successfulFlag = makeTempPath(NULL);
	char *resHeaderFile = makeTempPath(NULL);
	char *resBodyFile = makeTempPath(NULL);

	if (argIs("/DLE"))
	{
		DownloadedEventCommand = nextArg();
	}

	Account = strx(nextArg());
//	line2csym(Account); // old
	line2csym_ext(Account, "."); // アカウントに使える文字を必要に応じて追加
	DestDir = makeFullPath(nextArg());

	errorCase_m(hasArgs(1), "不明なコマンド引数");

	cout("アカウント: %s\n", Account);
	cout("追加出力先: %s\n", DestDir);

	errorCase(!existDir(DestDir));

//	HGetOption_03 = xcout("ds_user_id=%010I64u;", getCryptoRand64() % 10000000000ui64); // old
	HGetOption_03 = xcout("sessionid=%s;", getAppDataEnv("INSTAGRAM_SESSION_ID", "9999999999%%3Axxxxxxxxxxxxxx%%3A9"));

	coExecute_x(xcout(
		"START \"\" /B /WAIT \"%s\""
		" \"%s\""
		" \"%s\""
		" \"%s\""
		" /RSF \"%s\""
		" /RHF \"%s\""
		" /RBF \"%s\""
		" /L"
		" https://www.instagram.com/%s/"
		,HGetExeFile()
		,HGET_OPTION_01
		,HGET_OPTION_02
		,HGET_OPTION_03
		,successfulFlag
		,resHeaderFile
		,resBodyFile
		,Account
		));

	if (existFile(successfulFlag))
	{
		autoList_t *urls = ParseUrls(resBodyFile);
		char *url;
		uint index;

		LOGPOS();

		foreach (urls, url, index)
			if (IsKnownUrl(url))
				break;

		while (index)
		{
			LOGPOS();
			url = getLine(urls, --index);

			if (!Download(url))
				break;

			AddKnownUrl(url);
		}
		releaseDim(urls, 1);
	}
	removeFileIfExist(successfulFlag);
	removeFileIfExist(resHeaderFile);
	removeFileIfExist(resBodyFile);

	memFree(successfulFlag);
	memFree(resHeaderFile);
	memFree(resBodyFile);
}
int main(int argc, char **argv)
{
	Main2();
	termination(0);
}
