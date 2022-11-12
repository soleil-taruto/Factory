/*
	ファイル構成

		C:\Factory\tmp\FilingCase\<TABLE_ID>\<COLUMN_ID>\

			rv\<ROW_ID> == VALUE
			vr\<VALUE_ID>\<ROW_ID> == {}
*/

#include "FilingCase.h"

static int IsId(char *nameOrId)
{
	return lineExp("<32,09AFaf>", nameOrId);
}
static int IsIds(autoList_t *lines)
{
	char *line;
	uint index;

	foreach (lines, line, index)
		if (!IsId(line))
			return 0;

	return 1;
}
static void Paths2Ids(autoList_t *lines)
{
	char *line;
	uint index;

	eraseParents(lines);
	errorCase(!IsIds(lines));

	foreach (lines, line, index)
		toLowerLine(line);

	rapidSortLines(lines);
}
static char *NameOrIdToId(char *nameOrId)
{
	char *id;

	if (IsId(nameOrId))
		id = nameOrId;
	else
		id = sha512_128Line(nameOrId);

	id = strx(id);
	return id;
}
static char *ValueToId(autoBlock_t *value)
{
	return strx(sha512_128Block(value));
}
char *FC_GetNewId(void)
{
	return MakeRandHexID();
}

// ---- lock ----

#define COMMON_MTX "{fb4fc70a-88ba-4240-a674-3a86052a750c}"

static uint Mtx;

static void DoLock(void)
{
	errorCase(Mtx);
	Mtx = mutexLock(COMMON_MTX);
}
static void DoUnlock(void)
{
	errorCase(!Mtx);
	mutexUnlock(Mtx);
	Mtx = 0;
}

static uint LockedCount;

void FC_Lock(void)
{
	errorCase(IMAX < LockedCount);

	if (!LockedCount)
		DoLock();

	LockedCount++;
}
void FC_Unlock(void)
{
	errorCase(!LockedCount);
	LockedCount--;

	if (!LockedCount)
		DoUnlock();
}

// ---- table ----

#define ROOT_DIR "C:\\Factory\\tmp\\FilingCase"
//#define ROOT_DIR "C:\\appdata\\FilingCase" // appdata 抑止 @ 2021.3.19

static char *GetTableDir(char *tableId)
{
	return combine(ROOT_DIR, tableId);
}
autoList_t *FC_GetAllTableId(void)
{
	autoList_t *ret;

	FC_Lock();
	{
		if (existDir(ROOT_DIR))
			ret = lsDirs(ROOT_DIR);
		else
			ret = newList();
	}
	FC_Unlock();

	Paths2Ids(ret);
	return ret;
}
void FC_SwapTable(char *tableNameOrId1, char *tableNameOrId2)
{
	char *tableId1;
	char *tableId2;
	char *tmpId;
	char *dir1;
	char *dir2;
	char *dir3;

	errorCase(!tableNameOrId1);
	errorCase(!tableNameOrId2);

	tableId1 = NameOrIdToId(tableNameOrId1);
	tableId2 = NameOrIdToId(tableNameOrId2);
	tmpId = FC_GetNewId();

	dir1 = GetTableDir(tableId1);
	dir2 = GetTableDir(tableId2);
	dir3 = GetTableDir(tmpId);

	if (!strcmp(tableId1, tableId2))
		goto endFunc;

	FC_Lock();
	{
		createPath(dir1, 'd');
		createPath(dir2, 'd');

		moveFile(dir1, dir3);
		moveFile(dir2, dir1);
		moveFile(dir3, dir2);

		removeDirIfEmpty(dir1);
		removeDirIfEmpty(dir2);
	}
	FC_Unlock();

endFunc:
	memFree(tableId1);
	memFree(tableId2);
	memFree(tmpId);
	memFree(dir1);
	memFree(dir2);
	memFree(dir3);
}
void FC_DeleteTable(char *tableNameOrId)
{
	char *tableId;
	char *dir;

	errorCase(!tableNameOrId);

	tableId = NameOrIdToId(tableNameOrId);

	dir = GetTableDir(tableId);

	FC_Lock();
	{
		recurRemoveDirIfExist(dir);
	}
	FC_Unlock();

	memFree(tableId);
	memFree(dir);
}

// ---- column ----

