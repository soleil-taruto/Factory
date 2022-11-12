/*
	fileBox.exe /p 59999 /u homura-chan
	鯖名 homuhomu.net とすると http://homuhomu.net:59999/homura-chan/file.bin でファイルを落とせる。
	file.bin の部分は自由、URL に URL-phrase である "homura-chan" が入っていれば良い。

	fbSend.exe homuhomu.net:59999/homura-chan upfile.zip でアップロードできる。
	UL中にDL開始できるし、DL中にUL開始できるが、その時DLしたファイルの内容は保障しない。多分、ニコイチか尻切れ蜻蛉
	複数同時にUL開始できるが、その時ULしたファイルの内容は保障しない。多分、ごちゃ混ぜになっている。
*/

#include "C:\Factory\Common\Options\SockServer.h"

#define HDRHDR_LENMAX 4096
#define HTTP_NEWLINE "\r\n"
#define READ_BUFF_SIZE 3000000

static char *URLPhrase = "otankonasu"; // 簡単な認証のために URL に含めなければならない文字列
static uint64 UpSizeMax = 5ui64 * 1024 * 1024 * 1024;
static char *UpFile;

typedef struct Info_st
{
	time_t ConnectTime;
	int UpMode;
	uint64 ReadIndex;
	autoBlock_t *DLBuffer;
	autoBlock_t *UpBuffer;
}
Info_t;

static uint CreateInfo(void)
{
	Info_t *i = (Info_t *)memAlloc(sizeof(Info_t));

	i->ConnectTime = SockCurrTime;
	i->ReadIndex = 0;
	i->DLBuffer = NULL;
	i->UpBuffer = newBlock();
	i->UpMode = 0;

	return (uint)i;
}
static void ReleaseInfo(uint prm)
{
	Info_t *i = (Info_t *)prm;

	if (i->DLBuffer)
		releaseAutoBlock(i->DLBuffer);

	releaseAutoBlock(i->UpBuffer);
	memFree(i);
}
static int Perform(int sock, uint prm)
{
	Info_t *i = (Info_t *)prm;
	FILE *fp;
	static char *line;

	if (i->DLBuffer)
	{
		if (!getSize(i->DLBuffer))
		{
			uint64 upFileSize = getFileSize(UpFile);

			if (upFileSize <= i->ReadIndex) // ? 読み込み終了 || UpFile が更新されて ReadIndex より短くなった。
				return 0;

			nobSetSize(i->DLBuffer, m_min((uint64)READ_BUFF_SIZE, upFileSize - i->ReadIndex));

			fp = fileOpen(UpFile, "rb");
			fileSeek(fp, SEEK_SET, i->ReadIndex);
			fileRead(fp, i->DLBuffer);
			fileClose(fp);

			i->ReadIndex += getSize(i->DLBuffer);
		}
		return SockSendSequ(sock, i->DLBuffer, 1) != -1;
	}
	if (i->UpMode)
	{
		setSize(i->UpBuffer, 0);

		if (SockRecvSequ(sock, i->UpBuffer, 1) == -1 ||
			UpSizeMax < getFileSize(UpFile) + getSize(i->UpBuffer)) // Out of 'UpSizeMax' range
			return 0;

		fp = fileOpen(UpFile, "ab");
		writeBinaryBlock(fp, i->UpBuffer);
		fileClose(fp);
		return 1;
	}
	if (SockCurrTime < i->ConnectTime + 2L) // 機械的に URLPhrase を探られないように、少し待つ
	{
		Sleep(1);
		return 1;
	}
	if (SockRecvSequ(sock, i->UpBuffer, 1) == -1)
		return 0;

	if (line)
		memFree(line);

	line = SockNextLine(i->UpBuffer);

	if (!line)
		return getSize(i->UpBuffer) < HDRHDR_LENMAX + 2;

	{
	char *strw;
	line = lineToPrintLine(strw = line, 0); memFree(strw);
	}
	cout("[%s] %s\n", SockIp2Line(sockClientIp), line);

	if (HDRHDR_LENMAX < strlen(line) ||
		!strstr(line, URLPhrase)) // URLPhrase を URL に入れてね。
		return 0;

	if (line[0] == '!') // ? upload mode
	{
		writeBinary(UpFile, i->UpBuffer);
		i->UpMode = 1;
		return 1;
	}
	i->DLBuffer = newBlock();

	ab_addLine(i->DLBuffer, "HTTP/1.1 200 OK" HTTP_NEWLINE);
	ab_addLine(i->DLBuffer, "Content-Type: application/octet-stream" HTTP_NEWLINE);
	ab_addLine_x(i->DLBuffer, xcout("Content-Length: %I64u" HTTP_NEWLINE, getFileSize(UpFile)));
	ab_addLine(i->DLBuffer, HTTP_NEWLINE);

	return 1;
}

static int Idle(void)
{
	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("Exit.\n");
			return 0;
		}
		cout("Press esc to exit.\n");
	}
	return 1;
}
int main(int argc, char **argv)
{
	uint portno = 10080;

readArgs:
	if (argIs("/P")) // Port
	{
		portno = toValue(nextArg());
		goto readArgs;
	}
	if (argIs("/U")) // URL-phrase
	{
		URLPhrase = nextArg();
		goto readArgs;
	}
	if (argIs("/X")) // maX upload file size
	{
		UpSizeMax = toValue64(nextArg());
		goto readArgs;
	}

	UpFile = makeTempPath("up-file");
	writeOneLine(UpFile, "404 - Not Found");

	sockServerUserTransmit(Perform, CreateInfo, ReleaseInfo, portno, 10, Idle);

	removeFile(UpFile);
	memFree(UpFile);
}
