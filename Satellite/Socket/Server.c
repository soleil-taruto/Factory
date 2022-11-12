/*
	Server.exe 停止イベント名 受信ポート番号 送信先-Fortewave-Ident 受信先-Fortewave-Ident

	- - -

	終了方法

		エスケープキーを押すか、名前付きイベント「停止イベント名」をセットする。

	タイムアウト

		無し。
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Satellite\libs\Flowertact\Fortewave.h"

static uint EvStop;
static Frtwv_t *SendPipe;
static Frtwv_t *RecvPipe;

static int StopFlag;

static void CheckStop(void)
{
	if (handleWaitForMillis(EvStop, 0))
	{
		cout("停止イベントを拾いました。");
		StopFlag = 1;
	}
}
static int Perform(char *prmFile, char *ansFile)
{
	autoBlock_t *prmData;
	autoBlock_t *ansData;
	int ret = 0;

	LOGPOS();

	if (StopFlag)
		goto endFunc;

	prmData = readBinary(prmFile);

	cout("> %u\n", getSize(prmData));

	Frtwv_Clear(SendPipe);
	Frtwv_Clear(RecvPipe);
	Frtwv_Send(SendPipe, prmData);

	releaseAutoBlock(prmData);

	for (; ; )
	{
		ansData = Frtwv_Recv(RecvPipe, 2000);

		if (ansData)
		{
			cout("< %u\n", getSize(ansData));

			writeBinary(ansFile, ansData);
			releaseAutoBlock(ansData);
			ret = 1;
			break;
		}
		CheckStop();

		if (StopFlag)
			break;
	}
endFunc:
	LOGPOS();
	return ret;
}
int Idle(void)
{
	int ret = 1;

	while (hasKey())
		if (getKey() == 27) // escape
			ret = 0;

	CheckStop();

	if (StopFlag)
		ret = 0;

	return ret;
}
int main(int argc, char **argv)
{
	char *evStopName;
	uint portno;
	char *sendIdent;
	char *recvIdent;

	evStopName = nextArg();
	portno = toValue(nextArg());
	sendIdent = nextArg();
	recvIdent = nextArg();

	cout("停止イベント名 = %s\n", evStopName);
	cout("待受ポート = %u\n", portno);
	cout("送信-IDENT = %s\n", sendIdent);
	cout("受信-IDENT = %s\n", recvIdent);

	LOGPOS();

	EvStop = eventOpen(evStopName);
	SendPipe = Frtwv_Create(sendIdent);
	RecvPipe = Frtwv_Create(recvIdent);

	sockServer(Perform, portno, 20, 2100000, Idle);

	handleClose(EvStop);
	Frtwv_Release(SendPipe);
	Frtwv_Release(RecvPipe);

	LOGPOS();
}
