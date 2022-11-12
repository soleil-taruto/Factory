/*
	oCClient.exe mimiko o
	oCClient.exe mimiko C
*/

#include "C:\Factory\Common\Options\SClient.h"
#include "oCDefine.h"

static int Perform(int sock, uint prm)
{
	SockStream_t *ss = CreateSockStream(sock, 3600); // otoの受信(鯖でDL,z9している間の待ち)があるので、長めに
	char *command = (char *)prm;

	LOGPOS();

	SockSendToken(ss, COMMAND_PREFIX);
	SockSendLine(ss, command);

	LOGPOS();

	if (command[0] == 'o')
	{
		FILE *fp = fileOpen("oto.clu", "wb");
//		FILE *fp = fileOpen("oto.clu.gz.enc", "wb");
		uint64 wroteByteCount;

		LOGPOS();
		SockRecvChar(ss); // '1'
		LOGPOS();
		SockRecvChar(ss); // '2'
		LOGPOS();
		SockRecvChar(ss); // 'S'
		LOGPOS();

		wroteByteCount = 0;

		for (; ; )
		{
			int chr = SockRecvChar(ss);

			if (chr == EOF)
				break;

			if (eqIntPulseSec(5, NULL))
				cmdTitle_x(xcout("oCClient - wrote %I64u bytes", wroteByteCount));

			writeChar(fp, chr);
			wroteByteCount++;
		}
		cmdTitle_x(xcout("oCClient - wrote %I64u bytes OK", wroteByteCount));
		LOGPOS();

		fileClose(fp);
	}
	LOGPOS();

	ReleaseSockStream(ss);
	return 1;
}
int main(int argc, char **argv)
{
	char *server;
	char *command;
	uint portNo = PORTNO;

	sockConnectTimeoutSec = 2; // ローカル内なので

	if (argIs("/P"))
	{
		portNo = toValue(nextArg());
	}
	server  = nextArg();
	command = nextArg();

	LOGPOS();
	errorCase_m(!SClient(server, portNo, Perform, (uint)command), "接続できなかったようです。");
	LOGPOS();
}
