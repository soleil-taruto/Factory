/*
	Client.exe SERVER-DOMAIN SERVER-PORT (/L [LOOP-COUNT] | MESSAGE)
*/

#include "C:\Factory\SubTools\HTT\libs\Client\CRPC_Aes.h"
#include "C:\Factory\DevTools\libs\RandData.h"

static autoBlock_t *RawKey;
static int SilentMode;

static int Perform(int sock, uint prm)
{
	SockStream_t *ss = ClientBegin(sock, "EchoTest_Aes"); // ★開始
	char *message = (char *)prm;
	autoBlock_t gab;
	autoBlock_t *recvData;
	char *recvMessage;
	char *exMessage;
	int retval;

	if (!SilentMode) cout("message: %s\n", message);

	ClientCRPC_Begin(ss); // ★暗号通信の開始

	recvData = ClientCRPC(ss, gndBlockLineVar(message, gab), RawKey); // ★暗号通信

	ClientCRPC_End(); // ★暗号通信の終了

	recvMessage = unbindBlock2Line(recvData);
	if (!SilentMode) cout("recvMessage: %s\n", recvMessage);
	exMessage = xcout("You sent [%s]", message);
	if (!SilentMode) cout("exMessage: %s\n", exMessage);

	retval = !strcmp(recvMessage, exMessage); // ? 想定した応答メッセージと一致する。

	memFree(recvMessage);
	memFree(exMessage);

	ClientEnd(ss); // ★終了

	return retval;
}
int main(int argc, char **argv)
{
	char *serverHost;
	uint serverPort;

	// init
	{
		RawKey = makeBlockHexLine("6d636a95a420a7780639a2a1d13eacb4"); // .\\Server.c と合わせること。
	}

	serverHost = nextArg();
	serverPort = toValue(nextArg());

	errorCase(m_isEmpty(serverHost));
	errorCase(!m_isRange(serverPort, 1, 0xffff));

	if (argIs("/L")) // Loop
	{
		uint count;
		uint max;

		if (hasArgs(1))
			max = toValue(nextArg());
		else
			max = 10000;

		SockStartup();

		for (count = 0; count < max; count++)
		{
			char *message = MakeRandLineRange(0, 10000);

			cout("count: %u\n", count);
			cout("messageLen: %u\n", strlen(message));

			SilentMode = 1;
			errorCase(!SClient(serverHost, serverPort, Perform, (uint)message));
			SilentMode = 0;

			memFree(message);
		}

		SockCleanup();

		memFree(0); // for debug

		cout("DONE HIT_KEY!\n");
		clearGetKey();
		cout("OK\n");
		return;
	}

	{
		char *message = nextArg();
		int retval;

		retval = SClient(serverHost, serverPort, Perform, (uint)message);

		cout("result: %d\n", retval);
	}
}
