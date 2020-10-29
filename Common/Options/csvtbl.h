#pragma once

#include "C:\Factory\Common\all.h"
#include "autoTable.h"
#include "csv.h"

autoTable_t *tReadCSVFile(char *file);
void tWriteCSVFile(char *file, autoTable_t *csv);
void tWriteCSVFile_cx(char *file, autoTable_t *csv);
void tWriteCSVFile_xx(char *file, autoTable_t *csv);
