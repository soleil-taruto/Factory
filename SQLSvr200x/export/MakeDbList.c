#include "C:\Factory\Common\all.h"
#include "..\libs\RecReader.h"

int main(int argc, char **argv)
{
	char *dbRecFile;
	char *outFile;
	autoList_t *dbRec;
	autoList_t *row;
	uint index;
	autoList_t *outList = newList();

	dbRecFile = nextArg();
	outFile = nextArg();

	dbRec = SqlRecReader(dbRecFile); // db

	foreach (dbRec, row, index)
	{
		char *db;

		if (!index) // ? ヘッダー行
			continue;

		db = (char *)getElement(row, 0);

		// ? システムデータベース
		if (
			!_stricmp(db, "master") ||
			!_stricmp(db, "msdb") ||
			!_stricmp(db, "model") ||
			!_stricmp(db, "Resource") ||
			!_stricmp(db, "tempdb")
			)
			continue;

		addElement(outList, (uint)strx(db));
	}
	sortJLinesICase(outList);

	writeLines(outFile, outList);

	releaseDim(dbRec, 2);
	releaseDim(outList, 1);
}
