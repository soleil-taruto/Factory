/*
	HTTPProxy.exe RECV-PORT DMY-FWD-HOST DMY-FWD-PORT /S

		/S ... 停止する。

	HTTPProxy.exe RECV-PORT DMY-FWD-HOST DMY-FWD-PORT [/C CONNECT-MAX] [/O OK-IP-PFX]... [/N NG-IP-PFX]...

		CONNECT-MAX ... 最大接続数, 省略時は 30
		TRACK-NAME ... トラック名
		DMY-FWD-HOST ... ダミー
		DMY-FWD-PORT ... ダミー
		OK-IP-PFX ... 転送先として許可するIPアドレスの開始パターン
		NG-IP-PFX ... 転送先として許可しないIPアドレスの開始パターン

	----
	★問題点

		Connection: Keep-Alive 未対応
*/

#include "..\libs\Tunnel.h"
#include "..\libs\HTTPParse.h"
#include "C:\Factory\Common\Options\SockStream.h"

static autoList_t *OKIPPrfxList; // "" を含まない
static autoList_t *NGIPPrfxList; // "" を含まない

static int DoConnect(char *fwdHost, uint fwdPortNo) // ret: -1 == 接続できなかった。
{
	uchar ip[4];
	char *strIp;
	char *ipprfx;
	uint index;

	sockLookup(ip, fwdHost);
	strIp = SockIp2Line(ip);
	cout("FWD_IP: %s\n", strIp);

	if (!*(uint *)ip)
	{
		cout("NO_HOST\n");
		return -1;
	}
	if (getCount(OKIPPrfxList))
	{
		foreach (OKIPPrfxList, ipprfx, index)
		{
			if (startsWith(strIp, ipprfx))
				break;
		}
		if (!ipprfx)
		{
			cout("NOT_OKIP\n");
			return -1;
		}
	}
	foreach (NGIPPrfxList, ipprfx, index)
	{
		if (startsWith(strIp, ipprfx))
		{
			cout("IS_NGIP\n");
			return -1;
		}
	}
	return sockConnect(ip, NULL, fwdPortNo);
}
static void PreDataFltr(autoBlock_t *buff, uint uPData)
{
	autoBlock_t **pData = (autoBlock_t **)uPData;

	if (*pData)
	{
		autoBlock_t *nb = newBlock();

		ab_addBytes(nb, *pData);
		ab_addBytes(nb, buff);

		ab_swap(nb, buff);

		releaseAutoBlock(nb);

		*pData = NULL;
	}
}
static void PerformTh(int sock, char *strip)
{
	SockStream_t *ss = CreateSockStream(sock, 60);
	autoBlock_t *buff = newBlock();

	cout("接続 %d\n", sock);

	for (; ; )
	{
		int chr = SockRecvChar(ss);
		uint endPos;

		if (chr == EOF)
			break;

		addByte(buff, chr);
		endPos = getSize(buff);

		if (4 <= endPos && !memcmp((uchar *)directGetBuffer(buff) + endPos - 4, "\r\n\r\n", 4))
		{
			uint fndPos;
			char *host;
			char *p;
			char *fwdHost;
			uint fwdPortNo;
			int fwdSock;

			HTTPParse(buff);

			// ---- Host:

			fndPos = findLineCase(HttpDat.H_Keys, "Host", 1);

			if (fndPos == getCount(HttpDat.H_Keys))
			{
				cout("NO_HOST_FIELD\n");
				break;
			}
			host = getLine(HttpDat.H_Values, fndPos);

			// ---- Connection:

			fndPos = findLineCase(HttpDat.H_Keys, "Connection", 1);

			if (fndPos == getCount(HttpDat.H_Keys))
			{
				addElement(HttpDat.H_Keys, (uint)strx("Connection"));
				addElement(HttpDat.H_Values, (uint)NULL);
			}
			strzp((char **)directGetPoint(HttpDat.H_Values, fndPos), "close");

			// ----

			p = strchr(host, ':');

			if (p)
			{
				*p = '\0';
				p++;

				fwdHost = strx(host);
				fwdPortNo = toValue(p);
			}
			else
			{
				fwdHost = strx(host);
				fwdPortNo = 80;
			}
			cout("FWD_HOST: %s\n", fwdHost);
			cout("FWD_PORT: %u\n", fwdPortNo);

			fwdSock = DoConnect(fwdHost, fwdPortNo);

			cout("FWD_SOCK: %d\n", fwdSock);

			if (fwdSock != -1)
			{
				autoBlock_t *buffTmp = buff; // PreDataFltr() で NULL をセットされる。

				ab_addBytes_x(buff, SockRipRecvBuffer(ss));

				CrossChannel(sock, fwdSock, PreDataFltr, (uint)&buffTmp, NULL, 0);
				sockDisconnect(fwdSock);
			}
			memFree(fwdHost);
			break;
		}
		if (65000 < endPos)
		{
			cout("OVERFLOW_HEADER_SIZE\n");
			break;
		}
	}
	ReleaseSockStream(ss);
	releaseAutoBlock(buff);

	cout("切断 %d\n", sock);
}
static int ReadArgs(void)
{
	if (argIs("/O"))
	{
		char *ipprfx = nextArg();

		errorCase(!*ipprfx); // ? ""

		addElement(OKIPPrfxList, (uint)ipprfx);
		return 1;
	}
	if (argIs("/N"))
	{
		char *ipprfx = nextArg();

		errorCase(!*ipprfx); // ? ""

		addElement(NGIPPrfxList, (uint)ipprfx);
		return 1;
	}

	cout("OKIPPrfxNum: %u\n", getCount(OKIPPrfxList));
	cout("NGIPPrfxNum: %u\n", getCount(NGIPPrfxList));

	return 0;
}
int main(int argc, char **argv)
{
	ConnectMax = 30;

	OKIPPrfxList = newList();
	NGIPPrfxList = newList();

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "HTTPProxy_old", NULL);
	TunnelPerformTh = NULL;
}
