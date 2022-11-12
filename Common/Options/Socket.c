#include "Socket.h"

static autoList_t *Handles;
static uint MaxHandleNum = 30;
static int RXHNFlags[2];

void SockPostOpen(int sock)
{
	if (!Handles)
		Handles = newList();

	addElement(Handles, (uint)sock);

	if (MaxHandleNum < getCount(Handles))
	{
		MaxHandleNum = getCount(Handles);
		cout("Information: Maximum handles updated. %u\n", MaxHandleNum);
		RXHNFlags[0] = 1;
		RXHNFlags[1] = 1;
	}
}
static void ShowHandlesInformation(uint hdlnum)
{
	if (getCount(Handles) != hdlnum)
		return;

	if (RXHNFlags[hdlnum])
	{
		cout("Information: Handles == %u <- max.\n", hdlnum);
		RXHNFlags[hdlnum] = 0;
	}
	else if (sockServerMode)
	{
		cout("Information: Handles == %u.\n", hdlnum);
	}
}
void SockPreClose(int sock)
{
	uint index;

	errorCase(!Handles);
	index = findElement(Handles, (uint)sock, simpleComp);
	errorCase(index == getCount(Handles));

	fastDesertElement(Handles, index);

	ShowHandlesInformation(0);
	ShowHandlesInformation(1);
}

#define FD_STDIN 0

/*
	hasKey() は押してるのに 0 を返すことがある。
	sock_hasKey() は押してないのに !0 を返すことがある。
*/
int sock_hasKey(void)
{
	fd_set fs;
	struct timeval tv;

	FD_ZERO(&fs);
	FD_SET(FD_STDIN, &fs);
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	return select(FD_STDIN + 1, &fs, NULL, NULL, &tv);
}

