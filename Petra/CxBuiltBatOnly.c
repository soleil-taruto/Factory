#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *dirs = lsDirs(".");
	char *dir;
	uint index;

	foreach (dirs, dir, index)
	{
		addCwd(dir);
		{
			if (existFile("Built.bat"))
			{
				coExecute("cx **");
			}
		}
		unaddCwd();
	}
	releaseDim(dirs, 1);
}
