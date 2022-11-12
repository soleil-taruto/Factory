/*
	boomClient.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... ��~����B

	boomClient.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/SS SEND-SIZE-MAX]
	                                           [/RS RECV-SIZE-MAX] [/T SOCK-TIMEOUT]
	                                           [/NCT NO-CONNECT-TIMEOUT] [/B]

		CONNECT-MAX   ... �ő哯���ڑ���, �ȗ����� 1000
		SEND-SIZE-MAX ... �ő呗�M�f�[�^�T�C�Y, [�o�C�g] �ŏ�21
		RECV-SIZE-MAX ... �ő��M�f�[�^�T�C�Y, [�o�C�g] �ŏ�21
		SOCK-TIMEOUT  ... �P��̑���M�̃^�C���A�E�g, [�b]
		NO-CONNECT-TIMEOUT ... ���ڑ��^�C���A�E�g, [�b]
		/B ... �T�[�o�[�ւ̓����ڑ����ő�P�ɂ���, �ȗ����� CONNECT-MAX �Ɉˑ�

		���ڑ����Ƃ̃^�C���A�E�g�͖����B
*/

#include "Boomerang.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

static uint SendSizeMax = 60;
static uint RecvSizeMax = 2500;
static uint SockTimeoutSec = 20;
static uint NoConnectTimeoutSec = 180;
static int OneBoomerangMode;

thread_tls static autoBlock_t *RecvCredential;
thread_tls static int RecvFlag;

static critical_t BoomCrit;

