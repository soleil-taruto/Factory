/*
	nSyncServer.exe [/HPW hello-password] recv-port root-dir
*/

#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "libs\nSyncCommon.h"
#include "libs\nSyncDefine.h"

static char *HelloPassword;
static uint RecvPort;
static char *RootDir;

static char *ActiveDirDummy;
static char *ActiveDir;

static int CheckPath(char *path, char *parentDir)
{
#if 1
	// HACK: �S���Ȃ����낤��...
	return !m_isEmpty(path) && strlen(parentDir) + 1 + strlen(path) <= PATH_SIZE;
#else
	return isFairRelPath(path, strlen(parentDir));
#endif
}
static char *RecvPath(SockStream_t *ss, char *parentDir)
{
	char *path = SockRecvLine(ss, RECV_LINE_LENMAX);
	char *ret;

	if (!CheckPath(path, parentDir))
	{
		cout("�p�X���ɖ�肪����܂��I\n");
		line2JLine(path, 1, 0, 0, 1); // �\���̂���
		cout("! %s\n", path);

		memFree(path);
		return NULL;
	}
	ret = combine_cx(parentDir, path);
	cout("$ %s\n", ret);
	return ret;
}
static void SendDirsAndFiles(SockStream_t *ss, char *dir)
{
	autoList_t *paths = lss(dir);
	char *path;
	uint index;

	changeRoots(paths, ActiveDir, NULL);

	insertElement(paths, lastDirCount, (uint)strx(""));

	foreach (paths, path, index)
		SockSendLine(ss, path);

	releaseDim(paths, 1);

	SockSendLine(ss, "");
}
static int Perform(int sock, uint dummyPrm)
{
	SockStream_t *ss = CreateSockStream(sock, 2);
	char *command = NULL;

	LOGPOS();

	// �ŏ��ɗ��� ECHO_WORD_REQ �܂ł͒Z���^�C���A�E�g��ݒ肷��B
	// ����ȍ~�͖�����

	if (HelloPassword)
	{
		char *recvHPw;

		cout("�����t�� [%s] �ł���B\n", HelloPassword);

		recvHPw = SockRecvLine(ss, strlen(HelloPassword) + 1);

		if (strcmp(recvHPw, HelloPassword))
		{
			cout("�Ԃ��ԁ[�ł���I\n");
//			coSleep(3000); // �ԈႦ�Đڑ��Ƃ����̂����肻���Ȃ̂ŁA�҂��Ȃ��B
			memFree(recvHPw);
			goto endFunc;
		}
		memFree(recvHPw);
	}
	for (; ; )
	{
		command = SockRecvLine(ss, RECV_LINE_LENMAX);

		// �\���̂��� -- "", ECHO_WORD_REQ �ɒ��ӁI
		if (*command)
			line2csym_ext(command, ".{-}");

		cout("command: %s\n", command);

		if (!*command) // �ؒf�܂��͒ʐM�G���[
			break;

		if (!strcmp(command, ECHO_WORD_REQ))
		{
			LOGPOS();

			SetSockStreamTimeout(ss, 0);
			SockSendLine(ss, ECHO_WORD_ANS);
		}
		else if (!strcmp(command, "Start"))
		{
			char *dir = RecvPath(ss, RootDir);

			if (!dir)
				break;

			if (!existDir(dir))
			{
				cout("����ȃf�B���N�g�����݂��܂����B\n");
				memFree(dir);
				break;
			}
			memFree(ActiveDir);
			ActiveDir = dir;
			SendDirsAndFiles(ss, ActiveDir);
		}
		else if (!strcmp(command, "MD"))
		{
			char *dir = RecvPath(ss, ActiveDir);

			if (!dir)
				break;

			NS_CreateParent(dir);
			createDir(dir);
			memFree(dir);
		}
		else if (!strcmp(command, "Delete"))
		{
			char *path = RecvPath(ss, ActiveDir);

			if (!path)
				break;

			NS_DeletePath(path);
			memFree(path);
		}
		else if (!strcmp(command, "Clear"))
		{
			recurClearDir(ActiveDir);
		}
		else if (!strcmp(command, "Send"))
		{
			char *file = RecvPath(ss, ActiveDir);

			if (!file)
				break;

			NS_RecvFile(ss, file);
			memFree(file);
		}
		else if (!strcmp(command, "Recv"))
		{
			char *file = RecvPath(ss, ActiveDir);

			if (!file)
				break;

			if (!existFile(file))
			{
				cout("Recv_����ȃt�@�C������܂����B\n");
				memFree(file);
				break;
			}
			NS_SendFile(ss, file);
			memFree(file);
		}
		else if (!strcmp(command, "GetFileStamp"))
		{
			char *file = RecvPath(ss, ActiveDir);
			uint64 createStamp;
			uint64 writeStamp;

			if (!file)
				break;

			if (!existFile(file))
			{
				cout("GFS_����ȃt�@�C������܂����B\n");
				memFree(file);
				break;
			}
			getFileStamp(file, &createStamp, NULL, &writeStamp);

			SockSendValue64(ss, createStamp);
			SockSendValue64(ss, writeStamp);
			SockFlush(ss);

			memFree(file);
		}
		else
		{
			cout("�s���ȃR�}���h�ł���B\n");
			break;
		}
		memFree(command);
	}

endFunc:
	memFree(command);
	ReleaseSockStream(ss);
	return 0;
}
static int Idle(void)
{
	while (hasKey())
		if (getKey() == 0x1b)
			return 0;

	return 1;
}
int main(int argc, char **argv)
{
	if (argIs("/HPW"))
	{
		HelloPassword = nextArg();
	}
	RecvPort = toValue(nextArg());
	RootDir = nextArg();

	errorCase(!m_isRange(RecvPort, 1, 65535));
	errorCase(!existDir(RootDir));

	RootDir = makeFullPath(RootDir);

	cout("RecvPort: %u\n", RecvPort);
	cout("RootDir: %s\n", RootDir);

	ActiveDirDummy = makeTempDir(NULL);
	ActiveDir = strx(ActiveDirDummy);

	NS_AppTitle = "nSyncServer";
	cmdTitle(NS_AppTitle);

	// memo: �����ڑ��֎~�BPerform ���ؒf���Ă��玟�̐ڑ������ɍs���̂ŁAconnectmax �� 1 �ŗǂ��I
	sockServerUserTransmit(Perform, getZero, noop_u, RecvPort, 1, Idle);

	cmdTitle(NS_AppTitle);

	recurRemoveDir(ActiveDirDummy);
}
