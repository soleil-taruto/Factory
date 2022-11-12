#include "SockStream.h"

#define R_BUFF_SIZE (1024 *  3)
#define W_BUFF_SIZE (1024 * 64)

#define R_TIMEOUT 3000
#define W_TIMEOUT 3000

/*
	sock == -1 のとき、最初から閉じているソケットとして扱う。

	timeout: タイムアウトまでの秒数, 0 == 無制限
	blockTimeout: 無通信タイムアウトまでの秒数, 0 == 無制限
	recvSizeLimiter: 総受信サイズの上限, 0 == 無制限
*/
SockStream_t *CreateSockStream2(int sock, uint timeout, uint blockTimeout, uint64 recvSizeLimiter)
{
	SockStream_t *i = CreateSockStream(sock, timeout);

	SetSockStreamBlockTimeout(i, blockTimeout);
	i->Extra.RecvSizeLimiter = recvSizeLimiter;
	return i;
}

/*
	sock == -1 のとき、最初から閉じているソケットとして扱う。
*/
SockStream_t *CreateSockStream(int sock, uint timeout) // timeout: タイムアウトまでの秒数, 0: 無制限
{
	SockStream_t *i = (SockStream_t *)memAlloc(sizeof(SockStream_t));

	memset(i, 0x00, sizeof(SockStream_t));

	i->Sock = sock;
	i->R_Buff.Buffer = (uchar *)memAlloc(R_BUFF_SIZE);
	i->W_Buff.Buffer = (uchar *)memAlloc(W_BUFF_SIZE);
//	i->Timeout = 0;
//	i->BlockTimeout = 0;
//	i->CurrBlockTimeout = 0;

	SetSockStreamTimeout(i, timeout);

	return i;
}
void ReleaseSockStream(SockStream_t *i)
{
	memFree(i->R_Buff.Buffer);
	memFree(i->W_Buff.Buffer);
	memFree(i);
}
void SetSockStreamTimeout(SockStream_t *i, uint timeout) // timeout: 0 == 無制限
{
	if (timeout)
		i->Timeout = now() + timeout;
	else
		i->Timeout = UINTMAX;
}
uint GetSockStreamTimeout(SockStream_t *i) // ret: 0 == 無制限
{
	if (i->Timeout == UINTMAX)
		return 0;

	return i->Timeout;
}
void SetSockStreamBlockTimeout(SockStream_t *i, uint timeout) // timeout: 0 == 無制限
{
	i->BlockTimeout = timeout;
}
void DestroySockStream(SockStream_t *i) // i を閉じた状態にし、これ以上通信を行わないようにする。
{
	i->Sock = -1;
}
/*
	i が閉じられていても、未読バッファがあれば SockRecvChar() から読み込めるので注意！
	Sock はデータを読み込もうとして失敗するか、意図的に DestroySockStream() しない限り閉じられる (-1をセットされる) ことはないので、
	エラー判定として使えるかもしれない。
*/
int IsEOFSockStream(SockStream_t *i)
{
	return i->Sock == -1;
}
int GetSockStreamSock(SockStream_t *i)
{
	errorCase(i->Sock == -1);
	return i->Sock;
}

static int IsTimeout(SockStream_t *i, int blockFlag)
{
	uint nowTime = now();

	if (i->Timeout < nowTime)
		return 1;

	if (blockFlag && i->BlockTimeout)
	{
		if (!i->CurrBlockTimeout)
			i->CurrBlockTimeout = nowTime + i->BlockTimeout;

		{
			uint a = nowTime;
			uint b = i->CurrBlockTimeout;

//			cout("BLOCK_TIMEOUT: %u -> %u (%u)\n", a, b, b - a);
		}

		if (i->CurrBlockTimeout < nowTime)
			return 1;
	}
	return 0;
}
static void ResetBlockTimeout(SockStream_t *i)
{
	i->CurrBlockTimeout = 0;
}

