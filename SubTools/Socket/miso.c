/*
	miso.exe [/-W] [SERVER-HOST [SERVER-PORT]]
*/

#include "C:\Factory\Common\Options\SockClient.h"
#include "C:\Factory\Common\Options\PadFile.h"
#include "libs\ApplyStampData.h"

static int Idle(void)
{
	return 1;
}
int main(int argc, char **argv)
{
	uchar ip[4];
	char *server = "localhost";
	uint portno = 58946;
	char *prmFile = makeTempFile("prm");
	char *retFile;
	uint trycnt;
	FILE *fp;
	time_t retTime;
	int slewMode = 1;

	if (argIs("/-W"))
	{
		slewMode = 0;
	}

	if (hasArgs(1))
	{
		server = nextArg();
	}
	if (hasArgs(1))
	{
		portno = toValue(nextArg());
	}

	for (trycnt = 3; trycnt; trycnt--)
	{
		cout("connect...\n");

		*(uint *)ip = 0;
		retFile = sockClient(ip, server, portno, prmFile, Idle);

		cout("%u\n", retFile ? 1 : 0);

		if (retFile)
		{
			if (UnpadFile2(retFile, "MISO_Ans"))
			{
				fp = fileOpen(retFile, "rb");
				retTime = (time_t)readValue64(fp);
				fileClose(fp);

				if (!strcmp(server, "localhost"))
					cout("%I64d\n", retTime);
				else if (slewMode)
					SlewApplyTimeData(retTime);
				else
					ApplyStampData(getStampDataTime(retTime));

				trycnt = 1;
			}
			removeFile(retFile);
			memFree(retFile);
		}
	}
	removeFile(prmFile);
	memFree(prmFile);
}
