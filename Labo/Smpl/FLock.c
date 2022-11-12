/*
	FLock.exe (/1 ロックファイル | /L ロックファイル)

		ロックファイル ... ロックするファイル, 存在するファイルであること。
*/

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\FLock.h"

static void LockMain(char *file, int retrymax)
{
	FILE *fp;
	uint retrycnt;

	cout("ロックします...\n");

	for (retrycnt = 0; ; retrycnt++)
	{
		fp = FLock(file);

		if (fp)
			break;

		if (retrymax <= retrycnt)
			break;

		cout("失敗しました。2秒後にリトライします。\n");
		sleep(2000);
	}
	if (!fp)
	{
		cout("ロックできませんでした。\n");
		return;
	}
	cout("ロックしています...\n");
	getKey();

	FUnlock(fp);

	cout("ロック解除しました。\n");
}
int main(int argc, char **argv)
{
	if (argIs("/1"))
	{
		LockMain(nextArg(), 0);
		return;
	}
	if (argIs("/L"))
	{
		LockMain(nextArg(), 10);
		return;
	}
}
