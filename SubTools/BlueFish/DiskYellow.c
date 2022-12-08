/*
	DiskYellow.exe TARGET-DRIVE DISK-FREE-LIMIT YELLOW-COMMAND
*/

#include "C:\Factory\Common\all.h"

static int TargetDrive;
static uint64 DiskFreeLimit;
static char *YellowCommand;

static void SetTitle(void)
{
	cmdTitle_x(xcout("DiskYellow - [%c:] %I64u = %s", TargetDrive, DiskFreeLimit, YellowCommand));
}
static void SetTitleEnd(void)
{
	cmdTitle("DiskYellow");
}
static void Monitor(void)
{
	uint waitMillis = 2000;
	uint loopCount;

	cout("モニター開始 @ %s\n", c_makeJStamp(NULL, 0));
	SetTitle();

	for (loopCount = 0; ; loopCount++)
	{
		int key = waitKey(waitMillis);
		uint64 diskFree;
		uint64 diskSize;
		double rate;
		double rateLmt;

		if (key == 0x1b)
			break;

		updateDiskSpace(TargetDrive);
		diskFree = lastDiskFree_User;
		diskSize = lastDiskSize;

		if (key == 'T')
		{
			cout("########################\n");
			cout("## コマンドテスト実行 ##\n");
			cout("########################\n");

			diskFree = 0ui64;
		}
		else if (key)
		{
			cout("+---------------------+\n");
			cout("| ESC ⇒ モニター終了 |\n");
			cout("| T ⇒ コマンドテスト |\n");
			cout("+---------------------+\n");
		}
		if (loopCount % 10 == 0)
		{
			cout("----\n");
			cout("%s\n", c_makeJStamp(NULL, 0));
			cout("----\n");
		}
		rate    = diskFree      * 1.0 / diskSize;
		rateLmt = DiskFreeLimit * 1.0 / diskSize;

		     if (rateLmt * 10.0 < rate) { waitMillis = 20000; }
		else if (rateLmt *  7.5 < rate) { waitMillis = 15000; }
		else if (rateLmt *  5.0 < rate) { waitMillis = 10000; }
		else if (rateLmt *  2.5 < rate) { waitMillis =  5000; }
		else                            { waitMillis =  2000; }

		cout("DiskFree: %I64u / %I64u, rate: %.3f / %.3f, wait: %u\n", diskFree, DiskFreeLimit, rate, rateLmt, waitMillis);

		if (diskFree < DiskFreeLimit)
		{
			cout("コマンド実行 @ %s\n", c_makeJStamp(NULL, 0));
			coExecute(YellowCommand);
			cout("コマンド終了 @ %s\n", c_makeJStamp(NULL, 0));
			SetTitle();
		}
	}
	cout("モニター終了 @ %s\n", c_makeJStamp(NULL, 0));
	SetTitleEnd();
}
int main(int argc, char **argv)
{
	TargetDrive = nextArg()[0];
	DiskFreeLimit = toValue64(nextArg());
	YellowCommand = nextArg();

	errorCase(!m_isalpha(TargetDrive));

	cout("TargetDrive: %c\n", TargetDrive);
	cout("DiskFreeLimit: %I64u\n", DiskFreeLimit);
	cout("YellowCommand: %s\n", YellowCommand);

	errorCase(!m_isRange(DiskFreeLimit, 1ui64, IMAX_64));
	errorCase(m_isEmpty(YellowCommand));

	Monitor();
}
