/*
	ServiceMain()
		引数
			なし

		戻り値
			なし

	HttIPFile
		HTT_IP_FILE のフルパス

	HttRecvFile
		HTT_RECV_FILE のフルパス

	HttSendFile
		HTT_SEND_FILE のフルパス

	HttTightFile
		HTT_TIGHT_FILE のフルパス

	HttIP
		クライアントのIPアドレス

	BootTime
		このプロセスを起動した時刻（現時刻）

	ConnectTime
		接続した時刻

	LastRecvTime
		最後に受信した時刻

	LastSendTime
		最後に送信した時刻

	ConnectElapseTime
		接続してからの経過時間

	LastRecvElapseTime
		最後に受信してからの経過時間

	LastSendElapseTime
		最後に送信してからの経過時間
*/

#include "Service.h"

char *HttIPFile;
char *HttRecvFile;
char *HttSendFile;
char *HttTightFile;
char *HttIP;

time_t BootTime;
time_t ConnectTime;
time_t LastRecvTime;
time_t LastSendTime;
time_t ConnectElapseTime;
time_t LastRecvElapseTime;
time_t LastSendElapseTime;

void ServiceDisconnect(void)
{
	LOGPOS();
	removeFile(HttSendFile);
	termination(0);
}
static void ServiceErrorDisconnectExit(void)
{
	if (errorOccurred)
	{
		LOGPOS();
		removeFile(HttSendFile);
		termination(1);
	}
}
int main(int argc, char **argv)
{
	HttIPFile = makeFullPath(HTT_IP_FILE);
	HttRecvFile = makeFullPath(HTT_RECV_FILE);
	HttSendFile = makeFullPath(HTT_SEND_FILE);
	HttTightFile = makeFullPath(HTT_TIGHT_FILE);

	if (!existFile(HttIPFile) || !existFile(HttRecvFile) || !existFile(HttSendFile))
	{
		LOGPOS();
		termination(0);
	}
	addFinalizer(ServiceErrorDisconnectExit);

	BootTime = time(NULL);

	if (getFileSize(HttIPFile) != 0) // 送受信メイン
	{
		HttIP = readFirstLine(HttIPFile);

		ConnectTime = getFileModifyTime(HttIPFile);
		LastRecvTime = getFileModifyTime(HttRecvFile);
		LastSendTime = getFileModifyTime(HttSendFile);
		ConnectElapseTime = BootTime - ConnectTime;
		LastRecvElapseTime = BootTime - LastRecvTime;
		LastSendElapseTime = BootTime - LastSendTime;
cout("ConnectElapseTime: %I64d\n", ConnectElapseTime); // test
cout("LastRecvElapseTime: %I64d\n", LastRecvElapseTime); // test
cout("LastSendElapseTime: %I64d\n", LastSendElapseTime); // test

		ServiceMain();
	}
	else // 定期的な空実行
	{
		HttIP = strx("0.0.0.0");

		ConnectTime = BootTime;
		LastRecvTime = BootTime;
		LastSendTime = BootTime;
		ConnectElapseTime = 0;
		LastRecvElapseTime = 0;
		LastSendElapseTime = 0;

		ServicePeriod();
	}
	termination(0);
}
