#include "Client.h"

autoList_t *httpExtraHeader;
uint httpBlockTimeout; // 無通信タイムアウト, 秒, 0 == 無制限

static char *Domain;
static uint Portno;
static char *ProxyDomain;
static uint ProxyPortno;
static char *Path;
static uint Timeout; // 接続してからのタイムアウト, 秒, 0 == 無制限
static autoBlock_t *Content;
static uint RetContentSizeMax;
static autoBlock_t *RetContent;
static char *RetContFile;

static char *GetBoundary(void)
{
	uint index;

	for (index = 2; index < getSize(Content); index++) // 最初の２文字は "--"
	{
		int chr;

		if (100 < index) // boundary文字列は70文字以下
			break;

		chr = getByte(Content, index);

		if (chr == '\r')
			break;
	}
	return unbindBlock2Line(getSubBytes(Content, 2, index - 2));
}
static void Perform(int sock)
{
	SockStream_t *ss = CreateSockStream(sock, Timeout);
	char *header;
	uchar *content;
	uint index;
	uint csize;
	int ctflag = 0;

	SetSockStreamBlockTimeout(ss, httpBlockTimeout);

	// 1行目

	header = strx("");
	header = addLine(header, Content ? "POST" : "GET");
	header = addChar(header, ' ');

	if (ProxyDomain)
	{
		header = addLine(header, "http://");
		header = addLine(header, Domain);

		if (Portno != 80)
			header = addLine_x(header, xcout(":%u", Portno));
	}
	header = addLine(header, Path);
	header = addLine(header, " HTTP/1.1");

	SockSendLine(ss, header);
	memFree(header);

	// Hostフィールド

	header = strx("");
	header = addLine(header, "Host: ");
	header = addLine(header, Domain);

	if (Portno != 80)
		header = addLine_x(header, xcout(":%u", Portno));

	SockSendLine(ss, header);
	memFree(header);

	// 拡張ヘッダフィールド

	if (httpExtraHeader)
	{
		foreach (httpExtraHeader, header, index)
		{
			if (startsWithICase(header, "Content-Type:"))
				ctflag = 1;

			SockSendLine(ss, header);
		}
	}

	// ContentTypeフィールド

	if (Content && getSize(Content) && !ctflag)
	{
		int chr = getByte(Content, 0);

		if (chr == '-')
		{
			char *boundary = GetBoundary();

			SockSendLine_x(ss, xcout("Content-Type: multipart/form-data; boundary=\"%s\"", boundary));
			memFree(boundary);
		}
		else if (chr == '{')
		{
			SockSendLine(ss, "Content-Type: application/json");
		}
		else
		{
//			SockSendLine(ss, "Content-Type: application/octet-stream");
		}
	}

	// ContentLengthフィールド + Content

	if (Content)
	{
		SockSendLine_x(ss, xcout("Content-Length: %u", getSize(Content)));
		SockSendLine(ss, "");
		SockSendBlock(ss, directGetBuffer(Content), getSize(Content));
	}
	else
	{
		SockSendLine(ss, "");
	}

	SockFlush(ss);

	// 受信

	httpRecvRequestFirstWaitDisable = 1;

	/*
		httpRecvRequest*()をレスポンスの受信に使う。
	*/
	if (RetContFile)
	{
		httpRecvRequestFile(ss, &header, RetContFile); // RetContFile は必ず作成される。(失敗時は空)

		content = (uchar *)strx("Dummy");
		csize = strlen((char *)content) + 1;
	}
	else
	{
		httpRecvRequestMax(ss, &header, &content, &csize, RetContentSizeMax);
	}

	httpRecvRequestFirstWaitDisable = 0;
	memFree(header);

	RetContent = bindBlock(content, csize);

	if (IsEOFSockStream(ss) && !ss->Extra.M4UServerMode_Operated)
	{
		releaseAutoBlock(RetContent);
		RetContent = NULL;
	}
	ReleaseSockStream(ss);
}

/*
	httpクライアントとして鯖にリクエストを送り、レスポンスを受け取る。

	domain - 鯖のドメイン名 (""のとき多分localhost)
	portno - 鯖のポート番号 (1-65535)
	proxyDomain - プロキシ鯖のドメイン名           串を通さない場合は NULL を指定すること。(""のとき多分localhost)
	proxyPortno - プロキシ鯖のポート番号 (1-65535) 串を通さない場合は何でも良し。
	path - リクエストのパス。URLのドメイン名の後ろに連結するだけなので、クエリがある場合は後ろに連結する。(パスの先頭に "/" を忘れずに、たとえば "/index.html")
	content - NULL のとき GET そうでないとき content を CONTENT (マルチパート) とする POST リクエストを投げる。
	timeout - 応答タイムアウト (ミリ秒) 0: 無制限
	sizemax - 受信データ最大サイズ (バイト)

	受信したレスポンスのデータ部を返す。
	データ部が空の場合、0 バイトのブロックを返す。接続・通信に失敗すると NULL を返す。
*/
autoBlock_t *httpSendRequest(char *domain, uint portno, char *proxyDomain, uint proxyPortno, char *path, autoBlock_t *content, uint timeout, uint sizemax)
{
	uchar ip[4];

	errorCase(!domain);
	errorCase(!portno || 0xffff < portno);
	errorCase(proxyDomain && (!proxyPortno || 0xffff < proxyPortno));
	errorCase(!path);

	Domain = domain;
	Portno = portno;
	ProxyDomain = proxyDomain;
	ProxyPortno = proxyPortno;
	Path = path;
	Timeout = timeout;
	Content = content;
	RetContentSizeMax = sizemax;
	RetContent = NULL;

	if (proxyDomain)
	{
		domain = proxyDomain;
		portno = proxyPortno;
	}

	*(uint *)ip = 0;
	sockClientUserTransmit(ip, domain, portno, Perform);

	return RetContent;
}
/*
	戻り値: 成功したかどうか。

	retContFile:
		成功した場合、受信した CONTENT を出力する。
		失敗した場合、削除する。

	受信ファイルの最大サイズは httpMultiPartContentLenMax に設定すること。
*/
int httpSendRequestFile(char *domain, uint portno, char *proxyDomain, uint proxyPortno, char *path, autoBlock_t *content, uint timeout, char *retContFile)
{
	autoBlock_t *dmyCont;

	errorCase(!retContFile);

	RetContFile = retContFile;
	dmyCont = httpSendRequest(domain, portno, proxyDomain, proxyPortno, path, content, timeout, 0);
	RetContFile = NULL;

	if (dmyCont)
	{
		releaseAutoBlock(dmyCont);
		return 1;
	}
	removeFileIfExist(retContFile);
	return 0;
}
