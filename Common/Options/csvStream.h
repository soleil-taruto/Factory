#pragma once

#include "C:\Factory\Common\all.h"

char *readCSVCell(FILE *fp, int *p_termChr);
autoList_t *readCSVRow(FILE *fp);

void writeCSVCell(FILE *fp, char *cell);
void writeCSVRow(FILE *fp, autoList_t *row);

// _x
void writeCSVCell_x(FILE *fp, char *cell);
void writeCSVRow_x(FILE *fp, autoList_t *row);
