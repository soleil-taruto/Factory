#pragma once

#include "C:\Factory\Common\all.h"
#include "CRandom.h"

FILE *FLockLoop(char *file);
FILE *FLock(char *file);
void FUnlock(FILE *fp);
