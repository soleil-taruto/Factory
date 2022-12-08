#include "Receptor.h"

static int DoLock(uint mtx)
{
	if (!handleWaitForMillis(mtx, 3000))
	{
		cout("MUTEX FAULT!\n");
		return 0;
	}
	return 1;
}
void ReceptorLoop(
	char *startEventName,
	char *answerEventName,
	char *mutexName,
	char *prmFile,
	char *ansFile,
	int (*perform)(char *prmFile, char *ansFile),
	int (*idle)(void)
	)
{
	uint mtx;
	uint startEvent;
	uint answerEvent;

	errorCase(isEmptyJTkn(startEventName));
	errorCase(isEmptyJTkn(answerEventName));
	errorCase(isEmptyJTkn(mutexName));
	errorCase(isEmptyJTkn(prmFile));
	errorCase(isEmptyJTkn(ansFile));
	errorCase(!perform);
	errorCase(!idle);

	mtx = mutexOpen(mutexName);
	startEvent = eventOpen(startEventName);
	answerEvent = eventOpen(answerEventName);

	if (DoLock(mtx)) // Cleanup
	{
		removeFileIfExist(prmFile);
		removeFileIfExist(ansFile);

		mutexRelease(mtx);
	}
	while (idle())
	{
		collectEvents(startEvent, 3000); // 要求待ち

		if (DoLock(mtx))
		{
			if (existFile(prmFile)) // ? 要求アリ
			{
				char *filePrm = makeTempPath(NULL);
				char *fileAns = makeTempPath(NULL);
				int retval;

				moveFile(prmFile, filePrm);
				createFile(prmFile);
				createFile(fileAns);
				removeFileIfExist(ansFile); // ないはず

				mutexRelease(mtx);

				retval = perform(filePrm, fileAns);

				errorCase(!existFile(filePrm));
				errorCase(!existFile(fileAns));

				while (!DoLock(mtx))
					if (!idle())
						goto endFunc;

				removeFileIfExist(prmFile); // あるはず
				removeFileIfExist(ansFile); // ないはず

				if (retval)
					moveFile(fileAns, ansFile);

				mutexRelease(mtx);

				eventSet(answerEvent); // 送信側に応答を通知

				removeFile(filePrm);
				removeFileIfExist(fileAns); // retval 真のとき無い。

				memFree(filePrm);
				memFree(fileAns);
			}
			else
			{
				mutexRelease(mtx);
			}
		}
	}
	if (DoLock(mtx)) // Cleanup
	{
		removeFileIfExist(prmFile);
		removeFileIfExist(ansFile);

		mutexRelease(mtx);
	}
endFunc:
	handleClose(mtx);
	handleClose(startEvent);
	handleClose(answerEvent);
}
void ReceptorLoopResFile(char *resFile, int (*perform)(char *prmFile, char *ansFile), int (*idle)(void))
{
	autoList_t *resLines;

	if (!resFile)
		resFile = "Adapter.conf"; // default

	resLines = readResourceLines(innerResPathFltr(resFile));

	ReceptorLoop(
		getLine(resLines, 0),
		getLine(resLines, 1),
		getLine(resLines, 2),
		getLine(resLines, 3),
		getLine(resLines, 4),
		perform,
		idle
		);

	releaseDim(resLines, 1);
}
