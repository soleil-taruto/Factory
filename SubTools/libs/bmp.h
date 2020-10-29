#pragma once

#include "C:\Factory\Common\all.h"

autoList_t *readBMPFile(char *file);
void writeBMPFile(char *file, autoList_t *table);

// _x
void writeBMPFile_cx(char *file, autoList_t *table);
void writeBMPFile_xx(char *file, autoList_t *table);
