/*
	クラスタ作成
		resCluster.exe /L <RES-LIST-FILE> /D <ROOT-DIR> /K <RAW-KEY> /C <CLUSTER-FILE>

	復元モード
		resCluster.exe /D <DEST-DIR> /K <RAW-KEY> /C <CLUSTER-FILE> /RES
*/

#include "C:\Factory\Common\all.h"
#include "libs\resCluster.h"

int main(int argc, char **argv)
{
	char *resListFile = NULL;
	char *rootDir  = NULL;
	autoBlock_t *rawKey = NULL;
	char *clusterFile = NULL;
	int restoreMode = 0;

readArgs:
	if (argIs("/L"))
	{
		resListFile = nextArg();
		goto readArgs;
	}
	if (argIs("/D"))
	{
		rootDir = nextArg();
		goto readArgs;
	}
	if (argIs("/K"))
	{
		rawKey = makeBlockHexLine(nextArg());
		goto readArgs;
	}
	if (argIs("/C"))
	{
		clusterFile = nextArg();
		goto readArgs;
	}
	if (argIs("/RES"))
	{
		restoreMode = 1;
		goto readArgs;
	}
	errorCase(hasArgs(1));

	if (!restoreMode)
	{
		autoList_t *files;

		errorCase(!resListFile);
		errorCase(!rootDir);
		errorCase(!rawKey);
		errorCase(!clusterFile);

		files = readResourcePaths(resListFile);
		createResourceCluster(files, rootDir, rawKey, clusterFile);
		releaseDim(files, 1);
	}
	else
	{
		errorCase(!rootDir);
		errorCase(!rawKey);
		errorCase(!clusterFile);

		restoreResourceCluster(clusterFile, rawKey, rootDir);
	}
}
