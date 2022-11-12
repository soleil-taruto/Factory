/*
	Server.exe [/S 送信データ入力ファイル] [/I] [/R 受信データ出力ファイル] [/T セッション_タイムアウト秒] [待ち受けポート番号]

		/I ... エディタを開いて送信データを入力する。
*/

#include "C:\Factory\Common\Options\SockServer.h"

static char *SendFile;
static autoBlock_t *SendData;
static FILE *RecvFp;
static uint TimeoutSec;
static uint ConnectCount;

typedef struct Info_st
{
	autoBlock_t *RecvQueue;
	autoBlock_t *SendQueue;
	uint Timeout;
}
Info_t;

static void DispRecvData(Info_t *i)
{
	if (getSize(i->RecvQueue))
	{
		char *line = toPrintLine(i->RecvQueue, 1);

		cout("%p > %s\n", i, line);
		memFree(line);

		if (RecvFp)
			writeBinaryBlock(RecvFp, i->RecvQueue);

		setSize(i->RecvQueue, 0);
	}
}

static uint CreateInfo(void)
{
	Info_t *i = (Info_t *)memAlloc(sizeof(Info_t));

	cout("%p 接続\n", i);

	ConnectCount++;
	cmdTitle_x(xcout("Server - Connect %u", ConnectCount));

	i->RecvQueue = newBlock();
	i->SendQueue = newBlock();
	i->Timeout = TimeoutSec ? now() + TimeoutSec : 0;

	if (SendFile)
		ab_addBytes_x(i->SendQueue, readBinary(SendFile));

	if (SendData)
		addBytes(i->SendQueue, SendData);

	return (uint)i;
}
static void ReleaseInfo(uint prm)
{
	Info_t *i = (Info_t *)prm;

	cout("%p 切断\n", i);

	ConnectCount--;
	cmdTitle_x(xcout("Server - Connect %u", ConnectCount));

	DispRecvData(i);

	releaseAutoBlock(i->RecvQueue);
	releaseAutoBlock(i->SendQueue);

	memFree(i);
}

static int StopServer;

static int Perform(int sock, uint prm)
{
	Info_t *i = (Info_t *)prm;

	if (StopServer) // sockServerUserTransmit の場合 Idle(funcIdle) が 0 を返したら即切断・終了するので、ここで判定して 0 を返す必要は無い。でも無害なので放置する。
		return 0;

	if (SockRecvSequ(sock, i->RecvQueue, 1) == -1)
		return 0;

	DispRecvData(i);

	if (SockSendSequ(sock, i->SendQueue, 0) == -1)
		return 0;

	while (hasKey())
	{
		int chr = getKey();

		if (chr == 0x1b)
		{
			StopServer = 1;
			return 0;
		}
		if (chr == 'D')
			return 0;

		if (chr == 'I')
		{
			cmdTitle("Server - Input");

			ab_addLine_x(i->SendQueue, coInputLine());

			// CR-LF
			addByte(i->SendQueue, '\r');
			addByte(i->SendQueue, '\n');

			cmdTitle("Server");
		}
	}
	return !i->Timeout || now() <= i->Timeout;
}
static int Idle(void)
{
	if (!ConnectCount)
	{
		while (hasKey())
		{
			if (getKey() == 0x1b)
			{
				StopServer = 1;
			}
		}
	}
	return !StopServer;
}
int main(int argc, char **argv)
{
	uint portno = 23;

readArgs:
	if (argIs("/S"))
	{
		SendFile = nextArg();
		goto readArgs;
	}
	if (argIs("/I"))
	{
		SendData = inputTextAsBinary();
		goto readArgs;
	}
	if (argIs("/R"))
	{
		RecvFp = fileOpen(nextArg(), "wb");
		goto readArgs;
	}
	if (argIs("/T"))
	{
		TimeoutSec = toValue(nextArg());
		goto readArgs;
	}

	if (hasArgs(1))
	{
		portno = toValue(nextArg());
	}
	sockServerUserTransmit(Perform, CreateInfo, ReleaseInfo, portno, 10, Idle);

	if (RecvFp)
	{
		fileClose(RecvFp);
	}
}
