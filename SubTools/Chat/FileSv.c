/*
	FilerSv.exe [/S] [/CL リンク色] [/CB 背景色] [/CT 文字色] [/DD デフォルトDIR] [/VDDO] [ポート番号]

		/S ... 停止する。起動直後だと止まらないこともある。ポート番号の前であること。

		デフォルトDIR ... ローカルのファイル・ディレクトリを指定すること。

	----

	http://localhost/C:/aaa/bbb/ccc.txt                 -> 内容を表示
	http://localhost/C:/aaa/bbb/ccc.txt?mode=download   -> ダウンロード
	http://localhost/C:/aaa/bbb/ccc.txt?mode=html       -> リンクを表示
	http://localhost/C:/aaa/bbb/                        -> ディレクトリ内のリスト       ★パスの最後に '/' 必要
	http://localhost/C:/aaa/bbb?mode=html               -> ディレクトリへのリンクを表示 ★パスの最後に '/' 不要
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\Common\Options\crc.h"
#include "C:\Factory\Labo\Socket\libs\http\ExtToContentType.h"
#include "Common.h"

#define HEADER_FIRST_LINE_LENMAX 18000

// ---- I/O ----

FILE *TryFileOpen(char *file, char *mode)
{
	FILE *fp = fopen(file, mode);

	if (!fp)
		cout("**** FILE OPEN ERROR ****\n");

	return fp;
}
uint64 TryGetFileSize(FILE *fp)
{
	sint64 size;

	if (_fseeki64(fp, 0I64, SEEK_END) != 0) // ? 失敗
	{
		cout("**** FILE SEEK END ERROR ****\n");
		return 0;
	}
	size = _ftelli64(fp);

	if (size < 0I64)
	{
		cout("**** FILE SIZE ERROR ****\n");
		return 0;
	}
	if (_fseeki64(fp, 0I64, SEEK_SET) != 0) // ? 失敗
	{
		cout("**** FILE SEEK SET ERROR ****\n");
		return 0;
	}
	return size;
}
autoBlock_t *TryReadBlock(FILE *fp, uint size)
{
	void *block = memAlloc(size);
	uint readSize;

	readSize = fread(block, 1, size, fp);

	if (ferror(fp))
	{
		cout("**** FILE READ ERORR ****\n");
		memFree(block);
		return NULL;
	}
	return bindBlock(block, readSize);
}
void TryFileClose(FILE *fp)
{
	int ret = fclose(fp);

	if (ret)
		cout("**** FILE CLOSE ERROR ****\n");
}

static autoList_t *TGFL_InfoList; // 文字列 ...

autoList_t *TryGetFileList(char *findPtn)
{
	/*
		unsigned attrib;
			_A_ARCH
			_A_HIDDEN
			_A_NORMAL
			_A_RDONLY
			_A_SUBDIR
			_A_SYSTEM

		time_t time_create;
		time_t time_access;
		time_t time_write;
		_fsize_t size;
		char name[_MAX_PATH];
	*/
	struct _finddata_t fd;
	intptr_t h;
	autoList_t *list = newList();

	cout("findPtn: %s\n", findPtn);

	if (TGFL_InfoList) // ? already inited
	{
		releaseDim(TGFL_InfoList, 1);
	}
	TGFL_InfoList = newList();

	h = _findfirst(findPtn, &fd);

	if (h != -1)
	{
		do
		{
			char *name = fd.name;
			char *info;

			name = strx(name);

			if (!strcmp(name, ".") || !strcmp(name, "..") || fd.attrib & _A_SUBDIR)
			{
				name = addChar(name, '/');
				info = strx("ディレクトリ");
			}
			else
			{
				char *lSize = xcout("%I64u", (uint64)fd.size);
				time_t t = m_max(fd.time_create, fd.time_write);
				char *stamp;
				char *nameUrl = LiteUrlEncoder(name);

				lSize = thousandComma(lSize);
				stamp = makeJStamp(getStampDataTime(t), 0);
				info = xcout("%s バイト　%s　<a href=\"%s?mode=download\" download=\"%s\">[ダウンロード]</a>", lSize, stamp, nameUrl, nameUrl);
				memFree(lSize);
				memFree(stamp);
				memFree(nameUrl);
			}
			addElement(list, (uint)name);
			addElement(TGFL_InfoList, (uint)info);
		}
		while (_findnext(h, &fd) == 0);

		_findclose(h);
	}
	return list;
}

// ---- ConnInfo ----

typedef struct ConnInfo_st
{
	uint ConnectedTime;
	autoBlock_t *RecvBuff;
	autoBlock_t *SendBuff;
	FILE *RFp;
}
ConnInfo_t;

