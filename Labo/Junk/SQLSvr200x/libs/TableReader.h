#pragma once

#include "C:\Factory\Common\all.h"
#include "csv.h"

autoList_t *TR_GetSchema(char *csvFile);
autoList_t *TR_SelectTop1WhereAnd(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *retColNames);
uint64 TR_SelectWhereAnd(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *destColNames, char *destFile);
uint64 TR_DeleteWhereAnd(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, char *destCsvFile);
void TR_InsertInto(char *csvFile, autoList_t *colNames, autoList_t *colValueTable, char *destCsvFile);
void TR_CreateTable(char *csvFile, autoList_t *colNames);

// _x
autoList_t *TR_SelectTop1WhereAnd_cxxx(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *retColNames);
uint64 TR_SelectWhereAnd_cxxxc(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, autoList_t *destColNames, char *destFile);
uint64 TR_DeleteWhereAnd_cxxc(char *csvFile, autoList_t *whereColNames, autoList_t *whereColValues, char *destCsvFile);
void TR_InsertInto_cxcc(char *csvFile, autoList_t *colNames, autoList_t *colValueTable, char *destCsvFile);
void TR_InsertInto_cxxc(char *csvFile, autoList_t *colNames, autoList_t *colValueTable, char *destCsvFile);
void TR_CreateTable_cx(char *csvFile, autoList_t *colNames);
