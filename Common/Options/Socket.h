/*
	このヘッダは C:\Factory\Common\all.h より先にインクルードすること。
	C:\Factory\Common\all.h を中で呼んでいるので all.h のインクルードを省略しても良い。
*/

#pragma once

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wininet.lib")

#include <winsock2.h>
#include <ws2tcpip.h>

#include "C:\Factory\Common\all.h"
#include "Thread.h"

#define SOCK_SIGNATURE "X-GET\x20"

void SockPostOpen(int sock);
void SockPreClose(int sock);

int sock_hasKey(void);

char *SockIp2Line(uchar ip[4]);
void SockLine2Ip(uchar ip[4], char *strip);

extern time_t SockCurrTime;

typedef struct SockBlock_st // static member
{
	uchar *Block;
	uint BlockSize;
	uint Counter;
	time_t TransmitStartTime;
}
SockBlock_t;

typedef struct SockFile_st // static member
{
	char *File;
	uint64 FileSize;
	uint64 Counter;
	SockBlock_t *Block;
}
SockFile_t;

void SockStartup(void);
void SockCleanup(void);

extern uint SockWait_T1;
extern uint SockWait_T2;
extern uint SockWait_T3;
extern uint SockWait_T4;

extern uint64 SockTotalSendSize;
extern uint64 SockTotalRecvSize;

int SockWait(int sock, uint millis, uint forWrite);
int SockSend(int sock, uchar *buffData, uint dataSize);
int SockRecv(int sock, uchar *buffData, uint dataSize);
int SockTransmit(int sock, uchar *buffData, uint dataSize, uint waitMillis, uint forWrite);

SockBlock_t *SockCreateBlock(uint blockSize);
void SockReleaseBlock(SockBlock_t *i);
int SockTransmitBlock_WF1B(int sock, SockBlock_t *i, uint waitMillis, uint forWrite, int waitForeverFirstByte);
int SockTransmitBlock(int sock, SockBlock_t *i, uint waitMillis, uint forWrite);

SockFile_t *SockCreateFile(char *file, uint64 fileSize);
void SockReleaseFile(SockFile_t *i);
int SockSendFile(int sock, SockFile_t *i, uint waitMillis);
int SockRecvFile(int sock, SockFile_t *i, uint waitMillis);

int SockSendSequ(int sock, autoBlock_t *messageQueue, uint waitMillis);
int SockRecvSequ_RM(int sock, autoBlock_t *messageQueue, uint waitMillis, uint recvMax);
int SockRecvSequ(int sock, autoBlock_t *messageQueue, uint waitMillis);
char *SockNextLine(autoBlock_t *messageQueue);

int SockSendISequ(int sock, autoBlock_t *messageQueue, uint *pIndex, uint waitMillis);

int SockSendSequLoop(int sock, autoBlock_t *messageQueue, uint waitMillis);
int SockRecvSequLoop(int sock, autoBlock_t *messageQueue, uint waitMillis, uint maxMessageQueueSize);
int SockRecvSequLoopEnder(int sock, autoBlock_t *messageQueue, uint waitMillis, uint maxMessageQueueSize, char *endPtn);
char *SockNextLineLoop(int sock, uint waitMillis, uint maxLen);

int SockSendSequLoopET(int sock, autoBlock_t *messageQueue, uint endTimeSec);
int SockRecvSequLoopET(int sock, autoBlock_t *messageQueue, uint endTimeSec, uint maxMessageQueueSize);
