/*
	webmon.exe [/PS PERIOD-SEC] URL
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\SockStream.h"

static char *Domain;
static uint PortNo;

static uint PeriodSec = 60;
static char *ConnectStatus;
static uint ConnectFaultCount;
static int ConnectFault;

static void Perform(int sock)
{
	SockStream_t *ss = CreateSockStream(sock, 30);

	ConnectStatus = "[NG]_NO_RESPONSE";

	SockSendLine_x(ss, xcout("HEAD http://%s:%u/ HTTP/1.1", Domain, PortNo));
	SockSendLine_x(ss, xcout("Host: %s:%u", Domain, PortNo));
	SockSendLine(ss, "User-Agent: webmon");
	SockSendLine(ss, "");
	SockFlush(ss);

	{
		char *line = SockRecvLine(ss, 5);

		if (!memcmp(line, "HTTP/", 5))
		{
			ConnectStatus = "[OK]";
			ConnectFault = 0;
		}
		memFree(line);
	}

	ReleaseSockStream(ss);
}

static void ParseUrl(char *url)
{
	char *p = strstr(url, "://");

	if (p)
		url = p + 3;

	Domain = strx(url);
	p = strchr(Domain, '/');

	if (p)
		*p = '\0';

	p = strchr(Domain, ':');

	if (p)
	{
		*p = '\0';
		PortNo = toValue(p + 1);
		m_range(PortNo, 1, 65535);
	}
	else
	{
		PortNo = 80;
	}
}
static void WebMon(char *url)
{
	static uchar ip[4];

	SockStartup();

	ParseUrl(url);

	for (; ; )
	{
		uint c;

		for (c = 3; c; )
		{
			uint st;
			uint et;
			uint dt;

			ConnectStatus = "[NG]_NOT_CONNECTED";
			ConnectFault = 1;

			st = now();

			sockClientUserTransmit(ip, Domain, PortNo, Perform);

			et = now();
			dt = et - st;

			if (4 < dt)
				cout("Information: 通信に %d 秒掛かりました。\n", dt);

			if (!ConnectFault)
				break;

			c--;

			if (c)
				cout("Warning: 通信に失敗しました。%s あと %u 回リトライします。\n", ConnectStatus, c);

			Sleep(5000);
		}
		if (ConnectFault)
			ConnectFaultCount++;

		cout("%s %s:%u %s NG=%u\n", c_makeJStamp(NULL, 0), Domain, PortNo, ConnectStatus, ConnectFaultCount);

		// wait key
		{
			uint sec;

			for (sec = 0; sec < PeriodSec; sec++)
			{
				while (hasKey())
				{
					switch (getKey())
					{
					case 'R':
					case 'r':
						cout("*RESET\n");
						ConnectFaultCount = 0;
						break;

					case 0x1b:
						goto endLoop;
					}
				}
				sleep(1000);
			}
		}
	}

endLoop:
	SockCleanup();
}

int main(int argc, char **argv)
{
	char *url;

	if (argIs("/PS"))
	{
		PeriodSec = toValue(nextArg());
		m_range(PeriodSec, 1, 3600);
	}
	url = nextArg();
	WebMon(url);
}
