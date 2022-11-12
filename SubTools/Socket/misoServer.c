#include "C:\Factory\Common\Options\SockServer.h"
#include "C:\Factory\Common\Options\PadFile.h"

static int Perform(char *prmFile, char *ansFile)
{
	FILE *fp;
	time_t retTime;

	if (getFileSize(prmFile)) // ? 空ではない。
		return 0;

	retTime = time(NULL);
	cout("%I64d -> %s\n", retTime, SockIp2Line(sockClientIp));

	fp = fileOpen(ansFile, "wb");
	writeValue64(fp, (uint64)retTime);
	fileClose(fp);

	PadFile2(ansFile, "MISO_Ans");
	return 1;
}
static int Idle(void)
{
	int retval = 1;

	while (hasKey())
	{
		if (getKey() == 0x1b)
		{
			cout("終了しました。\n");
			retval = 0;
		}
		cout("エスケープキーを押すと終了します。\n");
	}
	return retval;
}
int main(int argc, char **argv)
{
	uint portno = hasArgs(1) ? toValue(nextArg()) : 58946;

	cmdTitle_x(xcout("misoServer - %u", portno));
	sockServer(Perform, portno, 3, 0, Idle);
	cmdTitle("misoServer");
}
