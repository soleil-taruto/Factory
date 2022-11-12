/*
	メッセージ長: 最大 140 文字 + 8 (時刻 time_t) + 4 (ip) == 152
	メッセージキュー:
		0 - 2999
		3000 まで溜まったら 1500 吐き出して 1500 にする。
	履歴ファイル:
		152 * 1500 バイトこれを 00000 - 99999 (100000 セット) 保持 == 22800000000 バイト == 最大、約 22.8 GB
		00000 からカウントを始め 99999 の次は 00000 とする。

	起動したら http://localhost/ にアクセスしてみて下さい。

	stdoutが喧しいけど、ウェイトちゃんと入れてないみたいなので、わかりやすいからこれでいいや。@ 2015.8.25
*/

#include "C:\Factory\Common\Options\SockServer.h"

#define HTTP_NEWLINE "\r\n"
#define HDRHDR_LENMAX 4096

#define TIMEOUT_SEC 20
#define MESSAGE_SIZE 140
#define MESSAGE_BUNDLE_SIZE 1500
#define HIST_MAX 100000
#define SAME_MESSAGE_MAX 50
#define SAME_MESSAGE_DISTANCE 30
#define SAME_MESSAGE_DISTANCE_MAX 5
/*
#define SAME_CLIENT_DISTANCE_SEC 15
#define SAME_CLIENT_DISTANCE_MAX 10
#define ACCKIN_TIMEOUT_SEC 300
*/

#define MESSAGE_MAX (MESSAGE_BUNDLE_SIZE * 2) // 2倍固定

static char *Title = "messagePool";
static char *ForeColor = "#197019";
static char *BackColor = "#f0fff8";
static char *Prompt = "message:";
static char *SubmitBtnText = "submit";
static char *ResetBtnText = "reset";

static char *HistFile;
static uint HistIndex;
static uint HistCount;

typedef struct Message_st
{
	time_t Time;
	uchar ClientIp[4];
	char Message[MESSAGE_SIZE + 1];
}
Message_t;

static Message_t MessageList[MESSAGE_MAX];
static uint MessageIndex;

static Message_t HistMessageList[MESSAGE_BUNDLE_SIZE];

static char *EncodeHtmlText(char *line)
{
	line = strx(line);

	line = replaceLine(line, "&", "&amp;", 0);
	line = replaceLine(line, "<", "&lt;", 0);
	line = replaceLine(line, ">", "&gt;", 0);

	return line;
}
static char *MkPrintMessage(Message_t *i)
{
	char *swrk;
	char *swrk2;
	char *sret;

	sret = xcout("%s　%s", swrk = makeStamp(i->Time), swrk2 = EncodeHtmlText(i->Message));
//	sret = xcout("%s <!--%s--> %s", swrk = makeStamp(i->Time), SockIp2Line(i->ClientIp), swrk2 = EncodeHtmlText(i->Message));
	memFree(swrk);
	memFree(swrk2);
	return sret;
}
static char *DecodeUrlMessage(char *line) // ret: strr(line)
{
	char *p;
	char *swrk;

	for (p = line; *p; p++)
	{
		if (*p == '%' && m_ishexadecimal(p[1]) && m_ishexadecimal(p[2]))
		{
			swrk = strxl(p + 1, 2);
			*p = toValueDigits(swrk, hexadecimal);
			memFree(swrk);

			copyLine(p + 1, p + 3);
		}
		else if (*p == '+')
		{
			*p = ' ';
		}
	}
	line2JLine(line, 1, 0, 0, 1);
	trim(line, ' ');

	if (MESSAGE_SIZE < strlen(line))
	{
		line[MESSAGE_SIZE] = '\0';
		line2JLine(line, 1, 0, 0, 1);
	}
	return line;
}
static char *MkLinkDiv(char *href, char *lref)
{
	return xcout("<div><a href=\"%s\">%s</a></div>", href, lref);
}
static int GetSameMessageCount(char *line, uint lbnd)
{
	uint num = 0;
	uint index;

	cout("GSMC: %u, %s\n", lbnd, line);

	for (index = lbnd; index < MessageIndex; index++)
	{
		char *lmess = MessageList[index].Message;

		if (!strcmp(lmess, line))
		{
			num++;
		}
	}
	cout("found-num: %u\n", num);
	return num;
}

