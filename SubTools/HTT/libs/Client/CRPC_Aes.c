/*
	�g����

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
autoBlock_t *ClientCRPC(SockStream_t *ss, autoBlock_t *sendData, autoBlock_t *rawKey) // ret: NULL �i�V, �ʐM���s -> 0 �o�C�g
{
	autoBlock_t *recvData;

	errorCase(!ss);
	errorCase(!sendData);
	errorCase(!rawKey);
	errorCase(getSize(rawKey) != 16);

	if (!EncCounter) // ? ClientCRPC_Begin() �����s�E������
	{
		cout("C_CRPC_Aes_�J�E���^�[�������I\n");
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
		cout("C_CRPC_Aes_��M�f�[�^�j��\n");
		setSize(recvData, 0);
	}
	return recvData;
}