static uint CreateConnInfo(void)
{
	ConnInfo_t *i = nb_(ConnInfo_t);

	i->ConnectedTime = now();
	i->RecvBuff = newBlock();

	return (uint)i;
}
static void ReleaseConnInfo(uint prm)
{
	ConnInfo_t *i = (ConnInfo_t *)prm;

	releaseAutoBlock(i->RecvBuff);

	if (i->SendBuff)
		releaseAutoBlock(i->SendBuff);

	if (i->RFp)
		TryFileClose(i->RFp);

	memFree(i);
}

// ----

static char *GetFileListTemplateHtml(void)
{
	static char *template;

	if (!template)
	{
		char *file = combine(getSelfDir(), "FileListTemplate.html_");

		template = readText(file);
		memFree(file);
	}
	return template;
}
static char *GetFileListElementTemplateHtml(void)
{
	static char *template;

	if (!template)
	{
		char *file = combine(getSelfDir(), "FileListElementTemplate.html_");

		template = readText(file);
		memFree(file);
	}
	return template;
}
// old
/*
static char *GetRandColor(uint hexlow, uint hexhi)
{
	char *str = strx("#999999");
	char *p;

	for (p = str + 1; *p; p++)
		*p = hexadecimal[mt19937_range(hexlow, hexhi)];

	return str;
}
*/

#define ESCPTN_1 "__\x1b_1__"
#define ESCPTN_2 "__\x1b_2__"
#define ESCPTN_3 "__\x1b_3__"

static char *B_LinkColor = "#0080ff";
static char *B_BackColor = "#ffffff";
static char *B_TextColor = "#000000";

static char *DefaultDir; // ローカル・フルパス

static int VisibleDefaultDirOnly;

