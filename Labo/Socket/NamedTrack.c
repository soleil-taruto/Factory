/*
	クライアントモード
		NamedTrack.exe [/FD FORWARD-DOMAIN] [/FP FORWARD-PORT] [/N TRACK-NAME] [/P PORT] [/C CONNECT-MAX]

	サーバーモード
		NamedTrack.exe /S [/F FORWARD-DOMAIN FORWARD-PORT TRACK-NAME]* [/P PORT] [/C CONNECT-MAX]

	逆流モード
		NamedTrack.exe /R [/FD FORWARD-DOMAIN] [/FP FORWARD-PORT] [/D POLLING-DOMAIN] [/P POLLING-PORT] [/N TRACK-NAME]

		FORWARD-DOMAIN  転送先ホスト名      デフォ: "localhost"
		FORWARD-PORT    転送先ポート番号    デフォ: 60000
		TRACK-NAME      このトラック名      デフォ: "" (因みに "" でも '\0' は送られるので、ただのトンネルになるわけではない)
		PORT            待ち受けポート番号  デフォ: 56789
		CONNECT-MAX     最大接続数          デフォ: 10
		POLLING-DOMAIN  転送元ホスト名      デフォ: "localhost"
		POLLING-PORT    転送元ポート番号    デフォ: 56789

		★ TRACK-NAME に _REFLUX_ を使ってはならない。

	------
	単線化

	クライアント側
		NamedTrack.exe /fd 鯖 /fp 50000 /n smtp /p 25
		NamedTrack.exe /fd 鯖 /fp 50000 /n pop /p 110
		NamedTrack.exe /fd 鯖 /fp 50000 /n http /p 80

	サーバー側
		NamedTrack.exe /s /f める鯖 25 smtp /f める鯖 110 pop /f http鯖 10080 http /p 50000

	これで以下のとおりトンネルする。(全て '鯖:50000' を通る)
		鞍:25  -> 鯖:50000 -> める鯖:25
		鞍:110 -> 鯖:50000 -> める鯖:110
		鞍:80  -> 鯖:50000 -> http鯖:10080

	------
	串透過

	クライアント側
		NamedTrack.exe /fd 中継鯖 /fp 50000 /n overProxy /p 55555

	中継鯖
		NamedTrack.exe /s /p 50000

	サーバー側
		NamedTrack.exe /r /fd 何かの鯖 /fp 54444 /d 中継鯖 /p 50000 /n overProxy

	これで以下のとおりトンネルする。(鯖は中継鯖に接続しに行くため、鯖が串の内側であっても良い)
		鞍:55555 -> 中継鯖:50000 -> 鯖 -> 何かの鯖:54444

	---------------------------
	串の向こうの RealVNC に繋ぐ

	クライアント側
		crypTunnel.exe /fp 59000 /p 5900 /kb 鍵束ファイル
		NamedTrack.exe /fd 中継鯖 /fp 56789 /p 59000 /n vnc

	中継鯖
		NamedTrack.exe /s

	サーバー側
		NamedTrack.exe /r /fp 59000 /d 中継鯖 /n vnc
		crypTunnel.exe /fp 5900 /p 59000 /kb 鍵束ファイル

	で、クライアント側 RealVNC から localhost へ接続すると、サーバー側の RealVNC に繋がる。
	繋がり方は、
	鞍RealVNC -> 鞍CT:59000 -> 鞍NT:5900 -> 中継NT:56789 <- 鯖NT -> 鯖CT:59000 -> 鯖RealVNC:5900
	こんな感じ。因みに中継鯖の TCP 56789 ポートだけ空けておけばおｋ。
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

	cout("上下接続: %p\n", channels);

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

	cout("上下切断: %p\n", channels);
}
static autoList_t *WaitInfos;

static void WaitClient(int sock, char *name, autoBlock_t *blockSend)
{
	WaitInfo_t *i = (WaitInfo_t *)memAlloc(sizeof(WaitInfo_t));
	uint waitcnt;
	uint index;

	cout("中継開始: %d\n", sock);

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
	cout("中継終了: %d\n", sock);
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

	cout("接続: %d\n", sock);

	if (ServerMode)
	{
		SockStream_t *ss = CreateSockStream(sock, 60);
		TrackInfo_t *i;
		char *name;
		uint index;

		name = SockRecvLine(ss, TRACKNAME_LENMAX);

		if (!strcmp(name, REFLUX_TRACKNAME))
		{
			cout("中継回収\n");
			refluxMode = 1;
			memFree(name);
			name = SockRecvLine(ss, TRACKNAME_LENMAX);
		}
		blockSend = SockRipRecvBuffer(ss);
		ReleaseSockStream(ss);

		{
			char *p_name = strx(name);

			line2JLine(p_name, 1, 0, 0, 0);
			cout("トラック名: %s\n", p_name);
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
	cout("転送先鯖: %s\n", fwdDomain);
	cout("転送先港: %u\n", fwdPort);

	*(uint *)ip = 0;
	fwdSock = sockConnect(ip, fwdDomain, fwdPort);

	cout("転送先接続: %d\n", fwdSock);

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
	cout("切断: %d\n", sock);
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

				if (sock != -1) // ? 接続
				{
					SockStream_t *ss = CreateSockStream(sock, 60);
					uchar signal[1];
					int retval;

					SockSendLine(ss, REFLUX_TRACKNAME);
					SockSendLine(ss, TrackName);
					ReleaseSockStream(ss);

					retval = SockTransmit(sock, signal, 1, REFLUX_POLLING_CYCLE * 3, 0); // 鯖で2回待つので、3回分待てば十分でしょ。

					if (retval == 1 && signal[0] == REFLUX_CONNECTED_SIGNAL)
					{
						int fwdSock = sockConnect(fwdip, ForwardDomain, ForwardPort);

						if (fwdSock != -1) // ? 接続
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

		cout("転送設定...\n");
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
		cout("逆流モード\n");
		cout("転送鯖: %s\n", ForwardDomain);
		cout("転送港: %u\n", ForwardPort);
		cout("中継鯖: %s\n", Domain);
		cout("中継港: %u\n", Port);
		cout("トラック名: %s\n", TrackName);
		cout("最大接続数: %u\n", connectmax);

		RefluxPerform(connectmax);
		return;
	}
	if (!ServerMode)
	{
		cout("クライアントモード\n");
		cout("転送鯖: %s\n", ForwardDomain);
		cout("転送港: %u\n", ForwardPort);
		cout("トラック名: %s\n", TrackName);
	}
	else
	{
		cout("サーバーモード\n");
	}
	cout("待ち受け港: %u\n", Port);
	cout("最大接続数: %u\n", connectmax);

	cout("開始\n");
	SockStartup();
	sockServerTh(PerformTh, Port, connectmax, IdleTh);
	SockCleanup();
	cout("終了\n");
}
