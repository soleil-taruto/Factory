/*
	ServiceRPC()
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
			NULL�̏ꍇ�u�ؒf�v����B
			�Ăяo�����ŊJ������B

		����̑��M�f�[�^�𑗐M���I������ؒf����ꍇ�A���̊֐����� RPC_SendCompleteAndDisconnect() ���ĂԂ��ƁB

	RPC_RecvSizeMax
		���̎�M�f�[�^�̃T�C�Y�̏��_[�o�C�g]
		0 �̂Ƃ��A���݂̑��M�f�[�^�𑗐M���I������ؒf����B
		�����l�� 1 MB
		ServiceRPC()���s���ɐݒ�E�ύX���邱�ƁB

	RPC_ConnectTimeoutSec
		�ڑ����Ă���̃^�C���A�E�g_[�b]
		�����l�� 120 �b
		ServiceRPC()���s���ɐݒ�E�ύX���邱�ƁB

	RPC_RecvTimeoutSec
		����M�^�C���A�E�g_[�b]
		�����l�� 5 �b
		ServiceRPC()���s���ɐݒ�E�ύX���邱�ƁB
		��1

	RPC_SendTimeoutSec
		�����M�^�C���A�E�g_[�b]
		�����l�� 5 �b
		ServiceRPC()���s���ɐݒ�E�ύX���邱�ƁB
		��1

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

	��1
		����M�^�C���A�E�g�E�����M�^�C���A�E�g�A���������Ń^�C���A�E�g�E�G���[�E�ؒf����B
		ServiceRPC(); ���Ɏ��Ԃ��|�����Ă��A����̑��M�f�[�^�̏����o���Ŗ����M���Ԃ����Z�b�g�����B
		-> ��M�J�n�`�I���Ŗ��ʐM�^�C���A�E�g�E���M�J�n�`�I���Ŗ��ʐM�^�C���A�E�g�A�ǂ��炩�Ń^�C���A�E�g�E�G���[�E�ؒf����B
*/

#include "sRPC.h"

#define PHASE_FILE "Phase.dat"
#define RECVSIZEMAX_FILE "RecvSizeMax.dat"
#define CONNECTTIMEOUT_FILE "ConnectTimeout.dat"
#define RECVTIMEOUT_FILE "RecvTimeout.dat"
#define SENDTIMEOUT_FILE "SendTimeout.dat"
#define MAIN_DIR "a"

uint RPC_RecvSizeMax;
uint RPC_ConnectTimeoutSec;
uint RPC_RecvTimeoutSec;
uint RPC_SendTimeoutSec;

void RPC_SendCompleteAndDisconnect(void) // ���M�f�[�^�𑗐M���I������ؒf����B
{
	RPC_RecvSizeMax = 0;
}

static uint ReadValueFile(char *file)
{
	char *line = readFirstLine(file);
	uint ret;

	ret = toValue(line);
	memFree(line);
	return ret;
}
static void WriteValueFile(char *file, uint value)
{
	char *line = xcout("%u", value);

	writeOneLine(file, line);
	memFree(line);
}

static void DeleteServiceNameFromHttRecvFile(void)
{
	FILE *fp = fileOpen(HttRecvFile, "rb");
	uint count;

	cout("DEL_SERVICE_NAME:[");

	for (count = 1; ; count++)
	{
		int chr = readChar(fp);

		errorCase(chr == EOF);

		if (!m_isRange(chr, 0x21, 0x7e)) // ? not ASCII
			break;

		cout("%c", chr);
	}
	cout("]\n");
	fileClose(fp);
	DeleteFileDataPart(HttRecvFile, 0, count);
}
static autoBlock_t *ReadRecvData(void)
{
	uint recvFileSize = (uint)getFileSize(HttRecvFile);
	FILE *fp;
	uint recvSize;
	autoBlock_t *recvData;

	if (recvFileSize < 4)
		return NULL;

	fp = fileOpen(HttRecvFile, "rb");
	recvSize = readValue(fp);
	cout("RPC_RECVSIZE: %u (MAX: %u)\n", recvSize, RPC_RecvSizeMax);
	errorCase_m(RPC_RecvSizeMax < recvSize, "RPC_RECVSIZE_OVERFLOW");

	if (recvFileSize - 4 < recvSize)
	{
		fileClose(fp);
		return NULL;
	}
	recvData = readBinaryStream(fp, recvSize);
	errorCase(!recvData);
	fileClose(fp);
	createFile(HttRecvFile); // �N���C�A���g���Ŏ�M��҂������̑��M�������ꍇ�A�����ō폜�����I
	return recvData;
}
static void WriteSendData(autoBlock_t *sendData)
{
	FILE *fp = fileOpen(HttSendFile, "ab"); // �O��̑��M�f�[�^���c���Ă���ꍇ������ -> �ǋL

	writeValue(fp, getSize(sendData));
	writeBinaryBlock(fp, sendData);

	fileClose(fp);
}

