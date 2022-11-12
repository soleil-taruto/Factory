/*
	あらゆる接続に対して同じ応答をして切断する。

	ConcreteSite.exe [/P ポート番号] [応答ファイル | /H 応答HTMLファイル]
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "tunnel\libs\TimeWaitMonitor.h"

#define HTTP_NEWLINE "\r\n"

#define SESSION_TIMEOUT_SEC 2

static uint PortNo = 80;
static autoBlock_t *ResData;

static void RecvHTTPHeader(SockStream_t *ss)
{
	for (; ; )
	{
		char *line = SockRecvLine(ss, 65536);

		if (!*line)
		{
			memFree(line);
			break;
		}
		memFree(line);
	}
}
static int Perform(int sock, uint dummy)
{
	SockStream_t *ss = CreateSockStream(sock, SESSION_TIMEOUT_SEC);

	LOGPOS();
	cout("%s\n", SockIp2Line(sockClientIp));

	RecvHTTPHeader(ss);

	SockSendBlock(ss, directGetBuffer(ResData), getSize(ResData));
	SockFlush(ss);

	ReleaseSockStream(ss);

	AddTimeWait();
	return 0; // 通信終了
}
static int Idle(void)
{
	while (hasKey())
		if (getKey() == 0x1b)
			return 0;

	return 1;
}

static void Main_Res(autoBlock_t *res)
{
	ResData = res;
	sockServerUserTransmit(Perform, getZero, noop_u, PortNo, 1, Idle);
	ResData = NULL;
}
static void Main_ResFile(char *file)
{
	autoBlock_t *res = readBinary(file);
	Main_Res(res);
	releaseAutoBlock(res);
}
static void Main_ResHTML(char *resHTML)
{
	autoBlock_t *res = newBlock();

	ab_addLine(res, "HTTP/1.1 200 OK" HTTP_NEWLINE);
	ab_addLine(res, "Content-Type: text/html" HTTP_NEWLINE);
	ab_addLine_x(res, xcout("Content-Length: %u" HTTP_NEWLINE, strlen(resHTML)));
	ab_addLine(res, HTTP_NEWLINE);
	ab_addLine(res, resHTML);

	Main_Res(res);

	releaseAutoBlock(res);
}
static void Main_ResHTMLFile(char *file)
{
	char *resHTML = readText_b(file);
	Main_ResHTML(resHTML);
	memFree(resHTML);
}

int main(int argc, char **argv)
{
	if (argIs("/P"))
	{
		PortNo = toValue(nextArg());
	}

	errorCase(!m_isRange(PortNo, 1, 65535));

	if (argIs("/H"))
	{
		Main_ResHTMLFile(nextArg());
	}
	else if (hasArgs(1))
	{
		Main_ResFile(nextArg());
	}
	else
	{
		Main_ResHTML("<h1>MAINTENANCE</h1>");
	}
}
