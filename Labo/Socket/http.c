/*
	http.exe C:\web\wwwroot
		http://localhost/
		http://localhost/index.html -> C:\web\wwwroot\index.html

		http://localhost/sub
		http://localhost/sub/index.html -> C:\web\wwwroot\sub\index.html

		http://localhost/abc/123/def.jpg -> C:\web\wwwroot\abc\123\def.jpg

	http.exe /p 59999 C:\HP
		http://localhost:59999/image/abc.gif -> C:\HP\image\abc.gif

	こんな感じ。
	行儀の良い鯖じゃない (200 以外の応答をしない) けど、、、まぁいいや。
	引数省略したら、カレントをルートにする。
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "libs\http\ExtToContentType.h"

#define HTTP_NEWLINE "\r\n"
#define INDEXFILE "index.html"

static autoList_t *ExtraHeader;

static char *PTknsToFile(char *rootDir, autoList_t *ptkns) // ret == NULL: not found
{
	char *file = NULL;
	char *ptkn;
	uint index;

	addCwd(rootDir);

	foreach (ptkns, ptkn, index)
	{
		cout("ptkn[%u]: %s\n", index, ptkn);

		if (PATH_SIZE < strlen(ptkn))
		{
			cout("長すぎるローカル名\n");
			break;
		}
		if (!isFairLocalPath(ptkn, 0))
		{
			cout("不正なローカル名\n");
			break;
		}
		if (existFile(ptkn))
		{
			cout("ファイルを発見\n");
			file = makeFullPath(ptkn);
			break;
		}
		if (!existDir(ptkn))
		{
			cout("存在しないローカル名\n");
			break;
		}
		changeCwd(ptkn);
	}
	if (!file && existFile(INDEXFILE))
	{
		cout("インデックスファイルを発見\n");
		file = makeFullPath(INDEXFILE);
	}
	unaddCwd();

	return file;
}

static char *RootDir;
static uint Port = 80;

static int KeepTheServer = 1;

static int Perform(int sock, uint dummy)
{
	int reqcmd;
	char *requrl;
	autoList_t *ptkns;
	char *file;
	char *mimeType;
	autoBlock_t *retBody;

	cout("接続\n");

	// requrl
	{
		autoBlock_t *buffer = newBlock();
		char *line;
		char *strw;
		autoList_t *tokens;

		do
		{
			if (!KeepTheServer)
			{
				cout("サーバー停止(受信時)\n");
				releaseAutoBlock(buffer);
				return 0;
			}
			if (SockRecvSequ(sock, buffer, 2000) == -1)
			{
				cout("受信エラー\n");
				releaseAutoBlock(buffer);
				return 0;
			}
			cout("recv-hdr: %u\n", getSize(buffer));

			if (getSize(buffer) == 0) // これもキープアライブなのか、IE9で繋いで何もしない接続がしばしばある。
			{
				cout("リクエスト無し\n");
				releaseAutoBlock(buffer);
				return 0;
			}
			line = SockNextLine(buffer);
		}
		while (!line); // タイムアウト無し

		releaseAutoBlock(buffer);
		line = lineToPrintLine(strw = line, 0); // 画面に出すので、念のため危ない文字を排除
		memFree(strw);
		cout("hdr-1st: %s\n", line);

		tokens = tokenize(line, ' ');
		memFree(line);

		reqcmd = refLine(tokens, 0)[0];
		requrl = strx(refLine(tokens, 1));
		releaseDim(tokens, 1);
	}
	reqcmd = m_toupper(reqcmd);
	m_range(reqcmd, 'A', 'Z');

	cout("reqcmd: %c\n", reqcmd);
	cout("requrl: %s\n", requrl);

	{
		char *query = strchr(requrl, '?');

		if (query)
			*query = '\0';
	}

	ptkns = tokenize(requrl, '/');
	memFree(requrl);

	if (3 <= getCount(ptkns) && getLine(ptkns, 1)[0] == '\0') // ? "<>//<>" -> ドメインまで削る。
	{
		memFree((void *)desertElement(ptkns, 0)); // プロトコル
		memFree((void *)desertElement(ptkns, 0)); // "//" の間
		memFree((void *)desertElement(ptkns, 0)); // ドメイン
	}
	trimLines(ptkns);
	file = PTknsToFile(RootDir, ptkns);
	releaseDim(ptkns, 1);

	cout("file: %s\n", file ? file : "<NULL>");

	if (file)
	{
		mimeType = httpExtToContentType(getExt(file));
		retBody = readBinary(file);
		memFree(file);
	}
	else // 404
	{
		mimeType = "text/html";
		retBody = ab_makeBlockLine(
			"<html>\n"
			"<body>\n"
			"<h1>404</h1>\n"
			"</body>\n"
			"</html>\n"
			);
	}

	cout("mimeType: %s\n", mimeType);
	cout("retBody: %u\n", getSize(retBody));

	// Response
	{
		autoBlock_t *buffer = newBlock();

		ab_addLine(buffer, "HTTP/1.1 200 OK" HTTP_NEWLINE);
		ab_addLine_x(buffer, xcout("Content-Type: %s" HTTP_NEWLINE, mimeType));
		ab_addLine_x(buffer, xcout("Content-Length: %u" HTTP_NEWLINE, getSize(retBody)));
//		ab_addLine(buffer, "Server: http.c" HTTP_NEWLINE);
//		ab_addLine(buffer, "Connection: close" HTTP_NEWLINE);
//		ab_addLine(buffer, "Cache-Control: no-cache" HTTP_NEWLINE);
//		ab_addLine(buffer, "Last-Modified: Wed, 21 Jun 2006 07:00:25 GMT" HTTP_NEWLINE);

		if (ExtraHeader)
		{
			char *field;
			uint index;

			foreach (ExtraHeader, field, index)
				ab_addLine(buffer, xcout("%s" HTTP_NEWLINE, field));
		}
		ab_addLine(buffer, HTTP_NEWLINE);

		if (reqcmd != 'H')
			addBytes(buffer, retBody);

		releaseAutoBlock(retBody);

		while (getSize(buffer)) // タイムアウト無し
		{
			if (!KeepTheServer)
			{
				cout("サーバー停止(送信時)\n");
				releaseAutoBlock(buffer);
				return 0;
			}
			cout("send-ret: %u\n", getSize(buffer));

			if (SockSendSequ(sock, buffer, 2000) == -1)
			{
				cout("送信エラー\n");
				releaseAutoBlock(buffer);
				return 0;
			}
		}
		releaseAutoBlock(buffer);
	}

	cout("切断\n");
	return 0;
}

static uint CreateInfo(void)
{
	return 0;
}
static void ReleaseInfo(uint dummy)
{
	// noop
}
static int Idle(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("サーバー停止\n");
			return 0;
		}
		cout("終了するにはエスケープキーを押して下さい。\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint portno = 80;

readArgs:
	if (argIs("/P")) // Port
	{
		portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/H"))
	{
		ExtraHeader = readLines(nextArg());
		trimLines(ExtraHeader);

		{
			char *field;
			uint index;

			cout("拡張フィールド\n");

			foreach (ExtraHeader, field, index)
				cout("[%02u]:[%s]\n", index, field);
		}
		goto readArgs;
	}
	if (argIs("/C"))
	{
		httpCharset = nextArg();
		goto readArgs;
	}

	RootDir = hasArgs(1) ? nextArg() : ".";
	errorCase(!existDir(RootDir));

	sockServerUserTransmit(Perform, CreateInfo, ReleaseInfo, portno, 1, Idle);
}
