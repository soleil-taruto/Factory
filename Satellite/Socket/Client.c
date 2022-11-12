/*
	Client.exe 送信先ホスト名 送信先ポート番号 送信データファイル 受信データファイル

	- - -

	成功時 ... 受信データファイルに受信データを書き出す。
	失敗時 ... 受信データファイルを作成しない。(存在したら削除する)
*/

#include "C:\Factory\Common\Options\SClient.h"

static int Idle(void)
{
	return 1;
}
int main(int argc, char **argv)
{
	uchar ip[4] = { 0, 0, 0, 0 };
	char *domain;
	uint portno;
	char *rFile;
	char *wFile;
	uint tryCount;
	char *ansFile;

	domain = nextArg();
	portno = toValue(nextArg());
	rFile = nextArg();
	wFile = nextArg();

	cout("接続先 = %s:%u\n", domain, portno);
	cout("送信ファイル = %s\n", rFile);
	cout("受信ファイル = %s\n", wFile);

	removeFileIfExist(wFile);

	for (tryCount = 1; ; tryCount++)
	{
		LOGPOS();
		ansFile = sockClient(ip, domain, portno, rFile, Idle);
		LOGPOS();

		cout("ansFile: %s\n", ansFile ? ansFile : "<NONE>");

		if (ansFile)
		{
			moveFile(ansFile, wFile);
			memFree(ansFile);
			break;
		}
		if (3 <= tryCount)
			break;

		coSleep(2000);
	}
	LOGPOS();
}
