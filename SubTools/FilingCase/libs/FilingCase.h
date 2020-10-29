#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"
#include "C:\Factory\OpenSource\sha512.h"

char *FC_GetNewId(void);

// ---- lock ----

void FC_Lock(void);
void FC_Unlock(void);

// ---- table ----

autoList_t *FC_GetAllTableId(void);
void FC_SwapTable(char *tableNameOrId1, char *tableNameOrId2);
void FC_DeleteTable(char *tableNameOrId);

// ---- column ----

autoList_t *FC_GetAllColumnId(char *tableNameOrId);
void FC_SwapColumn(char *tableNameOrId, char *columnNameOrId1, char *columnNameOrId2);
void FC_DeleteColumn(char *tableNameOrId, char *columnNameOrId);

// ---- value ----

autoBlock_t *FC_GetValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId);
void FC_SetValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId, autoBlock_t *value);
autoList_t *FC_GetRowIds(char *tableNameOrId, char *columnNameOrId, autoBlock_t *value);
char *FC_GetRowId(char *tableNameOrId, char *columnNameOrId, autoBlock_t *value);

// ---- str_value ----

char *FC_GetStrValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId);
void FC_SetStrValue(char *tableNameOrId, char *rowNameOrId, char *columnNameOrId, char *value);
autoList_t *FC_GetStrRowIds(char *tableNameOrId, char *columnNameOrId, char *value);
char *FC_GetStrRowId(char *tableNameOrId, char *columnNameOrId, char *value);

// ---- row ----

autoList_t *FC_GetAllRowId(char *tableNameOrId, char *columnNameOrId);
uint FC_GetRowCount(char *tableNameOrId, char *columnNameOrId);

// ----

autoList_t *FC_GetTableAllRowId(char *tableNameOrId);
void FC_SwapRow(char *tableNameOrId, char *rowNameOrId1, char *rowNameOrId2);
void FC_DeleteRow(char *tableNameOrId, char *rowNameOrId);
