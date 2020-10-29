#pragma once

#include "C:\Factory\Common\all.h"
#include "VTree.h"

extern int DTS_WithInfo;
extern int (*DTS_AcceptPath)(char *);
extern int STD_TrustMode;
extern int STD_ReadStop;

void VTreeToStream(VTree_t *vt, void (*streamWriter)(uchar *, uint));
void DirToStream(char *dir, void (*streamWriter)(uchar *, uint));
void StreamToDir(char *dir, void (*streamReader)(uchar *, uint));
