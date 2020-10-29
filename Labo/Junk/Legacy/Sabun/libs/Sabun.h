#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"
#include "Compress.h"
#include "Shift.h"

void makeSabun(char *sabunFile, char *beforeDir, char *afterDir, int correctAddDelete);
int sabunUpdate(char *sabunFile, char *targetDir);
