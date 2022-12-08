/*
	GeTunnel.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	GeTunnel.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/P PROXY-HOST PROXY-PORT] [/R]
	                                         [/E EMBED-MODE] [/BS BUFF-FULL] [/EF ERROR-PAGE-FILE]
	                                         [/XRH EXTRA-REQ-RES-HEADER-LINES-FILE]
	                                         [/FAV FAVICON-ICO-FILE]

		CONNECT-MAX ... 最大接続数, 省略時は 50
		PROXY-HOST  ... プロキシサーバー
		PROXY-PORT  ... プロキシサーバーのポート番号
		/R          ... サーバーモード
		EMBED-MODE  ... メッセージをどこに埋め込むか。整数を指定する。値は EMBED_ を参照
		BUFF-FULL   ... 送受信バッファの最大サイズ。リクエスト・レスポンスの最大サイズより少し大きめにすること。
		ERROR-PAGE-FILE ... エラー応答ファイル
		EXTRA-REQ-RES-HEADER-LINES-FILE ... 拡張ヘッダー行リストファイル
		FAVICON-ICO-FILE ... "favicon.ico" ファイル

		★タイムアウトは無い -> boomClient, boomServer の SOCK-TIMEOUT に依存する。
*/

#include "libs\Tunnel.h"
#include "libs\HTTPParse.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\Common\Options\Sequence.h"

#define DUMMY_BODY "<html><body><h1>503 Under Maintenance</h1></body></html>"
#define BODY_MESSAGE_PTN "{852bc2fd-9c36-402a-8834-39a71581f46e}"
#define HOST_LENMAX 255

static char *ErrorBodyFmt;
static autoBlock_t *FaviconBody;

/*
	プロキシ -> 本当の接続先
*/
static char *H_FwdHost;
static uint H_FwdPortNo;

static autoList_t *ExtraHeaderLines;
static int ProxyEnabled;

static char *GetUrlByReqFirstLine(char *reqFirstLine)
{
#if 1
	char *p = strchr(reqFirstLine, ' ');
	char *q;
	char *url;

	if (!p)
		goto noUrl;

	p++;
	q = strchr(p, ' ');

	if (!q)
		goto noUrl;

	url = strxl(p, (uint)q - (uint)p);
	goto endFunc;

noUrl:
	url = strx("");

endFunc:
	return url;
#else // old
	autoList_t *tokens = tokenize(reqFirstLine, ' ');
	char *url;

	url = strx(refLine(tokens, 1));
	releaseDim(tokens, 1);
	return url;
#endif
}
static char *c_GetHostFieldValue(void)
{
	static char *value;

	if (!value)
	{
		if (H_FwdPortNo == 80)
			value = strx(H_FwdHost);
		else
			value = xcout("%s:%u", H_FwdHost, H_FwdPortNo);
	}
	return value;
}

enum
{
	EMBED_MIN_PREV,

	EMBED_QUERY_BODY,
	EMBED_COOKIE,
	EMBED_XFIELD,
	EMBED_PATH,

	EMBED_MAX_NEXT,
};

static int ServerMode;
static int EmbedMode = EMBED_QUERY_BODY;
static uint BuffFull = 65000;

// ---- Info ----

typedef struct Info_st
{
	int DLMode;
	autoBlock_t *Buff;
	int *P_EmbedMode;
	autoBlock_t *EmgResData;
}
Info_t;

static Info_t *CreateInfo(int dlMode)
{
	Info_t *i = (Info_t *)memAlloc(sizeof(Info_t));

	i->DLMode = dlMode;
	i->Buff = newBlock();
	i->P_EmbedMode = NULL; // set later
	i->EmgResData = NULL;

	return i;
}
static void ReleaseInfo(Info_t *i)
{
	releaseAutoBlock(i->Buff);
	memFree(i);
}

// ----

static Info_t *CurrInfo;
static int DecodeErrorFlag;
static int RequestFaviconFlag;

// ---- HTTP ----

