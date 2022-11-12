/*
	newcom.exe コンポーネント名
*/

#include "C:\Factory\Common\all.h"

static void CopyResFile(char *file, char *name)
{
	char *localName = strx(getLocal(file));

	localName = replaceLine(localName, "Riot", name, 1);

	copyFile(file, localName);

	memFree(localName);
}
static void CreateRiotComponent(char *name)
{
	LOGPOS();

	errorCase(!isFairLocalPath(name, 0));

	{
		autoList_t *files = lsFiles(".");
		char *file;
		uint index;

		foreach (files, file, index)
		{
			char *localName = strx(getLocal(file));
			char *dot;

			dot = mbs_strchr(localName, '.');

			if (dot)
				*dot = '\0';

			errorCase(!mbs_stricmp(name, localName));

			memFree(localName);
		}
		releaseDim(files, 1);
	}

	LOGPOS();

	{
		autoList_t *files = lsFiles(innerResPathFltr("C:\\Factory\\Resource\\RiotComponent"));
		char *file;
		uint index;

		foreach (files, file, index)
		{
			CopyResFile(file, name);
		}
		releaseDim(files, 1);
	}

	LOGPOS();
}
int main(int argc, char **argv)
{
	CreateRiotComponent(nextArg());
}
