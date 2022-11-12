/*
	AntiSVN.exe [/F] [TARGET-DIR]
*/

#include "C:\Factory\Common\all.h"

static int ForceMode;

static void AntiSVN(char *targetDir)
{
	targetDir = makeFullPath(targetDir);

	if (!ForceMode)
	{
		cout("%s\n", targetDir);
		cout("ANTI-SVN PROCEED? [Any/ESC]\n");

		if (clearGetKey() == 0x1b)
			termination(1);

		cout("GO!\n");
	}
	for (; ; )
	{
		autoList_t *dirs = lssDirs(targetDir);
		char *dir;
		uint index;
		int found = 0;

		foreach (dirs, dir, index)
		{
			if (
				!_stricmp(".svn", getLocal(dir)) ||
				!_stricmp("_svn", getLocal(dir))
				)
			{
				coExecute_x(xcout("RD /S /Q \"%s\"", dir));
				found = 1;
			}
		}
		releaseDim(dirs, 1);

		if (!found)
			break;

		coSleep(1000);
	}
	memFree(targetDir);
}
int main(int argc, char **argv)
{
	if (argIs("/F"))
	{
		ForceMode = 1;
	}
	if (hasArgs(1))
	{
		AntiSVN(nextArg());
		return;
	}
	AntiSVN(c_dropDir());
}
