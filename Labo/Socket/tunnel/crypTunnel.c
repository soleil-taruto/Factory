/*
	crypTunnel.exe RECV-PORT FWD-HOST FWD-PORT /S

		停止する。

	crypTunnel.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/R] [/KW KEY-WIDTH] [/0] [/BSL BLOCK-SIZE-LIMIT] [/T] KEY-BUNDLE-FILE

		RECV-PORT       ... 待ち受けポート番号
		FWD-DOMAIN      ... 転送先ホスト名
		FWD-PORT        ... 転送先ポート番号
		CONNECT-MAX     ... 最大同時接続数, デフォルト: 50
		/R              ... リバースモード
		KEY-WIDTH       ... 鍵幅 (16, 24, 32) デフォルト: 32
		KEY-BUNDLE-FILE ... 鍵束ファイル ★1

		★1 * を指定すると手入力 -> 入力された文字列の SHA-512 を鍵とする。(p1kと同じ)
			*PASS を指定すると PASS の SHA-512 を鍵とする。(PASSは任意の文字列)

		/0               ... 数秒おきにゼロバイトのブロックを暗号化したデータを送り、送信が途絶する期間を数秒以内に留める。
		BLOCK-SIZE-LIMIT ... 一度に暗号化するブロックサイズの上限を設定します, デフォルト: 無制限
		/T               ... GeTunnelと併用したとき、長時間の無通信によるタイムアウトが頻発する場合これを指定して下さい。
*/

#include "libs\Tunnel.h"
#include "C:\Factory\Common\Options\CryptoRand.h"
#include "C:\Factory\Common\Options\RingCipher2.h"

#define CT_DEF_CONNECT_MAX 50
#define BLOCKSIZEMAX 1000000 // 送受信ブロックサイズの上限, Socket.c で 768 KB に制限されているので、パディングしても 1 MB は超えないだろう...
#define COUNTER_SIZE 64

static int ReverseMode;
static int SendZeroBytesMode;
static uint KeyWidth = 32;
static char *KeyBundleFile;
static autoBlock_t *KeyBundle;
static autoList_t *KeyTableList;
static uint BlockSizeLimit;
static uint BlockSizeMin = 1;
static autoBlock_t *CliVerifyPtn;

static uint GetNegotiationTimeoutMillis(void)
{
	return IsTight() ? 2000 : 60000;
}
static void IncrementCounter(uchar counter[COUNTER_SIZE])
{
	uint index;

	for (index = 0; index < COUNTER_SIZE; index++)
	{
		if (counter[index] < 0xff)
		{
			counter[index]++;
			break;
		}
		counter[index] = 0;
	}
}

