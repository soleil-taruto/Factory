#pragma once

#include "C:\Factory\Common\all.h"
#include "csv.h"

extern int RSTR_NoHeaderMode;

autoList_t *RSTR_SecondLineToColSpans(char *line);
autoList_t *RSTR_LineToValues(char *line, autoList_t *colSpans, int asColName);
void RedirSqlTableReader(char *file, FILE *outStrm, void (*writeColNames_x)(FILE *outStrm, autoList_t *colNames), void (*writeRow_x)(FILE *outStrm, autoList_t *row));
void RSTR_ToLinearFile(char *rfile, char *wfile);
void RSTR_ToCSVFile(char *rfile, char *wfile);
