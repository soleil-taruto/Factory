#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Common\Options\SockStream.h"
#include "oCDefine.h"

static void Perform_o(SockStream_t *ss)
{
	char *dir = makeTempDir(NULL);

	addCwd(dir);
	{
		// 合図
		SockSendChar(ss, '1');
		SockFlush(ss);

		coExecute("C:\\Factory\\Tools\\ncp.exe /S localhost /P 60022 DL oto oto");

		// 合図
		SockSendChar(ss, '2');
		SockFlush(ss);

		createDirIfNotExist("oto"); // oto\ が無かった場合 | 通信に失敗した場合 対策

		coExecute("C:\\Factory\\Tools\\Cluster.exe /M oto.clu oto");
//		coExecute("C:\\Factory\\Tools\\z9.bat /M oto oto.clu.gz.enc");

		// 合図
		SockSendChar(ss, 'S');

		{
			FILE *fp = fileOpen("oto.clu", "rb");
//			FILE *fp = fileOpen("oto.clu.gz.enc", "rb");

			for (; ; )
			{
				autoBlock_t *block = readBinaryStream(fp, 4 * 1024 * 1024);

				if (!block)
					break;

				SockSendBlock(ss, directGetBuffer(block), getSize(block));
				releaseAutoBlock(block);
			}
			fileClose(fp);
			SockFlush(ss);
		}
	}
	unaddCwd();

	recurRemoveDir_x(dir);
}
static void Perform_C(SockStream_t *ss)
{
	coExecute("C:\\Factory\\SubTools\\nrun.exe /S localhost /P 60123 /R 3 KCamera\\start");
}
static int Perform(int sock, uint dummyPrm)
{
	SockStream_t *ss = CreateSockStream(sock, 2);
	char *command;

	command = SockRecvLine(ss, 100);
	line2JLine(command, 1, 0, 0, 0);
	cout("command: %s\n", command);

	SetSockStreamTimeout(ss, 3600); // otoの送信(DL,z9している間の待ち)があるので、長めに

	if (!strcmp(command, COMMAND_PREFIX "o"))
	{
		Perform_o(ss);
	}
	else if (!strcmp(command, COMMAND_PREFIX "C"))
	{
		Perform_C(ss);
	}
	else
	{
		cout("不明なコマンド\n");
	}
	LOGPOS();
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
	cmdTitle("oCServer");

	LOGPOS();
	sockServerUserTransmit(Perform, getZero, noop_u, PORTNO, 1, Idle);
	LOGPOS();
}
