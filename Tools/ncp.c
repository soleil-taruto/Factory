/*
	ncp.exe [/S SERVER-DOMAIN] [/P SERVER-PORT] [/R RETRY-COUNT] [/T RETRY-WAIT-MILLIS] [/F] ...

		SERVER-DOMAIN     ... サーバードメイン, デフォルトは appDataEnv の SERVER= 無ければ localhost
		SERVER-PORT       ... サーバーポート番号, デフォルトは appDataEnv の NCP_SERVER_PORT= 無ければ 60022
		RETRY-COUNT       ... リトライ回数(0=リトライ無し), デフォルトは appDataEnv の NCP_RETRY= 無ければ 2
		RETRY-WAIT-MILLIS ... リトライ前の待ち時間(ミリ秒), デフォルトは appDataEnv の NCP_RETRY_WAIT_MILLIS= 無ければ 3000
		/F                ... /UP, /MV のとき、強制上書きモード

	ncp.exe ... (/UP | UP) LOCAL-PATH [SERVER-PATH]

		ファイル・ディレクトリのアップロード

		LOCAL-PATH に * を指定すると D&D になる。
		SERVER-PATH を省略すると LOCAL-PATH のローカル名を使用する。

		LOCAL-PATH は存在するディレクトリ又はファイルであること。

	ncp.exe ... (/DL | DL) LOCAL-PATH SERVER-PATH

		ファイル・ディレクトリのダウンロード

		LOCAL-PATH に * を指定すると C:\1, C:\2, C:\3... に SERVER-PATH のローカル名を連結したものを使用する。

		LOCAL-PATH は作成可能なパスであること。

	ncp.exe ... (/SZ | SZ) SERVER-PATH

		ファイル・ディレクトリのサイズを得る。

	ncp.exe ... (/MV | MV) SERVER-PATH-1 SERVER-PATH-2

		ファイル・ディレクトリを移動する。

	ncp.exe ... (/RM | RM) SERVER-PATH

		ファイル・ディレクトリを削除する。

	ncp.exe ... (/LS | LS)

		ルート直下のファイル・ディレクトリのリストを得る。

	ncp.exe ... (/LS | LS) SERVER-PATH

		ディレクトリ SERVER-PATH 配下のファイル・ディレクトリのリストを得る。

	ncp.exe ... (/LSS | LSS)

		全てのファイル・ディレクトリのリストを得る。

	- - -

	タイムスタンプ・属性はコピーしない。
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\DirToStream.h"
#include "C:\Factory\Common\Options\PadFile.h"

static uint MD5Counter;

static void MD5Interrupt(void)
{
	MD5Counter++;
	cmdTitle_x(xcout("ncp - MD5 %u", MD5Counter));
}

static char *ServerDomain = "localhost";
static uint ServerPort = 60022;
static uint RetryCount = 7;
static uint RetryWaitMillis = 3000;
static char *PrmFile;
static FILE *PrmFp;
static char *AnsFile;
static FILE *AnsFp;
static int ForceOverwriteMode;
static int RequestAborted;

#define GetSockFileCounter(sf) \
	((sf)->Counter + (uint64)((sf)->Block ? (sf)->Block->Counter : 0))

#define GetSockFileCntrMax(sf) \
	(m_max((sf)->FileSize, 1ui64))

static int Idle(void)
{
	if (pulseSec(3, NULL))
	{
		uint64 prmcnt = GetSockFileCounter(sockClientStatus.PrmFile);
		uint64 prmmax = GetSockFileCntrMax(sockClientStatus.PrmFile);
		uint64 anscnt = GetSockFileCounter(sockClientStatus.AnsFile);
		uint64 ansmax = GetSockFileCntrMax(sockClientStatus.AnsFile);

		cmdTitle_x(xcout("ncp - %I64u / %I64u (%u) %I64u / %I64u (%u)"
			,prmcnt
			,prmmax
			,(uint)((prmcnt * 100ui64) / prmmax)
			,anscnt
			,ansmax
			,(uint)((anscnt * 100ui64) / ansmax)
			));
	}

	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("ABORTED!\n");
			RequestAborted = 1;
			return 0;
		}
	}
	return 1;
}

static void CR_Init(void)
{
	PrmFile = makeTempFile("ncp-prm");
	PrmFp = fileOpen(PrmFile, "wb");
}
static void ClientRequest(void)
{
	uchar ip[4];

	errorCase(!PrmFp); // ? CR_Init() し忘れ

	fileClose(PrmFp);
	PrmFp = NULL;
	PadFile2(PrmFile, "NCP_Prm");

	memset(ip, 0, 4);
	SockStartup();

	for (; ; )
	{
		AnsFile = sockClient(ip, ServerDomain, ServerPort, PrmFile, Idle);

		if (AnsFile && UnpadFile2(AnsFile, "NCP_Ans"))
			break;

		if (RequestAborted || !RetryCount)
		{
			cout("CREATE-ANS-DUMMY\n");
			AnsFile = makeTempFile("ncp-ans-dummy");
			RequestAborted = 1;
			break;
		}
		cout("RETRY %u TIMES\n", RetryCount);
		RetryCount--;
		coSleep(RetryWaitMillis);
	}
	SockCleanup();
	AnsFp = fileOpen(AnsFile, "rb");
}
static void CR_Fnlz(void)
{
	if (PrmFp) fileClose(PrmFp);
	if (AnsFp) fileClose(AnsFp);

	if (PrmFile) removeFile(PrmFile);
	if (AnsFile) removeFile(AnsFile);

	cmdTitle("ncp");

	if (RequestAborted)
	{
		cout("+-------------------------+\n");
		cout("| 失敗または中断しました。|\n");
		cout("+-------------------------+\n");
	}
}

static uint64 IOCounter;

static void WriteToPrmFp(uchar *buffer, uint size)
{
	autoBlock_t gab;

	IOCounter += (uint64)size;

	if (2 <= size)
	{
		cmdTitle_x(xcout("ncp - %I64u bytes wrote", IOCounter));
	}
	writeBinaryBlock(PrmFp, gndBlockVar(buffer, size, gab));
}
static void ReadFromAnsFp(uchar *buffer, uint size)
{
	autoBlock_t *block = readBinaryBlock(AnsFp, size);

	if (size == getSize(block))
	{
		IOCounter += (uint64)size;

		if (2 <= size)
		{
			cmdTitle_x(xcout("ncp - %I64u bytes read", IOCounter));
		}
		memcpy(buffer, directGetBuffer(block), size);
	}
	else
	{
		STD_ReadStop = 1;
	}
	releaseAutoBlock(block);
}
static void ReadEndToStream(FILE *rfp, FILE *wfp)
{
	autoBlock_t *buffer;

	while (buffer = readBinaryStream(rfp, 128 * 1024 * 1024))
	{
		IOCounter += (uint64)getSize(buffer);
		cmdTitle_x(xcout("ncp - %I64u bytes copied", IOCounter));
		writeBinaryBlock_x(wfp, buffer);
	}
}

int main(int argc, char **argv)
{
	mkAppDataDir();

	ServerDomain = getAppDataEnv("SERVER", ServerDomain);
	ServerPort = getAppDataEnv32("NCP_SERVER_PORT", ServerPort);
	RetryCount = getAppDataEnv32("NCP_RETRY", RetryCount);
	RetryWaitMillis = getAppDataEnv32("NCP_RETRY_WAIT_MILLIS", RetryWaitMillis);
	sockClientAnswerFileSizeMax = UINT64MAX;
	md5_interrupt = MD5Interrupt;

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
	if (argIs("/R"))
	{
		RetryCount = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/T"))
	{
		RetryWaitMillis = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/F"))
	{
		ForceOverwriteMode = 1;
		goto readArgs;
	}

	CR_Init();

	if (argIs("/UP") || argIs("UP")) // Upload
	{
		char *clientPath;
		char *serverPath;

		cout("UPLOAD\n");

		clientPath = nextArg();

		if (clientPath[0] == '*')
			clientPath = dropDirFile(); // g

		if (hasArgs(1))
			serverPath = nextArg();
		else
			serverPath = getLocal(clientPath);

		cout("< %s\n", clientPath);
		cout("> %s\n", serverPath);

		errorCase(!*clientPath);
		errorCase(!*serverPath);

		// send commands
		writeLine(PrmFp, serverPath);
		writeLine(PrmFp, "Dummy");
		writeChar(PrmFp, 'U');
		writeChar(PrmFp, ForceOverwriteMode ? 'F' : '-');

		if (existDir(clientPath)) // Directory
		{
			writeChar(PrmFp, 'D');
			DirToStream(clientPath, WriteToPrmFp);
		}
		else if (existFile(clientPath)) // File
		{
			FILE *fp;

			writeChar(PrmFp, 'F');

			fp = fileOpen(clientPath, "rb");
			ReadEndToStream(fp, PrmFp);
			fileClose(fp);
		}
		else
		{
			cout("+--------------------------------------------+\n");
			cout("| クライアント側に指定されたパスはありません |\n");
			cout("+--------------------------------------------+\n");

			goto cr_fnlz;
		}
		cout("SEND...\n");
		ClientRequest();
		cout("SEND-END\n");
	}
	else if (argIs("/DL") || argIs("DL")) // Download
	{
		char *clientPath;
		char *serverPath;
		char *willOpenDir = NULL;
		int type;

		cout("DOWNLOAD\n");

		clientPath = nextArg();
		serverPath = nextArg();

		errorCase(!*clientPath);
		errorCase(!*serverPath);

		if (clientPath[0] == '*')
			clientPath = combine(willOpenDir = makeFreeDir(), getLocal(serverPath)); // g

		cout("> %s\n", clientPath);
		cout("< %s\n", serverPath);

		errorCase(existPath(clientPath));
		errorCase(!creatable(clientPath));

		// send commands
		writeLine(PrmFp, serverPath);
		writeLine(PrmFp, "Dummy");
		writeChar(PrmFp, 'D');
		writeChar(PrmFp, '-');

		cout("RECV...\n");
		ClientRequest();
		cout("RECV-END\n");

		if (RequestAborted)
			goto cr_fnlz;

		type = readChar(AnsFp);

		if (type == 'D') // Directory
		{
			createDir(clientPath);

//			STD_TrustMode = 1;
			StreamToDir(clientPath, ReadFromAnsFp);
//			STD_TrustMode = 0;
		}
		else if (type == 'F') // File
		{
			FILE *fp;

			fp = fileOpen(clientPath, "wb");
			ReadEndToStream(AnsFp, fp);
			fileClose(fp);
		}
		else
		{
			cout("Error: bad type. %02x\n", type);
		}

		if (willOpenDir)
		{
			execute_x(xcout("START \"\" \"%s\"", willOpenDir));
			memFree(willOpenDir);
		}
	}
	else if (argIs("/SZ") || argIs("SZ")) // Size
	{
		char *serverPath = nextArg();
		int type;

		errorCase(!*serverPath);

		// send commands
		writeLine(PrmFp, serverPath);
		writeLine(PrmFp, "Dummy");
		writeChar(PrmFp, 'S');
		writeChar(PrmFp, '-');

		ClientRequest();

		if (RequestAborted)
			goto cr_fnlz;

		type = readChar(AnsFp);

		if (type == 'D')
		{
			cout("%s byte(s) directory exists.\n", c_thousandComma(xcout("%I64u", readValue64(AnsFp))));
		}
		else if (type == 'F')
		{
			cout("%s byte(s) file exists.\n", c_thousandComma(xcout("%I64u", readValue64(AnsFp))));
		}
		else if (type == 'N')
		{
			cout("not exists.\n");
		}
		else
		{
			cout("+--------------+\n");
			cout("| わかんねーよ |\n");
			cout("+--------------+\n");
		}
	}
	else if (argIs("/MV") || argIs("MV")) // Move
	{
		char *serverPath1;
		char *serverPath2;

		serverPath1 = nextArg();
		serverPath2 = nextArg();

		errorCase(!*serverPath1);
		errorCase(!*serverPath2);

		cout("MOVE\n");

		// send commands
		writeLine(PrmFp, serverPath1);
		writeLine(PrmFp, serverPath2);
		writeChar(PrmFp, 'M');
		writeChar(PrmFp, ForceOverwriteMode ? 'F' : '-');

		ClientRequest();
	}
	else if (argIs("/RM") || argIs("RM")) // Remove
	{
		char *serverPath = nextArg();

		errorCase(!*serverPath);

		cout("REMOVE\n");

		// send commands
		writeLine(PrmFp, serverPath);
		writeLine(PrmFp, "Dummy");
		writeChar(PrmFp, 'X');
		writeChar(PrmFp, '-');

		ClientRequest();
	}
	else if (argIs("/LS") || argIs("LS")) // List
	{
		char *path;

		if (hasArgs(1))
		{
			char *serverPath = nextArg();

			// send commands
			writeLine(PrmFp, serverPath);
			writeLine(PrmFp, "Dummy");
			writeChar(PrmFp, 'J');
			writeChar(PrmFp, '-');
		}
		else
		{
			// send commands
			writeLine(PrmFp, "Dummy");
			writeLine(PrmFp, "Dummy");
			writeChar(PrmFp, 'K');
			writeChar(PrmFp, '-');
		}
		ClientRequest();

		while (path = readLine(AnsFp))
		{
			line2JLine(path, 1, 0, 0, 1);
			cout("%s%s\n", majorOutputLinePrefix, path);
			memFree(path);
		}
	}
	else if (argIs("/LSS") || argIs("LSS")) // List
	{
		char *path;

		// send commands
		writeLine(PrmFp, "Dummy");
		writeLine(PrmFp, "Dummy");
		writeChar(PrmFp, 'L');
		writeChar(PrmFp, '-');

		ClientRequest();

		while (path = readLine(AnsFp))
		{
			line2JLine(path, 1, 0, 0, 1);
			cout("%s%s\n", majorOutputLinePrefix, path);
			memFree(path);
		}
	}
	else
	{
		cout("不明なコマンド\n");
	}

cr_fnlz:
	CR_Fnlz();

	termination(RequestAborted ? 1 : 0);
}
