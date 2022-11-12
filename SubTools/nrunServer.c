/*
	nrunServer.exe [/P PORT-NO] [/C CONNECT-MAX] ROOT-DIR

		PORT-NO ... デフォルト == 60123
		CONNECT-MAX ... 送受信データは常に小さいので、大きくしてもあまり意味は無い。デフォルト == 10
		ROOT-DIR ... ルートDIR
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Common\Options\PadFile.h"
#include "C:\Factory\SubTools\BlueFish\libs\Lock.h"

#define REQIDHISTORY_MAX 1000

static uint P_PortNo;
static char *RootDir;
static autoList_t *ReqIdHistory;

static void SetTitle(void)
{
	cmdTitle_x(xcout("nrunServer - P:%u %s", P_PortNo, RootDir));
}
static void UnsetTitle(void)
{
	cmdTitle("nrunServer");
}

static int Perform(char *prmFile, char *ansFile)
{
	int retval = 0;
	FILE *fp;
	char *reqId = NULL;
	char *program = NULL;
	char *batFile = NULL;
	char *exeFile = NULL;

	LOGPOS();

	if (!UnpadFile2(prmFile, "nrun_Prm"))
		goto endFunc;

	fp = fileOpen(prmFile, "rb");
	reqId = nnReadLine(fp);
	program = nnReadLine(fp);
	fileClose(fp);

	line2JLine(reqId, 0, 0, 0, 0);
	program = lineToFairRelPath_x(program, strlen(RootDir));
	batFile = changeExt(program, "bat");
	exeFile = changeExt(program, "exe");

	cout("reqId: %s\n", reqId);
	cout("program: %s\n", program);
	cout("batFile: %s\n", batFile);
	cout("exeFile: %s\n", exeFile);

	if (findLine(ReqIdHistory, reqId) < getCount(ReqIdHistory))
	{
		cout("FOUND_REQID\n");
		retval = 1;
		goto makeAns;
	}

	if (existFile(batFile))
	{
		BlueFish_Lock();
		{
			coExecute_x(xcout("START \"\" /B /WAIT CMD /C \"%s\"", batFile));
		}
		BlueFish_Unlock();
	}
	else if (existFile(exeFile))
	{
		BlueFish_Lock();
		{
			coExecute_x(xcout("START \"\" /B /WAIT \"%s\"", exeFile));
		}
		BlueFish_Unlock();
	}
	else
	{
		cout("WRONG_PROGRAM\n");
		goto makeAns;
	}
	retval = 1;

	addElement(ReqIdHistory, (uint)strx(reqId));

	if (REQIDHISTORY_MAX < getCount(ReqIdHistory))
	{
		LOGPOS();
		memFree((char *)desertElement(ReqIdHistory, 0));
	}
	cout("ReqIdHistory_SIZE: %u\n", getCount(ReqIdHistory));

makeAns:
	cout("retval_makeAns: %d\n", retval);

	writeOneLine(ansFile, retval ? "OK" : "NG");
	PadFile2(ansFile, "nrun_Ans");
	retval = 1;

endFunc:
	cout("retval: %d\n", retval);

	memFree(reqId);
	memFree(program);
	memFree(batFile);
	memFree(exeFile);

	SetTitle();
	return retval;
}
static int Idle(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("nrunServer_END\n");
			return 0;
		}
		cout("Press ESCAPE to exit nrunServer.\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint portno = 60123;
	uint connectmax = 10;

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
	P_PortNo = portno;

	RootDir = nextArg();
	RootDir = makeFullPath(RootDir);
	cout("RootDir: %s\n", RootDir);

	ReqIdHistory = newList();

	SetTitle();

	addCwd(RootDir);
	sockServer(Perform, portno, connectmax, 1000, Idle);
	unaddCwd();

	memFree(RootDir);
	releaseDim(ReqIdHistory, 1);

	UnsetTitle();
}
