#include "C:\Factory\Common\all.h"
#include "..\libs\RecReader.h"

int main(int argc, char **argv)
{
	char *tableRecFile;
	char *schemaRecFile;
	char *dbName;
	char *outFile;
	autoList_t *tableRec;
	autoList_t *schemaRec;
	uint index;
	autoList_t *outList = newList();

	tableRecFile = nextArg();
	schemaRecFile = nextArg();
	dbName = nextArg();
	outFile = nextArg();

	tableRec = SqlRecReader(tableRecFile); // schema_id, name
	schemaRec = SqlRecReader(schemaRecFile); // schema_id, name

	releaseDim((autoList_t *)desertElement(tableRec, 0), 1); // ヘッダー行除去
	releaseDim((autoList_t *)desertElement(schemaRec, 0), 1); // ヘッダー行除去

	for (index = 0; index < getCount(tableRec); index++)
	{
		autoList_t *tRow = getList(tableRec, index);
		autoList_t *sRow;
		uint srIndex;

		foreach (schemaRec, sRow, srIndex)
			if (!strcmp(getLine(tRow, 0), getLine(sRow, 0))) // ? schema_id 一致
				break;

		errorCase(!sRow);

		addElement(outList, (uint)xcout("[%s].[%s].[%s]", dbName, getLine(sRow, 1), getLine(tRow, 1)));
	}
	sortJLinesICase(outList);

	writeLines(outFile, outList);

	releaseDim(tableRec, 2);
	releaseDim(schemaRec, 2);
	releaseDim(outList, 1);
}
