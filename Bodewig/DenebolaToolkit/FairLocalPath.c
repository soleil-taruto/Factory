#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	char *localPath;
	char *dir;
	char *ret;

	localPath = nextArg();
	dir = nextArg();

	ret = lineToFairLocalPath(localPath, strlen(dir));

	cout("%s%s\n", majorOutputLinePrefix, ret);

	memFree(ret);
}
