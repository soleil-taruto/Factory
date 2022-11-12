#include "sRPC.h"

uint RPC_RecvSizeMax = 1000000; // ��M�f�[�^�T�C�Y�̏���A���s�O�ɐݒ肷�邱�ƁB

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
autoBlock_t *ClientRPC(SockStream_t *ss, autoBlock_t *sendData) // ret: NULL �i�V, �ʐM���s -> 0 �o�C�g
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
