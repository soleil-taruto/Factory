#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Collabo.h"
#include "C:\Factory\Common\Options\RingCipher2.h"

extern int ZC_WithoutInfo;

void ZC_Cluster(char *rDir, char *wFile);
void ZC_Uncluster(char *rFile, char *wDir);

void ZC_Pack(char *rFile, char *wFile);
void ZC_Unpack(char *rFile, char *wFile);

void ZC_Encrypt(char *file, autoBlock_t *rawKey);
int ZC_Decrypt(char *file, autoBlock_t *rawKey);
