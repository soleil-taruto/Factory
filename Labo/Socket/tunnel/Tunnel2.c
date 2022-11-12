/*
	Tunnel2.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	Tunnel2.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/F LOG-FILE | /A LOG-FILE]

		CONNECT-MAX ... 最大接続数, 省略時は 1000
		/F          ... 新規 or 上書き
		/A          ... 新規 or 追記
		LOG-FILE    ... ログファイル
*/

#include "libs\Tunnel.h"
#include "C:\Factory\Common\Options\TimeData.h"

typedef struct ConInfo_st
{
	uint Id;
	int Direction;
}
ConInfo_t;

static autoBlock_t *EmptyBlock;

static int OutputFileAuto;
static char *OutputFile;
static char *OutputMode;
static FILE *OutputFp;

static void WriteLog(uint conId, int direction, autoBlock_t *rawData)
{
	static uint64 lastStamp;
	uint64 stamp = GetNowStamp() * 1000ui64;
	uint index;

	LOGPOS();
	cout("stamp: %I64u\n", stamp);

	if (lastStamp < stamp)
	{
		lastStamp = stamp;
	}
	else
	{
		lastStamp++;
		stamp = lastStamp;
	}
	writeToken_x(OutputFp, xcout("%I64u,%u,%c,\"", stamp, conId, direction));

	for (index = 0; index < getSize(rawData); index++)
	{
		int chr = getByte(rawData, index);

		if (m_isasciikana(chr) && chr != '"' && chr != '\\')
		{
			writeChar(OutputFp, chr);
		}
		else
		{
			writeChar(OutputFp, '\\');
			writeChar(OutputFp, hexadecimal[chr / 16]);
			writeChar(OutputFp, hexadecimal[chr % 16]);
		}
	}
	writeToken(OutputFp, "\"\r\n");

	LOGPOS();
}
static void PrintDataFltr(autoBlock_t *buff, uint prm)
{
	if (getSize(buff))
	{
		ConInfo_t *i = (ConInfo_t *)prm;

		WriteLog(i->Id, i->Direction, buff);
	}
}
static void Perform(int sock, int fwdSock)
{
	static uint conId = 1000;
	ConInfo_t *si = (ConInfo_t *)memAlloc(sizeof(ConInfo_t) * 2);
	ConInfo_t *ri;

	ri = si + 1;

	si->Id = conId;
	si->Direction = 'S';

	ri->Id = conId;
	ri->Direction = 'R';

	conId++;

	WriteLog(si->Id, 'C', EmptyBlock);

	CrossChannel(
		sock,
		fwdSock,
		PrintDataFltr,
		(uint)si,
		PrintDataFltr,
		(uint)ri
		);

	WriteLog(si->Id, 'D', EmptyBlock);

	memFree(si);
}
static int ReadArgs(void)
{
	if (argIs("/F"))
	{
		OutputFile = nextArg();
		OutputMode = "wb";
		return 1;
	}
	if (argIs("/A"))
	{
		OutputFile = nextArg();
		OutputMode = "ab";
		return 1;
	}

	if (!OutputFile)
	{
		char *stamp = makeCompactStamp(NULL);

		OutputFileAuto = 1;
		OutputFile = getOutFile_x(xcout("Tunnel2_%s.csv", stamp));
		OutputMode = "wb";

		memFree(stamp);
	}
	OutputFp = fileOpen(OutputFile, OutputMode);

	return 0;
}
int main(int argc, char **argv)
{
	EmptyBlock = newBlock();

	TunnelMain(ReadArgs, Perform, "Tunnel2", NULL);

errorCase(!OutputFile); // 2bs
errorCase(!OutputMode); // 2bs
	errorCase(!OutputFp); // 2bs

	fileClose(OutputFp);

	if (OutputFileAuto)
		openOutDir();
}
