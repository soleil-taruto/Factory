/*
	CRPC_Init()
		初期化

	ServiceCRPC()
		引数
			recvData
				受信データ
				変更しても良い。
				呼び出し側で開放する。
				常に非NULL

			phase
				1, 2, 3...

		戻り値
			送信データ
			NULLの場合、直ちに切断する。
			呼び出し側で開放する。

		送信データを送信し終えたら切断する場合 -> RPC_SendCompleteAndDisconnect()

	以下は RPC.c を参照
		RPC_RecvSizeMax
		RPC_ConnectTimeoutSec
		RPC_RecvTimeoutSec
		RPC_SendTimeoutSec

	以下は Service.c を参照
		HttIPFile
		HttRecvFile
		HttSendFile
		HttTightFile
		HttIP
		BootTime
		ConnectTime
		LastRecvTime
		LastSendTime
		ConnectElapseTime
		LastRecvElapseTime
		LastSendElapseTime
*/

#include "CRPC_RSA_Aes.h"

#define COUNTER_1_FILE "Counter_1.dat"
#define COUNTER_2_FILE "Counter_2.dat"
#define COUNTER_3_FILE "Counter_3.dat"
#define COUNTER_4_FILE "Counter_4.dat"
#define RAWKEY_1_FILE "RawKey_1.dat"
#define RAWKEY_2_FILE "RawKey_2.dat"
#define RAWKEY_3_FILE "RawKey_3.dat"
#define RAWKEY_4_FILE "RawKey_4.dat"
#define MAIN_DIR "a"

autoBlock_t *ServiceRPC(autoBlock_t *recvData, uint phase)
{
	autoBlock_t *encCounter1;
	autoBlock_t *encCounter2;
	autoBlock_t *encCounter3;
	autoBlock_t *encCounter4;
	autoBlock_t *rawKey1;
	autoBlock_t *rawKey2;
	autoBlock_t *rawKey3;
	autoBlock_t *rawKey4;

	if (!phase) // 初回
	{
		autoBlock_t *publicKey = recvData;
		autoBlock_t *seed = makeCryptoRandBlock(128);
		autoBlock_t *cipherSeed;
		autoBlock_t gab;

		writeBinary(COUNTER_1_FILE, gndSubBytesVar(seed,   0, 16, gab));
		writeBinary(COUNTER_2_FILE, gndSubBytesVar(seed,  16, 16, gab));
		writeBinary(COUNTER_3_FILE, gndSubBytesVar(seed,  32, 16, gab));
		writeBinary(COUNTER_4_FILE, gndSubBytesVar(seed,  48, 16, gab));
		writeBinary(RAWKEY_1_FILE,  gndSubBytesVar(seed,  64, 16, gab));
		writeBinary(RAWKEY_2_FILE,  gndSubBytesVar(seed,  80, 16, gab));
		writeBinary(RAWKEY_3_FILE,  gndSubBytesVar(seed,  96, 16, gab));
		writeBinary(RAWKEY_4_FILE,  gndSubBytesVar(seed, 112, 16, gab));
		createDir(MAIN_DIR);

		addCwd(MAIN_DIR);
		CRPC_Init();
		unaddCwd();

		cipherSeed = RSA_Encrypt(seed, publicKey);

//		releaseAutoBlock(publicKey); // == recvData
		releaseAutoBlock(seed);
//		releaseAutoBlock(cipherSeed); // -> ret

		return cipherSeed;
	}
	else // ２回目以降
	{
		autoBlock_t *sendData;

		encCounter1 = readBinary(COUNTER_1_FILE);
		encCounter2 = readBinary(COUNTER_2_FILE);
		encCounter3 = readBinary(COUNTER_3_FILE);
		encCounter4 = readBinary(COUNTER_4_FILE);
		rawKey1 = readBinary(RAWKEY_1_FILE);
		rawKey2 = readBinary(RAWKEY_2_FILE);
		rawKey3 = readBinary(RAWKEY_3_FILE);
		rawKey4 = readBinary(RAWKEY_4_FILE);

		Crypt(recvData, rawKey1, encCounter1);
		Crypt(recvData, rawKey2, encCounter2);
		Crypt(recvData, rawKey3, encCounter3);
		Crypt(recvData, rawKey4, encCounter4);

		errorCase_m(!UnputHash(recvData), "CRPC_RSA_Aes_受信データ破損");

		addCwd(MAIN_DIR);
		sendData = ServiceCRPC(recvData, phase);
		unaddCwd();

		errorCase_m(!sendData, "CRPC_RSA_Aes_ServiceCRPC_Fault");

		PutHash(sendData);

		Crypt(sendData, rawKey1, encCounter1);
		Crypt(sendData, rawKey2, encCounter2);
		Crypt(sendData, rawKey3, encCounter3);
		Crypt(sendData, rawKey4, encCounter4);

		writeBinary(COUNTER_1_FILE, encCounter1);
		writeBinary(COUNTER_2_FILE, encCounter2);
		writeBinary(COUNTER_3_FILE, encCounter3);
		writeBinary(COUNTER_4_FILE, encCounter4);

		releaseAutoBlock(encCounter1);
		releaseAutoBlock(encCounter2);
		releaseAutoBlock(encCounter3);
		releaseAutoBlock(encCounter4);
		releaseAutoBlock(rawKey1);
		releaseAutoBlock(rawKey2);
		releaseAutoBlock(rawKey3);
		releaseAutoBlock(rawKey4);

		return sendData;
	}
}