static autoBlock_t *Boomerang(autoBlock_t *credential, int flag, autoBlock_t *sendData)
{
	static uchar ip[4];
	int sock;
	SockStream_t *ss;
	autoBlock_t *recvData = NULL;
	uint crc16 = boom_GetCRC16(credential, flag, sendData);
	uint r_crc16;

	cout("CRC16=%04x\n", crc16);

	if (!RecvCredential)
		RecvCredential = nobCreateBlock(CREDENTIAL_SIZE);

	if (OneBoomerangMode)
	{
		inner_uncritical();
		{
			enterCritical(&BoomCrit); // ������Boom_�ǂ������灚����
		}
		inner_critical();
	}
	sock = sockConnect(ip, FwdHost, FwdPortNo);

	if (sock == -1)
	{
		LOGPOS();
		goto endBoom;
	}
	ss = CreateSockStream(sock, SockTimeoutSec);

	{
		uchar block[4];
		uint recvSize;

		valueToBlock(block, HEADER_SIZE + getSize(sendData));

		SockSendBlock(ss, block, 4);
		SockSendBlock(ss, directGetBuffer(credential), CREDENTIAL_SIZE);
		SockSendChar(ss, flag);
		SockSendChar(ss, 0x00); // Reserved
		SockSendChar(ss, crc16 & 0xff);
		SockSendChar(ss, crc16 >> 8 & 0xff);
		SockSendBlock(ss, directGetBuffer(sendData), getSize(sendData));
		SockFlush(ss);
		SockRecvBlock(ss, block, 4);

		recvSize = blockToValue(block);

		if (!m_isRange(recvSize, HEADER_SIZE, RecvSizeMax))
		{
			cout("OutOfRange_recvSize: %u (RecvSizeMax: %u) SS_EOF: %d\n", recvSize, RecvSizeMax, IsEOFSockStream(ss));
			goto disconnect;
		}
		SockRecvBlock(ss, directGetBuffer(RecvCredential), CREDENTIAL_SIZE);
		RecvFlag = SockRecvChar(ss);
		SockRecvChar(ss); // Reserved
		r_crc16 = SockRecvChar(ss);
		r_crc16 |= SockRecvChar(ss) << 8;

		recvSize -= HEADER_SIZE;
		recvData = nobCreateBlock(recvSize);

		if (!SockRecvBlock(ss, directGetBuffer(recvData), recvSize)) // ? �ǂݍ��݃G���[
		{
			LOGPOS();
			releaseAutoBlock(recvData);
			recvData = NULL;
			goto disconnect;
		}
		crc16 = boom_GetCRC16(RecvCredential, RecvFlag, recvData);

		cout("CRC16=%04x, R_CRC16=%04x\n", crc16, r_crc16);

		if (crc16 != r_crc16)
		{
			LOGPOS();
			releaseAutoBlock(recvData);
			recvData = NULL;
			goto disconnect;
		}
	}

disconnect:
	ReleaseSockStream(ss);
	sockDisconnect(sock);

endBoom:
	if (OneBoomerangMode)
		leaveCritical(&BoomCrit); // ������Boom_�ǂ����܂Ł�����

	return recvData;
}
static void PerformTh(int sock, char *strip)
{
	Comm_t *i = CreateComm(sock);
	autoBlock_t *credential = makeCryptoRandBlock(CREDENTIAL_SIZE);
	int foregroundFlag = 1;
	uint waitMillis = 0;

	for (; ; )
	{
		autoBlock_t *sendData = GetCommRecvData(i, SendSizeMax);
		autoBlock_t *recvData;
		int flag;
		uint nConTmout = now() + NoConnectTimeoutSec;

		if (IsCommDeadAndEmpty(i) && !getSize(sendData))
			flag = 'D'; // Disconnect
		else if (foregroundFlag)
			flag = 'F'; // Foreground
		else
			flag = 'B'; // Background

		foregroundFlag ^= 1;

		cout("Loop_1:%08x\n", *(uint *)directGetBuffer(credential));

		for (; ; )
		{
			recvData = Boomerang(credential, flag, sendData);

			if (recvData)
			{
				if (RecvFlag != 'E') // ! Error
					break;

				releaseAutoBlock(recvData);
			}
			if (ProcDeadFlag || nConTmout < now())
			{
				cout("���ڑ��^�C���A�E�g���̓v���Z�X�I���ɂ��ؒf\n");
				releaseAutoBlock(sendData);
				goto disconnect;
			}

			inner_uncritical();
			{
				sleep(2000); // �ʐM���s�ɂ��҂�
			}
			inner_critical();
		}

		cout("Loop_2:%08x\n", *(uint *)directGetBuffer(credential));

		for (; ; )
		{
			if (ProcDeadFlag || RecvFlag == 'D') // Disconnect
				break;

			if (AddCommSendData(i, recvData, 0))
				break;

			inner_uncritical();
			{
				sleep(2000); // �ǉ����s�ɂ��҂�
			}
			inner_critical();
		}

		if (getSize(sendData) || getSize(recvData))
			waitMillis = 0;
		else
			waitMillis = m_min(waitMillis + 100, 2000);

		releaseAutoBlock(sendData);
		releaseAutoBlock(recvData);

		if (ProcDeadFlag || RecvFlag == 'D') // Disconnect
			break;

		cout("waitMillis: %u\n", waitMillis);

		inner_uncritical();
		{
			sleep(waitMillis); // �ʐM�����ɂ��҂�
		}
		inner_critical();
	}

disconnect:
	ReleaseComm(i);
	releaseAutoBlock(credential);

	if (RecvCredential)
		releaseAutoBlock(RecvCredential);
}
static int ReadArgs(void)
{
	if (argIs("/SS"))
	{
		SendSizeMax = toValue(nextArg());
		return 1;
	}
	if (argIs("/RS"))
	{
		RecvSizeMax = toValue(nextArg());
		return 1;
	}
	if (argIs("/T"))
	{
		SockTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/NCT"))
	{
		NoConnectTimeoutSec = toValue(nextArg());
		return 1;
	}
	if (argIs("/B"))
	{
		OneBoomerangMode = 1;
		return 1;
	}

	errorCase(!m_isRange(SendSizeMax, HEADER_SIZE + 1, UINTMAX));
	errorCase(!m_isRange(RecvSizeMax, HEADER_SIZE + 1, UINTMAX));
	errorCase(!m_isRange(SockTimeoutSec, 1, 86400));
	errorCase(!m_isRange(NoConnectTimeoutSec, 1, 86400));
	// OneBoomerangMode

	SendSizeMax -= HEADER_SIZE; // to �f�[�^�T�C�Y

	return 0;
}
static char *GetTitleSuffix(void)
{
	return xcout("S:%u,R:%u T:%u,NCT:%u", SendSizeMax, RecvSizeMax, SockTimeoutSec, NoConnectTimeoutSec);
}
int main(int argc, char **argv)
{
	initCritical(&BoomCrit);

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "boomClient", GetTitleSuffix);
	TunnelPerformTh = NULL;

	fnlzCritical(&BoomCrit);
}
