#include "C:\Factory\Common\Options\SockClient.h"
#include "libs\all.h"

static void TryCreateParent(char *path, int mkDirMode)
{
	char *parent = changeLocal(path, "");

	cout("TryCreateParent() Start\n");
	cout("< %s\n", path);
	cout("> %s\n", parent);
	cout("MD: %d\n", mkDirMode);

	if (*parent && !isRootDir(parent))
	{
		TryCreateParent(parent, 1);
	}
	memFree(parent);

	if (mkDirMode)
	{
		createDirIfNotExist(path);
	}
	cout("TryCreateParent() End\n");
}

static char *ProcCommand = "FT";
static uint ScriptCounter;
static uint64 TransferCounter;

#define GetRate(numer, denom) \
	((denom) ? (double)(numer) / (double)(denom) : 0.0)

static int UserCancelled;

static void Interrupt(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("+---------------------------+\n");
			cout("| ユーザーにより中断します。|\n");
			cout("+---------------------------+\n");
			UserCancelled = 1;
			break;
		}
		cout("エスケープキーを押すと中断します。\n");
	}
}
static int Idle(void)
{
	if (eqIntPulseSec(2, NULL))
	{
		cmdTitle_x(xcout(
			"%s %u %I64u %.2f %.2f"
			,ProcCommand
			,ScriptCounter
			,TransferCounter
			,GetRate(sockClientStatus.PrmFile->Counter, sockClientStatus.PrmFile->FileSize)
			,GetRate(sockClientStatus.AnsFile->Counter, sockClientStatus.AnsFile->FileSize)
			));

		Interrupt();
	}
	return !UserCancelled;
}

static char *ServerDomain = "localhost";
static uint ServerPort = 65000; // 1-0xffff
static uint TransferBlockSize = 1000000; // 1-

static char *ParamsFile;
static FILE *ParamsFP;
static char *AnswerFile;
static FILE *AnswerFP;

static void OpenParamsFile(void)
{
	ParamsFile = makeTempPath("prm");
	ParamsFP = fileOpen(ParamsFile, "wb");
}
static int Perform(void)
{
	static uchar ip[4];
	int retval;

	cout("Perform() Start\n");
	cout("ProcCommand: %s\n", ProcCommand);

	fileClose(ParamsFP);
	HFS_PadFile(ParamsFile, 0);
	AnswerFile = sockClient(ip, ServerDomain, ServerPort, ParamsFile, Idle);

	cout("AnswerFile: [%p]\n", AnswerFile);

	if (!AnswerFile)
	{
		AnswerFile = makeTempFile("dummy_ans");
		retval = 0;
	}
	else
	{
		retval = HFS_UnpadFile(AnswerFile, 1);
	}

	AnswerFP = fileOpen(AnswerFile, "rb");

	cout("Perform() End %d\n", retval);
	return retval;
}
static void CloseAnswerFile(void)
{
	fileClose(AnswerFP);

	removeFile(ParamsFile);
	removeFile(AnswerFile);

	memFree(ParamsFile);
	memFree(AnswerFile);

	Interrupt(); // sockClient() が一瞬で終わると Idle() が一度も実行されない。
}

static void WriteParamLine(char *line)
{
	line = strx(line);
	line2JLine(line, 1, 0, 0, 1);

	cout("Param_line: %s\n", line);

	writeLine_x(ParamsFP, line);
}
static void WriteParamLine_x(char *line)
{
	WriteParamLine(line);
	memFree(line);
}
static void WriteParamBlock(autoBlock_t *block)
{
	cout("Param_block: (%u)\n", getSize(block));

	writeBinaryBlock(ParamsFP, block);
}
static char *ReadAnswerLine(void)
{
	char *line = readLineLenMax(AnswerFP, 1024);

	if (line)
		line2JLine(line, 1, 0, 0, 1);

	cout("Answer_line: %s\n", line ? line : "<NULL>");
	return line;
}
static autoBlock_t *ReadAnswerSubBlock(uint readSize) // ret: NULL == これ以上読めない
{
	return readBinaryStream(AnswerFP, readSize);
}
static autoBlock_t *ReadAnswerBlock(void)
{
	autoBlock_t *block = newBlock();

	for (; ; )
	{
		autoBlock_t *subBlock = ReadAnswerSubBlock(512 * 1024);

		if (!subBlock)
			break;

		ab_addBytes_x(block, subBlock);
	}
	cout("Answer_block: (%u)\n", getSize(block));
	return block;
}

