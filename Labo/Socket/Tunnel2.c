/*
	Tunnel.exe RECV-PORT FWRD-HOST FWRD-PORT [/S] [/C CONNECT-MAX]

		/S ... 停止する。
		CONNECT-MAX ... 最大接続数, 省略時は 1000
*/

#include "C:\Factory\Common\Options\SockServerTh.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "C:\Factory\Common\Options\SockClient.h"

#define STOPEVENTUUID "{0bc9759b-cb10-4fdc-a2e5-d67ef5facbeb}"

#define DEF_CONNECT_MAX 1000

static uint PortNo;
static char *FwdHost;
static uint FwdPortNo;
static uint ConnectMax = DEF_CONNECT_MAX;

static char *StopEventName;
static uint StopEventHdl;
static int StopServerRq;

static void TransmitProc(SockStream_t *rss, SockStream_t *wss, char *header)
{
	int retval = 0;
	uint c;
	char *pLine = strx("");

	for (c = 0; c < 65000; c++)
	{
		int chr;
		int pchr;

		if (
			!SockRecvCharWait(rss, 0) ||
			!SockSendCharWait(wss, 0)
			)
			if (
				!SockRecvCharWait(rss, 100) ||
				!SockSendCharWait(wss, 100)
				)
				break;

		chr = SockRecvChar(rss);

		if (chr == EOF)
			break;

		pchr = chr;
		m_toHalf(pchr);
		pLine = addChar(pLine, pchr);

		if (chr == '\n')
			pLine = addChar(pLine, '\n');

		SockSendChar(wss, chr);
	}
	TrySockFlush(wss, 0);

	if (*pLine)
	{
		char *p = strchr(pLine, '\0') - 1;

		if (*p == '\n')
			*p = '\0';

		if (*pLine)
		{
			cout("%s\n", header);
			cout("%s\n", pLine);
		}
	}
	memFree(pLine);
}
static void EndConnectProc(SockStream_t *ss)
{
	SetSockStreamTimeout(ss, 2);
	SockFlush(ss);
	ReleaseSockStream(ss);
}
static void PerformTh(int sock, char *strip)
{
	uchar ip[4] = { 0 };
	int fwdSock;
	SockStream_t *ss;
	SockStream_t *fss;

	fwdSock = sockConnect(ip, FwdHost, FwdPortNo);

	if (fwdSock == -1) // ? 失敗
	{
		cout("転送先に接続できませんでした。\n");
		return;
	}
	ss = CreateSockStream(sock, 0);
	fss = CreateSockStream(fwdSock, 0);

	cout("接続 %d\n", sock);

	while (!StopServerRq)
	{
		TransmitProc(ss, fss, "上り");
		TransmitProc(fss, ss, "下り");

		if (
			IsEOFSockStream(ss) ||
			IsEOFSockStream(fss)
			)
			break;
	}
	TransmitProc(ss, fss, "上り切断中");
	TransmitProc(fss, ss, "下り切断中");

	EndConnectProc(ss);
	EndConnectProc(fss);

	sockDisconnect(fwdSock);

	cout("切断 %d\n", sock);
}
static int IdleTh(void)
{
	while (hasKey())
		if (getKey() == 0x1b) // ? エスケープキー押下 -> 停止要求
			StopServerRq = 1;

	if (handleWaitForMillis(StopEventHdl, 0)) // ? 停止要求
		StopServerRq = 1;

	if (!StopServerRq)
		return 1;

	cout("停止します...\n");
	return 0;
}
int main(int argc, char **argv)
{
	cout("★これも古いバージョンです。\n");

	PortNo = toValue(nextArg());
	FwdHost = nextArg();
	FwdPortNo = toValue(nextArg());

	errorCase(!PortNo || 0xffff < PortNo);
	errorCase(m_isEmpty(FwdHost));
	errorCase(!FwdPortNo || 0xffff < FwdPortNo);

	StopEventName = xcout(STOPEVENTUUID ".%u", PortNo);
	StopEventHdl = eventOpen(StopEventName);

	if (argIs("/S"))
	{
		eventWakeupHandle(StopEventHdl);
		return;
	}
	if (argIs("/C"))
	{
		ConnectMax = toValue(nextArg());
	}

	cmdTitle_x(xcout("Tunnel from localhost:%u to %s:%u C:%u", PortNo, FwdHost, FwdPortNo, ConnectMax));

	sockServerTh(PerformTh, PortNo, ConnectMax, IdleTh);

	handleClose(StopEventHdl);
}
