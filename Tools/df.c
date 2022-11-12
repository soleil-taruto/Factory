#include "C:\Factory\Common\all.h"

static void DispDiskFree(int drive, char *dir)
{
	updateDiskSpace(drive);

	cout("%cドライブ\n", m_toupper(drive));
	cout("ディスク使用率 = %.3f %%\n", (lastDiskSize - lastDiskFree) * 100.0 / lastDiskSize);
	cout("ディスク使用 = %s バイト\n", c_thousandComma(xcout("%I64u", lastDiskSize - lastDiskFree)));
	cout("ディスク容量 = %s バイト\n", c_thousandComma(xcout("%I64u", lastDiskSize)));
	cout("ディスク空き = %s バイト\n", c_thousandComma(xcout("%I64u", lastDiskFree)));
	cout("利用不可領域 = %s バイト\n", c_thousandComma(xcout("%I64u", lastDiskFree - lastDiskFree_User)));
	cout("利用可能領域 = %s バイト\n", c_thousandComma(xcout("%I64u", lastDiskFree_User)));

	if (dir)
	{
		updateDiskSpace_Dir(dir);

		cout("利用可能領域 = %s バイト (ディレクトリ)\n", c_thousandComma(xcout("%I64u", lastDiskFree_User)));
	}
}
int main(int argc, char **argv)
{
	if (argIs("*"))
	{
		char drive[] = "_:\\";

		for (drive[0] = 'A'; drive[0] <= 'Z'; drive[0]++)
			if (existDir(drive))
				DispDiskFree(drive[0], NULL);

		return;
	}

	if (hasArgs(1))
	{
		char *arg = nextArg();

		if (strlen(arg) == 1)
		{
			DispDiskFree(arg[0], NULL);
		}
		else
		{
			char *dir = makeFullPath(arg);

			DispDiskFree(dir[0], dir);

			memFree(dir);
		}
	}
	else
	{
		char *dir = c_getCwd();

		DispDiskFree(dir[0], dir);
	}
}