static void GetDataList(char *file, char *command, int lsMode)
{
	uint retrycnt;

	ProcCommand = command;

	for (retrycnt = 0; retrycnt < 3; retrycnt++)
	{
		if (UserCancelled)
			break;

		cout("retrycnt: %u\n", retrycnt);

		OpenParamsFile();
		WriteParamLine(command);

		if (Perform())
		{
			FILE *fp = fileOpen(file, "wt");

			for (; ; )
			{
				char *ansLine = ReadAnswerLine();

				if (!ansLine)
					break;

				if (lsMode)
					ansLine = lineToFairLocalPath_x(ansLine, 0);

				if (*ansLine)
					writeLine(fp, ansLine);

				memFree(ansLine);
			}
			fileClose(fp);
			CloseAnswerFile();
			break;
		}
		CloseAnswerFile();
	}
	createFileIfNotExist(file);
}
static void GetFileList(char *file)
{
	GetDataList(file, "LS", 1);
}
static void GetStatusList(char *file)
{
	GetDataList(file, "ST", 0);
}
static void UploadFile(char *file, char *svrPath)
{
	FILE *fp = fileOpen(file, "rb");
	uint64 startPos = 0;

	ProcCommand = "UP";

	while (!UserCancelled)
	{
		autoBlock_t *block = readBinaryStream(fp, TransferBlockSize);

		cout("block: [%p]\n", block);

		if (!block)
			break;

		while (!UserCancelled)
		{
			cout("startPos: %I64u\n", startPos);
			cout("block: (%u)\n", getSize(block));

			OpenParamsFile();
			WriteParamLine("UP");
			WriteParamLine(svrPath);
			WriteParamLine_x(xcout("%I64u", startPos));
			WriteParamBlock(block);

			TransferCounter = startPos;

			if (Perform())
			{
				CloseAnswerFile();
				break;
			}
			CloseAnswerFile();
		}
		startPos += getSize(block);
		releaseAutoBlock(block);
	}
	fileClose(fp);

	if (UserCancelled)
	{
		cout("キャンセルにより、アップロード先を削除します。\n");

		OpenParamsFile();
		WriteParamLine("RM");
		WriteParamLine(svrPath);
		Perform();
		CloseAnswerFile();
	}
}
static void DownloadFile(char *file, char *svrPath, int autoCreateParentMode)
{
	char *midFile = addExt(strx(file), "$hemachi_download");
	FILE *fp;
	uint64 startPos = 0;

	cout("file: %s\n", file);
	cout("midFile: %s\n", midFile);
	cout("autoCreateParentMode: %d\n", autoCreateParentMode);

	ProcCommand = "DL";

	if (autoCreateParentMode)
		TryCreateParent(file, 0);

	fp = fileOpen(midFile, "wb");

	while (!UserCancelled)
	{
		cout("startPos: %I64u\n", startPos);

		OpenParamsFile();
		WriteParamLine("DL");
		WriteParamLine(svrPath);
		WriteParamLine_x(xcout("%I64u", startPos));
		WriteParamLine_x(xcout("%u", TransferBlockSize));

		TransferCounter = startPos;

		if (Perform())
		{
			autoBlock_t *block = ReadAnswerBlock();

			if (getSize(block) == 0)
			{
				releaseAutoBlock(block);
				CloseAnswerFile();
				break;
			}
			writeBinaryBlock(fp, block);
			startPos += getSize(block);
			releaseAutoBlock(block);
		}
		CloseAnswerFile();
	}
	fileClose(fp);

	if (!UserCancelled)
	{
		file = strx(file);

		/*
			へまちでは同じファイル名とディレクトリ名が存在できるので、ダウンロード先で重複することは有り得る。
			ex.
				ABC$DEF.dat$GHI.txt
				ABC$DEF.dat
		*/
		if (existPath(file))
			file = addExt(file, "overlapped"); // 上限は適当、重複は多くて１組なので１回の回避で十分だと思う。(ファイルが先だとアウト...)

		moveFile(midFile, file);
		memFree(file);
	}
	else
	{
		removeFile(midFile);
	}
	memFree(midFile);
}
static void Prv_RemoveFile(char *svrPath)
{
	ProcCommand = "RM";

	while (!UserCancelled)
	{
		OpenParamsFile();
		WriteParamLine("RM");
		WriteParamLine(svrPath);

		if (Perform())
		{
			CloseAnswerFile();
			break;
		}
		CloseAnswerFile();
	}
}
static void Prv_MoveFile(char *svrPath, char *newSvrPath)
{
	ProcCommand = "MV";

	while (!UserCancelled)
	{
		OpenParamsFile();
		WriteParamLine("MV");
		WriteParamLine(svrPath);
		WriteParamLine(newSvrPath);

		if (Perform())
		{
			CloseAnswerFile();
			break;
		}
		CloseAnswerFile();
	}
}

