/*
	最悪のケースの想定
		1 バイトのファイルを 64999 ファイル up してパート 31 MB を上げ続ける。
			削除速度 1/10 -> 100 回程度で全消去 -> 最悪 3.1 GB の超過
			但し HFS_KeepDiskFreeSpace に引っ掛かった場合は 1 回程度で全消去 -> ほぼ超過なし

		クラスタサイズ * n + 1 バイトのファイルを 65000 ファイル up した場合に無駄になる領域
			(4096 - 1) * 65000 -> およそ 266 MB
*/
#include "all.h"

#define DOWNLOAD_PARTSIZEMAX 30000000
#define UPLOAD_FILESIZEMAX (0x100000000ui64 * 4 + 4096) // 16GB + margin
#define UPLOAD_FILENUMMAX 65000

/*
	このモジュールを使用する前に初期化すること。

	HFS_StoreDir
		ドライブ名付きフルパスであること。(makeFullPath() の戻り値相当, ルートディレクトリ可)
		存在するディレクトリで、初回は空のディレクトリであること。
*/
char *HFS_StoreDir;
uint64 HFS_DiskFreeSpaceMin = 2148000000ui64; // 2GB + margin
uint64 HFS_StoreDirSizeMax = UINT64MAX;

static uint64 LastDiskFreeSpaceRemain;

static int CheckDiskFreeSpace(void) // ret: kept
{
	static uint lastSec;
	static uint chkSpan;
	uint currSec = now();
	int retval;

	cout("currSec: %u\n", currSec);
	cout("HFS_StoreDir: %s\n", HFS_StoreDir);

	if (lastSec + chkSpan <= currSec)
	{
		updateDiskSpace(HFS_StoreDir[0]);

		cout("MIN_DiskFree: %I64u\n", HFS_DiskFreeSpaceMin);
		cout("lastDiskFree: %I64u\n", lastDiskFree);
		cout("lastDiskSize: %I64u\n", lastDiskSize);

		lastSec = currSec;
		chkSpan = HFS_DiskFreeSpaceMin < lastDiskFree / 12 ? 60 : 5;

		if (HFS_DiskFreeSpaceMin < lastDiskFree)
		{
			LastDiskFreeSpaceRemain = lastDiskFree - HFS_DiskFreeSpaceMin;
			retval = 1;
		}
		else
		{
			LastDiskFreeSpaceRemain = 0;
			chkSpan = 0;
			retval = 0;
		}
		cout("LastDiskFreeSpaceRemain: %I64u\n", LastDiskFreeSpaceRemain);
		cout("chkSpan: %u\n", chkSpan);
	}
	else
	{
		retval = 1;
	}
	cout("retval: %d\n", retval);
	return retval;
}
static void RecoverDiskFreeSpace(void)
{
	autoList_t *files;
	uint fileCount;
	uint removeCount;
	uint index;

	cout("RecoverDiskFreeSpace() Start\n");

	files = ls(HFS_StoreDir);
	SortFilesByStamp(files);
//	sortJLinesICase(files);
	fileCount = getCount(files);
	removeCount = fileCount / 10;

	if (!removeCount)
		removeCount = 1;

	cout("fileCount: %u\n", fileCount);
	cout("removeCount: %u\n", removeCount);

	for (index = 0; removeCount && index < fileCount; index++)
	{
		char *file = getLine(files, index);

		cout("removeCount_Curr: %u\n", removeCount);
		cout("file: %s\n", file);

		if (HFS_NM_LockPath(file, 1))
		{
			cout("削除します。\n");
			removeFile(file);
			removeCount--;
		}
	}
	releaseDim(files, 1);
	cout("RecoverDiskFreeSpace() End\n");
}
void HFS_KeepDiskFreeSpace(void)
{
	uint rem;

	HFS_MutexEnter();
	{
		for (rem = 100; rem && !CheckDiskFreeSpace(); rem--)
//		if (!CheckDiskFreeSpace()) // old
		{
			RecoverDiskFreeSpace();
			cout("rem: %u\n", rem);
		}
	}
	HFS_MutexLeave();
}
void KeepStoreDirSize(uint64 writeSize)
{
	static uint64 estRemSize;

	cout("KeepStoreDirSize() Start\n");
	cout("estRemSize_1: %I64u\n", estRemSize);
	cout("writeSize: %I64u\n", writeSize);

	if (estRemSize < writeSize)
	{
		autoList_t *files = ls(HFS_StoreDir);
		char *file;
		uint index;
		uint64 dirSize = 0ui64;

		foreach (files, file, index)
			dirSize += getFileSize(file);

		releaseDim(files, 1);

		cout("HFS_StoreDirSizeMax: %I64u\n", HFS_StoreDirSizeMax);
		cout("dirSize: %I64u\n", dirSize);

		if (HFS_StoreDirSizeMax < dirSize)
		{
			RecoverDiskFreeSpace();
			estRemSize = 0;
		}
		else
		{
			estRemSize = HFS_StoreDirSizeMax - dirSize;
		}
	}
	else
	{
		estRemSize -= writeSize;
	}

	cout("estRemSize_2: %I64u\n", estRemSize);
	cout("KeepStoreDirSize() End\n");
}

