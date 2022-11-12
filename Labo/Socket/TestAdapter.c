#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *resFile;
	char *startEventName;
	char *answerEventName;
	char *mutexName;
	char *prmFile;
	char *ansFile;
	uint mtx;
	uint startEvent;
	uint answerEvent;
	char *outDir = makeTempPath("test-adapter");
	uint anscnt = 0;

	if (hasArgs(1))
		resFile = nextArg();
	else
		resFile = c_dropFile();

	{
		autoList_t *lines = readResourceLines(resFile);

		startEventName = (char *)desertElement(lines, 0);
		answerEventName = (char *)desertElement(lines, 0);
		mutexName = (char *)desertElement(lines, 0);
		prmFile = (char *)desertElement(lines, 0);
		ansFile = (char *)desertElement(lines, 0);

		releaseDim(lines, 1);
	}

	errorCase(isEmptyJTkn(startEventName));
	errorCase(isEmptyJTkn(answerEventName));
	errorCase(isEmptyJTkn(mutexName));
	errorCase(isEmptyJTkn(prmFile));
	errorCase(isEmptyJTkn(ansFile));

	mtx = mutexOpen(mutexName);
	startEvent = eventOpen(startEventName);
	answerEvent = eventOpen(answerEventName);

	handleWaitForever(mtx);
	{
		removeFileIfExist(prmFile);
		removeFileIfExist(ansFile);
	}
	mutexRelease(mtx);

	cout("TEST START\n");

	for (; ; )
	{
		char *file = dropPath();
		int stopLoop = 0;

		if (!file)
			break;

		handleWaitForever(mtx);
		{
			copyFile(file, prmFile);
			removeFileIfExist(ansFile); // 2bs: 無いはず。
		}
		mutexRelease(mtx);

		eventSet(startEvent);

		while (!stopLoop)
		{
			cout("応答待ち...\n");
			collectEvents(answerEvent, 3000);

			handleWaitForever(mtx);
			{
				if (existFile(ansFile))
				{
					char *outFile = combine_cx(outDir, xcout("%010u.txt", anscnt));

					cout("応答アリ\n");

					removeFileIfExist(prmFile); // 2bs: 無いはず。
					createDirIfNotExist(outDir);
					moveFile(ansFile, outFile);

					execute_x(xcout("START %s", outFile));

					memFree(outFile);
					anscnt++;
					stopLoop = 1;
				}
				else if (!existFile(prmFile))
				{
					cout("要求が削除されましたが、応答が無いので中止します。\n");
					stopLoop = 1;
				}
			}
			mutexRelease(mtx);

			while (hasKey())
			{
				if (getKey() == 0x1b)
				{
					cout("エスケープが押されたので中止します。\n");
					stopLoop = 1;
				}
			}
		}
		memFree(file);
	}

	handleWaitForever(mtx);
	{
		removeFileIfExist(prmFile);
		removeFileIfExist(ansFile);
	}
	mutexRelease(mtx);

	handleClose(mtx);
	handleClose(startEvent);
	handleClose(answerEvent);

	cout("TEST END\n");

	if (existDir(outDir))
		recurRemoveDir(outDir);

	memFree(outDir);
}