static char *GetColumnDir(char *tableId, char *columnId)
{
	return combine_xc(GetTableDir(tableId), columnId);
}
autoList_t *FC_GetAllColumnId(char *tableNameOrId)
{
	char *tableId;
	char *dir;
	autoList_t *ret;

	errorCase(!tableNameOrId);

	tableId = NameOrIdToId(tableNameOrId);

	dir = GetTableDir(tableId);

	FC_Lock();
	{
		if (existDir(dir))
			ret = lsDirs(dir);
		else
			ret = newList();
	}
	FC_Unlock();

	memFree(tableId);
	memFree(dir);

	Paths2Ids(ret);
	return ret;
}
void FC_SwapColumn(char *tableNameOrId, char *columnNameOrId1, char *columnNameOrId2)
{
	char *tableId;
	char *columnId1;
	char *columnId2;
	char *tmpId;
	char *dir1;
	char *dir2;
	char *dir3;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId1);
	errorCase(!columnNameOrId2);

	tableId   = NameOrIdToId(tableNameOrId);
	columnId1 = NameOrIdToId(columnNameOrId1);
	columnId2 = NameOrIdToId(columnNameOrId2);
	tmpId = FC_GetNewId();

	dir1 = GetColumnDir(tableId, columnId1);
	dir2 = GetColumnDir(tableId, columnId2);
	dir3 = GetColumnDir(tableId, tmpId);

	if (!strcmp(columnId1, columnId2))
		goto endFunc;

	FC_Lock();
	{
		createPath(dir1, 'd');
		createPath(dir2, 'd');

		moveFile(dir1, dir3);
		moveFile(dir2, dir1);
		moveFile(dir3, dir2);

		removeDirIfEmpty(dir1);
		removeDirIfEmpty(dir2);
	}
	FC_Unlock();

endFunc:
	memFree(tableId);
	memFree(columnId1);
	memFree(columnId2);
	memFree(tmpId);
	memFree(dir1);
	memFree(dir2);
	memFree(dir3);
}
void FC_DeleteColumn(char *tableNameOrId, char *columnNameOrId)
{
	char *tableId;
	char *columnId;
	char *tDir;
	char *dir;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId);

	tableId  = NameOrIdToId(tableNameOrId);
	columnId = NameOrIdToId(columnNameOrId);

	tDir = GetTableDir(tableId);
	dir = GetColumnDir(tableId, columnId);

	FC_Lock();
	{
		recurRemoveDirIfExist(dir);
		removeDirIfExistEmpty(tDir);
	}
	FC_Unlock();

	memFree(tableId);
	memFree(columnId);
	memFree(tDir);
	memFree(dir);
}

// ---- valueToRow ----

static char *GetVTRRootDir(char *tableId, char *columnId)
{
	return combine_xc(GetColumnDir(tableId, columnId), "vr");
}
static char *GetVTRDir(char *tableId, char *columnId, char *valueId)
{
	return combine_xc(GetVTRRootDir(tableId, columnId), valueId);
}
static autoList_t *GetVTRs(char *tableId, char *columnId, char *valueId)
{
	char *dir = GetVTRDir(tableId, columnId, valueId);
	autoList_t *ret;

	if (existDir(dir))
		ret = lsFiles(dir);
	else
		ret = newList();

	memFree(dir);
	Paths2Ids(ret);
	return ret;
}
static char *GetVTRFile(char *tableId, char *rowId, char *columnId, char *valueId)
{
	return combine_xc(GetVTRDir(tableId, columnId, valueId), rowId);
}

// ---- value ----

