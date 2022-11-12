/*
	MOVEコマンドでファイルを移動する。
	アクセス拒否などで失敗しても成功するまでリトライする。

	aakmv.exe [/R RETRY-MAX | /RX] SRC-FILE DEST-PATH

		RETRY-MAX ... リトライ回数の上限 0 ～ UINTMAX, def 10
		/RX       ... リトライ回数 UINTMAX 回 <- ほぼ永久リトライ
		SRC-FILE  ... 移動元のファイル
		DEST-PATH ... 移動先のファイル又はディレクトリ
*/

#include "C:\Factory\Common\all.h"

static uint RetryMax = 10; // 0 ～ UINTMAX

static void AAKMove(char *srcFile, char *destPath)
{
	char *destFile;
	uint retrycnt;

	srcFile = makeFullPath(srcFile);
	destPath = makeFullPath(destPath);

	cout("< %s\n", srcFile);
	cout("! %s\n", destPath);

	if (existDir(destPath))
		destFile = combine(destPath, getLocal(srcFile));
	else
		destFile = strx(destPath);

	cout("> %s\n", destFile);

	removeFileIfExist(destFile);

	for (retrycnt = 0; ; retrycnt++)
	{
		coExecute_x(xcout("MOVE \"%s\" \"%s\"", srcFile, destFile));

		if (existFile(destFile))
			break;

		errorCase(RetryMax <= retrycnt);

		coSleep(2000);
	}

	memFree(srcFile);
	memFree(destPath);
	memFree(destFile);
}
int main(int argc, char **argv)
{
	if (argIs("/R"))
	{
		RetryMax = toValue(nextArg());
	}
	if (argIs("/RX"))
	{
		RetryMax = UINTMAX;
	}

	AAKMove(getArg(0), getArg(1));
}
