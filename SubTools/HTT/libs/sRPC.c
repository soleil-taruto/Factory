/*
	ServiceRPC()
		引数
			recvData
				受信データ
				変更しても良い。
				呼び出し側で開放する。
				常に非NULL

			phase
				0, 1, 2...

		戻り値
			送信データ
			NULLの場合「切断」する。
			呼び出し側で開放する。

		今回の送信データを送信し終えたら切断する場合、この関数内で RPC_SendCompleteAndDisconnect() を呼ぶこと。

	RPC_RecvSizeMax
		次の受信データのサイズの上限_[バイト]
		0 のとき、現在の送信データを送信し終えたら切断する。
		初期値は 1 MB
		ServiceRPC()実行中に設定・変更すること。

	RPC_ConnectTimeoutSec
		接続してからのタイムアウト_[秒]
		初期値は 120 秒
		ServiceRPC()実行中に設定・変更すること。

	RPC_RecvTimeoutSec
		無受信タイムアウト_[秒]
		初期値は 5 秒
		ServiceRPC()実行中に設定・変更すること。
		★1

	RPC_SendTimeoutSec
		無送信タイムアウト_[秒]
		初期値は 5 秒
		ServiceRPC()実行中に設定・変更すること。
		★1

	以下は Service.c を参照
		HttIPFile
		HttRecvFile
		HttSendFile
		HttTightFile
		HttIP
		BootTime
		ConnectTime
		LastRecvTime
		LastSendTime
		ConnectElapseTime
		LastRecvElapseTime
		LastSendElapseTime

	★1
		無受信タイムアウト・無送信タイムアウト、両方成立でタイムアウト・エラー・切断する。
		ServiceRPC(); 中に時間が掛かっても、直後の送信データの書き出しで無送信時間がリセットされる。
		-> 受信開始～終了で無通信タイムアウト・送信開始～終了で無通信タイムアウト、どちらかでタイムアウト・エラー・切断する。
*/

#include "sRPC.h"

#define PHASE_FILE "Phase.dat"
#define RECVSIZEMAX_FILE "RecvSizeMax.dat"
#define CONNECTTIMEOUT_FILE "ConnectTimeout.dat"
#define RECVTIMEOUT_FILE "RecvTimeout.dat"
#define SENDTIMEOUT_FILE "SendTimeout.dat"
#define MAIN_DIR "a"

uint RPC_RecvSizeMax;
uint RPC_ConnectTimeoutSec;
uint RPC_RecvTimeoutSec;
uint RPC_SendTimeoutSec;

void RPC_SendCompleteAndDisconnect(void) // 送信データを送信し終えたら切断する。
{
	RPC_RecvSizeMax = 0;
}

static uint ReadValueFile(char *file)
{
	char *line = readFirstLine(file);
	uint ret;

	ret = toValue(line);
	memFree(line);
	return ret;
}
static void WriteValueFile(char *file, uint value)
{
	char *line = xcout("%u", value);

	writeOneLine(file, line);
	memFree(line);
}

static void DeleteServiceNameFromHttRecvFile(void)
{
	FILE *fp = fileOpen(HttRecvFile, "rb");
	uint count;

	cout("DEL_SERVICE_NAME:[");

	for (count = 1; ; count++)
	{
		int chr = readChar(fp);

		errorCase(chr == EOF);

		if (!m_isRange(chr, 0x21, 0x7e)) // ? not ASCII
			break;

		cout("%c", chr);
	}
	cout("]\n");
	fileClose(fp);
	DeleteFileDataPart(HttRecvFile, 0, count);
}
static autoBlock_t *ReadRecvData(void)
{
	uint recvFileSize = (uint)getFileSize(HttRecvFile);
	FILE *fp;
	uint recvSize;
	autoBlock_t *recvData;

	if (recvFileSize < 4)
		return NULL;

	fp = fileOpen(HttRecvFile, "rb");
	recvSize = readValue(fp);
	cout("RPC_RECVSIZE: %u (MAX: %u)\n", recvSize, RPC_RecvSizeMax);
	errorCase_m(RPC_RecvSizeMax < recvSize, "RPC_RECVSIZE_OVERFLOW");

	if (recvFileSize - 4 < recvSize)
	{
		fileClose(fp);
		return NULL;
	}
	recvData = readBinaryStream(fp, recvSize);
	errorCase(!recvData);
	fileClose(fp);
	createFile(HttRecvFile); // クライアント側で受信を待たず次の送信をした場合、ここで削除される！
	return recvData;
}
static void WriteSendData(autoBlock_t *sendData)
{
	FILE *fp = fileOpen(HttSendFile, "ab"); // 前回の送信データが残っている場合もある -> 追記

	writeValue(fp, getSize(sendData));
	writeBinaryBlock(fp, sendData);

	fileClose(fp);
}