static void HD_Trim(char *str)
{
	char *rp = str;
	char *wp = str;

	while (*rp)
	{
		if (*rp == '-')
		{
			*wp = '+';
			wp++;
		}
		else if (*rp == '_')
		{
			*wp = '/';
			wp++;
		}
		else if (m_isBase64Char(*rp))
		{
			*wp = *rp;
			wp++;
		}
		rp++;
	}
	*wp = '\0';
}
static void HD_Decode(char *str, tagRng_t *tagRng, autoBlock_t *wBuff) // str: 変更する。tagRng: NULL ok
{
	autoBlock_t gab;

	if (tagRng)
		str = strxRng(tagRng->innerBgn, tagRng->innerEnd);

	HD_Trim(str);
	ab_addBytes_x(wBuff, decodeBase64(gndBlockLineVar(str, gab)));

	if (tagRng)
		memFree(str);
}
static void DecodeUrl(char *url)
{
	char *rp = strchr(url, '%');
	char *wp;

	if (!rp)
		return;

	wp = rp;

	while (*rp)
	{
		if (*rp == '%' && rp[1] && rp[2])
		{
			*wp = m_c2i(rp[1]) << 4 | m_c2i(rp[2]);
			rp += 2;
		}
		else
		{
			*wp = *rp;
		}

		// HACK: '+' -> ' ' しなくていいのか？

		rp++;
		wp++;
	}
	*wp = '\0';
}

/*
	応答方法
		リクエストが Cookie 又は X-Field  ---> Cookie 又は X-Field で応答する。
		リクエストが Path 又は Query --------> プロセスの設定に従って応答する。
*/
static int HTTPDecode(autoBlock_t *rBuff, autoBlock_t *wBuff)
{
	if (!HTTPParse(rBuff))
		return 0;

	LOGPOS();
	removeBytes(rBuff, 0, HttpDat.EndPos);

	// from Body
	{
		char *body = unbindBlock2Line(copyAutoBlock(HttpDat.Body));

		if (mbs_stristr(body, BODY_MESSAGE_PTN) && (
			updateTagRng(body, "<caption>", "</caption>", 1) ||
			updateTagRng(body, "<th>", "</th>", 1) ||
			updateTagRng(body, "<td>", "</td>", 1)
			))
		{
			HD_Decode(body, &lastTagRng, wBuff);
			memFree(body);
			LOGPOS();
			return 1;
		}
		memFree(body);
	}

	// from Cookie, X-Field
	{
		char *key;
		uint index;

		foreach (HttpDat.H_Keys, key, index)
		{
			if (!_stricmp(key, "Cookie") || !_stricmp(key, "Set-Cookie"))
			{
				char *value = getLine(HttpDat.H_Values, index);

				value = strx(value);
				value = addChar(value, ';');

				if (updateTagRng(value, "blueSteel=", ";", 1))
				{
					*CurrInfo->P_EmbedMode = EMBED_COOKIE; // 埋め込み方法強制変更する。

					HD_Decode(value, &lastTagRng, wBuff);
					memFree(value);
					LOGPOS();
					return 1;
				}
				memFree(value);
			}
			else if (!_stricmp(key, "X-BlueSteel"))
			{
				*CurrInfo->P_EmbedMode = EMBED_XFIELD; // 埋め込み方法強制変更する。

				HD_Decode(getLine(HttpDat.H_Values, index), NULL, wBuff);
				LOGPOS();
				return 1;
			}
		}
	}

	// from Path | Query
	{
		char *url = GetUrlByReqFirstLine(HttpDat.H_Request);

		DecodeUrl(url);

		if (updateTagRng(url, "/blueSteel/", ".html", 1)) // from Path
		{
//			*CurrInfo->P_EmbedMode = EMBED_PATH; // 埋め込み方法強制変更しない。

			HD_Decode(url, &lastTagRng, wBuff);
			memFree(url);
			LOGPOS();
			return 1;
		}
		url = addChar(url, '&');

		if (updateTagRng(url, "blueSteel=", "&", 1)) // from Query
		{
//			*CurrInfo->P_EmbedMode = EMBED_QUERY_BODY; // 埋め込み方法強制変更しない。

			HD_Decode(url, &lastTagRng, wBuff);
			memFree(url);
			LOGPOS();
			return 1;
		}
		RequestFaviconFlag = (int)mbs_stristr(url, "/favicon.ico");
		memFree(url);
	}

	DecodeErrorFlag = 1;
	LOGPOS();
	return 0;
}

#define HE_DIV_LEN 60

