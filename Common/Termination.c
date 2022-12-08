#include "all.h"

static autoList_t *Finalizers;

void addFinalizer(void (*finalizer)(void))
{
	errorCase(!finalizer);

	if (!Finalizers)
		Finalizers = createAutoList(1);

	addElement(Finalizers, (uint)finalizer);
}
void unaddFinalizer(void (*finalizer)(void))
{
	errorCase(!finalizer);
	errorCase(!Finalizers);

	removeElement(Finalizers, (uint)finalizer);
}
void runFinalizers(void)
{
	if (Finalizers)
	{
		while (getCount(Finalizers))
		{
			((void (*)(void))unaddElement(Finalizers))();
		}
	}
}
void termination(uint errorlevel)
{
	runFinalizers();
	termination_fileCloseAll();
	termination_scheduledRemoveFile();
	exit(errorlevel);
}

int errorOccurred;
char *errorPosSource = "?";
uint errorPosLineno;
char *errorPosFunction = "?";
char *errorPosMessage = "";

void error2(char *source, uint lineno, char *function, char *message)
{
	static int busy;
	uint mtx;

	// 再帰防止
	if (busy)
	{
		if (!noErrorDlgMode)
			system("START ?_Error_In_Error"); // せめて何か出す。

		exit(2);
	}
	busy = 1;

	cout("+-------+\n");
	cout("| ERROR |\n");
	cout("+-------+\n");
	cout("%s (%u) %s ", source, lineno, function); cout("LastError=%08x\n", GetLastError());

	if (message)
	{
		cout("----\n%s\n----\n", message);
//		cout("%s\n", message);
//		cout("error-reason: %s\n", message);

		errorPosMessage = message;
	}
	errorOccurred = 1;
	errorPosSource = source;
	errorPosLineno = lineno;
	errorPosFunction = function;

	runFinalizers();

#define SRC_MUTEX "Mutex.c"
#define NM_MUTEX "cerulean.charlotte Factory error mutex object"

	if (noErrorDlgMode)
		goto endproc;

	if (!_stricmp(source, SRC_MUTEX)) // Mutex のエラーなら Mutex は使えないだろう。
	{
		system("START ?_Mutex_Error"); // せめて何か出す。
		goto endproc;
	}
	mtx = mutexLock(NM_MUTEX);

	{
		char *vbsfile = makeTempPath("vbs");
		char *strw;
		char *mbMessage;

		if (message)
		{
			mbMessage = strx(message);

			if (
				strchr(mbMessage, '\r') ||
				strchr(mbMessage, '\n') ||
				strchr(mbMessage, '"')
				)
			{
				mbMessage = replaceLine(mbMessage, "\"", "\"\"", 0);
				mbMessage = replaceLine(mbMessage, "\r", "", 0);
				mbMessage = replaceLine(mbMessage, "\n", "\" & vbCrLf & \"", 0);
			}
		}
		else
		{
			mbMessage = xcout("An error has occurred @ %s (%u) %s", source, lineno, function);
		}
		writeOneLine(vbsfile, strw = xcout("MsgBox \"%s\", 4096 + 16, \"Error\"", mbMessage));
		memFree(strw);
		memFree(mbMessage);

		/*
			@ 2015.7.23
			このプロセスのメモリが逼迫している(2G近くになる)と、下の vbs が実行されないことがあるぽい。
			system はちゃんとキックしてるぽい。
		*/

		if (isFactoryDirEnabled())
			execute(vbsfile);
		else
			execute_x(xcout("START \"\" /WAIT \"%s\"", vbsfile));

		removeFile(vbsfile);
		memFree(vbsfile);
	}

	mutexUnlock(mtx);

endproc:
	termination(1);
}
