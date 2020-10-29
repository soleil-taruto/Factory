#pragma once

#include "C:\Factory\Common\all.h"
#include "csv.h"
#include "RedirSqlTableReader.h"

extern char *TA_User;
extern char *TA_Pass;
extern char *TA_DBName;

void TA_GetTableData_R(char *tableName, char *redirFile);
void TA_GetTableSchema_R(char *tableName, char *redirFile);
void TA_GetTableList_R(char *redirFile);
void TA_GetTableData(char *tableName, char *csvFile);
void TA_GetTableSchema(char *tableName, char *csvFile);
void TA_GetTableList(char *csvFile);
void TA_InsertMultiRow(char *tableName, autoList_t *colNames, autoList_t *rows);
void TA_DeleteMultiRow(char *tableName, autoList_t *colNames, autoList_t *rows);
void TA_InsertRow(char *tableName, autoList_t *colNames, autoList_t *row);
void TA_DeleteRow(char *tableName, autoList_t *colNames, autoList_t *row);
void TA_InsertCSVFile(char *tableName, autoList_t *colNames, char *csvFile, uint rowcntPerExec, uint rdszPerExec);
void TA_DeleteCSVFile(char *tableName, autoList_t *colNames, char *csvFile, uint rowcntPerExec, uint rdszPerExec);
void TA_DeleteAll(char *tableName);
void TA_TableDataFltr(char *csvFile, autoList_t *colNames, char *outCsvFile);
void TA_TableDataFltr_cxc(char *csvFile, autoList_t *colNames, char *outCsvFile);
