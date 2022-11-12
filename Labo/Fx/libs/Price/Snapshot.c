#include "all.h"

#define SS_MID_DIR "S:\\Fx\\Snapshot"
#define SS_DIR "C:\\Factory\\tmp\\Fx_Snapshot"

// ---- Collect ----

void ExecuteSnapshot(void)
{
	autoList_t *files;
	char *file;
	uint index;

	coExecute("C:\\Factory\\SubTools\\nrun.exe /s mimiko Fx\\snapshot");

	recurRemoveDirIfExist(SS_DIR);
	createDir(SS_DIR);

	files = lsFiles(SS_MID_DIR);

	foreach (files, file, index)
	{
		char *wFile = combine(SS_DIR, getLocal(file));

		moveFile(file, wFile);
		memFree(wFile);
	}
	releaseDim(files, 1);
}

// ----

autoList_t *GetSnapshotPriceDay(uint date, char *pair)
{
	char *file = GetPriceDayFile(SS_DIR, date, pair);
	autoList_t *list;

	list = LoadPriceDay(file);
	memFree(file);
	return list;
}
int HasSnapshotPriceDay(uint date, char *pair)
{
	char *file = GetPriceDayFile(SS_DIR, date, pair);
	int ret;

	ret = existFile(file);
	memFree(file);
	return ret;
}
