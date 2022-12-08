/*
	ncpServer.exe [/P RECV-PORT] [/C CONNECT-MAX] [/X UPLOAD-MAX] [/R ROOT-DIR]

		RECV-PORT   ... 受信町ポート、デフォルトは 60022
		CONNECT-MAX ... 最大同時接続数、デフォルトは 10
		UPLOAD-MAX  ... 一回の接続で送信できるファイル・ディレクトリサイズの上限
		                sockServer() の引数なので数十バイト程度の誤差アリ、デフォルトは無制限
		ROOT-DIR    ... ファイル・ディレクトリのアップロード先のルートDIR（終了時削除しない）
		                省略した場合は C:\1, C:\2, C:\3... を作成し、プログラム終了時削除する。

	----

	★セキュリティを考慮していません。
	LANで使うか、crypTunnelとかで暗号化してね。

	以下の方法でドライブ・メモリをパンクさせたり、エラーにすることができる。
		・ドライブの空き領域より大きなファイル・ディレクトリをアップロードする。
		・大量のファイルをアップロードした上でリストを取得しようとする。
		・階層の深すぎるディレクトリをアップロードする。(ファイルは大丈夫)
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Common\Options\DirToStream.h"
#include "C:\Factory\Common\Options\PadFile.h"

static int ForceOverwriteMode;
static uint RootDirLen;

static int CheckFairRelPath(char *path)
{
	autoList_t *ptkns = tokenizeYen_heap(path); // ここで '/' を '\\' にすることも意図している。
	char *ptkn;
	uint index;
	int retval = 1;

	errorCase(getCount(ptkns) < 1); // 2bs

	foreach (ptkns, ptkn, index)
	{
		if (!isFairLocalPath(ptkn, 0))
		{
			cout("INVALID PATH TOKEN %u\n", index);
			retval = 0;
			break;
		}
	}
	releaseDim(ptkns, 1);

	if (PATH_SIZE < RootDirLen + strlen(path))
	{
		cout("INVALID PATH LENGTH\n");
		retval = 0;
	}
	return retval;
}
static void CreateParent(char *path)
{
	autoList_t *ptkns = tokenizeYen_heap(path);
	char *ptkn;
	uint index;
	char *homeDir = getCwd();

	foreach (ptkns, ptkn, index)
	{
		if (index + 1 < getCount(ptkns))
		{
			createDirIfNotExist(ptkn);
			changeCwd(ptkn);
		}
	}
	releaseDim(ptkns, 1);
	changeCwd(homeDir);
	memFree(homeDir);
}
static void SSPInterrupt(void)
{
	// 実行間隔調整
	{
		static uint lasttm;
		uint currtm = now();

		if (currtm <= lasttm + 2)
			return;

		lasttm = currtm;
	}

	sockServerPerformInterrupt();
}

static FILE *UDFp;
static int UDError;

static void UDReader(uchar *buffer, uint size)
{
	autoBlock_t *buff_ab = readBinaryBlock(UDFp, size);

	if (getSize(buff_ab) == size)
	{
		memcpy(buffer, directGetBuffer(buff_ab), size);
	}
	else
	{
		STD_ReadStop = 1;
		UDError = 1;
	}
	releaseAutoBlock(buff_ab);
	SSPInterrupt();
}

static int Pre_Upload(char *path) // ret: ? UP可
{
	if (existPath(path))
	{
		if (!ForceOverwriteMode)
			return 0;

		cout("FORCE REMOVE!\n");

		if (existDir(path))
			recurRemoveDir(path);
		else
			removeFile(path);
	}
	return 1;
}
static int UploadDir(char *dir, FILE *fp)
{
	if (!Pre_Upload(dir))
		return 0;

	CreateParent(dir);
	createDir(dir);

	UDFp = fp;
	UDError = 0;
//	STD_TrustMode = 1;
	StreamToDir(dir, UDReader);
//	STD_TrustMode = 0;

	return !UDError;
}
static int UploadFile(char *file, FILE *fp)
{
	FILE *outFp;
	autoBlock_t *buffer;

	if (!Pre_Upload(file))
		return 0;

	CreateParent(file);
	outFp = fileOpen(file, "wb");

	while (buffer = readBinaryStream(fp, 32 * 1024 * 1024))
	{
		writeBinaryBlock_x(outFp, buffer);
		SSPInterrupt();
	}
	fileClose(outFp);
	return 1;
}

static FILE *DDFp;

static void DDWriter(uchar *buffer, uint size)
{
	autoBlock_t gab;

	fileWrite(DDFp, gndBlockVar(buffer, size, gab));
	SSPInterrupt();
}

static int DownloadDir(char *dir, FILE *fp)
{
	writeChar(fp, 'D');

	DDFp = fp;
	DirToStream(dir, DDWriter);

	return 1;
}
static int DownloadFile(char *file, FILE *fp)
{
	FILE *readFp = fileOpen(file, "rb");
	autoBlock_t *buffer;

	writeChar(fp, 'F');

	while (buffer = readBinaryStream(readFp, 32 * 1024 * 1024))
	{
		writeBinaryBlock_x(fp, buffer);
		SSPInterrupt();
	}
	fileClose(readFp);
	return 1;
}

static int Perform(char *prmFile, char *ansFile)
{
	FILE *fp;
	char *path;
	char *subPath;
	int command;
	int retval = 0;

	cout("WELCOME %s %I64u\n", SockIp2Line(sockClientIp), getFileSize(prmFile));

	if (!UnpadFile2(prmFile, "NCP_Prm"))
		goto endFunc;

	fp = fileOpen(prmFile, "rb");
	path = readLineLenMax(fp, PATH_SIZE);
	subPath = readLineLenMax(fp, PATH_SIZE);

	if (!CheckFairRelPath(path))
		goto endPerform;

	if (!CheckFairRelPath(subPath))
		goto endPerform;

	command = readChar(fp);
	m_range(command, '\x21', '\x7e');
	ForceOverwriteMode = readChar(fp) == 'F';

	cout("COMMAND %c\n", command);
	cout("FORCE %d\n", ForceOverwriteMode);
	cout("P1 %s\n", path);
	cout("P2 %s\n", subPath);

	if (command == 'U') // Upload
	{
		int type = readChar(fp);

		if (type == 'D') // Directory
		{
			retval = UploadDir(path, fp);
		}
		else if (type == 'F') // File
		{
			retval = UploadFile(path, fp);
		}
	}
	else if (command == 'D') // Download
	{
		FILE *ansFp = fileOpen(ansFile, "wb");

		if (existDir(path))
		{
			retval = DownloadDir(path, ansFp);
		}
		else if (existFile(path))
		{
			retval = DownloadFile(path, ansFp);
		}
		fileClose(ansFp);
	}
	else if (command == 'S') // Size
	{
		FILE *ansFp = fileOpen(ansFile, "wb");

		if (existDir(path))
		{
			writeChar(ansFp, 'D');
			writeValue64(ansFp, getDirSize(path));
		}
		else if (existFile(path))
		{
			writeChar(ansFp, 'F');
			writeValue64(ansFp, getFileSize(path));
		}
		else // ? not exist
		{
			writeChar(ansFp, 'N');
		}
		fileClose(ansFp);
		retval = 1;
	}
	else if (command == 'M') // Move
	{
		/*
			path == subPath を考慮して path を後にチェック
		*/
		if (Pre_Upload(subPath) && existPath(path))
		{
			CreateParent(subPath);

			if (existDir(path))
			{
				createDir(subPath);
				moveDir(path, subPath);
				removeDir(path);
			}
			else
			{
				moveFile(path, subPath);
			}
			retval = 1;
		}
	}
	else if (command == 'X') // Remove
	{
		CreateParent(path);

		if (existDir(path))
		{
			recurRemoveDir(path);
		}
		else if (existFile(path))
		{
			removeFile(path);
		}
		retval = 1; // 存在しなかった場合も成功扱い。<- '念のため削除' を考慮
	}
	else if (command == 'J' || command == 'K' || command == 'L') // List
	{
		autoList_t *paths;
//		char *path;
		uint index;

		switch (command)
		{
		case 'J':
			if (existDir(path)) {
				paths = ls(path);
			}
			else {
				paths = newList();
			}
			break;

		case 'K':
			paths = ls(".");
			break;

		case 'L':
			paths = lss(".");
			break;

		default:
			error();
		}
		changeRoots(paths, c_getCwd(), NULL);

		foreach (paths, path, index)
		{
			if (index < lastDirCount)
			{
				path = addChar(path, '\\');
				setElement(paths, index, (uint)path);
			}
		}
		rapidSortLines(paths);
		writeLines(ansFile, paths);
		releaseDim(paths, 1);
		retval = 1;
	}
	PadFile2(ansFile, "NCP_Ans");

endPerform:
	fileClose(fp);
	memFree(path);
	memFree(subPath);

endFunc:
	cout("GOODBYE %d\n", retval);
	return retval;
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
	uint portno = 60022;
	uint connectmax = 10;
	uint64 uploadmax = UINT64MAX;
	char *rootDir = NULL;
	int useFreeDir = 0;

	md5_interrupt = SSPInterrupt;

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
	if (argIs("/R"))
	{
		rootDir = nextArg();
		goto readArgs;
	}

	if (!rootDir)
	{
		rootDir = makeFreeDir();
		useFreeDir = 1;
		cout("ROOT %s\n", rootDir);
	}

	cmdTitle_x(xcout("ncpServer - P:%u C:%u X:%I64u %s [%c]", portno, connectmax, uploadmax, c_makeFullPath(rootDir), useFreeDir ? 'F' : '-'));

	addCwd(rootDir);

	RootDirLen = strlen(c_getCwd());
	sockServer(Perform, portno, connectmax, uploadmax, Idle);

	unaddCwd();

	if (useFreeDir)
	{
		recurRemoveDir(rootDir);
		memFree(rootDir);
	}

	cmdTitle("ncpServer");
}