void ServiceMain(void)
{
	int inited = existFile(PHASE_FILE);
	uint phase;

	if (inited)
	{
		phase = ReadValueFile(PHASE_FILE);
		RPC_RecvSizeMax = ReadValueFile(RECVSIZEMAX_FILE);
		RPC_ConnectTimeoutSec = ReadValueFile(CONNECTTIMEOUT_FILE);
		RPC_RecvTimeoutSec = ReadValueFile(RECVTIMEOUT_FILE);
		RPC_SendTimeoutSec = ReadValueFile(SENDTIMEOUT_FILE);
	}
	else
	{
		phase = 0;
		RPC_RecvSizeMax = 1000000;
		RPC_ConnectTimeoutSec = 120;
		RPC_RecvTimeoutSec = 5;
		RPC_SendTimeoutSec = 5;

		DeleteServiceNameFromHttRecvFile();
		createDir(MAIN_DIR);
	}

	errorCase_m(RPC_ConnectTimeoutSec < ConnectElapseTime, "RPC_CONNECT_TIMEOUT");

#if 1
	{
		int rto = RPC_RecvTimeoutSec < LastRecvElapseTime;
		int sto = RPC_SendTimeoutSec < LastSendElapseTime;

		if (rto) cout("RPC_RECV_TIMEOUT ★片方だけならタイムアウトにならない！\n");
		if (sto) cout("RPC_SEND_TIMEOUT ★片方だけならタイムアウトにならない！\n");

		errorCase_m(rto && sto, "RPC_IO_TIMEOUT");
	}
#else // old
	errorCase_m(RPC_RecvTimeoutSec < LastRecvElapseTime, "RPC_RECV_TIMEOUT");
	errorCase_m(RPC_SendTimeoutSec < LastSendElapseTime, "RPC_SEND_TIMEOUT");
#endif

	if (!RPC_RecvSizeMax) // ? 送信し終えたら切断する。
	{
		if (getFileSize(HttSendFile) == 0) // ? 送信し終えた。
		{
			cout("RECV_SIZE_0_SendEnd_DISCONNECT_OK\n");
			ServiceDisconnect();
		}
		createFile(HttRecvFile); // もう受信しない。
	}
	else // 送受信メイン
	{
		autoBlock_t *recvData = ReadRecvData();
		autoBlock_t *sendData;

		if (!recvData)
			goto noRecvData;

		addCwd(MAIN_DIR);
		sendData = ServiceRPC(recvData, phase);
		unaddCwd();

		if (!sendData)
			ServiceDisconnect();

		WriteSendData(sendData);

		releaseAutoBlock(recvData);
		releaseAutoBlock(sendData);

		phase++;
	}

noRecvData:
	WriteValueFile(PHASE_FILE, phase);
	WriteValueFile(RECVSIZEMAX_FILE, RPC_RecvSizeMax);
	WriteValueFile(CONNECTTIMEOUT_FILE, RPC_ConnectTimeoutSec);
	WriteValueFile(RECVTIMEOUT_FILE, RPC_RecvTimeoutSec);
	WriteValueFile(SENDTIMEOUT_FILE, RPC_SendTimeoutSec);
}
void ServicePeriod(void)
{
	// noop
}
