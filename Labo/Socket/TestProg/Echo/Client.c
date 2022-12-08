#include "C:\Factory\Common\Options\SockClient.h"

#define GetSockFileCounter(sf) \
	((sf)->Counter + (uint64)((sf)->Block ? (sf)->Block->Counter : 0))

static int Idle(void)
{
	cout("Prm: %I64u / %I64u, Ans: %I64u / %I64u\n"
		,GetSockFileCounter(sockClientStatus.PrmFile)
		,sockClientStatus.PrmFile->FileSize
		,GetSockFileCounter(sockClientStatus.AnsFile)
		,sockClientStatus.AnsFile->FileSize
		);

	while (hasKey())
	{
		int key = getKey();

		if (key == 0x1b)
		{
			cout("’†~\n");
			return 0;
		}
		else if (key == 'P')
		{
			cout("ˆê’â~\n");
			getKey();
			cout("ÄŠJ\n");
		}
		else
		{
			cout("ESCAPE == ’†~, P == ˆê’â~\n");
		}
	}
	return 1;
}
int main(int argc, char **argv)
{
	char *domain;
	uint portno;
	int batchMode;

	if (hasArgs(1))
		domain = nextArg();
	else
		domain = "localhost";

	if (hasArgs(1))
		portno = toValue(nextArg());
	else
		portno = 65000;

	batchMode = hasArgs(1);

	for (; ; )
	{
		char *file = batchMode ? nextArg() : c_dropFile();
		char *retFile;
		uchar ip[4] = { 0, 0, 0, 0 };

		retFile = sockClient(ip, domain, portno, file, Idle);

		cout("retFile: %s\n", retFile ? retFile : "<NULL>");
		cout("ip: %s\n", SockIp2Line(ip));

		if (retFile)
		{
			cout("ƒ`ƒFƒbƒN’†...\n");
			errorCase(!isSameFile(file, retFile));
			cout("OK\n");

			removeFile(retFile);
			memFree(retFile);
		}
		if (batchMode && !hasArgs(1))
		{
			break;
		}
		cout("\n");
	}
}
