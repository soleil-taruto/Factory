/*
	使い方

		SockStream_t *ss = ClientBegin(sock, SERVICE_NAME);

		recvData = ClientCRPC(ss, sendData, RAW_KEY));
		recvData = ClientCRPC(ss, sendData, RAW_KEY));
		recvData = ClientCRPC(ss, sendData, RAW_KEY));
		...

		ClientEnd(ss);
*/

#include "CRPC_Aes2.h"

autoBlock_t *ClientCRPC(SockStream_t *ss, autoBlock_t *sendData, autoBlock_t *rawKey) // ret: NULL ナシ, 通信失敗 -> 0 バイト
{
	autoBlock_t *rawKey1;
	autoBlock_t *rawKey2;
	autoBlock_t *rawKey3;
	autoBlock_t *rawKey4;
	autoBlock_t *recvData;

	errorCase(!ss);
	errorCase(!sendData);
	errorCase(!rawKey);
	errorCase(getSize(rawKey) != 64);

	rawKey1 = ab_makeSubBytes(rawKey,  0, 16);
	rawKey2 = ab_makeSubBytes(rawKey, 16, 16);
	rawKey3 = ab_makeSubBytes(rawKey, 32, 16);
	rawKey4 = ab_makeSubBytes(rawKey, 48, 16);

	sendData = copyAutoBlock(sendData);

	Add_RandPart_Padding(sendData);
	PutHash(sendData);
	RCBCEncrypt(sendData, rawKey1);
	RCBCEncrypt(sendData, rawKey2);
	RCBCEncrypt(sendData, rawKey3);
	RCBCEncrypt(sendData, rawKey4);

	recvData = ClientRPC(ss, sendData);

	releaseAutoBlock(sendData);

	if (
		!RCBCDecrypt(recvData, rawKey4) ||
		!RCBCDecrypt(recvData, rawKey3) ||
		!RCBCDecrypt(recvData, rawKey2) ||
		!RCBCDecrypt(recvData, rawKey1) ||
		!UnputHash(recvData) ||
		!Unadd_RandPart_Padding(recvData)
		)
	{
		cout("C_CRPC_Aes2_受信データ破損\n");
		setSize(recvData, 0);
	}

	releaseAutoBlock(rawKey1);
	releaseAutoBlock(rawKey2);
	releaseAutoBlock(rawKey3);
	releaseAutoBlock(rawKey4);

	return recvData;
}
