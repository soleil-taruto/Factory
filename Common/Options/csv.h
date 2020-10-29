#pragma once

#include "C:\Factory\Common\all.h"

extern int CSVCellDelimiter;

autoList_t *readCSVFile(char *file);
autoList_t *readCSVFileTrim(char *file);
autoList_t *readCSVFileTR(char *file);
void writeCSVFile(char *file, autoList_t *table);

void csvTrim(autoList_t *table);
void csvRect(autoList_t *table);
void csvSquare(autoList_t *table);
void csvTwist(autoList_t *table);
void csvRot(autoList_t *table, sint degree);
void csvRot90(autoList_t *table);
void csvRot270(autoList_t *table);
void csvRot180(autoList_t *table);

char *LSrchComp(autoList_t *table, uint colidxFind, uint retColidx, char *cellFind, sint (*funcComp)(char *, char *));
char *LSrch(autoList_t *table, uint colidxFind, uint retColidx, char *cellFind);
char *LISrch(autoList_t *table, uint colidxFind, uint retColidx, char *cellFind);

// _x
autoList_t *readCSVFile_x(char *file);
autoList_t *readCSVFileTrim_x(char *file);
autoList_t *readCSVFileTR_x(char *file);
void writeCSVFile_cx(char *file, autoList_t *table);
void writeCSVFile_xc(char *file, autoList_t *table);
void writeCSVFile_xx(char *file, autoList_t *table);
