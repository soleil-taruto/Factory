/*
	使用例

		> SockUDP.exe a 59999 /R

		> SockUDP.exe localhost 59999 /S abc123.txt
*/

#include "C:\Factory\Common\Options\SockUDP.h"

int main(int argc, char **argv)
{
	static uchar ip[4];
	char *domain;
	uint portno;
	int sock;

	domain = nextArg();
	portno = toValue(nextArg());

	SockStartup(); // 必須！

	if (argIs("/S"))
	{
		autoList_t *lines = readLines(nextArg());
		char *line;
		uint index;

		sock = sockUDPOpenSend();

		foreach (lines, line, index)
			sockUDPSendBlock(sock, ip, domain, portno, line, strlen(line));

		sockUDPClose(sock);
	}
	if (argIs("/R"))
	{
		sock = sockUDPOpenRecv(portno);

		while (!waitKey(0))
		{
			autoBlock_t *data = sockUDPRecv(sock, 2000);

			if (data)
			{
				char *line = unbindBlock2Line(data);

				line2JLine(line, 1, 0, 1, 1);
				cout("受信: [%s]\n", line);
				memFree(line);
			}
		}
		sockUDPClose(sock);
	}
	SockCleanup(); // 必須！
}
