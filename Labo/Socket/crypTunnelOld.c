/*
	crypTunnel -> crypTunnelOld �Ƀ��l�[�������B

	----
	crypTunnel.exe [/FD FWD-DOMAIN] [/FP FWD-PORT] [/P PORT] [/C CONNECT-MAX]
	               [/R] [/T SESISON-TIMEOUT] [/XT CONNECT-WAIT NEGOTIATE-TIMEOUT SUICIDE-TIMEOUT] /KB KEY-BUNDLE-FILE [/KW KEY-WIDTH]

		FWD-DOMAIN   == �]����z�X�g��      �f�t�H: localhost
		FWD-PORT     == �]����|�[�g�ԍ�    �f�t�H: 60000
		PORT         == �҂��󂯃|�[�g�ԍ�  �f�t�H: 59999
		CONNECT-MAX  == �ő哯���ڑ���      �f�t�H: 10
		/R           == ���o�[�X���[�h
		SESSION-TIMEOUT   == �Z�b�V�����^�C���A�E�g (�b) �f�t�H: 3600, 0: ������
		CONNECT-WAIT      == �ڑ�����Ă���ʐM���J�n����܂ł̑҂����� (�~���b) �f�t�H: 100
		NEGOTIATE-TIMEOUT == �ʐM�����^�C���A�E�g   (�b) �f�t�H: 15
		SUICIDE-TIMEOUT   == ���A��ɂ���܂ł̎��� (�b) �f�t�H: 3
		KEY-BUNDLE-FILE   == �����t�@�C�� ��1
		KEY-WIDTH         == ���� (16, 24, 32) �f�t�H: 32

		��1 * ���w�肷��Ǝ���� -> ���͂��ꂽ������� SHA-512 �����Ƃ���B(p1k�Ɠ���)

	�I������ɂ́A�G�X�P�[�v�L�[���������邩 crypTunnel.exe [/P PORT]? /S �����s����B

	�]����ɑ��M����f�[�^���Í������A�]���悩���M�����f�[�^�𕜍�����B
	���o�[�X���[�h�ł́A�]����ɑ��M����f�[�^�𕡍����A�]���悩���M�����f�[�^���Í�������B

	�N���C�A���gPC -> ���[�J���l�b�g -> crypTunnel -> �C���^�[�l�b�g -> crypTunnel /R -> ���[�J���l�b�g -> �T�[�o�[PC

	�Ƃ������Ɍq�����Ƃ�z�肷��B

	----
	RealVNC���g���ꍇ

	�N���C�A���g��
		crypTunnel.exe /FD �I /FP 59999 /P 5900 /KB �����t�@�C�� /KW ����

	�T�[�o�[��
		crypTunnel.exe /FP 5900 /R /KB �����t�@�C�� /KW ����

	����ŁA�T�[�o�[���� RealVNC �T�[�o�[�𓮂����A�N���C�A���g�� RealVNC ���� localhost �֐ڑ�����B

	----
	�������Z�L�����e�B��̖�聚����

	�T�[�o�[�����ŃJ�E���^�����߂Ă��邹���ŁA�ߋ��̃T�[�o�[�̑��M�f�[�^��^�����邾���ŁA���̎��ƑS����������������T�[�o�[�ɂȂ��B
	�J�E���^�𗼕��Ő������Č������� (���݂��Ɏ����Ƒ���̃J�E���^��) �g�ݍ��킹��悤�ɏC�����ׂ��B== CPortFwd, tunnel/crypTunnel �̎���

	��./tunnel/crypTunnel.exe ���g�p���ĉ������I
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "C:\Factory\Common\Options\SockServerTh.h"
#include "C:\Factory\Common\Options\RingCipher.h"

static char *FwdDomain = "localhost";
static uint FwdPort = 60000;

static uint SessionTimeout = 3600;
static uint ConnectWait = 100;
static uint NegotiateTimeout = 15;
static uint SuicideTimeout = 3;

static char *KeyBundleFile;
static uint KeyWidth = 32;
static autoBlock_t *KeyBundle;
static autoList_t *KeyTableList;

static int ReverseMode;
static int KeepTheServer = 1;
static critical_t CritConnectWait;
static uint StopServerEventHandle;

typedef struct ChannelInfo_st
{
	struct ChannelInfo_st *OtherSide;
	int RecvSock;
	int SendSock;
	uint64 Counter2[2]; // [0]�����􂳂Ȃ��B
	autoBlock_t *CarryRecvBuffer;
	int EncryptMode;
	uint Timeout;
	int Death;
}
ChannelInfo_t;

#define SEND_MAX (48 * 1024)

static void MakeCounter2(uint64 counter2[2])
{
	counter2[0] = (uint64)getCryptoRand() << 32 | getCryptoRand();
	counter2[1] = (uint64)getCryptoRand() << 32 | getCryptoRand();

	cout("MkC2: %020I64u, %020I64u\n", counter2[0], counter2[1]);
}
static void AddCounter2(autoBlock_t *block, uint64 counter2[2])
{
	ab_addValue(block, counter2[0] >>  0);
	ab_addValue(block, counter2[0] >> 32);
	ab_addValue(block, counter2[1] >>  0);
	ab_addValue(block, counter2[1] >> 32);

	cout("adC2: %020I64u, %020I64u\n", counter2[0], counter2[1]);
}
static void UnaddCounter2(autoBlock_t *block, uint64 counter2[2])
{
	counter2[1]  = (uint64)ab_unaddValue(block) << 32;
	counter2[1] |= (uint64)ab_unaddValue(block);
	counter2[0]  = (uint64)ab_unaddValue(block) << 32;
	counter2[0] |= (uint64)ab_unaddValue(block);

	cout("uaC2: %020I64u, %020I64u\n", counter2[0], counter2[1]);
}

static void ChannelTh(uint prm)
{
	ChannelInfo_t *i = (ChannelInfo_t *)prm;

	critical();
	{
		autoBlock_t *recvBuffer = newBlock();
		autoBlock_t *sendBuffer = newBlock();

		if (SessionTimeout)
		{
			i->Timeout = now() + SessionTimeout;
		}
		if (i->CarryRecvBuffer)
		{
			ab_addBytes(recvBuffer, i->CarryRecvBuffer);
			nobSetSize(i->CarryRecvBuffer, 0);
		}
		while (KeepTheServer)
		{
			if (getSize(recvBuffer) || getSize(sendBuffer))
				cout("[%c%c] %u -> %u R:%4u S:%4u\n"
					,i->EncryptMode ? 'E' : 'D'
					,ReverseMode ? 'S' : 'C'
					,i->RecvSock
					,i->SendSock
					,getSize(recvBuffer)
					,getSize(sendBuffer)
					);

			if (i->Timeout && i->Timeout < now())
			{
				cout("�Z�b�V�����^�C���A�E�g\n");
				break;
			}
			if (getSize(sendBuffer))
			{
				if (SockSendSequ(i->SendSock, sendBuffer, 1000) == -1)
				{
					break;
				}
				continue;
			}
			if (i->EncryptMode)
			{
				if (getSize(recvBuffer))
				{
					uint sendSize = m_min(getSize(recvBuffer), SEND_MAX);

					ab_addSubBytes(sendBuffer, recvBuffer, 0, sendSize);
					removeBytes(recvBuffer, 0, sendSize);

					i->Counter2[0]++;
					rcphrEncryptorBlock(sendBuffer, KeyTableList, i->Counter2);

					errorCase(0xffff < getSize(sendBuffer));

					sendSize = getSize(sendBuffer);
					insertByteRepeat(sendBuffer, 0, 0x00, 2);
					setByte(sendBuffer, 0, sendSize >> 0 & 0xff);
					setByte(sendBuffer, 1, sendSize >> 8 & 0xff);

					continue;
				}
			}
			else // ? Decrypt mode
			{
				if (2 <= getSize(recvBuffer))
				{
					uint recvSize = getByte(recvBuffer, 1) << 8 | getByte(recvBuffer, 0);

					if (recvSize <= getSize(recvBuffer) - 2)
					{
						uint64 rc2[2];

						removeBytes(recvBuffer, 0, 2);
						ab_addSubBytes(sendBuffer, recvBuffer, 0, recvSize);
						removeBytes(recvBuffer, 0, recvSize);

						i->Counter2[0]++;

						if (!rcphrDecryptorBlock(sendBuffer, KeyTableList, rc2))
						{
							cout("�����G���[(�����f�[�^�̃n�b�V�����s��v)\n");
							break;
						}
						if (rc2[0] != i->Counter2[0] || rc2[1] != i->Counter2[1])
						{
							cout("�����G���[(�����f�[�^�̃J�E���^���s��v)\n");
							break;
						}
						continue;
					}
				}
			}
			if (SockRecvSequ(i->RecvSock, recvBuffer, 1000) == -1)
			{
				break;
			}
			if (getSize(recvBuffer) == 0 && i->Death)
			{
				break;
			}
		}
		releaseAutoBlock(recvBuffer);
		releaseAutoBlock(sendBuffer);

		i->OtherSide->Timeout = now() + SuicideTimeout;
		i->OtherSide->Death = 1;
	}
	uncritical();
}
static void PerformTh(int sock, char *strip)
{
	uchar ip[4];
	int fwdSock = -1;
	ChannelInfo_t channels[2];
	ChannelInfo_t *encChannel;
	ChannelInfo_t *decChannel;
	autoBlock_t *rippedBuffer;
	uint thhdls[2];
LOGPOS(); // test

	*(uint *)ip = 0;
LOGPOS(); // test

	inner_uncritical();
	{
		enterCritical(&CritConnectWait);
		{
			sleep(ConnectWait);
		}
		leaveCritical(&CritConnectWait);
	}
	inner_critical();
LOGPOS(); // test

	if (ReverseMode)
	{
LOGPOS(); // test
		fwdSock = -1; // �_�~�[
		cout("�ڑ�: %d -> ?\n", sock);
	}
	else
	{
LOGPOS(); // test
		fwdSock = sockConnect(ip, FwdDomain, FwdPort);
		cout("�ڑ�: %d -> %d\n", sock, fwdSock);

		if (fwdSock == -1)
			goto endfunc;
	}
LOGPOS(); // test
	memset(channels, 0x00, sizeof(channels));
LOGPOS(); // test

	channels[0].OtherSide = channels + 1;
	channels[0].RecvSock = sock;
	channels[0].SendSock = fwdSock;
LOGPOS(); // test

	channels[1].OtherSide = channels + 0;
	channels[1].RecvSock = fwdSock;
	channels[1].SendSock = sock;
LOGPOS(); // test

	// negotiate >
	{
		SockStream_t *ss = CreateSockStream(ReverseMode ? sock : fwdSock, NegotiateTimeout);
		autoBlock_t *block = createBlock(48);
		uint64 ec2[2];
		uint64 dc2[2];
		uint64 wc2[2];
		int fault;

		if (ReverseMode)
		{
			encChannel = channels + 1;
			decChannel = channels + 0;

			MakeCounter2(encChannel->Counter2);
			MakeCounter2(decChannel->Counter2);
			MakeCounter2(wc2);

			AddCounter2(block, encChannel->Counter2);
			AddCounter2(block, decChannel->Counter2);
			AddCounter2(block, wc2);

			encChannel->Counter2[0]++;
			decChannel->Counter2[0]++;

			rcphrEncryptor(block, KeyTableList, 1);

			SockSendBlock(ss, directGetBuffer(block), 48);
			SockFlush(ss);
			SockRecvBlock(ss, directGetBuffer(block), 48);

			rcphrEncryptor(block, KeyTableList, 0);

			UnaddCounter2(block, wc2);
			UnaddCounter2(block, ec2);
			UnaddCounter2(block, dc2);

			cout("ec21: %020I64u, %020I64u\n", ec2[0], encChannel->Counter2[0]);
			cout("ec22: %020I64u, %020I64u\n", ec2[1], encChannel->Counter2[1]);
			cout("dc21: %020I64u, %020I64u\n", dc2[0], decChannel->Counter2[0]);
			cout("dc22: %020I64u, %020I64u\n", dc2[1], decChannel->Counter2[1]);

			fault =
				ec2[0] != encChannel->Counter2[0] ||
				ec2[1] != encChannel->Counter2[1] ||
				dc2[0] != decChannel->Counter2[0] ||
				dc2[1] != decChannel->Counter2[1];
		}
		else
		{
			encChannel = channels + 0;
			decChannel = channels + 1;

			nobSetSize(block, 48);
			SockRecvBlock(ss, directGetBuffer(block), 48);
			rcphrEncryptor(block, KeyTableList, 0);

			UnaddCounter2(block, wc2);
			UnaddCounter2(block, ec2);
			UnaddCounter2(block, dc2);

			ec2[0]++;
			dc2[0]++;

			encChannel->Counter2[0] = ec2[0];
			encChannel->Counter2[1] = ec2[1];
			decChannel->Counter2[0] = dc2[0];
			decChannel->Counter2[1] = dc2[1];

			MakeCounter2(wc2);

			AddCounter2(block, ec2);
			AddCounter2(block, dc2);
			AddCounter2(block, wc2);

			rcphrEncryptor(block, KeyTableList, 1);
			SockSendBlock(ss, directGetBuffer(block), 48);
			SockFlush(ss);

			fault = 0;
		}
		releaseAutoBlock(block);
		rippedBuffer = SockRipRecvBuffer(ss);
		ReleaseSockStream(ss);

		if (fault)
		{
			cout("�s���Ȑڑ�(���̕s��v)\n");
			goto endnegotiated;
		}
	}
	// < negotiate

	if (ReverseMode)
	{
		fwdSock = sockConnect(ip, FwdDomain, FwdPort);
		cout("�ڑ�: %d -> %d\n", sock, fwdSock);

		if (fwdSock == -1)
			goto endnegotiated;

		channels[0].SendSock = fwdSock;
		channels[1].RecvSock = fwdSock;
	}
	decChannel->CarryRecvBuffer = rippedBuffer;
	encChannel->EncryptMode = 1;

	thhdls[0] = runThread(ChannelTh, (uint)encChannel);
	thhdls[1] = runThread(ChannelTh, (uint)decChannel);

	inner_uncritical();
	{
		waitThread(thhdls[0]);
		waitThread(thhdls[1]);
	}
	inner_critical();

endnegotiated:
	if (fwdSock != -1)
		sockDisconnect(fwdSock);

	releaseAutoBlock(rippedBuffer);

endfunc:
	cout("�ؒf: %d -> %d\n", sock, fwdSock);
}

static int IdleTh(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			KeepTheServer = 0;
		}
	}
	if (collectEvents(StopServerEventHandle, 0))
	{
		KeepTheServer = 0;
	}
	return KeepTheServer;
}
int main(int argc, char **argv)
{
	uint portno = 59999;
	uint connectmax = 10;
	char *ssen;

	initCritical(&CritConnectWait);

readArgs:
	// �]����
	if (argIs("/FD")) // Forward Domain
	{
		FwdDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/FP")) // Forward Port
	{
		FwdPort = toValue(nextArg());
		goto readArgs;
	}

	// �҂���
	if (argIs("/P")) // Port
	{
		portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/C")) // Connect max
	{
		connectmax = toValue(nextArg());
		goto readArgs;
	}

	if (argIs("/R")) // Reverse mode
	{
		ReverseMode = 1;
		goto readArgs;
	}
	if (argIs("/T")) // session Timeout
	{
		SessionTimeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/XT")) // eXtra Timeout
	{
		ConnectWait = toValue(nextArg());
		NegotiateTimeout = toValue(nextArg());
		SuicideTimeout = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/KB")) // Key Bundle file
	{
		KeyBundleFile = nextArg();
		goto readArgs;
	}
	if (argIs("/KW")) // Key Width
	{
		KeyWidth = toValue(nextArg());
		goto readArgs;
	}

	ssen = xcout("cerulean.charlotte Factory crypTunnel stop server event object %u", portno);

	if (argIs("/S"))
	{
		eventWakeup(ssen);
		return;
	}

	errorCase(!KeyBundleFile);

	KeyBundle = cphrLoadKeyBundleFileEx(KeyBundleFile);
	KeyTableList = cphrCreateKeyTableList(KeyBundle, KeyWidth);
	StopServerEventHandle = eventOpen(ssen);

	cmdTitle_x(xcout("crypTunnel - %u to %s:%u (connectmax:%u)", portno, FwdDomain, FwdPort, connectmax));

	cout("�J�n\n");
	SockStartup();
LOGPOS(); // test
	sockServerTh(PerformTh, portno, connectmax, IdleTh);
LOGPOS(); // test
	SockCleanup();
	cout("�I��\n");

	releaseAutoBlock(KeyBundle);
	cphrReleaseKeyTableList(KeyTableList);
	handleClose(StopServerEventHandle);

	fnlzCritical(&CritConnectWait);
}
