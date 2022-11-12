/*
	CRPC_GetRawKey()
		�߂�l
			16�o�C�g�̌�
			�Ăяo�����ŊJ������B

	CRPC_Init()
		������

	ServiceCRPC()
		����
			recvData
				��M�f�[�^
				�ύX���Ă��ǂ��B
				�Ăяo�����ŊJ������B
				��ɔ�NULL

			phase
				1, 2, 3...

		�߂�l
			���M�f�[�^
			NULL�̏ꍇ�A�����ɐؒf����B
			�Ăяo�����ŊJ������B

		���M�f�[�^�𑗐M���I������ؒf����ꍇ -> RPC_SendCompleteAndDisconnect()

	�ȉ��� RPC.c ���Q��
		RPC_RecvSizeMax
		RPC_ConnectTimeoutSec
		RPC_RecvTimeoutSec
		RPC_SendTimeoutSec

	�ȉ��� Service.c ���Q��
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

	if (!phase) // ����
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
	else // �Q��ڈȍ~
	{
		autoBlock_t *sendData;

		encCounter = readBinary(COUNTER_FILE);
		rawKey = readBinary(RAWKEY_FILE);

		Crypt(recvData, rawKey, encCounter);
		errorCase_m(!UnputHash(recvData), "CRPC_Aes_��M�f�[�^�j��");

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
