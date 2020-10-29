#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\md5.h"
#include "DirToStream.h"

void VTreeToSumStream(VTree_t *vt, void (*streamWriter)(uchar *, uint));
void DirToSumStream(char *dir, void (*streamWriter)(uchar *, uint));
int SumStreamToDir(char *dir, void (*streamReader)(uchar *, uint));
int CheckSumStream(void (*streamReader)(uchar *, uint), uint64 streamLength);
