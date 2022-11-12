#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	autoList_t *files = readLines(FOUNDLISTFILE);
	char *file;

	file = selectLine(files);

	if (file)
	{
		char *dir = changeLocal(file, ".");

		coExecute_x(xcout("START \"\" \"%s\"", dir));

		memFree(dir);
	}
	releaseDim(files, 1);
	memFree(file);
}
