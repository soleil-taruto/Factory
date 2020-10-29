#pragma once

#include "bmp.h"
#include "C:\Factory\Common\Options\AutoTable.h"

autoTable_t *tReadBMPFile(char *file);
void tWriteBMPFile(char *file, autoTable_t *bmp);

// _x
void tWriteBMPFile_cx(char *file, autoTable_t *bmp);
void tWriteBMPFile_xx(char *file, autoTable_t *bmp);
