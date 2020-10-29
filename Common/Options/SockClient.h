#pragma once

#include "Socket.h"
#include "ConsoleColor.h"

autoList_t *sockLookupList(char *domain);
void sockLookup(uchar ip[4], char *domain);

int sockConnectEx(uchar ip[4], char *domain, uint portno, int nonBlocking);
int sockConnect(uchar ip[4], char *domain, uint portno);
int sockConnect_NB(uchar ip[4], char *domain, uint portno);
void sockDisconnect(int sock);

typedef struct sockClientStatus_st // static member
{
	SockFile_t *PrmFile;
	SockFile_t *AnsFile;
}
sockClientStatus_t;
extern sockClientStatus_t sockClientStatus;
extern uint64 sockClientAnswerFileSizeMax;

char *sockClientEx(uchar ip[4], char *domain, uint portno, char *prmFile, int (*idleFunc)(void), void (*transmitFunc)(int));
char *sockClient(uchar ip[4], char *domain, uint portno, char *prmFile, int (*idleFunc)(void));
void sockClientUserTransmit(uchar ip[4], char *domain, uint portno, void (*transmitFunc)(int));
