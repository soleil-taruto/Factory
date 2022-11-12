#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\DevTools\libs\RandData.h"

static int KeepLoopRq = 1;

static int Idle(void)
{
	while (hasKey())
		if (getKey() == 0x1b)
			KeepLoopRq = 0;

	return KeepLoopRq;
}
int main(int argc, char **argv)
{
	char *domain = "localhost";
	uint portno = 65000;
	uint upFileSize = 1024 * 1024 * 1024;
	char *upFile = makeTempPath("tmp");
	uchar ip[4];

	if (hasArgs(2))
	{
		domain = nextArg();
		portno = toValue(nextArg());
	}
	if (hasArgs(1))
	{
		upFileSize = toValue(nextArg());
	}

	cmdTitle_x(xcout("%s (%u) %u", domain, portno, upFileSize));

	mt19937_init32(upFileSize);
	MakeRandBinaryFile(upFile, upFileSize);

	SockStartup();
	memset(ip, 0, 4);

	while (KeepLoopRq)
	{
		char *dlFile;

		cout("Request Start\n");
		dlFile = sockClient(ip, domain, portno, upFile, Idle);
		cout("Request End\n");

		if (!KeepLoopRq)
		{
			if (dlFile)
			{
				removeFile(dlFile);
				memFree(dlFile);
			}
			cout("Request Abort\n");
			break;
		}

		errorCase(!dlFile);
		errorCase(!isSameFile(upFile, dlFile));

		removeFile(dlFile);
		memFree(dlFile);

		cout("OK\n");
	}
	SockCleanup();

	removeFile(upFile);
	memFree(upFile);
}