static char *GetRealPath(char *virPath) // ret: strx()
{
	return lineToFairLocalPath(virPath, strlen(HFS_StoreDir));
}
static char *FP_GetRealPath(char *virPath) // ret: strx()
{
	return combine_cx(HFS_StoreDir, GetRealPath(virPath));
}
static char *FP_GetRealPath_x(char *virPath)
{
	char *out = FP_GetRealPath(virPath);
	memFree(virPath);
	return out;
}

/*
	呼び出し側で HFS_MutexEnter(), HFS_MutexLeave() すること。
	realPath は FP_GetRealPath() の戻り値相当であること。
	realPath は HFS_NM_LockPath(, 1) でロックされていないことを確認すること。
	FP_GetRealPath(), HFS_NM_LockPath(), HFS_NM_UnlockPath() は HFS_MutexEnter() ～ HFS_MutexLeave() で呼ぶこと。
*/

static autoList_t *GetFileList(void)
{
	autoList_t *files = ls(HFS_StoreDir); // サブディレクトリの存在を想定しない。

	eraseParents(files);
	sortJLinesICase(files);
	return files;
}
static void UploadPart(char *realPath, FILE *rfp, uint64 startPos)
{
	autoList_t *files;
	uint fileCount;
	FILE *wfp;
	uint64 fileSize;
	uint64 writeSize;

	cout("UploadPart() Start\n");
	cout("realPath: %s\n", realPath);
	cout("rfp: %p\n", rfp);
	cout("startPos: %I64u\n", startPos);

	if (existFile(realPath))
	{
		fileSize = getFileSize(realPath);

		cout("fileSize: %I64u\n", fileSize);

		if (startPos < fileSize)
		{
			cout("サイズを調整します。\n");
			setFileSize(realPath, startPos);
		}
	}
	else
	{
		files = GetFileList();
		fileCount = getCount(files);

		cout("fileCount: %u\n", fileCount);

		if (UPLOAD_FILENUMMAX <= fileCount)
		{
			RecoverDiskFreeSpace();
		}
		releaseDim(files, 1);
	}
	wfp = fileOpen(realPath, "ab"); // 存在しないときは作成する。
	writeSize = 0ui64;

	for (; ; )
	{
		autoBlock_t *block = readBinaryStream(rfp, 512 * 1024);

		if (!block)
			break;

		writeSize += (uint64)getSize(block);
		writeBinaryBlock_x(wfp, block);
	}
	fileClose(wfp);
	fileSize = getFileSize(realPath);

	cout("fileSize: %I64u\n", fileSize);
	cout("writeSize: %I64u\n", writeSize);

	if (UPLOAD_FILESIZEMAX < fileSize)
	{
		cout("削除します。\n");
		removeFile(realPath);
	}
	KeepStoreDirSize(writeSize);
	cout("UploadPart() End\n");
}
static void DownloadPart(char *realPath, FILE *wfp, uint64 startPos, uint readSize)
{
	cout("DownloadPart() Start\n");
	cout("realPath: %s\n", realPath);
	cout("wfp: %p\n", wfp);
	cout("startPos: %I64u\n", startPos);
	cout("readSize: %u\n", readSize);

	if (existFile(realPath))
	{
		uint64 fileSize = getFileSize(realPath);

		cout("fileSize: %I64u\n", fileSize);

		if (startPos < fileSize)
		{
			FILE *rfp = fileOpen(realPath, "rb");

			fileSeek(rfp, SEEK_SET, startPos);

			if (DOWNLOAD_PARTSIZEMAX < readSize)
				readSize = DOWNLOAD_PARTSIZEMAX;

			if (fileSize - startPos < readSize)
				readSize = (uint)(fileSize - startPos);

			cout("readSize_Resized: %u\n", readSize);

			while (readSize)
			{
				autoBlock_t *block = neReadBinaryBlock(rfp, m_min(readSize, 512 * 1024));

				errorCase(getSize(block) == 0); // assert

				writeBinaryBlock(wfp, block);
				readSize -= getSize(block);
				releaseAutoBlock(block);
			}
			fileClose(rfp);
		}
		else
		{
			cout("FAULT: startPos OUT OF RANGE!\n");
		}
	}
	else
	{
		cout("FAULT: IS NOT EXIST!\n");
	}
	cout("DownloadPart() End\n");
}
static void Prv_RemoveFile(char *realPath)
{
	cout("RemoveFile() Start\n");
	cout("realPath: %s\n", realPath);

	removeFileIfExist(realPath);

	cout("RemoveFile() End\n");
}
static void Prv_MoveFile(char *realPath, char *newRealPath)
{
	cout("MoveFile() Start\n");
	cout("realPath: %s\n", realPath);
	cout("newRealPath: %s\n", newRealPath);

	if (!mbs_stricmp(realPath, newRealPath))
	{
		cout("FAULT: SAME PATH!\n");
	}
	else
	{
		createFileIfNotExist(realPath);
		removeFileIfExist(newRealPath);
		moveFile(realPath, newRealPath);
	}
	cout("MoveFile() End\n");
}

