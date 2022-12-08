/*
	Server.exe [/P �|�[�g�ԍ�] [/C �ő哯���ڑ���] [/R ���[�gDIR] [/D �m�ۂ���f�B�X�N�̋󂫗̈�] [/E ��~�C�x���g��] ANTI-MIS-EXEC

		�|�[�g�ԍ�     ... 1 �` 65535, def: 65123
		�ő哯���ڑ��� ... 1 �` IMAX, def: 100
		���[�gDIR      ... �ߋ��Ɏw�肵�����Ƃ̂���f�B���N�g�� || ��̃f�B���N�g�� || createPath(, 'D') �\�ȃf�B���N�g��
		�m�ۂ���f�B�X�N�̋󂫗̈� ... �o�C�g�����w�肷��B1 �` IMAX_64, def: 500 MB
		��~�C�x���g�� ... ���̖��O�̃C�x���g�ɃZ�b�g����ƒ�~����B
*/

#include "C:\Factory\Common\Options\SockServerTh.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "C:\Factory\Common\Options\CRRandom.h"

#define FILEIO_MAX 20

static uint64 KeepDiskFree = 500000000ui64; // 500 MB
static char *RootDir = "C:\\Factory\\tmp\\FilingCase3\\LongPath_aaaaaaaaaa_bbbbbbbbbb_cccccccccc_dddddddddd_eeeeeeeeee_ffffffffff_gggggggggg";
//static char *RootDir = "C:\\appdata\\FilingCase3\\LongPath_aaaaaaaaaa_bbbbbbbbbb_cccccccccc_dddddddddd_eeeeeeeeee_ffffffffff_gggggggggg"; // 100 �������炢�B// appdata �}�~ @ 2021.3.19
static char *DataDir;
static char *TempDir;
static char *SigFile;
static uint EvStop;
static semaphore_t SmphFileIO;
static critical_t CritCommand;

static int RecvPrmData(SockStream_t *ss, char *dataFile, uint64 dataSize) // ret: ? ����
{
	uint64 count;

	enterSemaphore(&SmphFileIO);
	{
		FILE *fp = fileOpen(dataFile, "wb");

		for (count = 0; count < dataSize; count++)
		{
			int chr = SockRecvChar(ss);

			if (chr == EOF)
				break;

			writeChar(fp, chr);

			if (count % 30000000ui64 == 0ui64) // 30 MB
			{
				updateDiskSpace_Dir(RootDir);

				if (lastDiskFree < KeepDiskFree)
				{
					cout("���s�ɕK�v�ȃf�B�X�N�̋󂫗̈悪�s�����Ă��邽�߃f�[�^��M�͎��s���܂��B\n");
					break;
				}
			}
		}
		fileClose(fp);
	}
	leaveSemaphore(&SmphFileIO);

	return count == dataSize;
}
static void FC3_SendBlock(SockStream_t *ss, void *block, uint blockSize)
{
	SockSendValue(ss, blockSize);
	SockSendBlock(ss, block, blockSize);
}
static void FC3_SendLine(SockStream_t *ss, char *line)
{
	FC3_SendBlock(ss, line, strlen(line));
}

thread_tls static SockStream_t *SL_SS;

