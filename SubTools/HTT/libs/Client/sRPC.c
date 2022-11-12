#include "sRPC.h"

uint RPC_RecvSizeMax = 1000000; // 受信データサイズの上限、実行前に設定すること。

static void DoSend(SockStream_t *ss, autoBlock_t *sendData)
{
	SockSendValue(ss, getSize(sendData));
	SockSendBlock(ss, directGetBuffer(sendData), getSize(sendData));
	SockFlush(ss);
}
static autoBlock_t *DoRecv(SockStream_t *ss)
{
	uint recvSize = SockRecvValue(ss);
	void *recvData;

	if (RPC_RecvSizeMax < recvSize)
	{
		cout("ClientRPC_Overflow: %u (max: %u)\n", recvSize, RPC_RecvSizeMax);
		recvSize = 0;
		DestroySockStream(ss);
	}
	recvData = memAlloc(recvSize);
	SockRecvBlock(ss, recvData, recvSize);
	return bindBlock(recvData, recvSize);
}
autoBlock_t *ClientRPC(SockStream_t *ss, autoBlock_t *sendData) // ret: NULL ナシ, 通信失敗 -> 0 バイト
{
	autoBlock_t *recvData;

	errorCase(!ss);
	errorCase(!sendData);

	SetSockStreamTimeout(ss, Client_SockTimeoutSec);

	DoSend(ss, sendData);
	recvData = DoRecv(ss);

	errorCase(!recvData); // 2bs

	return recvData;
}
