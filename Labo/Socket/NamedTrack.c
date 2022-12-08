/*
	�N���C�A���g���[�h
		NamedTrack.exe [/FD FORWARD-DOMAIN] [/FP FORWARD-PORT] [/N TRACK-NAME] [/P PORT] [/C CONNECT-MAX]

	�T�[�o�[���[�h
		NamedTrack.exe /S [/F FORWARD-DOMAIN FORWARD-PORT TRACK-NAME]* [/P PORT] [/C CONNECT-MAX]

	�t�����[�h
		NamedTrack.exe /R [/FD FORWARD-DOMAIN] [/FP FORWARD-PORT] [/D POLLING-DOMAIN] [/P POLLING-PORT] [/N TRACK-NAME]

		FORWARD-DOMAIN  �]����z�X�g��      �f�t�H: "localhost"
		FORWARD-PORT    �]����|�[�g�ԍ�    �f�t�H: 60000
		TRACK-NAME      ���̃g���b�N��      �f�t�H: "" (���݂� "" �ł� '\0' �͑�����̂ŁA�����̃g���l���ɂȂ�킯�ł͂Ȃ�)
		PORT            �҂��󂯃|�[�g�ԍ�  �f�t�H: 56789
		CONNECT-MAX     �ő�ڑ���          �f�t�H: 10
		POLLING-DOMAIN  �]�����z�X�g��      �f�t�H: "localhost"
		POLLING-PORT    �]�����|�[�g�ԍ�    �f�t�H: 56789

		�� TRACK-NAME �� _REFLUX_ ���g���Ă͂Ȃ�Ȃ��B

	------
	�P����

	�N���C�A���g��
		NamedTrack.exe /fd �I /fp 50000 /n smtp /p 25
		NamedTrack.exe /fd �I /fp 50000 /n pop /p 110
		NamedTrack.exe /fd �I /fp 50000 /n http /p 80

	�T�[�o�[��
		NamedTrack.exe /s /f �߂�I 25 smtp /f �߂�I 110 pop /f http�I 10080 http /p 50000

	����ňȉ��̂Ƃ���g���l������B(�S�� '�I:50000' ��ʂ�)
		��:25  -> �I:50000 -> �߂�I:25
		��:110 -> �I:50000 -> �߂�I:110
		��:80  -> �I:50000 -> http�I:10080

	------
	������

	�N���C�A���g��
		NamedTrack.exe /fd ���p�I /fp 50000 /n overProxy /p 55555

	���p�I
		NamedTrack.exe /s /p 50000

	�T�[�o�[��
		NamedTrack.exe /r /fd �����̎I /fp 54444 /d ���p�I /p 50000 /n overProxy

	����ňȉ��̂Ƃ���g���l������B(�I�͒��p�I�ɐڑ����ɍs�����߁A�I�����̓����ł����Ă��ǂ�)
		��:55555 -> ���p�I:50000 -> �I -> �����̎I:54444

	---------------------------
	���̌������� RealVNC �Ɍq��

	�N���C�A���g��
		crypTunnel.exe /fp 59000 /p 5900 /kb �����t�@�C��
		NamedTrack.exe /fd ���p�I /fp 56789 /p 59000 /n vnc

	���p�I
		NamedTrack.exe /s

	�T�[�o�[��
		NamedTrack.exe /r /fp 59000 /d ���p�I /n vnc
		crypTunnel.exe /fp 5900 /p 59000 /kb �����t�@�C��

	�ŁA�N���C�A���g�� RealVNC ���� localhost �֐ڑ�����ƁA�T�[�o�[���� RealVNC �Ɍq����B
	�q������́A
	��RealVNC -> ��CT:59000 -> ��NT:5900 -> ���pNT:56789 <- �INT -> �ICT:59000 -> �IRealVNC:5900
	����Ȋ����B���݂ɒ��p�I�� TCP 56789 �|�[�g�����󂯂Ă����΂����B
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "C:\Factory\Common\Options\SockServerTh.h"

#define REFLUX_POLLING_CYCLE 3000
#define REFLUX_TRACKNAME "_REFLUX_"
#define REFLUX_DISCONNECT_POLLING_CYCLE 2000
#define REFLUX_CONNECTED_SIGNAL 'R'
#define TRACKNAME_LENMAX 1024

static int ServerMode;
static int RefluxMode;

static char *ForwardDomain;
static uint ForwardPort;
static char *TrackName;

typedef struct TrackInfo_st
{
	char *Domain;
	uint Port;
	char *Name;
}
TrackInfo_t;

static autoList_t *TrackInfos;

static char *Domain;
static uint Port;

typedef struct ChannelInfo_st
{
	autoBlock_t *SendBuffer;
	int RecvSock;
	int SendSock;
	int SendSockDead;
	int *OtherSideSendSockDead;
}
ChannelInfo_t;

typedef struct WaitInfo_st
{
	char *Name;
	int Sock;
	autoBlock_t *SendBuffer;
	int Connected;
	int Disconnected;
}
WaitInfo_t;

static int KeepTheServer = 1;

static void ChannelTh(uint prm)
{
	ChannelInfo_t *i = (ChannelInfo_t *)prm;

	critical();
	{
		autoBlock_t *buffBlock = i->SendBuffer ? i->SendBuffer : newBlock();
		uint index;

		for (; ; )
		{
			for (index = 0; ; )
			{
				if (!KeepTheServer)
					goto endLoop;

				if (i->SendSockDead)
					goto endLoop;

				if (index == getSize(buffBlock))
					break;

				if (SockSendISequ(i->SendSock, buffBlock, &index, 1000) == -1)
					goto endLoop;
			}
			setSize(buffBlock, 0);

			if (SockRecvSequ(i->RecvSock, buffBlock, 1000) == -1)
			{
				*i->OtherSideSendSockDead = 1;
				break;
			}
			cout("%d -> %d (%u)\n", i->RecvSock, i->SendSock, getSize(buffBlock));
		}
	endLoop:
		releaseAutoBlock(buffBlock);
	}
	uncritical();
}
static void NTCrossChannel(ChannelInfo_t channels[2])
{
	uint thhdls[2];

	cout("�㉺�ڑ�: %p\n", channels);

	channels[0].SendSockDead = 0;
	channels[1].SendSockDead = 0;
	channels[0].OtherSideSendSockDead = &channels[1].SendSockDead;
	channels[1].OtherSideSendSockDead = &channels[0].SendSockDead;

	thhdls[0] = runThread(ChannelTh, (uint)(channels + 0));
	thhdls[1] = runThread(ChannelTh, (uint)(channels + 1));

	inner_uncritical();
	{
		waitThread(thhdls[0]);
		waitThread(thhdls[1]);
	}
	inner_critical();

	cout("�㉺�ؒf: %p\n", channels);
}
static autoList_t *WaitInfos;

static void WaitClient(int sock, char *name, autoBlock_t *blockSend)
{
	WaitInfo_t *i = (WaitInfo_t *)memAlloc(sizeof(WaitInfo_t));
	uint waitcnt;
	uint index;

	cout("���p�J�n: %d\n", sock);

	i->Name = name;
	i->Sock = sock;
	i->SendBuffer = blockSend;
	i->Connected = 0;
	i->Disconnected = 0;

	addElement(WaitInfos, (uint)i);

	for (waitcnt = 1; waitcnt <= 2; waitcnt++)
	{
		cout("C");
		inner_uncritical();
		{
			sleep(REFLUX_POLLING_CYCLE);
		}
		inner_critical();
		cout("D");

		if (i->Connected)
			break;
	}
	cout("d");
	if (!i->Connected)
	{
		removeElement(WaitInfos, (uint)i);
		releaseAutoBlock(i->SendBuffer);
		goto endfunc;
	}
	cout("e");
	while (!i->Disconnected)
	{
		cout("E");
		inner_uncritical();
		{
			sleep(REFLUX_DISCONNECT_POLLING_CYCLE);
		}
		inner_critical();
		cout("F");
	}
	cout("���p�I��: %d\n", sock);
endfunc:
	memFree(i->Name);
	memFree(i);
}
static void PerformTh(int sock, char *strip)
{
	autoBlock_t *blockSend;
	char *fwdDomain;
	uint fwdPort;
	uchar ip[4];
	int fwdSock;
	ChannelInfo_t channels[2];
	int refluxMode = 0;

	cout("�ڑ�: %d\n", sock);

	if (ServerMode)
	{
		SockStream_t *ss = CreateSockStream(sock, 60);
		TrackInfo_t *i;
		char *name;
		uint index;

		name = SockRecvLine(ss, TRACKNAME_LENMAX);

		if (!strcmp(name, REFLUX_TRACKNAME))
		{
			cout("���p���\n");
			refluxMode = 1;
			memFree(name);
			name = SockRecvLine(ss, TRACKNAME_LENMAX);
		}
		blockSend = SockRipRecvBuffer(ss);
		ReleaseSockStream(ss);

		{
			char *p_name = strx(name);

			line2JLine(p_name, 1, 0, 0, 0);
			cout("�g���b�N��: %s\n", p_name);
			memFree(p_name);
		}

		if (refluxMode)
		{
			WaitInfo_t *wi;

			foreach (WaitInfos, wi, index)
				if (!strcmp(wi->Name, name))
					break;

			if (wi)
			{
				removeElement(WaitInfos, (uint)wi);

				wi->Connected = 1;
				insertByte(wi->SendBuffer, 0, REFLUX_CONNECTED_SIGNAL);

				channels[0].SendBuffer = blockSend;
				channels[0].RecvSock = sock;
				channels[0].SendSock = wi->Sock;

				channels[1].SendBuffer = wi->SendBuffer;
				channels[1].RecvSock = wi->Sock;
				channels[1].SendSock = sock;

				NTCrossChannel(channels);

				wi->Disconnected = 1;
			}
			goto endConnect;
		}
		foreach (TrackInfos, i, index)
			if (!strcmp(name, i->Name))
				break;

		if (i == NULL)
		{
			WaitClient(sock, name, blockSend);
			goto endConnect;
		}
		memFree(name);

		fwdDomain = i->Domain;
		fwdPort = i->Port;
	}
	else
	{
		blockSend = newBlock();

		ab_addLine(blockSend, TrackName);
		addByte(blockSend, 13); // CR
		addByte(blockSend, 10); // LF

		fwdDomain = ForwardDomain;
		fwdPort = ForwardPort;
	}
	cout("�]����I: %s\n", fwdDomain);
	cout("�]����`: %u\n", fwdPort);

	*(uint *)ip = 0;
	fwdSock = sockConnect(ip, fwdDomain, fwdPort);

	cout("�]����ڑ�: %d\n", fwdSock);

	if (fwdSock == -1)
		goto endConnect;

	channels[0].SendBuffer = blockSend;
	channels[0].RecvSock = sock;
	channels[0].SendSock = fwdSock;

	channels[1].SendBuffer = NULL;
	channels[1].RecvSock = fwdSock;
	channels[1].SendSock = sock;

	NTCrossChannel(channels);

	sockDisconnect(fwdSock);

endConnect:
	cout("�ؒf: %d\n", sock);
}
static void RefluxChannelTh(uint prm)
{
	ChannelInfo_t channels[2];
	int *socks = (int *)prm;

	channels[0].SendBuffer = NULL;
	channels[0].RecvSock = socks[0];
	channels[0].SendSock = socks[1];

	channels[1].SendBuffer = NULL;
	channels[1].RecvSock = socks[1];
	channels[1].SendSock = socks[0];

	critical();
	{
		NTCrossChannel(channels);

		sockDisconnect(socks[0]);
		sockDisconnect(socks[1]);

		memFree(socks);
	}
	uncritical();
}
static void RefluxPerform(uint connectmax)
{
	autoList_t *thhdls = newList();
	uchar ip[4];
	uchar fwdip[4];

	*(uint *)ip = 0;
	*(uint *)fwdip = 0;

	SockStartup();

	critical();
	{
		for (; ; )
		{
			if (getCount(thhdls) < connectmax)
			{
				int sock = sockConnect(ip, Domain, Port);

				if (sock != -1) // ? �ڑ�
				{
					SockStream_t *ss = CreateSockStream(sock, 60);
					uchar signal[1];
					int retval;

					SockSendLine(ss, REFLUX_TRACKNAME);
					SockSendLine(ss, TrackName);
					ReleaseSockStream(ss);

					retval = SockTransmit(sock, signal, 1, REFLUX_POLLING_CYCLE * 3, 0); // �I��2��҂̂ŁA3�񕪑҂ĂΏ\���ł���B

					if (retval == 1 && signal[0] == REFLUX_CONNECTED_SIGNAL)
					{
						int fwdSock = sockConnect(fwdip, ForwardDomain, ForwardPort);

						if (fwdSock != -1) // ? �ڑ�
						{
							int *socks = (int *)memAlloc(2 * sizeof(int));

							socks[0] = sock;
							socks[1] = fwdSock;

							addElement(thhdls, runThread(RefluxChannelTh, (uint)socks));
							goto endConnect;
						}
					}
					sockDisconnect(sock);
				endConnect:;
				}
			}
			inner_uncritical();
			{
				sleep(REFLUX_POLLING_CYCLE);
			}
			inner_critical();

			collectDeadThreads(thhdls);

			while (hasKey())
			{
				if (getKey() == 0x1b)
				{
					KeepTheServer = 0;
					goto endLoop;
				}
			}
		}
	endLoop:
		while (getCount(thhdls))
		{
			inner_uncritical();
			{
				sleep(500);
			}
			inner_critical();

			collectDeadThreads(thhdls);
		}
	}
	uncritical();

	SockCleanup();
}

static int IdleTh(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			KeepTheServer = 0;
			return 0;
		}
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint connectmax = 10;

	ForwardDomain = "localhost";
	ForwardPort = 60000;
	TrackName = "";

	TrackInfos = newList();

	Domain = "localhost";
	Port = 56789;

	WaitInfos = newList();

readArgs:
	if (argIs("/S")) // Server mode
	{
		ServerMode = 1;
		goto readArgs;
	}
	if (argIs("/R")) // Reflux mode
	{
		RefluxMode = 1;
		goto readArgs;
	}

	if (argIs("/FD")) // Forward Domain
	{
		ForwardDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/FP")) // Forward Port
	{
		ForwardPort = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/N")) // Name of track
	{
		TrackName = nextArg();
		goto readArgs;
	}

	if (argIs("/F")) // Forward
	{
		TrackInfo_t *i = (TrackInfo_t *)memAlloc(sizeof(TrackInfo_t));

		i->Domain = nextArg();
		i->Port = toValue(nextArg());
		i->Name = nextArg();

		cout("�]���ݒ�...\n");
		cout("< %s\n", i->Name);
		cout("> %s:%u\n", i->Domain, i->Port);

		addElement(TrackInfos, (uint)i);
		goto readArgs;
	}

	if (argIs("/D")) // Domain
	{
		Domain = nextArg();
		goto readArgs;
	}
	if (argIs("/P")) // Port
	{
		Port = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/C")) // Connect max
	{
		connectmax = toValue(nextArg());
		goto readArgs;
	}

	errorCase(ServerMode && RefluxMode);

	if (RefluxMode)
	{
		cout("�t�����[�h\n");
		cout("�]���I: %s\n", ForwardDomain);
		cout("�]���`: %u\n", ForwardPort);
		cout("���p�I: %s\n", Domain);
		cout("���p�`: %u\n", Port);
		cout("�g���b�N��: %s\n", TrackName);
		cout("�ő�ڑ���: %u\n", connectmax);

		RefluxPerform(connectmax);
		return;
	}
	if (!ServerMode)
	{
		cout("�N���C�A���g���[�h\n");
		cout("�]���I: %s\n", ForwardDomain);
		cout("�]���`: %u\n", ForwardPort);
		cout("�g���b�N��: %s\n", TrackName);
	}
	else
	{
		cout("�T�[�o�[���[�h\n");
	}
	cout("�҂��󂯍`: %u\n", Port);
	cout("�ő�ڑ���: %u\n", connectmax);

	cout("�J�n\n");
	SockStartup();
	sockServerTh(PerformTh, Port, connectmax, IdleTh);
	SockCleanup();
	cout("�I��\n");
}