static void Perform_FindPtn(ConnInfo_t *i, char *ttlPath, char *findPtn)
{
	char *body = GetFileListTemplateHtml();

	body = strx(body);

	body = replaceLine(body, "__COMPUTER-NAME__", getEnvLine("COMPUTERNAME"), 0);
	body = replaceLine(body, "__LIST-PATH__", ttlPath, 0);

	{
		char *title = xcout("[%s] %s", getEnvLine("COMPUTERNAME"), ttlPath);

		body = replaceLine(body, "__TITLE__", title, 0);
		memFree(title);
	}

	// old
	/*
	if (!B_LinkColor) // ? not inited B_
	{
		mt19937_initRnd(crc32CheckLine(getEnvLine("COMPUTERNAME")));

		B_LinkColor = GetRandColor(0xb, 0xf);
		B_BackColor = GetRandColor(6, 0xa);
		B_TextColor = GetRandColor(0, 5);
	}
	*/

	body = replaceLine(body, "__LINK-COLOR__", ESCPTN_1, 0);
	body = replaceLine(body, "__BACK-COLOR__", ESCPTN_2, 0);
	body = replaceLine(body, "__TEXT-COLOR__", ESCPTN_3, 0);

	body = replaceLine(body, ESCPTN_1, B_LinkColor, 0);
	body = replaceLine(body, ESCPTN_2, B_BackColor, 0);
	body = replaceLine(body, ESCPTN_3, B_TextColor, 0);

	{
		autoList_t *list = TryGetFileList(findPtn);
		char *name;
		uint index;
		autoList_t *wLines = newList();
		char *wText;

		if (getCount(list))
		{
			foreach (list, name, index)
			{
				char *element = GetFileListElementTemplateHtml();
				char *info = getLine(TGFL_InfoList, index);
				char *nameUrl = LiteUrlEncoder(name);

				element = strx(element);

				element = replaceLine(element, "__NAME__", ESCPTN_1, 0);
				element = replaceLine(element, "__INFO__", ESCPTN_2, 0);
				element = replaceLine(element, "__HREF__", ESCPTN_3, 0);

				element = replaceLine(element, ESCPTN_1, name, 0);
				element = replaceLine(element, ESCPTN_2, info, 0);
				element = replaceLine(element, ESCPTN_3, nameUrl, 0);

				addElement(wLines, (uint)element);

				// old
				/*
				addElement(wLines, (uint)strx("<DIV>"));
				addElement(wLines, (uint)xcout("<A HREF=\"%s\">%s</A>　%s", nameUrl, name, info));
				addElement(wLines, (uint)strx("</DIV>"));
				*/

				memFree(nameUrl);
			}
		}
		else
		{
			addElement(wLines, (uint)strx("<DIV>ファイルリストを取得できません。</DIV>"));
		}
		wText = untokenize(wLines, "\r\n");

		body = replaceLine(body, "__LIST__", wText, 0);

		releaseDim(list, 1);
		releaseDim(wLines, 1);
		memFree(wText);
	}

	i->SendBuff = newBlock();

	ab_addLine(i->SendBuff, "HTTP/1.1 200 OK\r\n");
	ab_addLine_x(i->SendBuff, xcout("Content-Length: %u\r\n", strlen(body)));
	ab_addLine(i->SendBuff, "Content-Type: text/html; charset=Shift_JIS\r\n");
	ab_addLine(i->SendBuff, "Connection: close\r\n");
	ab_addLine(i->SendBuff, "\r\n");
	ab_addLine(i->SendBuff, body);

	memFree(body);
}
static void Perform_Dir(ConnInfo_t *i, char *dir)
{
	char *wCard = strx(dir);//combine(dir, "*"); // HACK: combine() はネットワークパスを考慮しない。

	if (endsWith(wCard, "\\"))
		wCard = addChar(wCard, '*');
	else
		wCard = addLine(wCard, "\\*");

	Perform_FindPtn(i, dir, wCard);
	memFree(wCard);
}
static int ParseHeaderTokens(ConnInfo_t *i, autoList_t *tokens)
{
	uint token_num = getCount(tokens);

	if ((token_num == 3 || token_num == 4) && !_stricmp("GET", getLine(tokens, 0)))
	{
		char *url = getLine(tokens, 1);
		char *file;
		char *url302Pfx = "";
		FILE *fp;
		char *contentType;

		file = URLToPath(url);

		if (token_num == 4)
		{
			url302Pfx = getLine(tokens, 3);
			url302Pfx = strchrNext(url302Pfx, '=');
			line2JLine(url302Pfx, 0, 0, 0, 0); // 表示のため
			cout("★url302Pfx: %s\n", url302Pfx);
		}
		if (!file)
		{
		bad_url:
			cout("BAD URL\n");

			if (UTP_Path[0] == '\0') // ? パスを指定しなかった。
			{
				char *redDir = PathToURL(DefaultDir);

				cout("REDIRECT_1\n");

				i->SendBuff = newBlock();

				ab_addLine(i->SendBuff, "HTTP/1.1 302 REDIRECT\r\n");
				ab_addLine_x(i->SendBuff, xcout("Location: %s%s\r\n", url302Pfx, redDir));
				ab_addLine(i->SendBuff, "Connection: close\r\n");
				ab_addLine(i->SendBuff, "\r\n");

				memFree(redDir);
				return 1;
			}
			return 0;
		}
		cout("FILE: %s\n", file);

		if (VisibleDefaultDirOnly)
		{
			static char *ddYen;

			cout("VDDO-CHECK\n");

			if (!ddYen)
				ddYen = putYen(DefaultDir);

			if (!mbs_stricmp(file, DefaultDir) || startsWithICase(file, ddYen))
			{
				cout("VDDO-OK\n");
			}
			else
			{
				char *fbd_content = "<html><body><h1>VDDO-FORBIDDEN</h1><hr/>ホームディレクトリ以外へのアクセスは禁止されています。</body></html>";

				cout("VDDO-FORBIDDEN\n");

				i->SendBuff = newBlock();

				ab_addLine(i->SendBuff, "HTTP/1.1 200 FORBIDDEN\r\n");
				ab_addLine_x(i->SendBuff, xcout("Content-Length: %u\r\n", strlen(fbd_content)));
				ab_addLine(i->SendBuff, "Content-Type: text/html; charset=Shift_JIS\r\n");
				ab_addLine(i->SendBuff, "Connection: close\r\n");
				ab_addLine(i->SendBuff, "\r\n");
				ab_addLine(i->SendBuff, fbd_content);

				memFree(file);
				return 1;
			}
		}
		if (UTP_HtmlMode)
		{
			Perform_FindPtn(i, file, file);
			memFree(file);
			return 1;
		}
		if (UTP_EndSlash)
		{
			Perform_Dir(i, file);
			memFree(file);
			return 1;
		}
		fp = TryFileOpen(file, "rb");

		if (!fp)
		{
			if (m_isalpha(file[0]) && existDir(file)) // ? ローカル && 存在するディレクトリ
			{
				char *redDir = PathToURL(file);

				cout("REDIRECT_2\n");

				i->SendBuff = newBlock();

				ab_addLine(i->SendBuff, "HTTP/1.1 302 REDIRECT\r\n");
				ab_addLine_x(i->SendBuff, xcout("Location: %s%s\r\n", url302Pfx, redDir));
				ab_addLine(i->SendBuff, "Connection: close\r\n");
				ab_addLine(i->SendBuff, "\r\n");

				memFree(redDir);
				memFree(file);
				return 1;
			}
			memFree(file);
			return 0;
		}
		i->SendBuff = newBlock();
		i->RFp = fp;

		if (UTP_DownloadMode)
			contentType = "application/octet-stream";
		else
			contentType = httpExtToContentType(getExt(file));

		cout("contentType: %s\n", contentType);

		ab_addLine(i->SendBuff, "HTTP/1.1 200 OK\r\n");
		ab_addLine_x(i->SendBuff, xcout("Content-Length: %I64u\r\n", TryGetFileSize(i->RFp)));
		ab_addLine_x(i->SendBuff, xcout("Content-Type: %s\r\n", contentType));
		ab_addLine(i->SendBuff, "Connection: close\r\n");
		ab_addLine(i->SendBuff, "\r\n");

		memFree(file);
		return 1;
	}
	return 0;
}
static int Perform(int sock, uint prm)
{
	ConnInfo_t *i = (ConnInfo_t *)prm;

	if (!i->SendBuff)
	{
		char *header;
		char *p;
		autoList_t *tokens;
		int retval;

		if (SockRecvSequ(sock, i->RecvBuff, sockUserTransmitIndex ? 0 : 100) == -1)
		{
			cout("RECV ERROR\n");
			return 0;
		}
		header = ab_toLine(i->RecvBuff);
		p = strchr(header, '\r');

		if (!p)
		{
			if (HEADER_FIRST_LINE_LENMAX < strlen(header))
			{
				cout("RECV TOO-LONG\n");
				memFree(header);
				return 0;
			}
			if (i->ConnectedTime + 2 < now())
			{
				cout("RECV TIMEOUT\n");
				memFree(header);
				return 0;
			}
			return 1;
		}
		*p = '\0';
		line2JLine(header, 0, 0, 0, 1);
		cout("HEADER: %s\n", header);
		tokens = tokenize(header, ' ');
		retval = ParseHeaderTokens(i, tokens);
		memFree(header);
		releaseDim(tokens, 1);

		if (!retval)
		{
			cout("PARSE HEADER FAULT\n");
			return 0;
		}
	}
	if (SockRecvSequ(sock, i->RecvBuff, 0) == -1)
	{
		cout("RECV GOMI ERROR\n");
		return 0;
	}
	setSize(i->RecvBuff, 0);

	if (getSize(i->SendBuff) < 2000000 && i->RFp)
	{
		autoBlock_t *block = TryReadBlock(i->RFp, 3000000);

		if (!block)
			return 0;

		ab_addBytes_x(i->SendBuff, block);
	}
	if (SockSendSequ(sock, i->SendBuff, sockUserTransmitIndex ? 0 : 100) == -1)
	{
		cout("SEND ERROR\n");
		return 0;
	}
	if (!getSize(i->SendBuff))
	{
		cout("SEND COMPLETED\n");
		return 0;
	}
	return 1;
}

