/*
	使い方

		接続前

			CRPC_Before();

		接続中

			SockStream_t *ss = ClientBegin(sock, SERVICE_NAME);

			ClientCRPC_Begin(ss);

			recvData = ClientCRPC(ss, sendData));
			recvData = ClientCRPC(ss, sendData));
			recvData = ClientCRPC(ss, sendData));
			...

			ClientCRPC_End();

			ClientEnd(ss);
*/

#include "CRPC_RSA_Aes.h"

static int KeyGenerated;

void ClientCRPC_PreConnect(void)
{
	if (!KeyGenerated)
	{
		RSA_GenerateKey();
		KeyGenerated = 1;
	}
}

static autoBlock_t *S_EncCounter1;
static autoBlock_t *S_EncCounter2;
static autoBlock_t *S_EncCounter3;
static autoBlock_t *S_EncCounter4;
static autoBlock_t *S_RawKey1;
static autoBlock_t *S_RawKey2;
static autoBlock_t *S_RawKey3;
static autoBlock_t *S_RawKey4;

void ClientCRPC_Begin(SockStream_t *ss)
{
	autoBlock_t *sendData;
	autoBlock_t *recvData;
	autoBlock_t *seed;

	errorCase(!ss);
	errorCase(!KeyGenerated);

//	RSA_GenerateKey(); // 時間が掛かるので移動
	KeyGenerated = 0;
	sendData = RSA_GetPublicKey();

	recvData = ClientRPC(ss, sendData);

	seed = RSA_Decrypt(recvData);
	errorCase_m(!seed, "C_CRPC_RSA_Aes_鍵共有失敗");
	errorCase(getSize(seed) != 128);

	S_EncCounter1 = ab_makeSubBytes(seed,   0, 16);
	S_EncCounter2 = ab_makeSubBytes(seed,  16, 16);
	S_EncCounter3 = ab_makeSubBytes(seed,  32, 16);
	S_EncCounter4 = ab_makeSubBytes(seed,  48, 16);
	S_RawKey1     = ab_makeSubBytes(seed,  64, 16);
	S_RawKey2     = ab_makeSubBytes(seed,  80, 16);
	S_RawKey3     = ab_makeSubBytes(seed,  96, 16);
	S_RawKey4     = ab_makeSubBytes(seed, 112, 16);

	releaseAutoBlock(sendData);
	releaseAutoBlock(recvData);
	releaseAutoBlock(seed);
}
void ClientCRPC_End(void)
{
	if (S_EncCounter1)
	{
		releaseAutoBlock(S_EncCounter1);
		releaseAutoBlock(S_EncCounter2);
		releaseAutoBlock(S_EncCounter3);
		releaseAutoBlock(S_EncCounter4);
		releaseAutoBlock(S_RawKey1);
		releaseAutoBlock(S_RawKey2);
		releaseAutoBlock(S_RawKey3);
		releaseAutoBlock(S_RawKey4);

		S_EncCounter1 = NULL;
		S_EncCounter2 = NULL;
		S_EncCounter3 = NULL;
		S_EncCounter4 = NULL;
		S_RawKey1 = NULL;
		S_RawKey2 = NULL;
		S_RawKey3 = NULL;
		S_RawKey4 = NULL;
	}
}
static void ShowData(autoBlock_t *data, char *title)
{
	/*
	char *tmp;

	cout("CRPC_RSA_Aes_%s: [%s]\n", title, tmp = makeHexLine(data));
	memFree(tmp);
	*/
}
autoBlock_t *ClientCRPC(SockStream_t *ss, autoBlock_t *sendData) // ret: NULL ナシ, 通信失敗 -> 0 バイト
{
	autoBlock_t *recvData;

	errorCase(!ss);
	errorCase(!sendData);

	if (!S_EncCounter1) // ? ClientCRPC_Begin() 未実行・未成功
	{
		cout("C_CRPC_RSA_Aes_カウンターが無い！\n");
		return newBlock();
	}
	sendData = copyAutoBlock(sendData);

	PutHash(sendData);

ShowData(sendData, "sendData_P"); // test
	Crypt(sendData, S_RawKey1, S_EncCounter1);
	Crypt(sendData, S_RawKey2, S_EncCounter2);
	Crypt(sendData, S_RawKey3, S_EncCounter3);
	Crypt(sendData, S_RawKey4, S_EncCounter4);
ShowData(sendData, "sendData_C"); // test

	recvData = ClientRPC(ss, sendData);

	releaseAutoBlock(sendData);

ShowData(recvData, "recvData_C"); // test
	Crypt(recvData, S_RawKey1, S_EncCounter1);
	Crypt(recvData, S_RawKey2, S_EncCounter2);
	Crypt(recvData, S_RawKey3, S_EncCounter3);
	Crypt(recvData, S_RawKey4, S_EncCounter4);
ShowData(recvData, "recvData_P"); // test

	if (!UnputHash(recvData))
	{
		cout("C_CRPC_RSA_Aes_受信データ破損\n");
		setSize(recvData, 0);
	}
	return recvData;
}
