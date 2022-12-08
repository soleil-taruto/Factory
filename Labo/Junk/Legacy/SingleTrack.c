/*
	�N���C�A���g���[�h
		SingleTrack.exe [/DD DIVIDER-DOMAIN]? [/DP DIVIDER-PORT]? [/DN DIVIDER-NAME]? [/P PORT]? [/C CONNECT-MAX]?

	���z�I���[�h
		SingleTrack.exe [/F FORWARD-DOMAIN FORWARD-PORT FORWARD-NAME]+ [/P PORT]? [/C CONNECT-MAX]?

		DIVIDER-DOMAIN  �ڑ��敪�z�I�z�X�g��
		DIVIDER-PORT    �ڑ��敪�z�I�|�[�g�ԍ�
		DIVIDER-NAME    �ڑ��敪��
		FORWARD-DOMAIN  �]����z�X�g��
		FORWARD-PORT    �]����|�[�g�ԍ�
		FORWARD-NAME    �]���敪��
		PORT            �҂��󂯃|�[�g�ԍ�
		CONNECT-MAX     �ő�ڑ���

	- - -

	�N���C�A���g��
		SingleTrack.exe /dd �I /dp 50000 /dn smtp /p 25
		SingleTrack.exe /dd �I /dp 50000 /dn pop /p 110
		SingleTrack.exe /dd �I /dp 50000 /dn http /p 80

	�T�[�o�[��
		SingleTrack.exe /f �߂�I 25 smtp /f �߂�I 110 pop /f http�I 10080 http /p 50000

	����ňȉ��̂Ƃ���g���l������B(�S�� '�IPC 50000' ��ʂ�)
		��PC 25  -> �߂�I 25
		��PC 110 -> �߂�I 110
		��PC 80  -> http�I 10080
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "C:\Factory\Common\Options\SockServerTh.h"

typedef struct ForwardInfo_st
{
	char *Domain;
	uint Port;
	char *Name;
}
ForwardInfo_t;

static ForwardInfo_t Divider;
static autoList_t *ServerList; // == NULL ? client mode : divider mode

typedef struct ChannelInfo_st
{
	autoBlock_t *SendBuffer;
	int RecvSock;
	int SendSock;
	int SendSockDead;
	int *OtherSideSendSockDead;
}
ChannelInfo_t;

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
static void PerformTh(int sock, char *strip)
{
	autoBlock_t *blockSend;
	char *fwdDomain;
	uint fwdPort;
	uchar ip[4];
	int fwdSock;
	uint thhdls[2];
	ChannelInfo_t channels[2];

	cout("�ڑ�: %d\n", sock);

	if (ServerList)
	{
		SockStream_t *ss = CreateSockStream(sock, 60);
		char *name;
		ForwardInfo_t *i;
		uint index;

		name = SockRecvLine(ss, 1024);

		{
			char *p_name = strx(name);

			line2JLine(p_name, 1, 0, 0, 0);
			cout("����: %s\n", p_name);
			memFree(p_name);
		}

		foreach (ServerList, i, index)
			if (!strcmp(name, i->Name))
				break;

		memFree(name);

		if (!i)
		{
			ReleaseSockStream(ss);
			goto endConnect;
		}
		blockSend = SockRipRecvBuffer(ss);
		ReleaseSockStream(ss);

		fwdDomain = i->Domain;
		fwdPort = i->Port;
	}
	else
	{
		blockSend = newBlock();

		ab_addLine(blockSend, Divider.Name);
		addByte(blockSend, 13); // CR
		addByte(blockSend, 10); // LF

		fwdDomain = Divider.Domain;
		fwdPort = Divider.Port;
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
	channels[0].SendSockDead = 0;
	channels[0].OtherSideSendSockDead = &channels[1].SendSockDead;

	channels[1].SendBuffer = NULL;
	channels[1].RecvSock = fwdSock;
	channels[1].SendSock = sock;
	channels[1].SendSockDead = 0;
	channels[1].OtherSideSendSockDead = &channels[0].SendSockDead;

	thhdls[0] = runThread(ChannelTh, (uint)(channels + 0));
	thhdls[1] = runThread(ChannelTh, (uint)(channels + 1));

	inner_uncritical();
	{
		waitThread(thhdls[0]);
		waitThread(thhdls[1]);
	}
	inner_critical();

	sockDisconnect(fwdSock);

endConnect:
	cout("�ؒf: %d\n", sock);
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
	uint portno = 60000; // crypTunnel�̐ڑ���f�t�H
	uint connectmax = 10;

	Divider.Domain = "localhost";
	Divider.Port = 59999; // crypTunnel�̑҂��󂯃f�t�H
	Divider.Name = "forward";

readArgs:
	if (argIs("/DD")) // Divider Domain
	{
		Divider.Domain = nextArg();
		goto readArgs;
	}
	if (argIs("/DP")) // Divider Port
	{
		Divider.Port = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/DN")) // Divider Name
	{
		Divider.Name = nextArg();
		goto readArgs;
	}

	if (argIs("/F")) // Forward
	{
		ForwardInfo_t *i = (ForwardInfo_t *)memAlloc(sizeof(ForwardInfo_t));

		i->Domain = nextArg();
		i->Port = toValue(nextArg());
		i->Name = nextArg();

		cout("< %s\n", i->Name);
		cout("> %s:%u\n", i->Domain, i->Port);

		if (!ServerList)
			ServerList = newList();

		addElement(ServerList, (uint)i);
		goto readArgs;
	}

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

	if (!ServerList)
	{
		cout("�N���C�A���g���[�h\n");
		cout("���z�I: %s\n", Divider.Domain);
		cout("���z�`: %u\n", Divider.Port);
		cout("����: %s\n", Divider.Name);
	}
	else
	{
		cout("���z�I���[�h\n");
	}
	cout("�҂��󂯍`: %u\n", portno);
	cout("�ő�ڑ���: %u\n", connectmax);

	cout("�J�n\n");
	SockStartup();
	sockServerTh(PerformTh, portno, connectmax, IdleTh);
	SockCleanup();
	cout("�I��\n");
}
