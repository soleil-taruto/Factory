/*
	HTTPSpoof.exe RECV-PORT DMY-FWD-HOST DMY-FWD-PORT /S

		/S ... 停止する。

	HTTPSpoof.exe RECV-PORT DMY-FWD-HOST DMY-FWD-PORT [/D] [/R H_KEY H_VALUE]...

		/D ... リクエストURLにドメイン名を挿入する。
		/R ... 置き換え又は追加するヘッダフィールド

	----
	ローカルへのアクセスをプロキシを通して外部に飛ばす。

		_http://localhost/xxx を inet.com:8080 に飛ばしたい。

			tunnel 80 inet.com 8080

		更にプロキシ proxy:3210 を通したい。

			HTTPSpoof 80 proxy 3210 /d /r Host inet.com:8080

	----
	★問題点

		Connection: Keep-Alive 未対応
		下りのメッセージについて何もできない。
*/

#include "..\libs\Tunnel.h"
#include "..\libs\HTTPParse.h"
#include "C:\Factory\Common\Options\SockStream.h"

static int PutDomainUrl;
static autoList_t *R_Keys;
static autoList_t *R_Values;

static void SpoofHeader(void)
{
	// ヘッダフィールド置換・追加
	{
		char *r_key;
		uint r_ndx;

		foreach (R_Keys, r_key, r_ndx)
		{
			uint h_ndx = findLineCase(HttpDat.H_Keys, r_key, 1);
			char *h_key;
			char *h_val;
			char *r_val = getLine(R_Values, r_ndx);

			if (h_ndx == getCount(HttpDat.H_Keys))
			{
				addElement(HttpDat.H_Keys, (uint)strx(r_key));
				addElement(HttpDat.H_Values, (uint)strx("[未定義]"));
			}
			h_key = getLine(HttpDat.H_Keys, h_ndx);
			h_val = getLine(HttpDat.H_Values, h_ndx);

			cout("H_K: [%s]\n", h_key);
			cout("H_V: [%s] -> [%s]\n", h_val, r_val);

			memFree(h_val);
			setElement(HttpDat.H_Values, h_ndx, (uint)strx(r_val));
		}
	}

	if (PutDomainUrl) // ドメイン名をurlに挿入
	{
		char *host = refLine(HttpDat.H_Values, findLineCase(HttpDat.H_Keys, "Host", 1));
		char *insPtn;

		cout("PutDmnUrl\n");
		cout("host: %s\n", host);
		cout("< %s\n", HttpDat.H_Request);

		insPtn = xcout("GET http://%s/", host);

		if (5 <= strlen(HttpDat.H_Request))
			eraseLine(HttpDat.H_Request, 5); // -= "GET /"

		HttpDat.H_Request = insertLine_x(HttpDat.H_Request, 0, insPtn);

		cout("> %s\n", HttpDat.H_Request);
	}
}
static char *HFldFolding(char *line)
{
	uint index = 70; // こんな長いキーは無いだろう...

	while (index + 10 <= strlen(line))
	{
		if (line[index] <= '\x20') // ? WSP
		{
			line = insertLine(line, index, "\r\n");
			line[index + 2] = '\t';
			index += 50;
		}
		index++;
	}
	return line;
}
static void RemakeHeader(autoBlock_t *buff)
{
	uint index;

	setSize(buff, 0);
	ab_addLine(buff, HttpDat.H_Request);
	ab_addLine(buff, "\r\n");

	for (index = 0; index < getCount(HttpDat.H_Keys); index++)
	{
		char *key = getLine(HttpDat.H_Keys, index);
		char *value = getLine(HttpDat.H_Values, index);
		char *line;

		line = xcout("%s: %s", key, value);
		line = HFldFolding(line);

		ab_addLine_x(buff, line);
		ab_addLine(buff, "\r\n");
	}
	ab_addLine(buff, "\r\n");
}

static int DoConnect(void) // ret: -1 == 接続できなかった。
{
	static uchar ip[4];
	return sockConnect(ip, FwdHost, FwdPortNo);
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
			int fwdSock;

			HTTPParse(buff);
			SpoofHeader();
			RemakeHeader(buff);

			fwdSock = DoConnect();

			cout("FWD_SOCK: %d\n", fwdSock);

			if (fwdSock != -1)
			{
				autoBlock_t *buffTmp = buff; // PreDataFltr() で NULL をセットされる。

				ab_addBytes_x(buff, SockRipRecvBuffer(ss));

				CrossChannel(sock, fwdSock, PreDataFltr, (uint)&buffTmp, NULL, 0);
				sockDisconnect(fwdSock);
			}
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
	if (argIs("/D"))
	{
		PutDomainUrl = 1;
		return 1;
	}
	if (argIs("/R"))
	{
		addElement(R_Keys, (uint)nextArg());
		addElement(R_Values, (uint)nextArg());
		return 1;
	}

	// 必須
	addElement(R_Keys, (uint)"Connection");
	addElement(R_Values, (uint)"close");

	cout("PutDomainUrl: %d\n", PutDomainUrl);
	cout("R_Keys: %u\n", getCount(R_Keys));

	return 0;
}
int main(int argc, char **argv)
{
	ConnectMax = 30;

	R_Keys = newList();
	R_Values = newList();

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "HTTPSpoof", NULL);
	TunnelPerformTh = NULL;
}