static void EncryptFltr_Real(autoBlock_t *buff, uint encCounter)
{
	autoBlock_t *nBuff;
	uint origSize;

	if (!SendZeroBytesMode && !getSize(buff))
		return;

	origSize = getSize(buff);

	ab_addBlock(buff, (uchar *)encCounter, COUNTER_SIZE);
	IncrementCounter((uchar *)encCounter);
	rngcphrEncrypt(buff, KeyTableList);

	cout("E %u -> %u\n", origSize, getSize(buff));

	errorCase(BLOCKSIZEMAX < getSize(buff));

	nBuff = newBlock();

	ab_addValue(nBuff, getSize(buff));
	addBytes(nBuff, buff);

	ab_swap(nBuff, buff);

	releaseAutoBlock(nBuff);
}
static void EncryptFltr(autoBlock_t *buff, uint encCounter)
{
	if (BlockSizeLimit && BlockSizeLimit < getSize(buff))
	{
		autoBlock_t *dest = newBlock();
		uint rPos;
		uint buffSize = getSize(buff);

		for (rPos = 0; rPos < buffSize; )
		{
			uint size = buffSize - rPos;
			autoBlock_t *tmp;

			if (BlockSizeLimit < size)
			{
				size = (uint)(getCryptoRand64() % BlockSizeLimit);
				m_maxim(size, BlockSizeMin);
			}
			tmp = ab_makeSubBytes(buff, rPos, size);

			cout("DIV %u -> %u %u ", buffSize, rPos, size);

			EncryptFltr_Real(tmp, encCounter);

			ab_addBytes_x(dest, tmp);
			rPos += size;
		}
		ab_swap(dest, buff);

		releaseAutoBlock(dest);
	}
	else
	{
		EncryptFltr_Real(buff, encCounter);
	}
}
static void DecryptFltr(autoBlock_t *buff, uint decInfo)
{
	uchar *decCounter = ((uchar **)decInfo)[0];
	autoBlock_t *wBuff = ((autoBlock_t **)decInfo)[1];

	addBytes(wBuff, buff);
	setSize(buff, 0);

restart:
	if (sizeof(uint) <= getSize(wBuff))
	{
		uint size = ab_getValue(wBuff, 0);

		if (BLOCKSIZEMAX < size)
		{
			ChannelDeadFlag = 1;
		}
		else if (sizeof(uint) + size <= getSize(wBuff))
		{
			autoBlock_t *buffPart = ab_makeSubBytes(wBuff, sizeof(uint), size);

			ChannelDeadFlag = 1;
			removeBytes(wBuff, 0, sizeof(uint) + size);

			if (
				rngcphrDecrypt(buffPart, KeyTableList) &&
				COUNTER_SIZE <= getSize(buffPart)
				)
			{
				autoBlock_t *wc = unaddBytes(buffPart, COUNTER_SIZE);

				if (!memcmp(decCounter, directGetBuffer(wc), COUNTER_SIZE))
				{
					cout("D %u -> %u\n", size, getSize(buffPart));

					IncrementCounter(decCounter);
					addBytes(buff, buffPart);
					ChannelDeadFlag = 0;
				}
				releaseAutoBlock(wc);
			}
			releaseAutoBlock(buffPart);

			if (!ChannelDeadFlag)
				goto restart;
		}
	}
}

/*
	block - 0～4080バイト && 16の倍数バイト
*/
static int NgtSendBlockLoop(int sock, autoBlock_t *block) // ret: ? 成功
{
	autoBlock_t *szBlock = newBlock();
	uint size = getSize(block);
	int retval;

	LOGPOS();

	addByte(szBlock, size / 16);

	retval =
		SockSendSequLoop(sock, szBlock, GetNegotiationTimeoutMillis()) == 1 &&
		SockSendSequLoop(sock, block, GetNegotiationTimeoutMillis()) == size;

	releaseAutoBlock(szBlock);
	return retval;
}
static int NgtRecvBlockLoop(int sock, autoBlock_t *block) // ret: ? 成功
{
	autoBlock_t *szBlock = newBlock();
	int retval = 0;

	LOGPOS();

	if (SockRecvSequLoop(sock, szBlock, GetNegotiationTimeoutMillis(), 1) == 1)
	{
		uint size = getByte(szBlock, 0) * 16;

		retval = SockRecvSequLoop(sock, block, GetNegotiationTimeoutMillis(), size) == size;
	}
	releaseAutoBlock(szBlock);
	return retval;
}