void (*SockSendInterlude)(void); // SockFlush() 毎に呼ぶ。
void (*SockRecvInterlude)(void); // 読み込みバッファが尽きる度にに呼ぶ。(R_BUFF_SIZE バイト読み込む度ではないことに注意)

int SockRecvCharWait(SockStream_t *i, uint timeout) // ret: ? SockRecvChar() が文字か EOF をすぐに返せる。
{
	if (IsTimeout(i, 0))
		i->Sock = -1;

	return
		i->R_Buff.Index < i->R_Buff.Size ||
		i->Sock == -1 ||
		SockWait(i->Sock, timeout, 0) != 0;
}
int SockRecvChar(SockStream_t *i)
{
	int retval;

	if (i->R_Buff.Index < i->R_Buff.Size)
	{
		return i->R_Buff.Buffer[i->R_Buff.Index++];
	}
	if (SockRecvInterlude)
	{
		SockRecvInterlude();
	}
	if (i->Sock == -1) // ? 無効
	{
		goto sockClosed;
	}
	ResetBlockTimeout(i);

	for (; ; )
	{
		if (IsTimeout(i, 1))
		{
			break;
		}
		retval = SockTransmit(i->Sock, i->R_Buff.Buffer, R_BUFF_SIZE, R_TIMEOUT, 0);

		if (retval == -1)
		{
			break;
		}
		if (retval != 0)
		{
			errorCase(retval < 1 || R_BUFF_SIZE < retval);

			i->R_Buff.Size = retval;
			i->R_Buff.Index = 1;

			if (i->Extra.RecvSizeLimiter != 0UI64)
			{
				if (i->Extra.RecvSizeLimiter <= retval) // 0 で閉じる <- 0 になると無効になるので..
					i->Sock = -1;
				else
					i->Extra.RecvSizeLimiter -= retval;
			}
			return i->R_Buff.Buffer[0];
		}
	}
	i->Sock = -1;

sockClosed:
	return EOF;
}
int TrySockFlush(SockStream_t *i, uint timeout) // ret: ? これ以上送信する必要はない。
{
	int retval;

	if (IsTimeout(i, 0))
	{
		i->Sock = -1;
		return 1;
	}
	if (i->Sock == -1)
		return 1;

	if (i->W_Buff.Size == 0)
		return 1;

	if (SockSendInterlude)
		SockSendInterlude();

	retval = SockTransmit(i->Sock, i->W_Buff.Buffer, i->W_Buff.Size, timeout, 1);

	if (retval == -1)
	{
		i->Sock = -1;
		return 1;
	}
	errorCase(retval < 0 || i->W_Buff.Size < retval);

	i->W_Buff.Size -= retval;
	copyBlock(i->W_Buff.Buffer, i->W_Buff.Buffer + retval, i->W_Buff.Size);

	return i->W_Buff.Size == 0;
}
int SockSendCharWait(SockStream_t *i, uint timeout) // ret: ? SockSendChar() が１文字処理してすぐに制御を返せるか、切断した・している。
{
	if (IsTimeout(i, 0))
		i->Sock = -1;

	return
		i->W_Buff.Size < W_BUFF_SIZE ||
		TrySockFlush(i, timeout) ||
		i->W_Buff.Size < W_BUFF_SIZE;
}
void SockSendChar(SockStream_t *i, int chr)
{
	if (i->W_Buff.Size == W_BUFF_SIZE)
	{
		SockFlush(i);
	}
	i->W_Buff.Buffer[i->W_Buff.Size++] = chr;
}
void SockFlush(SockStream_t *i)
{
	int retval;

	if (SockSendInterlude)
	{
		SockSendInterlude();
	}
	if (i->Sock == -1) // ? 無効
	{
		goto writeEnd;
	}
	i->W_Buff.Index = 0;

	ResetBlockTimeout(i);

	for (; ; )
	{
		if (i->W_Buff.Index == i->W_Buff.Size)
		{
			goto writeEnd;
		}
		if (IsTimeout(i, 1))
		{
			break;
		}
		retval = SockTransmit(i->Sock, i->W_Buff.Buffer + i->W_Buff.Index, i->W_Buff.Size - i->W_Buff.Index, W_TIMEOUT, 1);

		if (retval == -1)
		{
			break;
		}
		errorCase(retval < 0 || (int)(i->W_Buff.Size - i->W_Buff.Index) < retval);

		i->W_Buff.Index += retval;
	}
	i->Sock = -1;

writeEnd:
	i->W_Buff.Size = 0;
//	i->W_Buff.Index = 0;
}
autoBlock_t *SockRipRecvBuffer(SockStream_t *i)
{
#if 1
	autoBlock_t *rippedBuffer = recreateBlock(i->R_Buff.Buffer + i->R_Buff.Index, i->R_Buff.Size - i->R_Buff.Index);

	i->R_Buff.Index = i->R_Buff.Size;
	return rippedBuffer;
#else // 同じコード..だと思う..
	autoBlock_t *rippedBuffer = newBlock();

	for (; i->R_Buff.Index < i->R_Buff.Size; i->R_Buff.Index++)
	{
		addByte(rippedBuffer, i->R_Buff.Buffer[i->R_Buff.Index]);
	}
	return rippedBuffer;
#endif
}

