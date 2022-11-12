/*
	Service.dat に以下を追加

		X-BIND C:\Factory\SubTools\HTT\Bind.exe /S

	ロック

		Bind.exe /L DOMAIN PORT-NO [/CEX CONNECT-ERROR-MAX]

			CONNECT-ERROR-MAX ...

				接続失敗回数がこれに達すると中断 (キャンセルと同じ) する。
				サーバーが停止していることを考慮するときのため。
				デフォルト == UINTMAX (無効)

	ロック解除

		Bind.exe /U

	- - -

	★同じPCで実行してね。
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
		cout("ロックしようとしています...\n");
		cout("+-----------------------------------+\n");
		cout("| F を押すとロックせずに続行します。|\n");
		cout("+-----------------------------------+\n");
		cout("■いつまでもロックできない理由＝サーバーが停止している。\n");
		cout("▲サーバーが停止しているなら、ロックする必要はありません。\n");
		cout("　F を押して続行して下さい。\n");
	}
	else // ? interrupt == noop
	{
		cout("ロックされています...\n");
		cout("+---------------------------------------+\n");
		cout("| F を押すと強制的にロックを解除します。|\n");
		cout("+---------------------------------------+\n");
		cout("■いつまでもロックが解除されない理由＝ロックした.batが異常終了した。\n");
		cout("▲ロックした.batが終了した事を確認の上、F を押して続行して下さい。\n");
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
			cout("| ロックせずに続行します。|\n");
			cout("+-------------------------+\n");
		}
		else // ? interrupt == noop
		{
			cout("+-------------------------------+\n");
			cout("| ロックを強制的に解除しました。|\n");
			cout("+-------------------------------+\n");
		}
	}
	else // ? 成功
	{
		if (interrupt == ThrowHTTRequest)
		{
			cout("ロックしました。\n");
		}
		else // ? interrupt == noop
		{
			cout("ロックは解除されました。\n");
		}
	}
	LOGPOS();
}

static void LockClient(void)
{
	int cancelled = 0;

	LOGPOS();

	cout("他のクライアントをロックしようとしています...\n");
	cout("■いつまでもロックできない理由＝ロックした.batが異常終了した。\n");
	cout("▲ロックした.batが終了した事を確認の上、F を押して続行して下さい。\n");

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

		sleep(2000); // 滅多に無さそうだから、スリープでいいや..

		while (hasKey())
			if (getKey() == 'F')
				cancelled = 1;

		if (cancelled)
			break;
	}
	ProgressEnd(cancelled);

	if (cancelled)
		cout("他のクライアントをロックせずに続行します。\n"); // イレギュラー
	else
		cout("他のクライアントをロックしました。\n"); // 正規

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
		cout("他のクライアントのロックを解除しました。\n"); // 正規
	else
		cout("他のクライアントはロックされていません。\n"); // イレギュラー

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

		collectEvents(hdlUnlock, 0); // ゴミイベント回収

		DoWakeup(hdlLocked);
		DoWait(hdlUnlock, noop);
	}
	else if (argIs("/L")) // Lock
	{
		Domain = nextArg();
		PortNo = toValue(nextArg());

		if (argIs("/CEX"))
			ConnectErrorMax = toValue(nextArg());

		errorCase(hasArgs(1)); // 不明なオプション
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
