/*
	Tunnel.c multi-thread ver.
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\SockServerTh.h"

static void DispData(autoBlock_t *block, int sock, int destSock, char *title)
{
	if (getSize(block))
	{
		char *line = toPrintLine(block, 1);

		cout("%s: %d -> %d\n", title, sock, destSock);
		cout("%s\n", line);
		cout("\n");

		memFree(line);
	}
}

static char *FwdDomain = "localhost";
static uint FwdPort = 80;
static uint SilentMode;
static uint SessionTimeout; // ïbêî, 0 == ñ≥êßå¿

typedef struct ChannelInfo_st
{
	char *StrDirect;
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
		autoBlock_t *buffBlock = newBlock();
		uint index;
		uint connectedTime = SessionTimeout ? now() : 0;

		for (; ; )
		{
			if (SockRecvSequ(i->RecvSock, buffBlock, 1000) == -1)
			{
				*i->OtherSideSendSockDead = 1;
				break;
			}
			if (!SilentMode)
			{
				DispData(buffBlock, i->RecvSock, i->SendSock, i->StrDirect);
			}
			for (index = 0; ; )
			{
				if (!KeepTheServer)
				{
					goto endLoop;
				}
				if (i->SendSockDead)
				{
					goto endLoop;
				}
				if (SessionTimeout && connectedTime + SessionTimeout < now())
				{
					goto endLoop;
				}
				if (index == getSize(buffBlock))
				{
					break;
				}
				if (SockSendISequ(i->SendSock, buffBlock, &index, 1000) == -1)
				{
					goto endLoop;
				}
			}
			setSize(buffBlock, 0);
		}
	endLoop:
		releaseAutoBlock(buffBlock);
	}
	uncritical();
}
static void PerformTh(int sock, char *strip)
{
	uchar ip[4];
	int fwdSock;
	uint thhdls[2];
	ChannelInfo_t channels[2];

	*(uint *)ip = 0;
	fwdSock = sockConnect(ip, FwdDomain, FwdPort);

	cout("ê⁄ë±: %d -> %d\n", sock, fwdSock);

	if (fwdSock == -1) goto endConnect;

	channels[0].StrDirect = "è„ÇË";
	channels[0].RecvSock = sock;
	channels[0].SendSock = fwdSock;
	channels[0].SendSockDead = 0;
	channels[0].OtherSideSendSockDead = &channels[1].SendSockDead;

	channels[1].StrDirect = "â∫ÇË";
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
	cout("êÿíf: %d -> %d\n", sock, fwdSock);
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
	uint portno = 50080;
	uint connectmax = 10;

readArgs:
	// ì]ëóêÊ
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

	// ë“ÇøéÛÇØ
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

	if (argIs("/S")) // Silent mode
	{
		SilentMode = 1;
		goto readArgs;
	}
	if (argIs("/T")) // session Timeout
	{
		SessionTimeout = toValue(nextArg());
		goto readArgs;
	}

	cmdTitle_x(xcout("Tunnel2 - %u to %s:%u (connectmax:%u)", portno, FwdDomain, FwdPort, connectmax));

	cout("äJén\n");
	SockStartup();
	sockServerTh(PerformTh, portno, connectmax, IdleTh);
	SockCleanup();
	cout("èIóπ\n");
}