static FILE *ParamsFP;
static FILE *AnswerFP;

static char *ReadParamLine(void)
{
	char *line = readLineLenMax(ParamsFP, 1024);

	if (!line)
		line = strx("");

	return line;
}
static char *FP_ReadRealPath(void)
{
	return FP_GetRealPath_x(ReadParamLine());
}
int HFS_Perform(char *prmFile, char *ansFile)
{
	char *command;
	char *realPath = NULL;
	char *newRealPath = NULL;
	uint64 startPos;
	uint readSize;
	int retval = 0;

	ParamsFP = fileOpen(prmFile, "rb");
	AnswerFP = fileOpen(ansFile, "wb");

	command = ReadParamLine();

	if (!strcmp(command, "LS"))
	{
		HFS_MutexEnter();
		{
			writeLines2Stream_x(AnswerFP, GetFileList());
		}
		HFS_MutexLeave();
		retval = 1;
	}
	else if (!strcmp(command, "ST"))
	{
		HFS_MutexEnter();
		{
			writeLine_x(AnswerFP, xcout("%I64u", LastDiskFreeSpaceRemain));
			writeLine_x(AnswerFP, xcout("%I64u", HFS_StoreDirSizeMax));
		}
		HFS_MutexLeave();
		retval = 1;
	}
	else if (!strcmp(command, "UP"))
	{
		realPath = FP_ReadRealPath();
		startPos = toValue64_x(ReadParamLine());

		HFS_MutexEnter();
		{
			if (HFS_NM_LockPath(realPath, 1))
			{
				UploadPart(realPath, ParamsFP, startPos);
				retval = 1;
			}
		}
		HFS_MutexLeave();
	}
	else if (!strcmp(command, "DL"))
	{
		realPath = FP_ReadRealPath();
		startPos = toValue64_x(ReadParamLine());
		readSize = toValue64(ReadParamLine());

		HFS_MutexEnter();
		{
			if (HFS_NM_LockPath(realPath, 1))
			{
				DownloadPart(realPath, AnswerFP, startPos, readSize);
				retval = 1;
			}
		}
		HFS_MutexLeave();
	}
	else if (!strcmp(command, "RM"))
	{
		realPath = FP_ReadRealPath();

		HFS_MutexEnter();
		{
			if (HFS_NM_LockPath(realPath, 1))
			{
				Prv_RemoveFile(realPath);
				retval = 1;
			}
		}
		HFS_MutexLeave();
	}
	else if (!strcmp(command, "MV"))
	{
		realPath = FP_ReadRealPath();
		newRealPath = FP_ReadRealPath();

		HFS_MutexEnter();
		{
			if (
				HFS_NM_LockPath(realPath, 1) &&
				HFS_NM_LockPath(newRealPath, 1)
				)
			{
				Prv_MoveFile(realPath, newRealPath);
				retval = 1;
			}
		}
		HFS_MutexLeave();
	}
	else
	{
		cout("Unknown Command!\n");
	}

	memFree(command);
	memFree(realPath);
	memFree(newRealPath);

	fileClose(ParamsFP);
	fileClose(AnswerFP);

	cout("retval: %d\n", retval);
	return retval;
}

char *HFS_GetRealPath(char *virPath) // ret: strx(), NULL == ロック失敗
{
	char *realPath = FP_GetRealPath(virPath);

	if (!HFS_LockPath(realPath, 0))
	{
		memFree(realPath);
		realPath = NULL;
	}
	cout("realPath: %p -> %s\n", realPath, realPath ? realPath : "(null)");
	return realPath;
}
void HFS_ReleaserealPath(char *realPath) // realPath: 開放する。
{
	HFS_UnlockPath(realPath);
	memFree(realPath);
}
