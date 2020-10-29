#pragma once

#include "C:\Factory\Common\all.h"

extern uint lastWAV_Hz;

autoList_t *readWAVFile(char *file);
void writeWAVFile(char *file, autoList_t *wavData, uint hz);
