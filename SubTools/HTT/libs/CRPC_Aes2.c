/*
	CRPC_GetRawKey()
		�߂�l
			64�o�C�g�̌�
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
				0, 1, 2...

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

#include "CRPC_Aes2.h"

#define RAWKEY_1_FILE "RawKey_1.dat"
#define RAWKEY_2_FILE "RawKey_2.dat"
#define RAWKEY_3_FILE "RawKey_3.dat"
#define RAWKEY_4_FILE "RawKey_4.dat"
#define MAIN_DIR "a"

autoBlock_t *ServiceRPC(autoBlock_t *recvData, uint phase)
{
	autoBlock_t *rawKey1;
	autoBlock_t *rawKey2;
	autoBlock_t *rawKey3;
	autoBlock_t *rawKey4;
	autoBlock_t *sendData;

	if (!phase) // ����
	{
		autoBlock_t *rawKey = CRPC_GetRawKey();

		errorCase(!rawKey);
		errorCase(getSize(rawKey) != 64);

		rawKey1 = ab_makeSubBytes(rawKey,  0, 16);
		rawKey2 = ab_makeSubBytes(rawKey, 16, 16);
		rawKey3 = ab_makeSubBytes(rawKey, 32, 16);
		rawKey4 = ab_makeSubBytes(rawKey, 48, 16);

		releaseAutoBlock(rawKey);

		writeBinary(RAWKEY_1_FILE, rawKey1);
		writeBinary(RAWKEY_2_FILE, rawKey2);
		writeBinary(RAWKEY_3_FILE, rawKey3);
		writeBinary(RAWKEY_4_FILE, rawKey4);
		createDir(MAIN_DIR);

		addCwd(MAIN_DIR);
		CRPC_Init();
		unaddCwd();
	}
	else
	{
		rawKey1 = readBinary(RAWKEY_1_FILE);
		rawKey2 = readBinary(RAWKEY_2_FILE);
		rawKey3 = readBinary(RAWKEY_3_FILE);
		rawKey4 = readBinary(RAWKEY_4_FILE);
	}

	errorCase_m(
		!RCBCDecrypt(recvData, rawKey4) ||
		!RCBCDecrypt(recvData, rawKey3) ||
		!RCBCDecrypt(recvData, rawKey2) ||
		!RCBCDecrypt(recvData, rawKey1) ||
		!UnputHash(recvData) ||
		!Unadd_RandPart_Padding(recvData),
		"CRPC_Aes2_��M�f�[�^�j��"
		);

	addCwd(MAIN_DIR);
	sendData = ServiceCRPC(recvData, phase);
	unaddCwd();

	errorCase_m(!sendData, "CRPC_Aes2_ServiceCRPC_Fault");

	Add_RandPart_Padding(sendData);
	PutHash(sendData);
	RCBCEncrypt(sendData, rawKey1);
	RCBCEncrypt(sendData, rawKey2);
	RCBCEncrypt(sendData, rawKey3);
	RCBCEncrypt(sendData, rawKey4);

	releaseAutoBlock(rawKey1);
	releaseAutoBlock(rawKey2);
	releaseAutoBlock(rawKey3);
	releaseAutoBlock(rawKey4);

	return sendData;
}