char *SockIp2Line(uchar ip[4])
{
	static char strip[16];
	sprintf(strip, "%u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
	return strip;
}
void SockLine2Ip(uchar ip[4], char *strip)
{
	autoList_t *tokens = tokenize(strip, '.');
	uint index;

	if (getCount(tokens) != 4)
		goto broken;

	for (index = 0; index < 4; index++)
	{
		uint n = toValue(getLine(tokens, index));

		if (255 < n)
			goto broken;

		ip[index] = n;
	}
	releaseDim(tokens, 1);
	return;

broken:
	releaseDim(tokens, 1);
	*(uint *)ip = 0;
}

time_t SockCurrTime;

#define SEND_MAX (512 * 1024)
#define RECV_MAX (768 * 1024)
#define FILEBLOCKSIZE (1024 * 1024)
#define BLOCKTIMEOUTSEC 180

static uint SockStartupCount;
static uint SockStartupDepth;

void SockStartup(void)
{
	WORD ver;
	WSADATA wsd;
	int retval;

	if (SockStartupDepth++)
		return;

	if (SockStartupCount++)
		cout("Warning: SockStartup() has been performed many times!\n");

	ver = MAKEWORD(2, 2);
	retval = WSAStartup(ver, &wsd);
	errorCase(retval);
}
void SockCleanup(void)
{
	errorCase(!SockStartupDepth);

	if (--SockStartupDepth)
		return;

	WSACleanup();
}

/*
	GetTickCount() の値 <- 起動から 49.7 日後のカンストは考慮しない！！！
*/
uint SockWait_T1;
uint SockWait_T2;
uint SockWait_T3;
uint SockWait_T4;

int SockWait(int sock, uint millis, uint forWrite)
{
	fd_set fs;
	struct timeval tv;
	int retval;
	uint t[4];

	t[0] = GetTickCount();

	inner_uncritical();
	{
		FD_ZERO(&fs);
		FD_SET(sock, &fs);
		tv.tv_sec = millis / 1000;
		tv.tv_usec = (millis % 1000) * 1000;

		t[1] = GetTickCount();

		retval = select(sock + 1,
			forWrite ? NULL : &fs,
			forWrite ? &fs : NULL,
			NULL,
			&tv
			);

		t[2] = GetTickCount();
	}
	inner_critical();

	t[3] = GetTickCount();

	SockWait_T1 = t[0];
	SockWait_T2 = t[1];
	SockWait_T3 = t[2];
	SockWait_T4 = t[3];

	errorCase(retval < -1 || 1 < retval);
	return retval;
}

uint64 SockTotalSendSize;
uint64 SockTotalRecvSize;

int SockSend(int sock, uchar *buffData, uint dataSize)
{
	int retval = send(sock, buffData, m_min(dataSize, SEND_MAX), 0);

	if (1 <= retval)
		SockTotalSendSize += (uint64)retval;

	return retval;
}
int SockRecv(int sock, uchar *buffData, uint dataSize)
{
	int retval = recv(sock, buffData, dataSize, 0);

	if (1 <= retval)
		SockTotalRecvSize += (uint64)retval;

	return retval;
}
int SockTransmit(int sock, uchar *buffData, uint dataSize, uint waitMillis, uint forWrite) // ret: -1 == エラー
{
	int retval;

	if (dataSize == 0)
		return 0;

	retval = SockWait(sock, waitMillis, forWrite);

	if (retval <= 0)
		goto endfunc;

// test
{
uint t = now();
	retval = (forWrite ? SockSend : SockRecv)(sock, buffData, dataSize);
errorCase(t + 2 < now());
}
	errorCase(retval < -1 || (sint)dataSize < retval);

	if (retval == 0) // ? select() が 1 を返したのに、0 バイト -> sock が相手側で閉じられたときの挙動
		retval = -1;

endfunc:
	return retval;
}

SockBlock_t *SockCreateBlock(uint blockSize)
{
	SockBlock_t *i = (SockBlock_t *)memAlloc(sizeof(SockBlock_t));

	i->Block = memAlloc(blockSize);
	i->BlockSize = blockSize;
	i->Counter = 0;
	i->TransmitStartTime = 0L;

	return i;
}
void SockReleaseBlock(SockBlock_t *i)
{
	memFree(i->Block);
	memFree(i);
}
int SockTransmitBlock_WF1B(int sock, SockBlock_t *i, uint waitMillis, uint forWrite, int waitForeverFirstByte)
{
	int retsize = SockTransmit(sock, i->Block + i->Counter, i->BlockSize - i->Counter, waitMillis, forWrite);

	if (0 < retsize)
	{
		i->Counter += retsize;
	}
	if (0 <= retsize && i->Counter < i->BlockSize && (!waitForeverFirstByte || i->Counter)) // timeout check
	{
		if (!i->TransmitStartTime)
			i->TransmitStartTime = SockCurrTime;

		if (i->TransmitStartTime + BLOCKTIMEOUTSEC < SockCurrTime) // ? timeout
			return -1;
	}
	return retsize;
}
int SockTransmitBlock(int sock, SockBlock_t *i, uint waitMillis, uint forWrite)
{
	return SockTransmitBlock_WF1B(sock, i, waitMillis, forWrite, 0);
}

SockFile_t *SockCreateFile(char *file, uint64 fileSize) // Grip file!
{
	SockFile_t *i = (SockFile_t *)memAlloc(sizeof(SockFile_t));

	i->File = file;
	i->FileSize = fileSize;
	i->Counter = 0;
	i->Block = NULL;

	return i;
}
void SockReleaseFile(SockFile_t *i)
{
	if (i->Block)
		SockReleaseBlock(i->Block);

	removeFile(i->File);

	memFree(i->File);
	memFree(i);
}
int SockSendFile(int sock, SockFile_t *i, uint waitMillis)
{
	int retsize;

	if (!i->Block)
	{
		FILE *fp = fileOpen(i->File, "rb");
		uint blockSize;
		uint64 bs2;

		bs2 = i->FileSize - i->Counter;
		bs2 = m_min(bs2, (uint64)FILEBLOCKSIZE);
		blockSize = (uint)bs2;

		i->Block = SockCreateBlock(blockSize);

		if (_fseeki64(fp, i->Counter, SEEK_SET) != 0)
		{
			error();
		}
		if (fread(i->Block->Block, 1, blockSize, fp) != blockSize)
		{
			error();
		}
		fileClose(fp);
	}
	retsize = SockTransmitBlock(sock, i->Block, waitMillis, 1);

	if (i->Block->Counter == i->Block->BlockSize)
	{
		i->Counter += (uint64)i->Block->BlockSize;

		SockReleaseBlock(i->Block);
		i->Block = NULL;
	}
	return retsize;
}
int SockRecvFile(int sock, SockFile_t *i, uint waitMillis)
{
	int retsize;

	if (!i->Block)
	{
		uint blockSize;
		uint64 bs2;

		bs2 = i->FileSize - i->Counter;
		bs2 = m_min(bs2, (uint64)FILEBLOCKSIZE);
		blockSize = (uint)bs2;

		i->Block = SockCreateBlock(blockSize);
	}
	retsize = SockTransmitBlock(sock, i->Block, waitMillis, 0);

	if (i->Block->Counter == i->Block->BlockSize)
	{
		FILE *fp = fileOpen(i->File, "ab");

		if (fwrite(i->Block->Block, 1, i->Block->BlockSize, fp) != i->Block->BlockSize)
		{
			error();
		}
		fileClose(fp);

		i->Counter += (uint64)i->Block->BlockSize;

		SockReleaseBlock(i->Block);
		i->Block = NULL;
	}
	return retsize;
}

int SockSendSequ(int sock, autoBlock_t *messageQueue, uint waitMillis)
{
	int retval = SockTransmit(sock, directGetBuffer(messageQueue), getSize(messageQueue), waitMillis, 1);

	if (0 < retval)
	{
		removeBytes(messageQueue, 0, retval);
	}
	return retval;
}
int SockRecvSequ_RM(int sock, autoBlock_t *messageQueue, uint waitMillis, uint recvMax)
{
	int retval;
	uint retsize;

	nobSetSize(messageQueue, getSize(messageQueue) + recvMax);
	retval = SockTransmit(sock, (uchar *)directGetBuffer(messageQueue) + getSize(messageQueue) - recvMax, recvMax, waitMillis, 0);
	retsize = retval == -1 ? 0 : retval;
	setSize(messageQueue, getSize(messageQueue) - recvMax + retsize);

	return retval;
}
int SockRecvSequ(int sock, autoBlock_t *messageQueue, uint waitMillis)
{
	return SockRecvSequ_RM(sock, messageQueue, waitMillis, RECV_MAX);
}
char *SockNextLine(autoBlock_t *messageQueue) // ret: NULL == まだ次の行が無い。
{
	uint index;

	for (index = 0; index < getSize(messageQueue); index++)
	{
		if (getByte(messageQueue, index) == '\n') // ? LF
		{
			char *line = ab_makeLine_x(ab_makeSubBytes(
				messageQueue,
				0,
				index && getByte(messageQueue, index - 1) == '\r' ? index - 1 : index // CR を除ける
				));

			removeBytes(messageQueue, 0, index + 1);
			return line;
		}
	}
	return NULL;
}

/*
	getSize(messageQueue) == *pIndex で、送信完了
	ret == -1: エラー
*/
int SockSendISequ(int sock, autoBlock_t *messageQueue, uint *pIndex, uint waitMillis)
{
	uint index = *pIndex;
	int retval;

//	errorCase(getSize(messageQueue) < index);
	retval = SockTransmit(sock, (uchar *)directGetBuffer(messageQueue) + index, getSize(messageQueue) - index, waitMillis, 1);

	if (0 < retval)
	{
		index += retval;
//		errorCase(getSize(messageQueue) < index);
		*pIndex = index;
	}
	return retval;
}

/*
	messageQueue を全て送り終えるまで送信を試みる。

	ret:
		-1 == エラー又は切断
		0... == 送信したバイト数
		waitMillis == 0 の時は、何もせず 0 を返す。
*/
int SockSendSequLoop(int sock, autoBlock_t *messageQueue, uint waitMillis)
{
	uint64 startTick = nowTick();
	uint passedTick = 0;
	int retval = 0;

	while (getSize(messageQueue) && passedTick < waitMillis)
	{
		int ret = SockSendSequ(sock, messageQueue, waitMillis - passedTick);

		if (ret == -1)
		{
			retval = -1;
			break;
		}
		retval += ret;
		passedTick = (uint)getUDiff64(startTick, nowTick());
	}
	return retval;
}
/*
	messageQueue が maxMessageQueueSize になるまで受信を試みる。

	ret:
		-1 == エラー又は切断
		0... == 受信したバイト数
		waitMillis == 0 の時は、何もせず 0 を返す。
*/
int SockRecvSequLoop(int sock, autoBlock_t *messageQueue, uint waitMillis, uint maxMessageQueueSize)
{
	uint64 startTick = nowTick();
	uint passedTick = 0;
	int retval = 0;

	while (getSize(messageQueue) < maxMessageQueueSize && passedTick < waitMillis)
	{
		int ret = SockRecvSequ_RM(sock, messageQueue, waitMillis - passedTick, maxMessageQueueSize - getSize(messageQueue));

		if (ret == -1)
		{
			retval = -1;
			break;
		}
		retval += ret;
		passedTick = (uint)getUDiff64(startTick, nowTick());
	}
	return retval;
}
/*
	messageQueue が maxMessageQueueSize になるか endPtn が出現するまで受信を試みる。

	ret:
		-1 == エラー又は切断
		0... == 受信したバイト数
		waitMillis == 0 の時は、何もせず 0 を返す。
*/
int SockRecvSequLoopEnder(int sock, autoBlock_t *messageQueue, uint waitMillis, uint maxMessageQueueSize, char *endPtn)
{
	uint endPtnLen = strlen(endPtn);
	uint64 startTick = nowTick();
	uint passedTick = 0;
	int retval = 0;

	if (!*endPtn)
		return 0;

	while (getSize(messageQueue) < maxMessageQueueSize && passedTick < waitMillis)
	{
		int ret = SockRecvSequ_RM(sock, messageQueue, waitMillis - passedTick, 1);

		if (ret == -1)
		{
			retval = -1;
			break;
		}
		if (ret == 1)
		{
			retval++;

			if (strlen(endPtn) <= getSize(messageQueue))
			{
				if (!memcmp(
					(uchar *)directGetBuffer(messageQueue) + getSize(messageQueue) - strlen(endPtn),
					endPtn,
					strlen(endPtn)
					))
				{
					break;
				}
			}
		}
		passedTick = (uint)getUDiff64(startTick, nowTick());
	}
	return retval;
}
char *SockNextLineLoop(int sock, uint waitMillis, uint maxLen)
{
	autoBlock_t *buff = newBlock();
	int retval;
	char *line;

	retval = SockRecvSequLoopEnder(sock, buff, waitMillis, maxLen + 2, "\r\n");

	if (retval == -1 && getSize(buff) == 0)
	{
		releaseAutoBlock(buff);
		return NULL;
	}
	line = unbindBlock2Line(buff);
	disctrl(line); // remove CRLF
	return line;
}

static uint ET2WM(uint endTime)
{
	uint nowTime = now();

	if (endTime < nowTime)
		return 0;

	if (endTime == nowTime)
		return 500;

	return (endTime - nowTime) * 1000;
}
int SockSendSequLoopET(int sock, autoBlock_t *messageQueue, uint endTime)
{
	return SockSendSequLoop(sock, messageQueue, ET2WM(endTime));
}
int SockRecvSequLoopET(int sock, autoBlock_t *messageQueue, uint endTime, uint maxMessageQueueSize)
{
	return SockRecvSequLoop(sock, messageQueue, ET2WM(endTime), maxMessageQueueSize);
}