static void RunScript(char *file)
{
	autoList_t *lines = readResourceLines(file);

	reverseElements(lines);
	cout("RunScript() Start\n");

	while (getCount(lines) && !UserCancelled)
	{
		char *command;
		char *file;
		char *svrPath;
		char *newSvrPath;

		command = (char *)unaddElement(lines);
		file = (char *)unaddElement(lines);
		svrPath = (char *)unaddElement(lines);
		newSvrPath = (char *)unaddElement(lines);

		cout("command: %s\n", command);
		cout("file: %s\n", file);
		cout("svrPath: %s\n", svrPath);
		cout("newSvrPath: %s\n", newSvrPath);

		if (!strcmp(command, "UP"))
		{
			UploadFile(file, svrPath);
		}
		else if (!strcmp(command, "DL"))
		{
			DownloadFile(file, svrPath, 0);
		}
		else if (!strcmp(command, "DL+P"))
		{
			DownloadFile(file, svrPath, 1);
		}
		else if (!strcmp(command, "RM"))
		{
			Prv_RemoveFile(svrPath);
		}
		else if (!strcmp(command, "MV"))
		{
			Prv_MoveFile(svrPath, newSvrPath);
		}
		else
		{
			error(); // unknown command
		}
		memFree(command);
		memFree(file);
		memFree(svrPath);
		memFree(newSvrPath);

		ScriptCounter++;
	}
	releaseDim(lines, 1);
	cout("RunScript() End\n");
}

int main(int argc, char **argv)
{
	sockClientAnswerFileSizeMax = 31000000;

	SockStartup();

readArgs:
	if (argIs("/S"))
	{
		ServerDomain = nextArg();
		goto readArgs;
	}
	if (argIs("/P"))
	{
		ServerPort = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/TBS"))
	{
		TransferBlockSize = toValue(nextArg());
		goto readArgs;
	}

	if (argIs("/CWD"))
	{
		addCwd(nextArg());
		goto readArgs;
	}

	errorCase(!ServerPort || 0xffff < ServerPort);
	errorCase(!TransferBlockSize);

	if (argIs("/LS"))
	{
		GetFileList(nextArg());
		goto endFunc;
	}
	if (argIs("/ST"))
	{
		GetStatusList(nextArg());
		goto endFunc;
	}
	if (argIs("/UP"))
	{
		UploadFile(getArg(0), getArg(1)); // file -> svrPath
		goto endFunc;
	}
	if (argIs("/DL"))
	{
		DownloadFile(getArg(0), getArg(1), 0); // file <- svrPath
		goto endFunc;
	}
	if (argIs("/DL+P"))
	{
		DownloadFile(getArg(0), getArg(1), 1); // file <- svrPath
		goto endFunc;
	}
	if (argIs("/RM"))
	{
		Prv_RemoveFile(nextArg()); // svrPath
		goto endFunc;
	}
	if (argIs("/MV"))
	{
		Prv_MoveFile(getArg(0), getArg(1)); // svrPath -> newSvrPath
		goto endFunc;
	}
	if (argIs("/RES"))
	{
		RunScript(nextArg());
		goto endFunc;
	}
	if (argIs("/RES-S")) // "/RES" + Delete Script
	{
		char *scriptFile = nextArg();

		RunScript(scriptFile);
		remove(scriptFile); // へまち起動時に *.tmp を削除する。-> 処理中に再起動して scriptFile が削除された場合を考慮する。removeFile() -> remove()
		goto endFunc;
	}

	error(); // ? unknown command

endFunc:
	SockCleanup();

	unaddAllCwd();
}
