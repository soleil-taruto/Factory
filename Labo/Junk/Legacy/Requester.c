#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\OpenSource\md5.h"

static void (*Serializer)(char *rFile, char *wFile);   // == ParamsFileFltr
static void (*Deserializer)(char *rFile, char *wFile); // == AnswerFileFltr

static char *ServerDomain;
static uint ServerPort;
static char *MutexName;
static char *StartEventName;
static char *AnswerEventName;
static char *ParamsFile;
static char *AnswerFile;

static uint MutexHandle;
static uint StartEventHandle;
static uint AnswerEventHandle;
static uint StopAppEventHandle;

static uint StopAppEventCaught;

static void TextFltr(char *rFile, char *wFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = fileOpen(wFile, "wt");
	char *line;

	cout("TextFltr\n");
	cout("< %s\n", rFile);
	cout("> %s\n", wFile);

	while (line = readLine(rfp))
	{
		line2JLine(line, 1, 0, 1, 1);
		writeLine_x(wfp, line);
	}
	fileClose(rfp);
	fileClose(wfp);
}
static int DoLock(void)
{
	while (!handleWaitForMillis(MutexHandle, 3000))
	{
		cout("MUTEX FAULT!\n");

		if (collectEvents(StopAppEventHandle, 0))
			return 0;
	}
	return 1;
}
static int Idle(void)
{
	if (collectEvents(StopAppEventHandle, 0))
	{
		StopAppEventCaught = 1;
		return 0;
	}
	return 1;
}
static void MainLoop(void)
{
	uint ip = 0;

	if (!DoLock())
		return;

	removeFileIfExist(ParamsFile);
	removeFileIfExist(AnswerFile);

	mutexRelease(MutexHandle);

	SockStartup();
	cout("START\n");

	while (!collectEvents(StopAppEventHandle, 0))
	{
		collectEvents(StartEventHandle, 3000);

		if (!DoLock())
			break;

		if (existFile(ParamsFile))
		{
			char *ansFile;

			if (Serializer)
			{
				char *wrkFile = makeTempFile(NULL);

				Serializer(ParamsFile, wrkFile);
				removeFile(ParamsFile);
				moveFile(wrkFile, ParamsFile);
				memFree(wrkFile);
			}
			removeFileIfExist(AnswerFile); // Cleanup
			mutexRelease(MutexHandle);

			cout("REQUEST START %I64u\n", getFileSize(ParamsFile));
			ansFile = sockClient((uchar *)&ip, ServerDomain, ServerPort, ParamsFile, Idle);
			cout("REQUEST END %p\n", ansFile);

			if (StopAppEventCaught || !DoLock())
			{
				if (ansFile)
				{
					removeFile(ansFile);
					memFree(ansFile);
				}
				break;
			}
			removeFileIfExist(AnswerFile); // Cleanup (2bs)

			if (ansFile)
			{
				if (Deserializer)
				{
					Deserializer(ansFile, AnswerFile);
					removeFile(ansFile);
				}
				else
				{
					moveFile(ansFile, AnswerFile);
				}
				memFree(ansFile);
				cout("ANSWER %I64u\n", getFileSize(AnswerFile));
			}
			removeFileIfExist(ParamsFile); // Cleanup
			eventSet(AnswerEventHandle); // リクエストの完了(応答)を通知
		}
		mutexRelease(MutexHandle);
	}
	cout("END\n");
	SockCleanup();

	if (handleWaitForMillis(MutexHandle, 2000))
	{
		// Cleanup
		removeFileIfExist(ParamsFile);
		removeFileIfExist(AnswerFile);

		mutexRelease(MutexHandle);
	}
}
int main(int argc, char **argv)
{
	ServerDomain = nextArg();
	ServerPort = toValue(nextArg());
	MutexName = nextArg();
	StartEventName = nextArg();
	AnswerEventName = nextArg();
	ParamsFile = nextArg();
	AnswerFile = nextArg();

	errorCase(isEmptyJTkn(ServerDomain));
	errorCase(ServerPort < 1 || 0xffff < ServerPort);
	errorCase(isEmptyJTkn(MutexName));
	errorCase(isEmptyJTkn(StartEventName));
	errorCase(isEmptyJTkn(AnswerEventName));
	errorCase(isEmptyJTkn(ParamsFile));
	errorCase(isEmptyJTkn(AnswerFile));

	MutexHandle = mutexOpen(MutexName);
	StartEventHandle = eventOpen(StartEventName);
	AnswerEventHandle = eventOpen(AnswerEventName);
	StopAppEventHandle = eventOpen_x(xcout("cerulean.charlotte Factory Requester stop app event object %s %u", c_md5_makeHexHashLine(ServerDomain), ServerPort));

	if (argIs("/T"))
	{
		Serializer = TextFltr;
		Deserializer = TextFltr;
	}
	if (argIs("/TS") || argIs("/TP"))
	{
		Serializer = TextFltr;
	}
	if (argIs("/TD") || argIs("/TA"))
	{
		Deserializer = TextFltr;
	}

	if (argIs("/S"))
	{
		eventSet(StopAppEventHandle);
	}
	else if (argIs("/1"))
	{
		error(); // HACK: request once
	}
	else
	{
		MainLoop();
	}

	handleClose(MutexHandle);
	handleClose(StartEventHandle);
	handleClose(AnswerEventHandle);
	handleClose(StopAppEventHandle);
}
