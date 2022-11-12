/*
	Tunnel.exe RECV-PORT FWD-HOST FWD-PORT /S

		/S ... 停止する。

	Tunnel.exe RECV-PORT FWD-HOST FWD-PORT [/C CONNECT-MAX] [/F LOG-FILE | /A LOG-FILE] [/U | /D]

		CONNECT-MAX ... 最大接続数, 省略時は 1000
		/F          ... 新規 or 上書き
		/A          ... 新規 or 追記
		LOG-FILE    ... ログファイル
		/U          ... アップロードのみ
		/D          ... ダウンロードのみ
*/

#include "libs\Tunnel.h"

enum
{
	DM_BOTH,
	DM_UPLOAD_ONLY,
	DM_DOWNLOAD_ONLY,
};

static char *OutputFile;
static char *OutputMode;
static FILE *OutputFp;
static int DirectionMode;

static void Cout_WrLine(FILE *fp, char *line)
{
	cout("%s\n", line);
	writeLine(fp, line);
}
static void KeyEvent(int key)
{
	if (key == '+' && OutputFp)
	{
		Cout_WrLine(OutputFp, "++++++++++++++++++++++++++");
		Cout_WrLine(OutputFp, "++++++++++ PLUS ++++++++++");
		Cout_WrLine(OutputFp, "++++++++++++++++++++++++++");
	}
}
static void PrintDataFltr(autoBlock_t *buff, uint header)
{
	autoBlock_t *pBuff = newBlock();
	char *pLine;

	if (getSize(buff))
	{
		uint index;

		if (OutputFp)
		{
			writeToken(OutputFp, (char *)header);
			writeChar(OutputFp, ' ');
			writeLine_x(OutputFp, makeJStamp(NULL, 1));
		}
		for (index = 0; index < getSize(buff); index++)
		{
			int chr = getByte(buff, index);
			int pchr;

			if (OutputFp)
				writeChar(OutputFp, chr);

			pchr = chr;
			m_toHalf(pchr);
			addByte(pBuff, pchr);

			if (chr == '\n')
				addByte(pBuff, '\n');
		}
		if (OutputFp)
			writeChar(OutputFp, '\n');

#if 0 // test
		{
			static autoList_t *fpQueue;
			thread_tls static FILE *fp;

			if (!fpQueue)
				fpQueue = newList();

			if (!fp)
			{
				static uint counter;
				char *file;

				file = xcout("C:\\temp\\%u_%u_%u.txt", PortNo, FwdPortNo, counter);
				counter++;
				fp = fileOpen(file, "wb");

				if (30 < getCount(fpQueue))
				{
					fileClose((FILE *)desertElement(fpQueue, 0));
				}
				addElement(fpQueue, (uint)fp);
			}
			writeBinaryBlock(fp, buff);
		}
#endif
	}
	pLine = unbindBlock2Line(pBuff);

	if (*pLine)
	{
		char *p = strchr(pLine, '\0') - 1;

		if (*p == '\n')
			*p = '\0';

		if (*pLine)
		{
			cout("%s\n", header);
			cout("%s\n", pLine);
		}
	}
	memFree(pLine);
}
static void Perform(int sock, int fwdSock)
{
	static uint conId;
	char *upHeader;
	char *downHeader;

	conId++;
	upHeader = xcout("[UP] ID:%u", conId);
	downHeader = xcout("[DOWN] ID:%u", conId);

	cout("接続\n");

	CrossChannel(
		sock,
		fwdSock,
		DirectionMode == DM_DOWNLOAD_ONLY ? NULL : PrintDataFltr,
		(uint)upHeader,
		DirectionMode == DM_UPLOAD_ONLY ? NULL : PrintDataFltr,
		(uint)downHeader
		);

	cout("切断\n");

	memFree(upHeader);
	memFree(downHeader);
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
	if (argIs("/U"))
	{
		DirectionMode = DM_UPLOAD_ONLY;
		return 1;
	}
	if (argIs("/D"))
	{
		DirectionMode = DM_DOWNLOAD_ONLY;
		return 1;
	}

	if (OutputFile)
		OutputFp = fileOpen(OutputFile, OutputMode);

	return 0;
}
int main(int argc, char **argv)
{
	TunnelKeyEvent = KeyEvent;
	TunnelMain(ReadArgs, Perform, "Tunnel", NULL);

	if (OutputFp)
		fileClose(OutputFp);
}
