/*
	revServer.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	revServer.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX]

		FWD-HOST ... 不使用
		FWD-PORT ... 不使用
		CONNECT-MAX ... 最大接続数, 省略時は 1000

	トラック名

		NAME
		NAME + ":" + POLE
		NAME + ":" + POLE + ".R"

		同じ NAME で POLE が異なる接続同士を紐付ける。
		同じ NAME で POLE が "default" の接続同士も紐付ける。
		POLE を省略した場合 "default" を使用する。
		POLE の後ろが ".R" である場合、紐付けした直後に接続元へ 'C' を送る。

		例えば 60001 と 60002 を紐付けたい場合は ...

			revServer.exe 50000 a 1
			namedTrack.exe 60001 localhost 50000 cure:heart
			namedTrack.exe 60002 localhost 50000 cure:diamond

		単独のポート(60000)で紐付けたい場合は ...

			revServer.exe 50000 a 1
			namedTrack.exe 60000 localhost 50000 precure

		revClient.exe と紐付ける場合、接続したことを返さなければならないので POLE の最後に ".R" が必要です。

			hostA:60001 -> hostC:55555

			[hostA]
				revServer.exe 50000 a 1
				namedTrack.exe 60001 localhost 50000 cure:heart
				namedTrack.exe 60002 localhost 50000 cure:diamond.R

			[hostB]
				revClient.exe hostA 60002 hostC 55555
*/

#include "libs\Tunnel.h"
#include "libs\IntSleep.h"

typedef struct ConnectInfo_st
{
	int Sock;
	char *Name;
	char *Pole;
	int R;
	int Closed;
}
ConnectInfo_t;

static void ReleaseConnectInfo(ConnectInfo_t *i)
{
	memFree(i->Name);
	memFree(i->Pole);
	memFree(i);
}

#define JOIN_TIMEOUT_SEC 20
#define JOIN_CYCLE_MILLIS 2000
#define RECV_TRACKNAME_TIMEOUT_MILLIS 60000
#define SEND_C_TIMEOUT_MILLIS 60000
#define TRACKNAME_LENMAX 100
#define DEF_POLE "default"

static autoList_t *ConnectList;

static int BeforeJoin(ConnectInfo_t *i)
{
	if (i->R)
	{
		autoBlock_t *buff = newBlock();
		int retval;

		addByte(buff, 'C');
		retval = SockSendSequLoop(i->Sock, buff, SEND_C_TIMEOUT_MILLIS);
		releaseAutoBlock(buff);

		if (retval != 1)
			return 0;
	}
	return 1;
}
static void JoinMain(ConnectInfo_t *i, ConnectInfo_t *osi)
{
	if (
		BeforeJoin(i) &&
		BeforeJoin(osi)
		)
	{
		CrossChannel(i->Sock, osi->Sock, NULL, 0, NULL, 0);
	}

//	i->Closed = 1;
	osi->Closed = 1;
	IntSleepInt();

	ReleaseConnectInfo(i);
}
static void PerformTh(int sock, char *strip)
{
	char *trackName = SockNextLineLoop(sock, RECV_TRACKNAME_TIMEOUT_MILLIS, TRACKNAME_LENMAX);
	autoList_t *tnTkns;
	ConnectInfo_t *i;
	ConnectInfo_t *osi;
	uint index;

	if (!trackName)
		return;

	tnTkns = tokenizeMinMax(trackName, ':', 2, 2, DEF_POLE);

	i = (ConnectInfo_t *)memAlloc(sizeof(ConnectInfo_t));
	i->Sock = sock;
	i->Name = getLine(tnTkns, 0);
	i->Pole = getLine(tnTkns, 1);
	i->R = 0;
	i->Closed = 0;

	releaseAutoList(tnTkns);

	line2JLine(i->Name, 1, 0, 0, 0);
	line2JLine(i->Pole, 1, 0, 0, 0);

	if (endsWith(i->Pole, ".R") || endsWith(i->Pole, ".r"))
	{
		i->Pole[strlen(i->Pole) - 2] = '\0';
		i->R = 1;
	}

	foreach (ConnectList, osi, index)
	{
		if (!_stricmp(i->Name, osi->Name))
		{
			if (_stricmp(i->Pole, osi->Pole) || !_stricmp(i->Pole, DEF_POLE) && !_stricmp(osi->Pole, DEF_POLE))
			{
LOGPOS();
				fastDesertElement(ConnectList, index); // remove osi
				JoinMain(i, osi);
				return;
			}
		}
	}
	addElement(ConnectList, (uint)i);

LOGPOS();
	{
		uint tmoutTime = now() + JOIN_TIMEOUT_SEC;

		do
		{
			inner_uncritical();
			{
				IntSleep(JOIN_CYCLE_MILLIS);
			}
			inner_critical();

			if (tmoutTime != UINTMAX && tmoutTime < now()) // ? Joinタイムアウト
			{
				uint iPos = findElement(ConnectList, (uint)i, simpleComp);

				if (iPos < getCount(ConnectList)) // ? 見つかった。-> Joinしていない。
				{
					fastDesertElement(ConnectList, iPos); // remove i
					break;
				}
				tmoutTime = UINTMAX; // Joinした。-> Joinタイムアウトはもう無い。
			}
		}
		while (!i->Closed && !ProcDeadFlag);
	}
LOGPOS();

	ReleaseConnectInfo(i);
}
static int ReadArgs(void)
{
	return 0;
}
int main(int argc, char **argv)
{
	ConnectList = newList();

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "revServer", NULL);
	TunnelPerformTh = NULL;
}
