/*
	Snapshot.exe [/M | /-S]
*/

#include "C:\Factory\Common\all.h"

int main(int argc, char **argv)
{
	int manualCopyMode = 0;
	int ignoreSubDirFlag = 0;
	char *path;

	if(argIs("/M"))
	{
		manualCopyMode = 1;
	}
	if(argIs("/-S"))
	{
		ignoreSubDirFlag = 1;
	}

	cout("+----------+\n");
	cout("| Snapshot |\n");
	cout("+----------+\n");

	if(manualCopyMode)
	{
		cout("#################\n");
		cout("## MANUAL-COPY ##\n");
		cout("#################\n");
	}
	if(ignoreSubDirFlag)
	{
		cout("###################\n");
		cout("## Ignore-SubDir ##\n");
		cout("###################\n");
	}

	path = dropDirFile();

	coExecute_x(xcout("C:\\app\\Rico\\Snapshot.exe %s%s\"%s\"", manualCopyMode ? "/M " : "", ignoreSubDirFlag ? "/-S " : "", path));

	memFree(path);
}
