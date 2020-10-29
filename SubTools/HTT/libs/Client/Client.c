#include "Client.h"

uint Client_SockTimeoutSec = 20;

SockStream_t *ClientBegin(int sock, char *serviceName) // ret: NULL ƒiƒV
{
	SockStream_t *ss;

	// sock
	errorCase(m_isEmpty(serviceName));

	ss = CreateSockStream(sock, Client_SockTimeoutSec);

	SockSendToken(ss, serviceName);
	SockSendChar(ss, '\n');
	SockFlush(ss);

	return ss;
}
void ClientEnd(SockStream_t *ss)
{
	ReleaseSockStream(ss);
}