static char *GetValueDir(char *tableId, char *columnId)
{
	return combine_xc(GetColumnDir(tableId, columnId), "rv");
}
static char *GetValueFile(char *tableId, char *rowId, char *columnId)
{
	return combine_xc(GetValueDir(tableId, columnId), rowId);
}
autoBlock_t *FC_GetValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId)
{
	char *tableId;
	char *rowId;
	char *columnId;
	char *file;
	autoBlock_t *ret;

	errorCase(!tableNameOrId);
	errorCase(!rowNameOrId);
	errorCase(!columnNameOrId);

	tableId  = NameOrIdToId(tableNameOrId);
	rowId    = NameOrIdToId(rowNameOrId);
	columnId = NameOrIdToId(columnNameOrId);

	file = GetValueFile(tableId, rowId, columnId);

	FC_Lock();
	{
		if (existFile(file))
			ret = readBinary(file);
		else
			ret = newBlock();
	}
	FC_Unlock();

	memFree(tableId);
	memFree(rowId);
	memFree(columnId);
	memFree(file);

	return ret;
}
void FC_SetValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId, autoBlock_t *value)
{
	char *tableId;
	char *rowId;
	char *columnId;
	char *file;

	errorCase(!tableNameOrId);
	errorCase(!rowNameOrId);
	errorCase(!columnNameOrId);
	errorCase(!value);

	tableId  = NameOrIdToId(tableNameOrId);
	rowId    = NameOrIdToId(rowNameOrId);
	columnId = NameOrIdToId(columnNameOrId);

	file = GetValueFile(tableId, rowId, columnId);

	FC_Lock();
	{
		if (existFile(file))
		{
			{
				autoBlock_t *oldValue = readBinary(file);
				char *oldValueId;
				char *vtrFile;
				char *vtrDir;
				char *vtrRootDir;

				oldValueId = ValueToId(oldValue);

				vtrFile    = GetVTRFile(tableId, rowId, columnId, oldValueId);
				vtrDir     = GetVTRDir(tableId, columnId, oldValueId);
				vtrRootDir = GetVTRRootDir(tableId, columnId);

				removeFile(vtrFile);
				removeDirIfEmpty(vtrDir);
				removeDirIfEmpty(vtrRootDir);

				releaseAutoBlock(oldValue);
				memFree(oldValueId);
				memFree(vtrFile);
				memFree(vtrDir);
				memFree(vtrRootDir);
			}

			removeFile(file);

			{
				char *dir = GetValueDir(tableId, columnId);
				char *cDir = GetColumnDir(tableId, columnId);
				char *tDir = GetTableDir(tableId);

				removeDirIfEmpty(dir);
				removeDirIfEmpty(cDir);
				removeDirIfEmpty(tDir);

				memFree(dir);
				memFree(cDir);
				memFree(tDir);
			}
		}
		if (getSize(value))
		{
			createPath(file, 'X');
			writeBinary(file, value);

			{
				char *valueId = ValueToId(value);
				char *vtrFile;

				vtrFile = GetVTRFile(tableId, rowId, columnId, valueId);

				createPath(vtrFile, 'F');

				memFree(valueId);
				memFree(vtrFile);
			}
		}
	}
	FC_Unlock();

	memFree(tableId);
	memFree(rowId);
	memFree(columnId);
	memFree(file);
}
autoList_t *FC_GetRowIds(char *tableNameOrId, char *columnNameOrId, autoBlock_t *value)
{
	char *tableId;
	char *columnId;
	char *valueId;
	autoList_t *ret;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId);
	errorCase(!value);

	if (!getSize(value))
	{
		autoList_t *rowIds = FC_GetTableAllRowId(tableNameOrId);
		autoList_t *cIds = FC_GetAllRowId(tableNameOrId, columnNameOrId);
		autoList_t *mIds;

		mIds = mergeLines(rowIds, cIds);

		errorCase(getCount(cIds)); // 2bs

		releaseDim(mIds, 1);
		releaseAutoList(cIds);

		return rowIds;
	}
	tableId  = NameOrIdToId(tableNameOrId);
	columnId = NameOrIdToId(columnNameOrId);
	valueId = ValueToId(value);

	FC_Lock();
	{
		ret = GetVTRs(tableId, columnId, valueId);
	}
	FC_Unlock();

	memFree(tableId);
	memFree(columnId);
	memFree(valueId);

	return ret;
}
char *FC_GetRowId(char *tableNameOrId, char *columnNameOrId, autoBlock_t *value) // ret: NULL == 見つからない。
{
	autoList_t *rowIds;
	char *ret;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId);
	errorCase(!value);

	rowIds = FC_GetRowIds(tableNameOrId, columnNameOrId, value);

	if (getCount(rowIds))
		ret = (char *)unaddElement(rowIds);
	else
		ret = NULL;

	releaseDim(rowIds, 1);
	return ret;
}

// ---- str_value ----

