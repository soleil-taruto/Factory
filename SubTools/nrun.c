/*
	nrun.exe [/S SERVER-HOST] [/P SERVER-PORT] [/T TIMEOUT] [/R RETRY-MAX] PROGRAM

		SERVER-HOSE ... �f�t�H���g == localhost
		SERVER-PORT ... �f�t�H���g == 60123
		TIMEOUT ... ����M����������܂ŌJ��Ԃ�����, ���̎��ԓ��ɏI���Ȃ���� error() �ɂȂ�B�P�� == �b, �f�t�H���g == 1����
		PROGRAM ... ���s����v���O����, .bat�܂���.exe�łȂ���΂Ȃ�Ȃ�, �T�[�o�[�ɑ��݂��Ȃ���� error() �ɂȂ�B
		RETRY-MAX ... �Ď��s�񐔂̏��, �f�t�H���g == ������(UINTMAX)

	��:
		�T�[�o�[ == Lovely
		�|�[�g�ԍ� == 60123
		�T�[�o�[�̃��[�gDIR == C:\Princess
		���s�������v���O���� == C:\Princess\Honey\Fortune.bat

		nrun.exe /S Lovely Hoeny\Fortune

	�I�����̐U�镑��:
		�^�C���A�E�g       -> error();
		�Ď��s�񐔃I�[�o�[ -> �G���[�R�[�h 2 �ŏI��
		�G�X�P�[�v�L�[���� -> �G���[�R�[�h 1 �ŏI��
		����I��           -> �G���[�R�[�h 0 �ŏI��
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
