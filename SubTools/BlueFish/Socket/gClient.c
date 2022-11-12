/*
	gClient.exe �T�[�o�[�� �|�[�g�ԍ� (u | d) ���[���� �N���C�A���g���f�B���N�g��
*/

#include "C:\Factory\Common\Options\SClient.h"
#include "gDefine.h"

static char *Server;
static uint PortNo;
static char *Command;
static char *Lane;
static char *ClientDir;

#define BUFFSIZE (4 * 1024 * 1024)

static uchar Buff[BUFFSIZE];

static int RecvResponse(SockStream_t *ss, char *expectedLine)
{
	char *line;
	int ret;

	SockFlush(ss); // �����Ńt���b�V�����Ă��I

	line = SockRecvLine(ss, 100);
	ret = !strcmp(line, expectedLine);
	memFree(line);

	cout("expectedLine: %s -> %d\n", expectedLine, ret);

	if (!ret)
		cout("�����������G���[�I\n");

	return ret;
}
static void Upload(SockStream_t *ss)
{
	autoList_t *files;
	char *file;
	uint index;

	LOGPOS();
	files = lsFiles(ClientDir);
	LOGPOS();
	sortJLinesICase(files);
	LOGPOS();

	foreach (files, file, index)
	{
		uint64 fileSize;
		FILE *fp;

		LOGPOS();

		if (!RecvResponse(ss, "READY-NEXT-FILE"))
			goto netError;

		LOGPOS();
		fileSize = getFileSize(file);
		LOGPOS();
		SockSendChar(ss, 'B');
		LOGPOS();
		SockSendValue64(ss, fileSize);
		LOGPOS();

		if (!RecvResponse(ss, "READY-FILE-ENTITY"))
			goto netError;

		LOGPOS();
		SockSendLine(ss, getLocal(file));
		LOGPOS();
		fp = fileOpen(file, "rb");

		while (0ui64 < fileSize)
		{
			uint sendSize = (uint)m_min((uint64)BUFFSIZE, fileSize);
			autoBlock_t gab;

			fileRead(fp, gndBlockVar(Buff, sendSize, gab));
			SockSendBlock(ss, Buff, sendSize);
			fileSize -= sendSize;
		}
		fileClose(fp);
		LOGPOS();
		SockSendChar(ss, 'E');
		LOGPOS();

		if (!RecvResponse(ss, "RECV-FILE-COMPLETED"))
			goto netError;

		LOGPOS();
		removeFile(file);
		LOGPOS();
	}
	LOGPOS();

	if (!RecvResponse(ss, "READY-NEXT-FILE"))
		goto netError;

	LOGPOS();
	SockSendChar(ss, 0xff);
	LOGPOS();

	if (!RecvResponse(ss, "OK"))
		goto netError;

	cout("+--------------------------------+\n");
	cout("| �A�b�v���[�h�͑S�Đ������܂��� |\n");
	cout("+--------------------------------+\n");

netError:
	LOGPOS();
	releaseDim(files, 1);
	LOGPOS();
}
static void Download(SockStream_t *ss)
{
	LOGPOS();

	for (; ; )
	{
		uint64 fileSize;
		char *name;
		char *file;
		FILE *fp;
		int errorFlag = 0;

		LOGPOS();

		if (!SockRecvValue(ss))
			break;

		LOGPOS();
		fileSize = SockRecvValue64(ss);
		cout("fileSize: %I64u\n", fileSize);
		name = SockRecvLine(ss, 100);
		name = lineToFairLocalPath_x(name, strlen(ClientDir));
		cout("name: %s\n", name);
		file = combine(ClientDir, name);
		cout("file: %s\n", file);
		fp = fileOpen(file, "wb"); // �������O�̃t�@�C���͏㏑���I
		LOGPOS();

		while (0ui64 < fileSize)
		{
			uint recvSize = (uint)m_min((uint64)BUFFSIZE, fileSize);
			autoBlock_t gab;

			if (!SockRecvBlock(ss, Buff, recvSize))
			{
				cout("�������t�@�C���f�[�^��M�G���[�I\n");
				errorFlag = 1;
				break;
			}
			writeBinaryBlock(fp, gndBlockVar(Buff, recvSize, gab));
			fileSize -= recvSize;
		}
		LOGPOS();
		fileClose(fp);
		LOGPOS();
		SockSendChar(ss, 'C'); // memo: !SockRecvBlock() �ɂȂ����ꍇ Sock == -1 �Ȃ̂ŁA�ȍ~����M���Ȃ��͂��B
		LOGPOS();
		SockFlush(ss);
		LOGPOS();

		if (SockRecvChar(ss) != 'D')
		{
			cout("�������t�@�C���I�[����M�G���[�I\n");
			errorFlag = 1;
		}
		LOGPOS();

		if (errorFlag)
		{
			LOGPOS();
			removeFile(file);
			LOGPOS();
		}
		LOGPOS();
		memFree(file);
		memFree(name);
		LOGPOS();

		if (errorFlag)
			goto netError;

		LOGPOS();
	}
	cout("+--------------------------------+\n");
	cout("| �_�E�����[�h�͑S�Đ������܂��� |\n");
	cout("+--------------------------------+\n");

netError:
	LOGPOS();
}
static int Perform(int sock, uint prm)
{
	SockStream_t *ss = CreateSockStream(sock, 3600); // �t�@�C������M������̂ŁA���߂�

	LOGPOS();

	SockSendToken(ss, COMMAND_PREFIX);
	SockSendLine(ss, Command);
	SockSendLine(ss, Lane);

	LOGPOS();

	if (Command[0] == 'u')
	{
		Upload(ss);
	}
	else if (Command[0] == 'd')
	{
		Download(ss);
	}
	else
	{
		cout("�������s���ȃR�}���h�I");
	}
	LOGPOS();

	ReleaseSockStream(ss);
	return 1;
}
int main(int argc, char **argv)
{
	Server    = nextArg();
	PortNo    = toValue(nextArg());
	Command   = nextArg();
	Lane      = nextArg();
	ClientDir = nextArg();

	errorCase(m_isEmpty(Server));
	errorCase(!m_isRange(PortNo, 1, 65535));
	errorCase(m_isEmpty(Command));
	errorCase(m_isEmpty(Lane));
	errorCase(!existDir(ClientDir));

	LOGPOS();

	if (!SClient(Server, PortNo, Perform, 0))
		cout("�������ڑ��G���[�I\n");

	LOGPOS();
}