static char *HE_Encode(autoBlock_t *buff)
{
	buff = encodeBase64(buff);

	{
		uint size = getSize(buff);
		uint index;

		for (index = 0; index < size; index++)
		{
			if (b_(buff)[index] == '+')
			{
				b_(buff)[index] = '-';
			}
			else if (b_(buff)[index] == '/')
			{
				b_(buff)[index] = '_';
			}
			else if (b_(buff)[index] == '=')
			{
				b_(buff)[index] = '\0'; // パディング ('=') が無くても decodeBase64 できる。
				return unbindBlock(buff);
			}
		}
	}

	return unbindBlock2Line(buff);
}
static char *HE_DivText_xc(char *text, char *indent)
{
	autoBlock_t *buff = newBlock();
	char *p = text;

	while (*p)
	{
		uint len = strlen(p);

		m_minim(len, HE_DIV_LEN);

		if (text < p)
		{
			ab_addLine(buff, "\r\n");
			ab_addLine(buff, indent);
		}
		ab_addBlock(buff, p, len);
		p += len;
	}
	memFree(text);
	return unbindBlock2Line(buff);
}
static char *HE_ToBody_x(char *text)
{
	autoBlock_t *buff = newBlock();

	ab_addLine(buff, "<html>\r\n");
	ab_addLine(buff, "\t<body>\r\n");
	ab_addLine(buff, "\t\t<h1>");
	ab_addLine(buff, BODY_MESSAGE_PTN);
	ab_addLine(buff, "</h1>\r\n");
	ab_addLine(buff, "\t\t<table>\r\n");

	if (strlen(text) < 50)
	{
		ab_addLine(buff, "\t\t\t<caption>");
		ab_addLine(buff, text);
		ab_addLine(buff, "</caption>\r\n");
		ab_addLine(buff, "\t\t\t<tr>\r\n");
		ab_addLine(buff, "\t\t\t\t<td>BlueSteel</td>\r\n");
		ab_addLine(buff, "\t\t\t</tr>\r\n");
	}
	else if (strlen(text) < 1000)
	{
		text = HE_DivText_xc(text, "\t\t\t\t\t");

		ab_addLine(buff, "\t\t\t<tr>\r\n");
		ab_addLine(buff, "\t\t\t\t<th>\r\n");
		ab_addLine(buff, "\t\t\t\t\t");
		ab_addLine(buff, text);
		ab_addLine(buff, "\r\n");
		ab_addLine(buff, "\t\t\t\t</th>\r\n");
		ab_addLine(buff, "\t\t\t\t<td>BlueSteel</td>\r\n");
		ab_addLine(buff, "\t\t\t</tr>\r\n");
	}
	else
	{
		text = HE_DivText_xc(text, "\t");

		ab_addLine(buff, "\r\n");
		ab_addLine(buff, "<tr>\r\n");
		ab_addLine(buff, "<td>\r\n");
		ab_addLine(buff, "\t");
		ab_addLine(buff, text);
		ab_addLine(buff, "\r\n");
		ab_addLine(buff, "</td>\r\n");
		ab_addLine(buff, "</tr>\r\n");
		ab_addLine(buff, "\r\n");
	}
	ab_addLine(buff, "\t\t</table>\r\n");
	ab_addLine(buff, "\t</body>\r\n");
	ab_addLine(buff, "</html>");

	memFree(text);
	return unbindBlock2Line(buff);
}

