#include "nSyncCommon.h"

char *NS_AppTitle = "_";

void NS_DeletePath(char *path)
{
	cout("- %s\n", path);

	recurRemovePathIfExist(path);
}
void NS_CreateParent(char *path)
{
	cout("+ %s\n", path);

	recurRemovePathIfExist(path);
	createPath(path, 'X');
}
void NS_SendFile(SockStream_t *ss, char *file)
{
	uint64 createStamp;
	uint64 writeStamp;
	uint64 fileSize;
	FILE *fp;

	cout("SEND-FILE\n");

	getFileStamp(file, &createStamp, NULL, &writeStamp);
	fileSize = getFileSize(file);

	SockSendValue64(ss, createStamp);
	SockSendValue64(ss, writeStamp);
	SockSendValue64(ss, fileSize);

	SockSendChar(ss, 'A'); // Alive

	fp = fileOpen(file, "rb");

	for (; ; )
	{
		autoBlock_t *buff = readBinaryStream(fp, 20000000); // 20 MB

		if (!buff)
			break;

		if (pulseSec(1, NULL))
		{
			uint64 count = getSeekPos(fp);
			cmdTitle_x(xcout("%s Send %I64u / %I64u = %.3fPct", NS_AppTitle, count, fileSize, count * 100.0 / fileSize));

			if (IsEOFSockStream(ss))
			{
				cout("接続が死んでるずら。SF\n");
				break;
			}
		}
		SockSendBlock(ss, directGetBuffer(buff), getSize(buff));
		releaseAutoBlock(buff);
	}
	fileClose(fp);

	SockSendChar(ss, 'A'); // Alive
	SockFlush(ss);

	cmdTitle_x(xcout("%s Send Ok", NS_AppTitle));
	cout("SEND-FILE-OK\n");
}
void NS_RecvFile(SockStream_t *ss, char *file)
{
	char *midFile = makeTempPath(NULL);
	uint64 createStamp;
	uint64 writeStamp;
	uint64 fileSize;
	uint64 count;
	uint64 repSpan;
	FILE *fp;

	cout("RECV-FILE\n");

	createStamp = SockRecvValue64(ss);
	writeStamp  = SockRecvValue64(ss);
	fileSize    = SockRecvValue64(ss);

	if (SockRecvChar(ss) != 'A') // Alive
	{
		cout("この接続は死んでますわ。(STAMP-SIZE-RECVED)\n");
		goto endFunc;
	}

	fp = fileOpen(midFile, "wb");

	repSpan = fileSize / 10;
	m_minim(repSpan, 30000000); // 30 MB

	for (count = 0; count < fileSize; count++)
	{
		if (eqIntPulseSec(1, NULL))
		{
			cmdTitle_x(xcout("%s Recv %I64u / %I64u = %.3fPct", NS_AppTitle, count, fileSize, count * 100.0 / fileSize));

			if (IsEOFSockStream(ss))
			{
				cout("接続が死んでるずら。RF\n");
				break;
			}
		}
		writeChar(fp, SockRecvChar(ss));
	}
	fileClose(fp);

	if (SockRecvChar(ss) != 'A') // Alive
//	if (IsEOFSockStream(ss))
	{
		cout("この接続は死んでますわ。(DATA-RECVED)\n");
		removeFile(midFile);
		goto endFunc;
	}

	setFileStamp(midFile, createStamp, 0, writeStamp);
	NS_CreateParent(file);
	moveFile(midFile, file);

endFunc:
	memFree(midFile);

	cmdTitle_x(xcout("%s Recv Ok", NS_AppTitle));
	cout("RECV-FILE-OK\n");
}
