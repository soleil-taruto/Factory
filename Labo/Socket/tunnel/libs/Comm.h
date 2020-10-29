#pragma once

#include "C:\Factory\Common\Options\SockClient.h" // •K—v‚È‚Ì‚Í Socket.h
#include "CCCCommon.h"
#include "IntSleep.h"

typedef struct Comm_st // static member
{
	int Sock;
	int SockClosedFlag;
	int SockRecvClosedFlag;
	autoBlock_t *RecvBuff;
	autoBlock_t *SendBuff;
	int DeadFlag;
	int KillFlag;
	int ThreadEndedFlag;
	uint RecvTh;
	uint SendTh;
}
Comm_t;

Comm_t *CreateComm(int sock);
void EndCommThread(Comm_t *i);
void ReleaseComm(Comm_t *i);

autoBlock_t *GetCommRecvData(Comm_t *i, uint size);
int AddCommSendData(Comm_t *i, autoBlock_t *data, int forceMode);
int IsCommDead(Comm_t *i);
int IsCommDeadAndEmpty(Comm_t *i);
