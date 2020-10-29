#pragma once

#include "C:\Factory\Common\all.h"

char *LineToCSVCell(char *line);
char *RowToCSVLine(autoList_t *row);
void RowToCSVStream(FILE *fp, autoList_t *row);
autoList_t *CSVStreamToRow(FILE *fp);
void TrimCSVRow(autoList_t *row, uint mincolcnt);

// _x
char *RowToCSVLine_x(autoList_t *row);
void RowToCSVStream_x(FILE *fp, autoList_t *row);