static void SaveHistMessageList(uint index)
{
	FILE *fp;
	autoBlock_t gab;

	cout("Save-Hist: %u (%u)\n", index, HistCount);
	errorCase(HistCount < index);

	fp = fileOpen(HistFile, "r+b"); // "ab" だと fseek できない。r+ の場合 HistFile は存在している必要がある。
	fileSeek(fp, SEEK_SET, (sint64)index * sizeof(HistMessageList));
	fileWrite(fp, gndBlockVar(HistMessageList, sizeof(HistMessageList), gab));
	fileClose(fp);

	if (index == HistCount)
		HistCount++;
}
static void LoadHistMessageList(uint index)
{
	FILE *fp;
	autoBlock_t gab;

	cout("Load-Hist: %u (%u)\n", index, HistCount);
	errorCase(HistCount <= index);

	fp = fileOpen(HistFile, "rb");
	fileSeek(fp, SEEK_SET, (sint64)index * sizeof(HistMessageList));
	fileRead(fp, gndBlockVar(HistMessageList, sizeof(HistMessageList), gab));
	fileClose(fp);
}
static void AddToMessageList(char *message)
{
	Message_t *i;

	errorCase(MESSAGE_MAX <= MessageIndex);
	errorCase(!message);
	errorCase(MESSAGE_SIZE < strlen(message));

	i = MessageList + MessageIndex;

	i->Time = SockCurrTime;
	*(uint *)i->ClientIp = *(uint *)sockClientIp;
	strcpy(i->Message, message);

	MessageIndex++;

	if (MessageIndex == MESSAGE_MAX)
	{
		memcpy(HistMessageList, MessageList, MESSAGE_BUNDLE_SIZE * sizeof(Message_t));
		memcpy(MessageList, MessageList + MESSAGE_BUNDLE_SIZE, MESSAGE_BUNDLE_SIZE * sizeof(Message_t));
		MessageIndex = MESSAGE_BUNDLE_SIZE;

		SaveHistMessageList(HistIndex);
		HistIndex = (HistIndex + 1) % HIST_MAX;
	}
}

typedef struct Info_st
{
	autoBlock_t *RecvData;
	autoBlock_t *SendData;
	time_t ConnectTime;
}
Info_t;

