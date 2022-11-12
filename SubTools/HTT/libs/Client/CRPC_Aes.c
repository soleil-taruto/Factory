/*
	使い方

		SockStream_t *ss = ClientBegin(sock, SERVICE_NAME);

		ClientCRPC_Begin(ss);

		recvData = ClientCRPC(ss, sendData, RAW_KEY));
		recvData = ClientCRPC(ss, sendData, RAW_KEY));
		recvData = ClientCRPC(ss, sendData, RAW_KEY));
		...

		ClientCRPC_End();

		ClientEnd(ss);
*/

#include "CRPC_Aes.h"

static autoBlock_t *EncCounter;

void ClientCRPC_Begin(SockStream_t *ss)
{
	autoBlock_t *clSeed;
	autoBlock_t *svSeed;

	errorCase(!ss);

	clSeed = MakeEncCounterSeed();
	svSeed = ClientRPC(ss, clSeed);

	if (EncCounter)
		releaseAutoBlock(EncCounter);

	if (getSize(svSeed) == 16)
		EncCounter = MakeEncCounter(clSeed, svSeed);
	else
		EncCounter = NULL;

	releaseAutoBlock(clSeed);
	releaseAutoBlock(svSeed);
}
void ClientCRPC_End(void)
{
	if (EncCounter)
	{
		releaseAutoBlock(EncCounter);
		EncCounter = NULL;
	}
}
autoBlock_t *ClientCRPC(SockStream_t *ss, autoBlock_t *sendData, autoBlock_t *rawKey) // ret: NULL ナシ, 通信失敗 -> 0 バイト
{
	autoBlock_t *recvData;

	errorCase(!ss);
	errorCase(!sendData);
	errorCase(!rawKey);
	errorCase(getSize(rawKey) != 16);

	if (!EncCounter) // ? ClientCRPC_Begin() 未実行・未成功
	{
		cout("C_CRPC_Aes_カウンターが無い！\n");
		return newBlock();
	}
	sendData = copyAutoBlock(sendData);

	PutHash(sendData);
	Crypt(sendData, rawKey, EncCounter);

	recvData = ClientRPC(ss, sendData);

	releaseAutoBlock(sendData);

	Crypt(recvData, rawKey, EncCounter);

	if (!UnputHash(recvData))
	{
		cout("C_CRPC_Aes_受信データ破損\n");
		setSize(recvData, 0);
	}
	return recvData;
}
