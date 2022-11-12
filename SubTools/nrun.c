/*
	nrun.exe [/S SERVER-HOST] [/P SERVER-PORT] [/T TIMEOUT] [/R RETRY-MAX] PROGRAM

		SERVER-HOSE ... デフォルト == localhost
		SERVER-PORT ... デフォルト == 60123
		TIMEOUT ... 送受信が成功するまで繰り返す時間, この時間内に終わらなければ error() になる。単位 == 秒, デフォルト == 1時間
		PROGRAM ... 実行するプログラム, .batまたは.exeでなければならない, サーバーに存在しなければ error() になる。
		RETRY-MAX ... 再試行回数の上限, デフォルト == 無制限(UINTMAX)

	例:
		サーバー == Lovely
		ポート番号 == 60123
		サーバーのルートDIR == C:\Princess
		実行したいプログラム == C:\Princess\Honey\Fortune.bat

		nrun.exe /S Lovely Hoeny\Fortune

	終了時の振る舞い:
		タイムアウト       -> error();
		再試行回数オーバー -> エラーコード 2 で終了
		エスケープキー押下 -> エラーコード 1 で終了
		正常終了           -> エラーコード 0 で終了
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\CRandom.h"
#include "C:\Factory\Common\Options\PadFile.h"

#define RETRY_SLEEP_MILLIS 2000

static char *ServerHost = "localhost";
static uint ServerPort = 60123;
static uint TimeoutSec = 3600;
static uint RetryMax = UINTMAX;

static uint ErrorLevel;

static int Idle(void)
{
	return 1;
}
static void RPCPerform(char *program)
{
	char *prmFile = makeTempPath("nrun-prm");
	FILE *prmFp;
	char *reqId = MakeUUID(1);
	uchar ip[4];
	uint deadTime = now() + TimeoutSec;
	uint retryNum = 0;

	errorCase(m_isEmpty(program));

	cout("reqId: %s\n", reqId);
	cout("program: %s\n", program);

	prmFp = fileOpen(prmFile, "wt");
	writeLine(prmFp, reqId);
	writeLine(prmFp, program);
	fileClose(prmFp);

	PadFile2(prmFile, "nrun_Prm");

	memset(ip, 0, 4);
	SockStartup();

	for (; ; )
	{
		char *ansFile = sockClient(ip, ServerHost, ServerPort, prmFile, Idle);

		if (ansFile)
		{
			if (UnpadFile2(ansFile, "nrun_Ans"))
			{
				char *ansLine = readFirstLine(ansFile);

				removeFile(prmFile);
				removeFile(ansFile);

				if (strcmp(ansLine, "OK"))
					error_m("nrun_WRONG_PROGRAM");

				memFree(ansLine);
				memFree(ansFile);
				break;
			}
			removeFile(ansFile);
			memFree(ansFile);
		}

		{
			uint nowTime = now();

			if (deadTime < nowTime)
				error_m("nrun_TIME-OUT");

			cout("nrun_RETRY: %u (%u)\n", deadTime - nowTime, RetryMax - retryNum);
		}

		sleep(RETRY_SLEEP_MILLIS);

		while (hasKey())
		{
			switch (getKey())
			{
			case 0x1b:
				cout("nrun_CANCELLED\n");
				ErrorLevel = 1;
				goto endLoop;

			default:
				cout("PRESS ESCAPE TO CANCEL\n");
				break;
			}
		}

		if (RetryMax < UINTMAX)
			retryNum++;

		if (RetryMax < retryNum)
		{
			cout("nrun_OVER_RETRY-MAX\n");
			ErrorLevel = 2;
			break;
		}
	}
endLoop:
	SockCleanup();

	removeFileIfExist(prmFile);
	memFree(prmFile);
	memFree(reqId);
}

int main(int argc, char **argv)
{
	char *program;

	mkAppDataDir();

	ServerHost = getAppDataEnv("SERVER", "localhost");
	ServerPort = getAppDataEnv32("NRUN_SERVER_PORT", 60123);
	RetryMax = getAppDataEnv32("NRUN_RETRY", UINTMAX);

readArgs:
	if (argIs("/S"))
	{
		ServerHost = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		ServerPort = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/T"))
	{
		TimeoutSec = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/R"))
	{
		RetryMax = toValue(nextArg());
		goto readArgs;
	}
	program = nextArg();

	RPCPerform(program);

	cout("endcode: %u\n", ErrorLevel);
	termination(ErrorLevel);
}
