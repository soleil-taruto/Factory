/*
	ファイルのダウンロード
		hget.exe http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c

	ダウンロードして指定ファイルに保存する。
		hget.exe /o 123.c http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c
		hget.exe http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c 123.c

	ダウンロードして画面に表示する。
		hget.exe /o * http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c
		hget.exe http://www.cs.ucdavis.edu/~rogaway/ocb/ocb-ref/rijndael-alg-fst.c *
*/

#include "C:\Factory\Labo\Socket\libs\http\Client.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\Common\Options\UTF.h"

#define MUTEX_ID "{c2a09038-995b-492b-ae74-f45d04823852}"

static void TextFltr(char *file, uint lineLenMax)
{
	char *midFile = makeTempPath(NULL);
	FILE *rfp;
	FILE *wfp;
	char *line;

	cout("TextFltr lineLenMax=%u\n", lineLenMax);

	rfp = fileOpen(file, "rt");
	wfp = fileOpen(midFile, "wt");

	while (line = readLineLenMax(rfp, lineLenMax))
	{
		line2JLine(line, 1, 0, 1, 1);
		writeLine_x(wfp, line);
	}
	fileClose(rfp);
	fileClose(wfp);

	removeFile(file);
	moveFile(midFile, file);
	memFree(midFile);

	cout("TextFltr ok\n");
}
static void Progress_Wrap(void)
{
	static uint callcnt;

	callcnt++;

	if (eqIntPulseSec(1, NULL))
	{
		cmdTitle_x(xcout("hget - %u (S:%I64u, R:%I64u)", callcnt, SockTotalSendSize, SockTotalRecvSize));
		Progress();
	}
}
int main(int argc, char **argv)
{
	char *serverDomain = NULL;
	uint portno = 80;
	char *proxyServerDomain = NULL;
	uint proxyPortno = 80;
	char *path = NULL;
	uint timeout = 0;
	autoBlock_t *content = NULL;
	char *outFile = NULL;
	char *resFile = makeTempFile("res");
	uint retrycnt = 2;
	int jflag = 0;
	int jflag_utf8 = 0;
	int retval;
	uint outFileTextFltrLineLenMax = 0; // 0 as disabled
	int successful = 0;
	uint lastStartTime;
	int outputAndOpenOutDir = 0;
	char *hdrOutFile = NULL;
	int mtxFlag = 0;
	uint mtx;

	httpM4UServerMode = 1;
	timeout = 86400;
	httpBlockTimeout = 180;

readArgs:
	if (argIs("/S")) // Server domain
	{
		serverDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // server Port-no
	{
		portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/PS")) // Proxy Server domain
	{
		proxyServerDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/PP")) // Proxy server Port-no
	{
		proxyPortno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/R")) // Request path (and query)
	{
		path = nextArg();
		goto readArgs;
	}
	if (argIs("/C")) // request Content (これを指定するとPOSTになる)
	{
		content = readBinary(nextArg());
		goto readArgs;
	}
	if (argIs("/T")) // Timeout (接続してからのタイムアウト, 秒, 0 == 無制限)
	{
		timeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/BT")) // Block Timeout (無通信タイムアウト, 秒, 0 == 無通信)
	{
		httpBlockTimeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/O")) // Output file
	{
		outFile = nextArg();
		goto readArgs;
	}
	if (argIs("/RET")) // RETry (0でリトライ無し)
	{
		retrycnt = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/J")) // 受信データを表示するとき日本語も読めるようにする。但し SJIS じゃないと駄目だし JIS0208.txt が必要、あんま意味ないかもね。
	{
		jflag = 1;
		goto readArgs;
	}
	if (argIs("/J8")) // 表示データを表示する。utf-8
	{
		jflag = 1;
		jflag_utf8 = 1;
		goto readArgs;
	}
	if (argIs("/RSX")) // Recv content Size maX
	{
		httpMultiPartContentLenMax = toValue(nextArg()); // デフォルトは 2G くらい。
		goto readArgs;
	}
	if (argIs("/RTX")) // Recv content (output file) Text filter line-length maX
	{
		outFileTextFltrLineLenMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/H")) // 拡張ヘッダフィールド
	{
		httpExtraHeader = readLines(nextArg());
		goto readArgs;
	}
	if (argIs("/-M4U")) // m4uサーバーモードoff
	{
		httpM4UServerMode = 0;
		goto readArgs;
	}
	if (argIs("/HH")) // 受信ヘッダを表示する。
	{
		httpRecvedHeader = newList();
		goto readArgs;
	}
	if (argIs("/HHO")) // 受信ヘッダをファイルに保存する。
	{
		httpRecvedHeader = newList();
		hdrOutFile = nextArg();
		goto readArgs;
	}
	if (argIs("/M")) // 排他的に実行する。
	{
		mtxFlag = 1;
		goto readArgs;
	}
	if (argIs("/L")) // BlueFish 内のバッチ用
	{
		cout("#### LITE MODE ####\n");
		sockConnectTimeoutSec = 5;
//		sockConnectTimeoutSec = 2; // ieServerでよくtimeoutする。@ 2019.3.14
		httpM4UServerMode = 0;
		timeout = 30;
		httpBlockTimeout = 20;
		retrycnt = 0;
		httpMultiPartContentLenMax = 2000000; // 2mb
		outFile = "*";
		httpRecvedHeader = newList();
	}

	if (hasArgs(1))
	{
		char *p;

		serverDomain = strx(nextArg());

		if (startsWith(serverDomain, "http://"))
			eraseLine(serverDomain, 7);

		if (p = strchr(serverDomain, '/'))
		{
			path = strx(p);
			*p = '\0';
		}
		if (p = strchr(serverDomain, ':'))
		{
			portno = toValue(p + 1);
			*p = '\0';
		}
	}
	if (hasArgs(1))
		outFile = nextArg();

	if (!path)
		path = "/";

	if (outFile)
	{
		if (*outFile == '*')
			outFile = NULL;
	}
	else
	{
		char *p = strrchr(path, '/');

		if (p)
			p++;
		else
			p = outFile;

		outFile = getOutFile(lineToFairLocalPath(p, 50));
		outputAndOpenOutDir = 1;
	}
	if (outFile)
		remove(outFile);

retry:
	lastStartTime = now();

	if (httpRecvedHeader)
	{
		releaseDim(httpRecvedHeader, 1);
		httpRecvedHeader = newList();
	}

	if (mtxFlag)
	{
		LOGPOS();
		mtx = mutexLock(MUTEX_ID);
		LOGPOS();
	}

	LOGPOS();

	ProgressBegin();
	SockSendInterlude = Progress_Wrap;
	SockRecvInterlude = Progress_Wrap;

	retval = httpSendRequestFile(serverDomain, portno, proxyServerDomain, proxyPortno, path, content, timeout, resFile);

	SockSendInterlude = NULL;
	SockRecvInterlude = NULL;
	ProgressEnd(retval ? 0 : 1);

	LOGPOS();

	if (mtxFlag)
	{
		LOGPOS();
		mutexUnlock(mtx);
		LOGPOS();
	}

	cmdTitle("hget - done");

	if (retval)
	{
		if (httpRecvedHeader)
		{
			char *rh_line;
			uint rh_index;

			cout("★受信ヘッダここから\n");

			foreach (httpRecvedHeader, rh_line, rh_index)
				cout("%s\n", rh_line);

			cout("★受信ヘッダここまで\n");

			if (hdrOutFile)
				writeLines(hdrOutFile, httpRecvedHeader);

			releaseDim(httpRecvedHeader, 1);
			httpRecvedHeader = NULL;
		}
		if (outFile)
		{
			if (existFile(outFile))
				removeFile(outFile);

			moveFile(resFile, outFile);

			if (outFileTextFltrLineLenMax)
				TextFltr(outFile, outFileTextFltrLineLenMax);

			if (outputAndOpenOutDir)
				openOutDir();
		}
		else
		{
			FILE *fp = fileOpen(resFile, "rt");
			char *line;

			cout("■受信データ\n");

			while (line = readLineLenMax(fp, 128 * 1024 * 1024))
			{
				char *pLine;

				if (jflag)
				{
					if (jflag_utf8)
					{
						char *tmpfile = makeTempPath("utf");

						writeOneLineNoRet_b(tmpfile, line);
						memFree(line);
						UTF8ToSJISFile(tmpfile, tmpfile);
						line = readText_b(tmpfile);

						removeFile(tmpfile);
						memFree(tmpfile);
					}
					line2JLine(pLine = strx(line), 1, 1, 0, 1);
				}
				else
				{
					pLine = lineToPrintLine(line, 0);
				}

				cout("%s\n", pLine);

				memFree(pLine);
				memFree(line);
			}
			fileClose(fp);
			removeFile(resFile);
		}
		successful = 1;
	}
	else
	{
		cout("失敗しました。\n");

		if (retrycnt)
		{
			cout("あと %u 回リトライします。\n", retrycnt);
			retrycnt--;

			LOGPOS();
			while (now() <= lastStartTime + 2) sleep(300);
			LOGPOS();

			goto retry;
		}
	}
	memFree(resFile);

	return successful ? 0 : 1;
}