static void AddExtraHeaderLines(autoBlock_t *wBuff)
{
	if (ExtraHeaderLines)
	{
		char *line;
		uint index;

		foreach (ExtraHeaderLines, line, index)
		{
			if (*line)
			{
				ab_addLine(wBuff, line);
				ab_addLine(wBuff, "\r\n");
			}
		}
	}
}
static void HTTPEncode(autoBlock_t *buff)
{
	autoBlock_t *wBuff = newBlock();
	char *resText;

	LOGPOS();
	resText = HE_Encode(buff);

	if (ServerMode) // レスポンス
	{
		ab_addLine(wBuff, "HTTP/1.1 200 OK\r\n");

		if (*CurrInfo->P_EmbedMode == EMBED_QUERY_BODY || *CurrInfo->P_EmbedMode == EMBED_PATH)
		{
			resText = HE_ToBody_x(resText);

			ab_addLine_x(wBuff, xcout("Content-Length: %u\r\n", strlen(resText)));
			ab_addLine(wBuff, "Content-Type: text/html; charset=Shift_JIS\r\n");
			ab_addLine(wBuff, "Connection: close\r\n");
			AddExtraHeaderLines(wBuff);
			ab_addLine(wBuff, "\r\n");
			ab_addLine(wBuff, resText);
		}
		else // ? Cookie, X-Field
		{
			resText = HE_DivText_xc(resText, "\t");

			if (*CurrInfo->P_EmbedMode == EMBED_COOKIE)
			{
				ab_addLine_x(wBuff, xcout("Set-Cookie: blueSteel=%s; expires=Thu, 31-Dec-2099 23:59:59 GMT\r\n", resText));
			}
			else // ? X-Field
			{
				ab_addLine_x(wBuff, xcout("X-BlueSteel: %s\r\n", resText));
			}
			ab_addLine_x(wBuff, xcout("Content-Length: %u\r\n", strlen(DUMMY_BODY)));
			ab_addLine(wBuff, "Content-Type: text/html; charset=Shift_JIS\r\n");
			ab_addLine(wBuff, "Connection: close\r\n");
			AddExtraHeaderLines(wBuff);
			ab_addLine(wBuff, "\r\n");
			ab_addLine(wBuff, DUMMY_BODY);
		}
	}
	else // リクエスト
	{
		char *urlBeforePath = "";

		if (ProxyEnabled)
			urlBeforePath = xcout("http://%s", c_GetHostFieldValue());

		if (*CurrInfo->P_EmbedMode == EMBED_QUERY_BODY || *CurrInfo->P_EmbedMode == EMBED_PATH)
		{
			char *urlLnFmt;

			if (*CurrInfo->P_EmbedMode == EMBED_QUERY_BODY)
			{
				urlLnFmt = "GET %s/index.html?blueSteel=%s HTTP/1.1\r\n";
			}
			else // ? Path
			{
				urlLnFmt = "GET %s/blueSteel/%s.html HTTP/1.1\r\n";
			}
			ab_addLine_x(wBuff, xcout(urlLnFmt, urlBeforePath, resText));
			ab_addLine_x(wBuff, xcout("Host: %s\r\n", c_GetHostFieldValue()));
			AddExtraHeaderLines(wBuff);
			ab_addLine(wBuff, "\r\n");
		}
		else // ? Cookie, X-Field
		{
			ab_addLine_x(wBuff, xcout("GET %s/index.html HTTP/1.1\r\n", urlBeforePath));
			ab_addLine_x(wBuff, xcout("Host: %s\r\n", c_GetHostFieldValue()));
			AddExtraHeaderLines(wBuff);

			if (*CurrInfo->P_EmbedMode == EMBED_COOKIE)
			{
				resText = HE_DivText_xc(resText, "\t");

				ab_addLine_x(wBuff, xcout("Cookie: blueSteel=%s\r\n", resText));
				ab_addLine(wBuff, "\r\n");
			}
			else // ? X-Field
			{
				resText = HE_DivText_xc(resText, "\t");

				ab_addLine_x(wBuff, xcout("X-BlueSteel: %s\r\n", resText));
				ab_addLine(wBuff, "\r\n");
			}
		}
		if (ProxyEnabled)
			memFree(urlBeforePath);
	}
	memFree(resText);

	ab_swap(buff, wBuff);
	releaseAutoBlock(wBuff);

	LOGPOS();
}

// ---- Boomerang ----

static int BoomerangDecode(autoBlock_t *rBuff, autoBlock_t *wBuff)
{
	uint size;

	LOGPOS();

	if (getSize(rBuff) < 4)
	{
		LOGPOS();
		return 0;
	}
	size = blockToValue(directGetBuffer(rBuff));

	if (BuffFull < size + 4) // ? バッファに入りきらない。
	{
		DecodeErrorFlag = 1;
		LOGPOS();
		return 0;
	}
	if (getSize(rBuff) - 4 < size)
	{
		LOGPOS();
		return 0;
	}
	ab_addBytes_x(wBuff, desertBytes(rBuff, 4, size));
	removeBytes(rBuff, 0, 4);
	LOGPOS();
	return 1;
}
static void BoomerangEncode(autoBlock_t *buff)
{
	uchar block[4];
	autoBlock_t gab;

	LOGPOS();
	valueToBlock(block, getSize(buff));
	insertBytes(buff, 0, gndBlockVar(block, 4, gab));
	LOGPOS();
}

