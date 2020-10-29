#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\csvStream.h"
#include "wav.h"

void readWAVFileToCSVFile(char *rFile, char *wFile);
void writeWAVFileFromCSVFile(char *rFile, char *wFile, uint hz);