static int Perform(int sock, uint prm)
{
	Info_t *i = (Info_t *)prm;
	char *line;
	char *swrk;
	autoList_t *tokens;
	char *lreq;
	autoList_t *divList;
	uint index;
	uint midx;
	char *sDivList;
	autoBlock_t *retBody;
	int doPutOnload = 0;

	cout("Perform: %p %s [%s]\n", i, swrk = makeStamp(0), SockIp2Line(sockClientIp));
	memFree(swrk);

	if (TIMEOUT_SEC <= SockCurrTime - i->ConnectTime) // ? time-out
	{
		cout("time-out\n");
		return 0;
	}
	if (i->SendData)
	{
		if (SockSendSequ(sock, i->SendData, 1) == -1)
		{
			cout("送信エラー\n");
			return 0;
		}
		cout("送信残り: %u\n", getSize(i->SendData));

		if (!getSize(i->SendData))
		{
			cout("送信完了\n");
			return 0;
		}
		return 1;
	}

	if (SockRecvSequ(sock, i->RecvData, 1) == -1)
	{
		cout("受信エラー\n");
		return 0;
	}
	cout("受信サイズ: %u\n", getSize(i->RecvData));

	line = SockNextLine(i->RecvData);
	if (!line)
	{
		if (HDRHDR_LENMAX < getSize(i->RecvData))
		{
			cout("最初の行、長すぎ (BIN)\n");
			return 0;
		}
		return 1;
	}
	if (HDRHDR_LENMAX < strlen(line))
	{
		cout("最初の行、長すぎ (STR)\n");
		memFree(line);
		return 0;
	}
	line2JLine(line, 1, 0, 0, 1);
	cout("Hdr: %s\n", line);

	tokens = tokenize(line, ' ');
	memFree(line);
	line = strx(refLine(tokens, 1));
	releaseDim(tokens, 1);
	cout("Req-URL: %s\n", line);

	lreq = line;
	if (swrk = strstr(lreq, "//")) // ? ホスト名有り
	{
		lreq = strchrEnd(swrk + 2, '/'); // ホスト名まで削る。
	}
	if (lreq[0] == '/')
	{
		lreq++;
	}
	cout("Req-Path: %s\n", lreq);

	divList = newList();

	if (lineExp("<5,09><>", lreq)) // more
	{
		cout("過去ログ参照\n");

		swrk = strxl(lreq, 5);
		index = toValue(swrk);
		memFree(swrk);

		cout("index: %u\n", index);

		if (index < HistCount)
		{
			LoadHistMessageList(index);

			for (midx = MESSAGE_BUNDLE_SIZE; midx; )
			{
				midx--;

				swrk = MkPrintMessage(HistMessageList + midx);
				addElement(divList, (uint)MkLinkDiv("q", swrk));
				memFree(swrk);
			}
			goto add_more;
		}
		addElement(divList, (uint)MkLinkDiv("q", "no-data"));
	}
	else if (swrk = strchr(lreq, '?')) // submit
	{
		cout("メッセージ入力\n");

		swrk = strchr(lreq, '=');
		if (!swrk)
		{
			goto ret_index;
		}
		swrk = strx(swrk + 1);
		swrk = DecodeUrlMessage(swrk);

		cout("メッセージ: %s\n", swrk);

		if (*swrk &&
			GetSameMessageCount(swrk, 0) < SAME_MESSAGE_MAX &&
			GetSameMessageCount(swrk, m_max((sint)MessageIndex - SAME_MESSAGE_DISTANCE, 0)) < SAME_MESSAGE_DISTANCE_MAX
			)
		{
			AddToMessageList(swrk);
		}
		memFree(swrk);

		// make refresh-page
		memFree(line);
		releaseDim(divList, 1);
		line = xcout(
			"<html>\n"
			"<head>\n"
			"<meta http-equiv=\"refresh\" content=\"0; url=q\"/>\n"
			"</head>\n"
			"<body bgcolor=\"%s\">\n"
			"<a href=\"q\">*</a>\n"
			"</body>\n"
			"</html>"
			,BackColor
			);

		goto made_retBody_line;
	}
	else // index
	{
	ret_index:
		cout("インなんとかさん\n");

		addElement(divList, (uint)xcout(
			"<div>"
			"<form method=\"GET\" name=\"q\" action=\"q\" accept-charset=\"Shift_JIS\">"
			"%s\n"
			"<input type=\"text\" size=\"%u\" name=\"q\"/>"
			"<input type=\"submit\" value=\"%s\"/>"
			"<input type=\"reset\" value=\"%s\"/>"
			"</form>"
			"</div>"
			,Prompt
			,MESSAGE_SIZE
			,SubmitBtnText
			,ResetBtnText
			));
		doPutOnload = 1;

		for (midx = MessageIndex; midx; )
		{
			midx--;

			swrk = MkPrintMessage(MessageList + midx);
			addElement(divList, (uint)MkLinkDiv("q", swrk));
			memFree(swrk);
		}
		index = HistIndex;

	add_more:
		index = (index + HIST_MAX - 1) % HIST_MAX;

		if (index < HistCount)
		{
			addElement(divList, (uint)MkLinkDiv(swrk = xcout("%05u", index), "more"));
			memFree(swrk);
		}
	}
	memFree(line);
	cout("divList: %u\n", getCount(divList));

	sDivList = untokenize(divList, "\n");
	releaseDim(divList, 1);

	// <me>.html_ -> line
	{
		char *mph_file = getSelfFile();
		autoList_t *mph_lines;

		mph_file = changeExt(mph_file, "html_");
		mph_lines = readLines(mph_file);

		line = untokenize(mph_lines, "\n");

		memFree(mph_file);
		releaseDim(mph_lines, 1);
	}

	line = replaceLine(line, "*title*", Title, 1);
	line = replaceLine(line, "*fore-color*", ForeColor, 1);
	line = replaceLine(line, "*back-color*", BackColor, 1);
	line = replaceLine(line, "*div-list*", sDivList, 1);

	if (doPutOnload)
		line = replaceLine(line,
			"<body>",
			"<body onload=\"document.q.q.focus()\">", 1);

	memFree(sDivList);
made_retBody_line:
	retBody = ab_makeBlockLine_x(line);
	cout("retBody: %u\n", getSize(retBody));

	{
	autoBlock_t *sdBuff = newBlock();

	ab_addLine(sdBuff, "HTTP/1.1 200 OK" HTTP_NEWLINE);
	ab_addLine(sdBuff, "Content-Type: text/html" HTTP_NEWLINE);
	ab_addLine_x(sdBuff, xcout("Content-Length: %u" HTTP_NEWLINE, getSize(retBody)));
	ab_addLine(sdBuff, HTTP_NEWLINE);
	addBytes(sdBuff, retBody);

	i->SendData = sdBuff;
	}
	releaseAutoBlock(retBody);
	cout("送信サイズ: %u\n", getSize(i->SendData));
	return 1;
}

