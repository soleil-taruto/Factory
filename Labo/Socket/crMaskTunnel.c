/*
	クライアント側で生成したパターンをサーバーに渡し、
	以降の全ての通信をそのパターンでマスクする。

	-> これも何かマズい気がするｗ
*/

#include "C:\Factory\Labo\Socket\tunnel\libs\Tunnel.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

#define SENDRECV_TIMEOUT_MILLIS 2000
#define MASKPTN_SIZE 16

static int ServerMode;

typedef struct FltrInfo_st
{
	autoBlock_t *MaskPtn;
	uint RPos;
}
FltrInfo_t;

static void Fltr(autoBlock_t *buff, uint prm)
{
	FltrInfo_t *i = (FltrInfo_t *)prm;
	uchar *data;
	uint dataSize;
	uint index;

	data = directGetBuffer(buff);
	dataSize = getSize(buff);

	for (index = 0; index < dataSize; index++)
	{
		data[index] ^= getByte(i->MaskPtn, i->RPos);

		i->RPos++;
		i->RPos %= MASKPTN_SIZE;
	}
}
static void Perform(int sock, int fwdSock)
{
	autoBlock_t *maskPtn;
	FltrInfo_t fltrInfos[2];

	if (ServerMode)
	{
		maskPtn = newBlock();

		if (SockRecvSequLoop(sock, maskPtn, SENDRECV_TIMEOUT_MILLIS, MASKPTN_SIZE) != MASKPTN_SIZE)
			goto disconnect;
	}
	else
	{
		maskPtn = makeCryptoRandBlock(MASKPTN_SIZE);

		if (SockSendSequLoop(fwdSock, maskPtn, SENDRECV_TIMEOUT_MILLIS) != MASKPTN_SIZE)
			goto disconnect;
	}

	fltrInfos[0].MaskPtn = maskPtn;
	fltrInfos[0].RPos = 0;
	fltrInfos[1].MaskPtn = maskPtn;
	fltrInfos[1].RPos = 0;

	CrossChannel(sock, fwdSock, NULL, (uint)fltrInfos, NULL, (uint)(fltrInfos + 1));

disconnect:
	releaseAutoBlock(maskPtn);
}
static int ReadArgs(void)
{
	if (argIs("/R"))
	{
		ServerMode = 1;
		return 1;
	}
	return 0;
}
static char *GetTitleSuffix(void)
{
	return xcout("[%s]", ServerMode ? "Server" : "Client");
}
int main(int argc, char **argv)
{
	TunnelMain(ReadArgs, Perform, "crMaskTunnel", GetTitleSuffix);
}
