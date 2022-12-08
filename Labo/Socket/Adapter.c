/*
	Adapter.conf
		ミューテックス名
		イベント名_1
		イベント名_2
		ファイル名_1 ... *1
		ファイル名_2 ... *1

		*1 ... フルパス or 実行時のカレントからの相対
*/

#include "C:\Factory\Common\Options\SockServer.h"

static void (*Deserializer)(char *rFile, char *wFile); // == ParamsFileFltr
static void (*Serializer)(char *rFile, char *wFile);   // == AnswerFileFltr
static uint TextLineLenMax = UINTMAX;

static char *MutexName;
static char *StartEventName;
static char *AnswerEventName;
static char *ParamsFile;
static char *AnswerFile;

static uint MutexHandle;
static uint StartEventHandle;
static uint AnswerEventHandle;

static void TextFltr(char *rFile, char *wFile)
{
	FILE *rfp = fileOpen(rFile, "rb");
	FILE *wfp = fileOpen(wFile, "wt");
	char *line;

	while (line = readLineLenMax(rfp, TextLineLenMax))
	{
		line2JLine(line, 1, 0, 1, 1);
		writeLine_x(wfp, line);
	}
	fileClose(rfp);
	fileClose(wfp);

	cout("< %I64u\n", getFileSize(rFile));
	cout("> %I64u\n", getFileSize(wFile));
}
static int DoLock(void)
{
	uint trycnt;

	for (trycnt = 0; ; trycnt++)
	{
		while (hasKey())
		{
			int chr = getKey();

			if (chr == 0x1b || chr == 'D')
			{
				cout("Disconnect\n");
				return 0;
			}
			cout("Press ESCAPE or D to disconnet.\n");
		}
		if (handleWaitForMillis(MutexHandle, 3000))
			break;

		cout("MUTEX FAULT! %u\n", trycnt);
	}
	return 1;
}
static int Perform(char *prmFile, char *ansFile)
{
	cout("Perform Params: %I64u\n", getFileSize(prmFile));

	if (!DoLock())
		return 0;

	if (!Deserializer)
	{
		removeFileIfExist(ParamsFile);
		moveFile(prmFile, ParamsFile);
		createFile(prmFile);
	}
	else
	{
		Deserializer(prmFile, ParamsFile);
	}
	removeFileIfExist(AnswerFile); // Cleanup
	mutexRelease(MutexHandle);

	for (; ; )
	{
		eventSet(StartEventHandle); // 受信側に通知・再通知
		collectEvents(AnswerEventHandle, 3000); // 応答待ち

		if (!DoLock())
			return 0;

		if (existFile(AnswerFile)) // ? 応答アリ
			break;

		if (!existFile(ParamsFile)) // ? 事故
		{
			mutexRelease(MutexHandle);
			return 0;
		}
		mutexRelease(MutexHandle);
		sockServerPerformInterrupt();
	}
	removeFileIfExist(ParamsFile); // Cleanup

	if (Serializer)
	{
		Serializer(AnswerFile, ansFile);
		removeFile(AnswerFile);
	}
	else
	{
		removeFile(ansFile);
		moveFile(AnswerFile, ansFile);
	}
	mutexRelease(MutexHandle);

	cout("Perform Answer: %I64u\n", getFileSize(ansFile));
	return 1;
}
static int Idle(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("Exit the server.\n");
			return 0;
		}
		cout("Press ESCAPE to exit the server.\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint portno = 59000;
	uint connectmax = 10;
	uint64 uploadmax = 66000; // 64K + a

readArgs:
	if (argIs("/P"))
	{
		portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/C"))
	{
		connectmax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/X"))
	{
		uploadmax = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/T"))
	{
		Deserializer = TextFltr;
		Serializer = TextFltr;
		goto readArgs;
	}
	if (argIs("/TD") || argIs("/TP"))
	{
		Deserializer = TextFltr;
		goto readArgs;
	}
	if (argIs("/TS") || argIs("/TA"))
	{
		Serializer = TextFltr;
		goto readArgs;
	}
	if (argIs("/TLX"))
	{
		TextLineLenMax = toValue(nextArg());
		goto readArgs;
	}

	if (argIs("/RES"))
	{
		autoList_t *lines = readResourceLines(nextArg());

		StartEventName = (char *)desertElement(lines, 0);
		AnswerEventName = (char *)desertElement(lines, 0);
		MutexName = (char *)desertElement(lines, 0);
		ParamsFile = (char *)desertElement(lines, 0);
		AnswerFile = (char *)desertElement(lines, 0);

		errorCase(getCount(lines));

		releaseAutoList(lines);
	}
	else
	{
		StartEventName = nextArg();
		AnswerEventName = nextArg();
		MutexName = nextArg();
		ParamsFile = nextArg();
		AnswerFile = nextArg();

		errorCase(hasArgs(1)); // 余計な引数
	}
	errorCase(isEmptyJTkn(StartEventName));
	errorCase(isEmptyJTkn(AnswerEventName));
	errorCase(isEmptyJTkn(MutexName));
	errorCase(isEmptyJTkn(ParamsFile));
	errorCase(isEmptyJTkn(AnswerFile));

	MutexHandle = mutexOpen(MutexName);
	StartEventHandle = eventOpen(StartEventName);
	AnswerEventHandle = eventOpen(AnswerEventName);
	ParamsFile = makeFullPath(ParamsFile);
	AnswerFile = makeFullPath(AnswerFile);

	if (DoLock())
	{
		// Locked
		{
			// Cleanup
			removeFileIfExist(ParamsFile);
			removeFileIfExist(AnswerFile);

			mutexRelease(MutexHandle);
		}

		cmdTitle_x(xcout("Adapter - %u", portno));
		sockServer(Perform, portno, connectmax, uploadmax, Idle);
		cmdTitle("Adapter");

		if (handleWaitForMillis(MutexHandle, 2000))
		{
			// Cleanup
			removeFileIfExist(ParamsFile);
			removeFileIfExist(AnswerFile);

			mutexRelease(MutexHandle);
		}
	}
	handleClose(MutexHandle);
	handleClose(StartEventHandle);
	handleClose(AnswerEventHandle);
	memFree(ParamsFile);
	memFree(AnswerFile);
}
