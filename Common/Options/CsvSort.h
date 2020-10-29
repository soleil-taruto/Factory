#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csvStream.h"
#include "MergeSort.h"

void CsvSortComp(char *srcFile, char *destFile, uint sortColumnIndex, sint (*funcComp)(char *, char *), uint partSize);
void CsvSort(char *srcFile, char *destFile, uint sortColumnIndex, int sortDirection, int compareMode, uint partSize);
