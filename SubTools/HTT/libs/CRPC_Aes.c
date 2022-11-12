/*
	CRPC_GetRawKey()
		戻り値
			16バイトの鍵
			呼び出し側で開放する。

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

#include "CRPC_Aes.h"

#define COUNTER_FILE "Counter.dat"
#define RAWKEY_FILE "RawKey.dat"
#define MAIN_DIR "a"

autoBlock_t *ServiceRPC(autoBlock_t *recvData, uint phase)
{
	autoBlock_t *encCounter;
	autoBlock_t *rawKey;

	if (!phase) // 初回
	{
		autoBlock_t *clSeed = recvData;
		autoBlock_t *svSeed = MakeEncCounterSeed();

		errorCase(getSize(clSeed) != 16);
		errorCase(getSize(svSeed) != 16);

		encCounter = MakeEncCounter(clSeed, svSeed);
		rawKey = CRPC_GetRawKey();

		errorCase(!rawKey);
		errorCase(getSize(rawKey) != 16);

		writeBinary(COUNTER_FILE, encCounter);
		writeBinary(RAWKEY_FILE, rawKey);
		createDir(MAIN_DIR);

		addCwd(MAIN_DIR);
		CRPC_Init();
		unaddCwd();

//		releaseAutoBlock(clSeed); // == recvData
//		releaseAutoBlock(svSeed); // -> ret
		releaseAutoBlock(encCounter);
		releaseAutoBlock(rawKey);
		return svSeed;
	}
	else // ２回目以降
	{
		autoBlock_t *sendData;

		encCounter = readBinary(COUNTER_FILE);
		rawKey = readBinary(RAWKEY_FILE);

		Crypt(recvData, rawKey, encCounter);
		errorCase_m(!UnputHash(recvData), "CRPC_Aes_受信データ破損");

		addCwd(MAIN_DIR);
		sendData = ServiceCRPC(recvData, phase);
		unaddCwd();

		errorCase_m(!sendData, "CRPC_Aes_ServiceCRPC_Fault");

		PutHash(sendData);
		Crypt(sendData, rawKey, encCounter);

		writeBinary(COUNTER_FILE, encCounter);

		releaseAutoBlock(encCounter);
		releaseAutoBlock(rawKey);

		return sendData;
	}
}