char *FC_GetStrValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId)
{
	errorCase(!tableNameOrId);
	errorCase(!rowNameOrId);
	errorCase(!columnNameOrId);

	return unbindBlock2Line(FC_GetValue(tableNameOrId, rowNameOrId, columnNameOrId));
}
void FC_SetStrValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId, char *value)
{
	autoBlock_t gab;

	errorCase(!tableNameOrId);
	errorCase(!rowNameOrId);
	errorCase(!columnNameOrId);
	errorCase(!value);

	FC_SetValue(tableNameOrId, rowNameOrId, columnNameOrId, gndBlockLineVar(value, gab));
}
autoList_t *FC_GetStrRowIds(char *tableNameOrId, char *columnNameOrId, char *value)
{
	autoBlock_t gab;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId);
	errorCase(!value);

	return FC_GetRowIds(tableNameOrId, columnNameOrId, gndBlockLineVar(value, gab));
}
char *FC_GetStrRowId(char *tableNameOrId, char *columnNameOrId, char *value)
{
	autoBlock_t gab;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId);
	errorCase(!value);

	return FC_GetRowId(tableNameOrId, columnNameOrId, gndBlockLineVar(value, gab));
}

// ---- row ----

autoList_t *FC_GetAllRowId(char *tableNameOrId, char *columnNameOrId)
{
	char *tableId;
	char *columnId;
	char *dir;
	autoList_t *ret;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId);

	tableId  = NameOrIdToId(tableNameOrId);
	columnId = NameOrIdToId(columnNameOrId);

	dir = GetValueDir(tableId, columnId);

	FC_Lock();
	{
		if (existDir(dir))
			ret = lsFiles(dir);
		else
			ret = newList();
	}
	FC_Unlock();

	memFree(tableId);
	memFree(columnId);
	memFree(dir);

	Paths2Ids(ret);
	return ret;
}
uint FC_GetRowCount(char *tableNameOrId, char *columnNameOrId)
{
	char *tableId;
	char *columnId;
	char *dir;
	uint ret;

	errorCase(!tableNameOrId);
	errorCase(!columnNameOrId);

	tableId  = NameOrIdToId(tableNameOrId);
	columnId = NameOrIdToId(columnNameOrId);

	dir = GetValueDir(tableId, columnId);

	FC_Lock();
	{
		ret = lsCount(dir);
	}
	FC_Unlock();

	memFree(tableId);
	memFree(columnId);
	memFree(dir);

	return ret;
}

// ----

autoList_t *FC_GetTableAllRowId(char *tableNameOrId)
{
	autoList_t *columnIds;
	char *columnId;
	uint index;
	autoList_t *ret = newList();

	errorCase(!tableNameOrId);

	columnIds = FC_GetAllColumnId(tableNameOrId);

	foreach (columnIds, columnId, index)
	{
		autoList_t *rowIds = FC_GetAllRowId(tableNameOrId, columnId);
		autoList_t *mIds;

		mIds = mergeLines(ret, rowIds);
		addElements(ret, rowIds);
		addElements(ret, mIds);

		releaseAutoList(rowIds);
		releaseAutoList(mIds);
	}
	releaseDim(columnIds, 1);
	return ret;
}
void FC_SwapRow(char *tableNameOrId, char *rowNameOrId1, char *rowNameOrId2)
{
	autoList_t *columnIds;
	char *columnId;
	uint index;

	errorCase(!tableNameOrId);
	errorCase(!rowNameOrId1);
	errorCase(!rowNameOrId2);

	columnIds = FC_GetAllColumnId(tableNameOrId);

	foreach (columnIds, columnId, index)
	{
		autoBlock_t *value1 = FC_GetValue(tableNameOrId, rowNameOrId1, columnId);
		autoBlock_t *value2 = FC_GetValue(tableNameOrId, rowNameOrId2, columnId);

		FC_SetValue(tableNameOrId, rowNameOrId2, columnId, value1); // 1 -> 2
		FC_SetValue(tableNameOrId, rowNameOrId1, columnId, value2); // 2 -> 1

		releaseAutoBlock(value1);
		releaseAutoBlock(value2);
	}
	releaseDim(columnIds, 1);
}
void FC_DeleteRow(char *tableNameOrId, char *rowNameOrId)
{
	autoList_t *columnIds;
	char *columnId;
	uint index;

	errorCase(!tableNameOrId);
	errorCase(!rowNameOrId);

	columnIds = FC_GetAllColumnId(tableNameOrId);

	foreach (columnIds, columnId, index)
	{
		FC_SetStrValue(tableNameOrId, rowNameOrId, columnId, "");
	}
	releaseDim(columnIds, 1);
}
