/*
	sockServerのマルチスレッド版, 接続数分スレッドを生成する。

	----

	最悪のメモリ使用量 (*1)
		0 [byte] (*2)

	最悪のディスク使用量 (*1)
		0 [byte]

	(*1) システム側・コールバック側で消費する分、スタックに積まれる分など細かいものは考慮していない。
	(*2) 接続毎に (TransInfos, TransInfoStock, TransInfoDead に確保されるブロック == 28 バイト) を使用するが、
	     最大接続数が 65000 だとしても 1.82 MB 程度なので無視する。

	コールバックも考慮した最悪のメモリ・ディスク使用量
		コールバックとして funcTransmit, funcIdle がある。
		それぞれの呼び出しのために大規模に展開されるファイル・メモリブロックは無いが、それぞれ別々のスレッドから呼び出され、
		最悪の場合 (funcTransmit * 最大接続数 + funcIdle * 1) が同時に実行される。(*3)
		最悪の使用量は (funcTransmit での最大使用量) * connectmax + (funcIdle での最大使用量) として考える。

	(*3) funcIdle は(普通は)通信を行わないが funcTransmit の sockWait 中に呼び出されるのでマルチで動く。
*/

#include "SockServerTh.h"

#define MILLIS_TIMEOUT_SELECT_ACCEPT 1000
#define MILLIS_TIMEOUT_TRANSMIT_END 300

#define TRANSMIT_END_EVENT "cerulean.charlotte Factory SockServerTh transmit end event object"

typedef struct TransInfo_st
{
	int Sock;
	char ClientStrIp[16];
	uint Handle;
}
TransInfo_t;

static TransInfo_t *TransInfos;
static TransInfo_t **TransInfoStock;
static uint TransInfoStockCount;

static TransInfo_t **TransInfoDead;
static uint TransInfoDeadCount;

static void (*FuncTransmit)(int, char *);

static void PerformTransmit(uint prm)
{
	TransInfo_t *i = (TransInfo_t *)prm;

	critical();
	{
		FuncTransmit(i->Sock, i->ClientStrIp);

		SockPreClose(i->Sock);
		shutdown(i->Sock, SD_BOTH);
		closesocket(i->Sock);

		TransInfoDead[TransInfoDeadCount] = i;
		TransInfoDeadCount++;

		sockConnectedCount--;
		sockConnectMonUpdated();

		eventWakeup(TRANSMIT_END_EVENT);
	}
	uncritical();
}
/*
	funcTransmit(), funcIdle() は critical(); ～ uncritical(); の中から呼ぶ。
*/
void sockServerTh(void (*funcTransmit)(int, char *), uint portno, uint connectmax, int (*funcIdle)(void))
{
	int tmpval;
	int sock;
	int clsock;
	struct sockaddr_in sa;
	struct sockaddr_in clsa;
	uint transmitEndEvent = eventOpen(TRANSMIT_END_EVENT);
	TransInfo_t *i;
	uint index;
	int shutdownTheServer = 0;

	errorCase(!funcTransmit);
	errorCase(!portno || 0xffff < portno);
	errorCase(!connectmax || UINTMAX / sizeof(TransInfo_t) < connectmax);
	errorCase(!funcIdle);

	SockStartup();

	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	errorCase(sock == -1);
	SockPostOpen(sock);

	memset(&sa, 0x00, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = htonl(INADDR_ANY);
	sa.sin_port = htons((unsigned short)portno);

	if (bind(sock, (struct sockaddr *)&sa, sizeof(sa)) != 0) // ? == -1
	{
		error_m("TCPポートのバインドに失敗しました。");
	}
	if (listen(sock, SOMAXCONN) != 0) // ? == -1
	{
		error();
	}

	TransInfos     = (TransInfo_t  *)memAlloc(connectmax * sizeof(TransInfo_t));
	TransInfoStock = (TransInfo_t **)memAlloc(connectmax * sizeof(TransInfo_t *));
	TransInfoDead  = (TransInfo_t **)memAlloc(connectmax * sizeof(TransInfo_t *));

	for (index = 0; index < connectmax; index++)
	{
		TransInfoStock[index] = TransInfos + index;
	}
	TransInfoStockCount = connectmax;
//	TransInfoDeadCount = 0;

	FuncTransmit = funcTransmit;

	sockConnectMax = connectmax;
	sockConnectMonUpdated();

	sockServerMode = 1;

	// ここからマルチで動く --->

	critical();
	{
		for (; ; )
		{
//			SockCurrTime = time(NULL);

			if (funcIdle() == 0) // ? サーバー停止
			{
				shutdownTheServer = 1;
			}
			else if (TransInfoStockCount)
			{
				int hasConnect;

				/*
					高負荷時、SockWait(, 1000, 0) が 20 秒待たされることがある。
					別スレッドが動いているのでブロックされていない。-> select で待たされていると思われる。
					<- connect() でブロックされて select 直後のCSに入れなかった..
					20秒 -> connect のタイムアウト？

					-> sockConnet() の connect() がブロックしていた事が原因でした。
				*/

#if 0
				hasConnect = SockWait(sock, MILLIS_TIMEOUT_SELECT_ACCEPT, 0);
#else
				{
					uint st;
					uint et;
					uint dt;

					st = now();

					hasConnect = SockWait(sock, MILLIS_TIMEOUT_SELECT_ACCEPT, 0);

					et = now();
					dt = et - st;

					if (2 < dt)
					{
						cout("Warning: 待ち受けポートの接続待ちに %u 秒掛かりました。hasConnect: %d\n", dt, hasConnect);
						cout("SockWait_T1: %u\n", SockWait_T1);
						cout("SockWait_T2: %u\n", SockWait_T2);
						cout("SockWait_T3: %u\n", SockWait_T3);
						cout("SockWait_T4: %u\n", SockWait_T4);
						setConsoleColor(0x5f);
					}
				}
#endif

				if (hasConnect) // ? 接続あり
				{
					tmpval = sizeof(clsa);
					clsock = accept(sock, (struct sockaddr *)&clsa, &tmpval);
					errorCase(clsock == -1);
					SockPostOpen(clsock);

					TransInfoStockCount--;
					i = TransInfoStock[TransInfoStockCount];

					i->Sock = clsock;
					strcpy(i->ClientStrIp, inet_ntoa(clsa.sin_addr));
					i->Handle = runThread(PerformTransmit, (uint)i);

					sockConnectedCount++;
					sockConnectMonUpdated();
				}
			}
			else
			{
			waitForTransmitEnd:
				inner_uncritical();
				{
					collectEvents(transmitEndEvent, MILLIS_TIMEOUT_TRANSMIT_END);
				}
				inner_critical();
			}
			collectEvents(transmitEndEvent, 0); // cleanup

			for (index = 0; index < TransInfoDeadCount; index++)
			{
				i = TransInfoDead[index];

				eventSleep(i->Handle); // i->Handle はスレッドだけど処理は同じだからこれでいいや...

				TransInfoStock[TransInfoStockCount] = i;
				TransInfoStockCount++;
			}
			TransInfoDeadCount = 0;

			if (shutdownTheServer)
			{
				if (TransInfoStockCount == connectmax)
					break;

				goto waitForTransmitEnd;
			}
		}
	}
	uncritical();

	// <--- ここまでマルチで動く

	SockPreClose(sock);

	if (closesocket(sock) != 0) // ? fault
	{
		error();
	}
	SockCleanup();

	memFree(TransInfos);
	memFree(TransInfoStock);
	memFree(TransInfoDead);
}