#define STOP_EV_UUID "{a4d9ba43-9760-4d4f-8c1b-19e630951b60}"

static char *StopEvName;
static uint StopEv;

static int Idle(void)
{
	if (handleWaitForMillis(StopEv, 0))
		return 0;

	while (hasKey())
		if (getKey() == 0x1b)
			return 0;

	return 1;
}
int main(int argc, char **argv)
{
	int stopFlag = 0;
	uint portno = 80;//60002;

readArgs:
	if (argIs("/S"))
	{
		stopFlag = 1;
		goto readArgs;
	}
	if (argIs("/CL"))
	{
		B_LinkColor = nextArg(); // ex. "#0080ff"
		goto readArgs;
	}
	if (argIs("/CB"))
	{
		B_BackColor = nextArg(); // ex. "#0080ff"
		goto readArgs;
	}
	if (argIs("/CT"))
	{
		B_TextColor = nextArg(); // ex. "#0080ff"
		goto readArgs;
	}
	if (argIs("/DD"))
	{
		DefaultDir = nextArg();
		goto readArgs;
	}
	if (argIs("/VDDO"))
	{
		LOGPOS();
		VisibleDefaultDirOnly = 1;
		goto readArgs;
	}

	if (hasArgs(1))
		portno = toValue(nextArg());

	if (!DefaultDir)
		DefaultDir = getSelfDir();
	else
		DefaultDir = makeFullPath(DefaultDir);

	StopEvName = xcout(STOP_EV_UUID "_%u", portno);

	if (stopFlag)
	{
		LOGPOS();
		eventWakeup(StopEvName);
		return;
	}
	StopEv = eventOpen(StopEvName);

	sockServerUserTransmit(Perform, CreateConnInfo, ReleaseConnInfo, portno, 20, Idle);

	handleClose(StopEv);
}