static int SL_Action(struct _finddata_t *i)
{
	char *localPath;

	if (
		!strcmp(i->name, ".") ||
		!strcmp(i->name, "..")
		)
		goto endFunc;

	if (i->attrib & _A_SUBDIR)
		localPath = xcout("%s\\", i->name);
	else
		localPath = strx(i->name);

	FC3_SendLine(SL_SS, localPath);
	memFree(localPath);
endFunc:
	return 1;
}
static void SendResList(SockStream_t *ss, char *relPath)
{
	char *wCard = combine_cx(DataDir, xcout("%s\\*", relPath));

	enterSemaphore(&SmphFileIO);
	{
		SL_SS = ss;
		fileSearch(wCard, SL_Action);
		SL_SS = NULL;
	}
	leaveSemaphore(&SmphFileIO);

	memFree(wCard);

	FC3_SendLine(ss, "/LIST/e");
}
static void SendResFile(SockStream_t *ss, char *relPath)
{
	char *file = combine(DataDir, relPath);

	if (existFile(file))
	{
		enterSemaphore(&SmphFileIO);
		{
			FILE *fp = fileOpen(file, "rb");

			SockSendValue64(ss, getFileSizeFPSS(fp));

			for (; ; )
			{
				autoBlock_t *buff = readBinaryStream(fp, 2000000); // 2 MB

				if (!buff)
					break;

				SockSendBlock(ss, directGetBuffer(buff), getSize(buff));
			}
			fileClose(fp);
		}
		leaveSemaphore(&SmphFileIO);
	}
	else
	{
		SockSendValue64(ss, 0ui64);
	}

	memFree(file);

	FC3_SendLine(ss, "/GET/e");
}
static void PerformTh(int sock, char *strip)
{
	SockStream_t *ss = CreateSockStream2(sock, 0, 30, 0);
	int keepConn;

	LOGPOS();

	do
	{
		char *command;
		char *path;
		char *sDataSize;
		uint64 dataSize;
		char *dataFile;

		keepConn = 0;

		command   = SockRecvLine(ss, 30);
		path      = SockRecvLine(ss, 1000);
		sDataSize = SockRecvLine(ss, 30);

		if (IsEOFSockStream(ss))
		{
			cout("�ؒf����܂����B\n");
			goto fault;
		}
		coutJLine_x(xcout("command: %s", command));
		coutJLine_x(xcout("path: %s", path));
		coutJLine_x(xcout("dataSize: %s", sDataSize));

		dataSize = toValue64(sDataSize);

		if (!isFairRelPath(path, strlen(DataDir)))
		{
			char *tmp;

			cout("�s���ȃp�X�ł��B\n");
			tmp = lineToFairRelPath(path, strlen(DataDir));
			cout("���K�� -> %s\n", tmp);
			memFree(tmp);
			goto fault;
		}
		updateDiskSpace_Dir(RootDir);

		if (lastDiskFree < KeepDiskFree)
		{
			cout("���s�ɕK�v�ȃf�B�X�N�̋󂫗̈悪�s�����Ă��܂��B\n");
			goto fault;
		}
		if (lastDiskFree - KeepDiskFree < dataSize)
		{
			cout("�f�B�X�N�̋󂫗̈悪�v���f�[�^�T�C�Y�ɑ΂��ĕs�����Ă��܂��B\n");
			goto fault;
		}
		dataFile = combine_cx(TempDir, MakeUUID(1));

		if (!RecvPrmData(ss, dataFile, dataSize))
		{
			cout("�f�[�^�̎�M�Ɏ��s���܂����B\n");
			goto fault2;
		}

		{
			char *ender = SockRecvLine(ss, 30);

			if (_stricmp(ender, "/SEND/e"))
			{
				cout("�s���ȏI�[�ł��B\n");
				memFree(ender);
				goto fault2;
			}
			memFree(ender);
		}

		if (!_stricmp(command, "LIST"))
		{
			enterCritical(&CritCommand);
			{
				SendResList(ss, path);
			}
			leaveCritical(&CritCommand);
		}
		else if (!_stricmp(command, "GET"))
		{
			enterCritical(&CritCommand);
			{
				SendResFile(ss, path);
			}
			leaveCritical(&CritCommand);
		}
		else if (!_stricmp(command, "POST"))
		{
			char *file = combine(DataDir, path);

			enterCritical(&CritCommand);
			{
				LOGPOS();
				recurRemovePathIfExist(file);
				LOGPOS();
				createPath(file, 'X');
				LOGPOS();
				moveFile(dataFile, file);
				LOGPOS();
				createFile(dataFile);
				LOGPOS();
			}
			leaveCritical(&CritCommand);

			memFree(file);

			FC3_SendLine(ss, "/POST/e");
		}
		else if (!_stricmp(command, "GET-POST"))
		{
			char *file = combine(DataDir, path);

			enterCritical(&CritCommand);
			{
				SendResFile(ss, path);

				LOGPOS();
				recurRemovePathIfExist(file);
				LOGPOS();
				createPath(file, 'X');
				LOGPOS();
				moveFile(dataFile, file);
				LOGPOS();
				createFile(dataFile);
				LOGPOS();
			}
			leaveCritical(&CritCommand);

			memFree(file);

			FC3_SendLine(ss, "/GET-POST/e");
		}
		else if (!_stricmp(command, "DELETE"))
		{
			char *file = combine(DataDir, path);

			enterCritical(&CritCommand);
			{
				LOGPOS();
				recurRemovePathIfExist(file);
				LOGPOS();
			}
			leaveCritical(&CritCommand);

			memFree(file);

			FC3_SendLine(ss, "/DELETE/e");
		}
		else if (!_stricmp(command, "HELLO"))
		{
			FC3_SendLine(ss, "/HELLO/e");
		}
		else
		{
			cout("�s���ȃR�}���h�ł��B\n");
			goto fault2;
		}
		SockFlush(ss);
		keepConn = 1;
		cout("�R�}���h�͐���Ɏ��s����܂����B\n");

	fault2:
		removeFile(dataFile);
		memFree(dataFile);

	fault:
		memFree(command);
		memFree(path);
		memFree(sDataSize);
	}
	while (keepConn);

	LOGPOS();
	ReleaseSockStream(ss);
}
static int IdleTh(void)
{
	static int keep = 1;

	if (EvStop && handleWaitForMillis(EvStop, 0))
		keep = 0;

	while (hasKey())
		if (getKey() == 0x1b)
			keep = 0;

	if (!keep)
		LOGPOS();

	return keep;
}
static int HasOtherRootDirParent(char *dir)
{
	dir = getParent(dir);

	for (; ; )
	{
		char *file = combine(dir, getLocal(SigFile));

		cout("chk_sig: %s\n", file);

		if (existFile(file))
		{
			memFree(file);
			memFree(dir);
			return 1;
		}
		memFree(file);

		if (isAbsRootDir(dir))
			break;

		dir= getParent_x(dir);
	}
	memFree(dir);
	return 0;
}
static void CheckRootDir(void)
{
	if (!existDir(RootDir))
		return;

	if (accessible(SigFile))
		return;

	errorCase_m(lsCount(RootDir), "�w�肳�ꂽ�f�B���N�g���́A�ʂ̃A�v���P�[�V�����ɂ���Ďg�p����Ă��܂��B"); // ? ��ł͂Ȃ��B

	errorCase_m(HasOtherRootDirParent(RootDir), "�w�肳�ꂽ�f�B���N�g���́A�ʂ̃T�[�r�X�ɂ���Ďg�p����Ă��܂��B"); // ? �ʂ� RootDir �̔z��
}
int main(int argc, char **argv)
{
	uint portNo = 65123;
	uint connectMax = 100;

readArgs:
	if (argIs("/P"))
	{
		portNo = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/C"))
	{
		connectMax = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/R"))
	{
		RootDir = nextArg();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		KeepDiskFree = toValue64(nextArg());
		goto readArgs;
	}
	if (argIs("/E"))
	{
		char *name = nextArg();

		cout("��~�C�x���g��: %s\n", name);

		errorCase(m_isEmpty(name));

		EvStop = eventOpen(name);
		goto readArgs;
	}

	errorCase_m(strcmp(nextArg(), "ANTI-MIS-EXEC"), "no ANTI-MIS-EXEC");

	cout("�|�[�g�ԍ�: %u\n", portNo);
	cout("�ő哯���ڑ���: %u\n", connectMax);
	cout("�m�ۂ���f�B�X�N�̋󂫗̈�: %I64u\n", KeepDiskFree);
	cout("RootDir.0: %s\n", RootDir);

	errorCase(!m_isRange(portNo, 1, 65535));
	errorCase(!m_isRange(connectMax, 1, IMAX));

	RootDir = makeFullPath(RootDir);
	RootDir = toFairFullPathFltr_x(RootDir);

	cout("RootDir.1: %s\n", RootDir);

	DataDir = combine(RootDir, "d");
	TempDir = combine(RootDir, "w");
	SigFile = combine(RootDir, "FilingCase3_{20276b27-459e-4bed-b744-cb8f57c5af91}.sig");

	cout("DataDir: %s\n", DataDir);
	cout("TempDir: %s\n", TempDir);
	cout("SigFile: %s\n", SigFile);

	CheckRootDir();

	createPath(DataDir, 'd');
	createDirIfNotExist(TempDir);
	createFileIfNotExist(SigFile);

	recurClearDir(TempDir);

	initSemaphore(&SmphFileIO, FILEIO_MAX);
	initCritical(&CritCommand);

	sockServerTh(PerformTh, portNo, connectMax, IdleTh);

	memFree(RootDir);
	memFree(DataDir);
	memFree(TempDir);
	memFree(SigFile);

	if (EvStop)
		handleClose(EvStop);

	fnlzSemaphore(&SmphFileIO);
	fnlzCritical(&CritCommand);

	cout("FC3_SERVER_END\n");
}
