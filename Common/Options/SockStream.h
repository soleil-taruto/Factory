#pragma once

#include "Socket.h"

typedef struct SockBuffer_st // static member
{
	uchar *Buffer;
	uint Size;
	uint Index;
}
SockBuffer_t;

typedef struct SockStream_st // static member
{
	int Sock;
	SockBuffer_t R_Buff;
	SockBuffer_t W_Buff;
	uint Timeout;
	uint BlockTimeout;
	uint CurrBlockTimeout;

	struct // pub_member
	{
		char *SaveFile;
		char *ServerName;
		int M4UServerMode_Operated;
		uint64 RecvSizeLimiter;
	}
	Extra;
}
SockStream_t;

SockStream_t *CreateSockStream2(int sock, uint timeout, uint blockTimeout, uint64 recvSizeLimiter);
SockStream_t *CreateSockStream(int sock, uint timeout);
void ReleaseSockStream(SockStream_t *i);
void DestroySockStream(SockStream_t *i);
int IsEOFSockStream(SockStream_t *i);
int GetSockStreamSock(SockStream_t *i);
void SetSockStreamTimeout(SockStream_t *i, uint timeout);
uint GetSockStreamTimeout(SockStream_t *i);
void SetSockStreamBlockTimeout(SockStream_t *i, uint timeout);

extern void (*SockSendInterlude)(void);
extern void (*SockRecvInterlude)(void);

int SockRecvCharWait(SockStream_t *i, uint timeout);
int SockRecvChar(SockStream_t *i);
int SockSendCharWait(SockStream_t *i, uint timeout);
void SockSendChar(SockStream_t *i, int chr);
void SockFlush(SockStream_t *i);
int TrySockFlush(SockStream_t *i, uint timeout);
autoBlock_t *SockRipRecvBuffer(SockStream_t *i);

char *SockRecvLine(SockStream_t *i, uint lenmax);
void SockSendToken(SockStream_t *i, char *token);
void SockSendLine(SockStream_t *i, char *line);
void SockSendLine_NF(SockStream_t *i, char *line);

extern uint SockRecvBlock_LastRecvSize;

int SockRecvBlock(SockStream_t *i, void *block, uint blockSize);
void SockSendBlock(SockStream_t *i, void *block, uint blockSize);

uint SockRecvValue(SockStream_t *i);
void SockSendValue(SockStream_t *i, uint value);
void SockSendValue64(SockStream_t *i, uint64 value);
uint64 SockRecvValue64(SockStream_t *i);

// _x
void SockSendToken_x(SockStream_t *i, char *token);
void SockSendLine_x(SockStream_t *i, char *line);