char *SockRecvLine(SockStream_t *i, uint lenmax)
{
	autoBlock_t *buffer = newBlock();
	char *line;
	int chr;

	for (; ; )
	{
		chr = SockRecvChar(i);

		if (chr == '\r') // CR
		{
			continue;
		}
		if (chr == EOF || chr == '\0' || chr == '\n') // LF
		{
			break;
		}
		if (lenmax <= getSize(buffer)) // Overflow -> 改行と見なす。
		{
			break;
		}
		addByte(buffer, chr);
	}
	addByte(buffer, '\0');
	line = unbindBlock(buffer);
	return line;
}
void SockSendBlock(SockStream_t *i, void *block, uint blockSize)
{
	uint index;

	for (index = 0; index < blockSize; index++)
	{
		SockSendChar(i, ((uchar *)block)[index]);
	}
}
void SockSendToken(SockStream_t *i, char *token)
{
	SockSendBlock(i, token, strlen(token));
}
void SockSendLine(SockStream_t *i, char *line)
{
	SockSendToken(i, line);
	SockSendToken(i, "\r\n"); // CR-LF
	SockFlush(i);
}
void SockSendLine_NF(SockStream_t *i, char *line)
{
	SockSendToken(i, line);
	SockSendToken(i, "\r\n"); // CR-LF
}
void SockSendValue(SockStream_t *i, uint value)
{
	uchar block[4];

	valueToBlock(block, value);
	SockSendBlock(i, block, 4);
}
void SockSendValue64(SockStream_t *i, uint64 value)
{
	uchar block[8];

	value64ToBlock(block, value);
	SockSendBlock(i, block, 8);
}

uint SockRecvBlock_LastRecvSize;

int SockRecvBlock(SockStream_t *i, void *block, uint blockSize) // ret: EOFを読み込まなかった。
{
	uint index;

	for (index = 0; index < blockSize; index++)
	{
		int chr = SockRecvChar(i);

		if (chr == EOF)
		{
			memset((uchar *)block + index, 0x00, blockSize - index);
			SockRecvBlock_LastRecvSize = index;
			return 0;
		}
		((uchar *)block)[index] = chr;
	}
	SockRecvBlock_LastRecvSize = blockSize;
	return 1;
}
uint SockRecvValue(SockStream_t *i)
{
	uchar block[4];

	SockRecvBlock(i, block, 4);
	return blockToValue(block);
}
uint64 SockRecvValue64(SockStream_t *i)
{
	uchar block[8];

	SockRecvBlock(i, block, 8);
	return blockToValue64(block);
}

// _x
void SockSendToken_x(SockStream_t *i, char *token)
{
	SockSendToken(i, token);
	memFree(token);
}
void SockSendLine_x(SockStream_t *i, char *line)
{
	SockSendLine(i, line);
	memFree(line);
}