static uint CreateInfo(void)
{
	Info_t *i = (Info_t *)memAlloc(sizeof(Info_t));

	i->RecvData = newBlock();
	i->SendData = NULL;
	i->ConnectTime = SockCurrTime;

	cout("Ctor: %p [%s]\n", i, SockIp2Line(sockClientIp));
	return (uint)i;
}
static void ReleaseInfo(uint prm)
{
	Info_t *i = (Info_t *)prm;

	cout("Dtor: %p [%s]\n", i, SockIp2Line(sockClientIp));

	releaseAutoBlock(i->RecvData);
	if (i->SendData) releaseAutoBlock(i->SendData);

	memFree(i);
}
static int Idle(void)
{
	while (hasKey())
	{
		if (getKey() == 'Q')
		{
			cout("Q osareta!\n");
			return 0;
		}
		cout("Q oshitara shu-ryo-.\n");
	}
	return 1;
}

int main(int argc, char **argv)
{
	uint portno = 80;

readArgs:
	if (argIs("/T")) // Title
	{
		Title = nextArg();
		goto readArgs;
	}
	if (argIs("/F")) // Fore-color
	{
		ForeColor = nextArg();
		goto readArgs;
	}
	if (argIs("/B")) // Back-color
	{
		BackColor = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // Prompt
	{
		Prompt = nextArg();
		goto readArgs;
	}
	if (argIs("/S")) // Submit-button title
	{
		SubmitBtnText = nextArg();
		goto readArgs;
	}
	if (argIs("/R")) // Reset-button title
	{
		ResetBtnText = nextArg();
		goto readArgs;
	}

	if (hasArgs(1))
	{
		portno = toValue(nextArg());
	}
	cout("待ち受けポート番号: %u\n", portno);

	HistFile = makeTempFile("mp-hist");

	sockServerUserTransmit(Perform, CreateInfo, ReleaseInfo, portno, 20, Idle);

	removeFile(HistFile);
	memFree(HistFile);

	cout("\\e\n");
}