static void PerformTh(int sock, char *strip)
{
	int fwdSock = -1;
	int encSock;
	int noEncSock;
	autoBlock_t *dc = NULL;
	autoBlock_t *ec = NULL;
	autoBlock_t *wc = NULL;
	uchar decCounter[COUNTER_SIZE];
	uchar encCounter[COUNTER_SIZE];

	if (CliVerifyPtn)
	{
		static autoBlock_t *rvBlock;

		LOGPOS();

		if (!rvBlock)
			rvBlock = newBlock();

		setSize(rvBlock, 0);

		if (SockRecvSequLoop(sock, rvBlock, 2000, getSize(CliVerifyPtn)) != getSize(CliVerifyPtn))
		{
			LOGPOS();
			return;
		}
		if (!isSameBlock(CliVerifyPtn, rvBlock))
		{
			LOGPOS();
			return;
		}
		LOGPOS();
	}

	if (ReverseMode)
	{
		encSock = sock;
	}
	else
	{
		uchar ip[4] = { 0 };

		fwdSock = sockConnect(ip, FwdHost, FwdPortNo);

		if (fwdSock == -1)
			goto disconnect;

		encSock = fwdSock;
		noEncSock = sock;
	}

	// Negotiation
	{
		dc = makeCryptoRandBlock(COUNTER_SIZE);
		ec = newBlock();
		wc = newBlock();

		memcpy(decCounter, directGetBuffer(dc), COUNTER_SIZE);
		rngcphrEncrypt(dc, KeyTableList);

		if (
			!NgtSendBlockLoop(encSock, dc) ||
			!NgtRecvBlockLoop(encSock, ec) ||
			!rngcphrDecrypt(ec, KeyTableList) ||
			getSize(ec) != COUNTER_SIZE
			)
			goto disconnect;

		memcpy(encCounter, directGetBuffer(ec), COUNTER_SIZE);
		IncrementCounter(encCounter);
		memcpy(directGetBuffer(ec), encCounter, COUNTER_SIZE);
		rngcphrEncrypt(ec, KeyTableList);

		if (
			!NgtSendBlockLoop(encSock, ec) ||
			!NgtRecvBlockLoop(encSock, wc) ||
			!rngcphrDecrypt(wc, KeyTableList) ||
			getSize(wc) != COUNTER_SIZE
			)
			goto disconnect;

		IncrementCounter(decCounter);

		if (memcmp(decCounter, directGetBuffer(wc), COUNTER_SIZE))
			goto disconnect;

		releaseAutoBlock(dc);
		releaseAutoBlock(ec);
		releaseAutoBlock(wc);

		dc = NULL;
		ec = NULL;
		wc = NULL;
	}

	if (ReverseMode)
	{
		uchar ip[4] = { 0 };

		fwdSock = sockConnect(ip, FwdHost, FwdPortNo);

		if (fwdSock == -1)
			goto disconnect;

		noEncSock = fwdSock;
	}

	IncrementCounter(decCounter);
	IncrementCounter(encCounter);

	{
		void *decInfo[2];

		decInfo[0] = decCounter;
		decInfo[1] = newBlock();

		CrossChannel(noEncSock, encSock, EncryptFltr, (uint)encCounter, DecryptFltr, (uint)decInfo);

		releaseAutoBlock((autoBlock_t *)decInfo[1]);
	}

disconnect:
	if (fwdSock != -1)
		sockDisconnect(fwdSock);

	if (dc)
		releaseAutoBlock(dc);

	if (ec)
		releaseAutoBlock(ec);

	if (wc)
		releaseAutoBlock(wc);
}
static int ReadArgs(void)
{
	if (argIs("/R"))
	{
		ReverseMode = 1;
		return 1;
	}
	if (argIs("/KW"))
	{
		KeyWidth = toValue(nextArg());
		return 1;
	}
	if (argIs("/0"))
	{
		SendZeroBytesMode = 1;
		return 1;
	}
	if (argIs("/BSL"))
	{
		BlockSizeLimit = toValue(nextArg());
		return 1;
	}
	if (argIs("/T")) // geTunnel combination mode
	{
		BlockSizeLimit = 5000;
		return 1;
	}
	if (argIs("/T2")) // for BlueFish
	{
		BlockSizeLimit = 60000;
		BlockSizeMin   = 50000;
		return 1;
	}
	if (argIs("/CVP")) // for Hechima etc.
	{
		CliVerifyPtn = ab_makeBlockLine(nextArg());
		return 1;
	}
	KeyBundleFile = nextArg();

	errorCase_m(hasArgs(1), "不明なコマンド引数");

	errorCase(
		KeyWidth != 16 &&
		KeyWidth != 24 &&
		KeyWidth != 32
		);

	KeyBundle = cphrLoadKeyBundleFileEx(KeyBundleFile);
	KeyTableList = cphrCreateKeyTableList(KeyBundle, KeyWidth);

	return 0;
}
static char *GetTitleSuffix(void)
{
	return xcout("[%s]", ReverseMode ? "Dec" : "Enc");
}
int main(int argc, char **argv)
{
	ConnectMax = CT_DEF_CONNECT_MAX;

	TunnelPerformTh = PerformTh;
	TunnelMain(ReadArgs, NULL, "crypTunnel", GetTitleSuffix);
	TunnelPerformTh = NULL;
}