// ----

static void DataFltr(autoBlock_t *buff, uint prm)
{
	Info_t *i = (Info_t *)prm;

	addBytes(i->Buff, buff);
	setSize(buff, 0);

	CurrInfo = i;
	DecodeErrorFlag = 0;

	if (m_01(i->DLMode) ^ m_01(ServerMode))
	{
		if (HTTPDecode(i->Buff, buff))
			BoomerangEncode(buff);
	}
	else
	{
		if (BoomerangDecode(i->Buff, buff))
			HTTPEncode(buff);
	}

	if (BuffFull < getSize(i->Buff))
	{
		cout("バッファに溜まり過ぎているので破棄します。size=%u (BuffFull=%u)\n", getSize(i->Buff), BuffFull);
		setSize(i->Buff, 0);
	}
	if (DecodeErrorFlag)
	{
		cout("デコードに失敗しました。切断します。\n");
		ChannelDeadFlag = 1;

		if (!i->DLMode && ServerMode) // ? サーバー側の上り
		{
			autoBlock_t *buff = newBlock();

			LOGPOS();

			if (RequestFaviconFlag && FaviconBody)
			{
				ab_addLine(buff, "HTTP/1.1 200 OK\r\n");
				ab_addLine_x(buff, xcout("Content-Length: %u\r\n", getSize(FaviconBody)));
				ab_addLine(buff, "Content-Type: image/x-icon\r\n");
				ab_addLine(buff, "\r\n");
				ab_addBytes(buff, FaviconBody);
			}
			else if (ErrorBodyFmt)
			{
#if 1 // 変数の展開アリ
				char *body = strx(ErrorBodyFmt);

				// 変数の展開
				{
					char *hostname;
					char *domain;
					char *path;
//					char *randColorDark;
//					char *randColorLight;
//					char *beerUma;

					hostname = strx(refLine(HttpDat.H_Values, findLineCase(HttpDat.H_Keys, "host", 1)));
					domain = strx(hostname);
					*strchrEnd(domain, ':') = '\0';
					path = GetUrlByReqFirstLine(HttpDat.H_Request);

					if (startsWithICase(path, "http://"))
						copyLine(path, strchrEnd(path + 7, '/'));

					if (*path == '/')
						eraseChar(path);

#if 0 // del
					randColorDark = xcout(
						"%02x%02x%02x",
						mt19937_range(0x00, 0x50),
						mt19937_range(0x00, 0x50),
						mt19937_range(0x00, 0x50)
						);

					randColorLight = xcout(
						"%02x%02x%02x",
						mt19937_range(0xa0, 0xff),
						mt19937_range(0xa0, 0xff),
						mt19937_range(0xa0, 0xff)
						);

					{
						autoList_t *parts1 = newList();
						autoList_t *parts2 = newList();
						uint index;
						char *buff;

						addElement(parts1, (uint)"今日");
						addElement(parts1, (uint)"元気");
						addElement(parts1, (uint)"ビール");
						addElement(parts1, (uint)"うまい！");

						addElement(parts2, (uint)"も");
						addElement(parts2, (uint)"だ");
						addElement(parts2, (uint)"が");

						shuffle(parts1);
						shuffle(parts2);

						buff = strx("");

						for (index = 0; index < 3; index++)
						{
							buff = addLine(buff, getLine(parts1, index));
							buff = addLine(buff, getLine(parts2, index));
						}
						buff = addLine(buff, getLine(parts1, 3));

						beerUma = buff;
					}
#endif

					cout("HOSTNAME = [%s]\n", hostname);
					cout("DOMAIN   = [%s]\n", domain);
					cout("PATH     = [%s]\n", path);
//					cout("RNDCLR_D = [%s]\n", randColorDark);
//					cout("RNDCLR_L = [%s]\n", randColorLight);
//					cout("BEER_UMA = [%s]\n", beerUma);

					body = replaceLine(body, "$(HOSTNAME)", hostname, 1);
					body = replaceLine(body, "$(DOMAIN)", domain, 1);
					body = replaceLine(body, "$(PATH)", path, 1);
//					body = replaceLine(body, "$(RNDCLR_D)", randColorDark, 1);
//					body = replaceLine(body, "$(RNDCLR_L)", randColorLight, 1);
//					body = replaceLine(body, "$(BEER_UMA)", beerUma, 1);

					memFree(hostname);
					memFree(domain);
					memFree(path);
//					memFree(randColorDark);
//					memFree(randColorLight);
//					memFree(beerUma);
				}

				ab_addLine(buff, "HTTP/1.1 200 OK\r\n");
				ab_addLine_x(buff, xcout("Content-Length: %u\r\n", strlen(body)));
				ab_addLine(buff, "Content-Type: text/html\r\n");
				ab_addLine(buff, "\r\n");
				ab_addLine(buff, body);

				memFree(body);
#else // 変数の展開ナシ
				ab_addLine(buff, "HTTP/1.1 200 OK\r\n");
				ab_addLine_x(buff, xcout("Content-Length: %u\r\n", strlen(ErrorBodyFmt)));
				ab_addLine(buff, "Content-Type: text/html\r\n");
				ab_addLine(buff, "\r\n");
				ab_addLine(buff, ErrorBodyFmt);
#endif
			}
			else
			{
				ab_addLine(buff, "HTTP/1.1 503 Under Maintenance\r\n");
				ab_addLine_x(buff, xcout("Content-Length: %u\r\n", strlen(DUMMY_BODY)));
				ab_addLine(buff, "Content-Type: text/html\r\n");
				ab_addLine(buff, "\r\n");
				ab_addLine(buff, DUMMY_BODY);
			}
			i->EmgResData = buff;
		}
	}
}
static void Perform(int sock, int fwdSock)
{
	Info_t *i = CreateInfo(0);
	Info_t *j = CreateInfo(1);
	int em = EmbedMode;

	i->P_EmbedMode = &em;
	j->P_EmbedMode = &em;

	cout("接続\n");

	CrossChannel(sock, fwdSock, DataFltr, (uint)i, DataFltr, (uint)j);

	if (i->EmgResData)
	{
		LOGPOS();
		SockSendSequLoop(sock, i->EmgResData, 2000);
		LOGPOS();
		releaseAutoBlock(i->EmgResData);
	}

	cout("切断\n");

	ReleaseInfo(i);
	ReleaseInfo(j);
}
static int ReadArgs(void)
{
	if (argIs("/P"))
	{
		H_FwdHost = nextArg();
		H_FwdPortNo = toValue(nextArg());
		return 1;
	}
	if (argIs("/R"))
	{
		ServerMode = 1;
		return 1;
	}
	if (argIs("/E"))
	{
		EmbedMode = toValue(nextArg());
		return 1;
	}
	if (argIs("/BS"))
	{
		BuffFull = toValue(nextArg());
		return 1;
	}
	if (argIs("/EF"))
	{
		ErrorBodyFmt = readText(nextArg());
		return 1;
	}
	if (argIs("/XRH")) // eXtra Req-Res Header lines file
	{
		ExtraHeaderLines = readLines(nextArg());
		return 1;
	}
	if (argIs("/FAV"))
	{
		FaviconBody = readBinary(nextArg());
		return 1;
	}

	if (H_FwdHost) // ? プロキシが指定された。
	{
		m_swap(H_FwdHost, FwdHost, char *);
		m_swap(H_FwdPortNo, FwdPortNo, uint);

		cout("■■■PROX -> %s:%u\n", FwdHost, FwdPortNo);
		cout("■■■DEST -> %s:%u\n", H_FwdHost, H_FwdPortNo);

		ProxyEnabled = 1;
	}
	else
	{
		H_FwdHost = strx(FwdHost);
		H_FwdPortNo = FwdPortNo;
	}

	errorCase(m_isEmpty(H_FwdHost) || HOST_LENMAX < strlen(H_FwdHost));
	errorCase(!m_isRange(H_FwdPortNo, 1, 0xffff));
	// ServerMode
	errorCase(!m_isRange(EmbedMode, EMBED_MIN_PREV + 1, EMBED_MAX_NEXT - 1));
	errorCase(!m_isRange(BuffFull, 1, IMAX));

	return 0;
}
static char *GetTitleSuffix(void)
{
	return xcout("[%c] E:%u", ServerMode ? 'S' : 'C', EmbedMode);
}
int main(int argc, char **argv)
{
	ConnectMax = 50;

	mt19937_init();

	TunnelMain(ReadArgs, Perform, "GeTunnel", GetTitleSuffix);
}
