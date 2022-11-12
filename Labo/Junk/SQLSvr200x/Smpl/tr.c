#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csv.h"
#include "..\libs\TableReader.h"

int main(int argc, char **argv)
{
	char *rFile = NULL;
	char *wFile = NULL;
	autoList_t *whereColNames = NULL;
	autoList_t *whereColValues = NULL;
	autoList_t *resultColNames = NULL;

readArgs:
	if (argIs("/F"))
	{
		memFree(rFile);
		rFile = strx(nextArg());
	}
	if (argIs("/W"))
	{
		memFree(wFile);
		wFile = strx(nextArg());
	}
	if (argIs("+"))
	{
		if (!whereColNames)
			whereColNames = newList();

		addElement(whereColNames, (uint)strx(nextArg()));
		goto readArgs;
	}
	if (argIs("="))
	{
		if (!whereColValues)
			whereColValues = newList();

		addElement(whereColValues, (uint)strx(nextArg()));
		goto readArgs;
	}
	if (argIs(":"))
	{
		if (!resultColNames)
			resultColNames = newList();

		addElement(resultColNames, (uint)strx(nextArg()));
		goto readArgs;
	}

	if (argIs("/S"))
	{
		writeLines_cx(wFile, TR_GetSchema(rFile));
		goto readArgs;
	}
	if (argIs("/T"))
	{
		/*
			wFile - TEXT (resultColNames ÇÃí∑Ç≥)çs
		*/
		writeLines_cx(
			wFile,
			TR_SelectTop1WhereAnd_cxxx(rFile, whereColNames, whereColValues, resultColNames)
			);
		whereColNames = NULL;
		whereColValues = NULL;
		resultColNames = NULL;
		goto readArgs;
	}
	if (argIs("/R"))
	{
		/*
			rFile == wFile <- NG
			rFile - CSV
			wFile - TEXT (TR_SelectWhereAnd ÇÃñﬂÇËíl * resultColNames ÇÃí∑Ç≥)çs
		*/
		cout("%I64u\n"
			,TR_SelectWhereAnd_cxxxc(rFile, whereColNames, whereColValues, resultColNames, wFile)
			);
		whereColNames = NULL;
		whereColValues = NULL;
		resultColNames = NULL;
		goto readArgs;
	}
	if (argIs("/D"))
	{
		/*
			rFile == wFile <- NG
			rFile - CSV
			wFile - CSV
		*/
		cout("%I64u\n"
			,TR_DeleteWhereAnd_cxxc(rFile, whereColNames, whereColValues, wFile)
			);
		whereColNames = NULL;
		whereColValues = NULL;
		goto readArgs;
	}
	if (argIs("/I"))
	{
		autoList_t *colValueTable = readCSVFileTR(nextArg());
		autoList_t *colNames;

		colNames = (autoList_t *)fastDesertElement(colValueTable, 0);

		/*
			rFile == wFile <- NG
			rFile - CSV
			wFile - CSV
		*/
		TR_InsertInto_cxxc(rFile, colNames, colValueTable, wFile);
		goto readArgs;
	}
	if (argIs("/C"))
	{
		/*
			wFile - CSV
		*/
		TR_CreateTable_cx(wFile, resultColNames);
		resultColNames = NULL;
		goto readArgs;
	}

	memFree(rFile);
	memFree(wFile);

	if (whereColNames)  releaseDim(whereColNames, 1);
	if (whereColValues) releaseDim(whereColValues, 1);
	if (resultColNames) releaseDim(resultColNames, 1);
}