void ServiceMain(void)
{
	int inited = existFile(PHASE_FILE);
	uint phase;

	if (inited)
	{
		phase = ReadValueFile(PHASE_FILE);
		RPC_RecvSizeMax = ReadValueFile(RECVSIZEMAX_FILE);
		RPC_ConnectTimeoutSec = ReadValueFile(CONNECTTIMEOUT_FILE);
		RPC_RecvTimeoutSec = ReadValueFile(RECVTIMEOUT_FILE);
		RPC_SendTimeoutSec = ReadValueFile(SENDTIMEOUT_FILE);
	}
	else
	{
		phase = 0;
		RPC_RecvSizeMax = 1000000;
		RPC_ConnectTimeoutSec = 120;
		RPC_RecvTimeoutSec = 5;
		RPC_SendTimeoutSec = 5;

		DeleteServiceNameFromHttRecvFile();
		createDir(MAIN_DIR);
	}

	errorCase_m(RPC_ConnectTimeoutSec < ConnectElapseTime, "RPC_CONNECT_TIMEOUT");

#if 1
	{
		int rto = RPC_RecvTimeoutSec < LastRecvElapseTime;
		int sto = RPC_SendTimeoutSec < LastSendElapseTime;

		if (rto) cout("RPC_RECV_TIMEOUT ���Е������Ȃ�^�C���A�E�g�ɂȂ�Ȃ��I\n");
		if (sto) cout("RPC_SEND_TIMEOUT ���Е������Ȃ�^�C���A�E�g�ɂȂ�Ȃ��I\n");

		errorCase_m(rto && sto, "RPC_IO_TIMEOUT");
	}
#else // old
	errorCase_m(RPC_RecvTimeoutSec < LastRecvElapseTime, "RPC_RECV_TIMEOUT");
	errorCase_m(RPC_SendTimeoutSec < LastSendElapseTime, "RPC_SEND_TIMEOUT");
#endif

	if (!RPC_RecvSizeMax) // ? ���M���I������ؒf����B
	{
		if (getFileSize(HttSendFile) == 0) // ? ���M���I�����B
		{
			cout("RECV_SIZE_0_SendEnd_DISCONNECT_OK\n");
			ServiceDisconnect();
		}
		createFile(HttRecvFile); // ������M���Ȃ��B
	}
	else // ����M���C��
	{
		autoBlock_t *recvData = ReadRecvData();
		autoBlock_t *sendData;

		if (!recvData)
			goto noRecvData;

		addCwd(MAIN_DIR);
		sendData = ServiceRPC(recvData, phase);
		unaddCwd();

		if (!sendData)
			ServiceDisconnect();

		WriteSendData(sendData);

		releaseAutoBlock(recvData);
		releaseAutoBlock(sendData);

		phase++;
	}

noRecvData:
	WriteValueFile(PHASE_FILE, phase);
	WriteValueFile(RECVSIZEMAX_FILE, RPC_RecvSizeMax);
	WriteValueFile(CONNECTTIMEOUT_FILE, RPC_ConnectTimeoutSec);
	WriteValueFile(RECVTIMEOUT_FILE, RPC_RecvTimeoutSec);
	WriteValueFile(SENDTIMEOUT_FILE, RPC_SendTimeoutSec);
}
void ServicePeriod(void)
{
	// noop
}
