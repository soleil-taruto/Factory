/*
	Service.dat �Ɉȉ���ǉ�

		X-BIND C:\Factory\SubTools\HTT\Bind.exe /S

	���b�N

		Bind.exe /L DOMAIN PORT-NO [/CEX CONNECT-ERROR-MAX]

			CONNECT-ERROR-MAX ...

				�ڑ����s�񐔂�����ɒB����ƒ��f (�L�����Z���Ɠ���) ����B
				�T�[�o�[����~���Ă��邱�Ƃ��l������Ƃ��̂��߁B
				�f�t�H���g == UINTMAX (����)

	���b�N����

		Bind.exe /U

	- - -

	������PC�Ŏ��s���ĂˁB
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\Progress.h"

#define MUTEX_UUID "{c4049337-18db-4a1d-84ab-eb0a82310c85}"
#define LOCKED_EVENT_UUID "{57349467-d90b-4e77-804e-516c4d07c0d6}"
#define UNLOCK_EVENT_UUID "{715811de-35ff-4d11-928f-e94f1690ab23}"

#define CLIENT_LOCK_FLAG_FILE "C:\\Factory\\tmp\\Bind_ClientLock.flg"
#define SERVER_LOCK_FLAG_FILE "C:\\Factory\\tmp\\Bind_ServerLock.flg"

static char *Domain;
static uint PortNo;
static uint ConnectErrorMax = UINTMAX;
static uint ConnectErrorCount;

static autoBlock_t *GetHTTRequestMessage(void)
{
	return ab_makeBlockLine("X-BIND\x20");
}
static void ThrowHTTRequest(void)
{
	static uchar ip[4];
	static int sock = -1;
	autoBlock_t *message;

	if (sock != -1)
	{
//		LOGPOS();
		sockDisconnect(sock);
	}
//	LOGPOS();
	sock = sockConnect(ip, Domain, PortNo);
//	cout("sock: %d\n", sock);

	if (sock == -1)
	{
		ConnectErrorCount++;
		return;
	}
//	LOGPOS();
	message = GetHTTRequestMessage();
	SockSendSequLoop(sock, message, 2000);
	releaseAutoBlock(message);
//	LOGPOS();
}

static void DoWakeup(uint hdl)
{
	LOGPOS();
	eventSet(hdl);
	LOGPOS();
}
static void DoWait(uint hdl, void (*interrupt)(void))
{
	int cancelled = 0;

	LOGPOS();

	if (interrupt == ThrowHTTRequest)
	{
		cout("���b�N���悤�Ƃ��Ă��܂�...\n");
		cout("+-----------------------------------+\n");
		cout("| F �������ƃ��b�N�����ɑ��s���܂��B|\n");
		cout("+-----------------------------------+\n");
		cout("�����܂ł����b�N�ł��Ȃ����R���T�[�o�[����~���Ă���B\n");
		cout("���T�[�o�[����~���Ă���Ȃ�A���b�N����K�v�͂���܂���B\n");
		cout("�@F �������đ��s���ĉ������B\n");
	}
	else // ? interrupt == noop
	{
		cout("���b�N����Ă��܂�...\n");
		cout("+---------------------------------------+\n");
		cout("| F �������Ƌ����I�Ƀ��b�N���������܂��B|\n");
		cout("+---------------------------------------+\n");
		cout("�����܂ł����b�N����������Ȃ����R�����b�N����.bat���ُ�I�������B\n");
		cout("�����b�N����.bat���I�����������m�F�̏�AF �������đ��s���ĉ������B\n");
	}
	ProgressBegin();

	for (; ; )
	{
		Progress();
		interrupt();

		if (ConnectErrorMax != UINTMAX && ConnectErrorMax <= ConnectErrorCount)
		{
			cout("CONNECT-ERROR-MAX\n");
			cancelled = 1;
			break;
		}
		if (handleWaitForMillis(hdl, 2000))
			break;

		while (hasKey())
			if (getKey() == 'F')
				cancelled = 1;

		if (cancelled)
			break;
	}
	ProgressEnd(cancelled);

	if (cancelled)
	{
		if (interrupt == ThrowHTTRequest)
		{
			cout("+-------------------------+\n");
			cout("| ���b�N�����ɑ��s���܂��B|\n");
			cout("+-------------------------+\n");
		}
		else // ? interrupt == noop
		{
			cout("+-------------------------------+\n");
			cout("| ���b�N�������I�ɉ������܂����B|\n");
			cout("+-------------------------------+\n");
		}
	}
	else // ? ����
	{
		if (interrupt == ThrowHTTRequest)
		{
			cout("���b�N���܂����B\n");
		}
		else // ? interrupt == noop
		{
			cout("���b�N�͉�������܂����B\n");
		}
	}
	LOGPOS();
}

static void LockClient(void)
{
	int cancelled = 0;

	LOGPOS();

	cout("���̃N���C�A���g�����b�N���悤�Ƃ��Ă��܂�...\n");
	cout("�����܂ł����b�N�ł��Ȃ����R�����b�N����.bat���ُ�I�������B\n");
	cout("�����b�N����.bat���I�����������m�F�̏�AF �������đ��s���ĉ������B\n");

	ProgressBegin();

	for (; ; )
	{
		Progress();

		mutex();
		{
			if (!existFile(CLIENT_LOCK_FLAG_FILE))
			{
				createFile(CLIENT_LOCK_FLAG_FILE);
				unmutex();
				break;
			}
		}
		unmutex();

		sleep(2000); // �ő��ɖ�������������A�X���[�v�ł�����..

		while (hasKey())
			if (getKey() == 'F')
				cancelled = 1;

		if (cancelled)
			break;
	}
	ProgressEnd(cancelled);

	if (cancelled)
		cout("���̃N���C�A���g�����b�N�����ɑ��s���܂��B\n"); // �C���M�����[
	else
		cout("���̃N���C�A���g�����b�N���܂����B\n"); // ���K

	LOGPOS();
}
static void UnlockClient(void)
{
	int unlocked = 0;

	LOGPOS();

	mutex();
	{
		if (existFile(CLIENT_LOCK_FLAG_FILE))
		{
			removeFile(CLIENT_LOCK_FLAG_FILE);
			unlocked = 1;
		}
	}
	unmutex();

	if (unlocked)
		cout("���̃N���C�A���g�̃��b�N���������܂����B\n"); // ���K
	else
		cout("���̃N���C�A���g�̓��b�N����Ă��܂���B\n"); // �C���M�����[

	LOGPOS();
}

int main(int argc, char **argv)
{
	int hdlMtx = mutexOpen(MUTEX_UUID);
	int hdlLocked = eventOpen(LOCKED_EVENT_UUID);
	int hdlUnlock = eventOpen(UNLOCK_EVENT_UUID);

	if (argIs("/S")) // Service
	{
//		LOGPOS();
		handleWaitForever(hdlMtx);
		{
			if (!existFile(SERVER_LOCK_FLAG_FILE))
			{
				mutexRelease(hdlMtx);
//				LOGPOS();
				goto endProc;
			}
			removeFile(SERVER_LOCK_FLAG_FILE);
		}
		mutexRelease(hdlMtx);
		LOGPOS();

		collectEvents(hdlUnlock, 0); // �S�~�C�x���g���

		DoWakeup(hdlLocked);
		DoWait(hdlUnlock, noop);
	}
	else if (argIs("/L")) // Lock
	{
		Domain = nextArg();
		PortNo = toValue(nextArg());

		if (argIs("/CEX"))
			ConnectErrorMax = toValue(nextArg());

		errorCase(hasArgs(1)); // �s���ȃI�v�V����
		errorCase(m_isEmpty(Domain));
		errorCase(!PortNo || 0xffff < PortNo);

		LockClient();

		LOGPOS();
		handleWaitForever(hdlMtx);
		{
			createFile(SERVER_LOCK_FLAG_FILE);
		}
		mutexRelease(hdlMtx);
		LOGPOS();

		SockStartup();
		DoWait(hdlLocked, ThrowHTTRequest);
		SockCleanup();
	}
	else if (argIs("/U")) // Unlock
	{
		DoWakeup(hdlUnlock);
		UnlockClient();
	}

endProc:
	handleClose(hdlMtx);
	handleClose(hdlLocked);
	handleClose(hdlUnlock);
}
