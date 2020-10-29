#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"

extern int MRLChrLow;
extern int MRLChrUpper;
int MRLGetCharRange(void);
extern int (*MRLGetChar)(void);

extern char *MRLChrSet;
int MRLGetCharSet(void);

char *MakeRandLine(uint count);
char *MakeRandLineRange(uint lenmin, uint lenmax);
autoList_t *MakeRandTextLinesRange(uint row, uint lenmin, uint lenmax);
autoList_t *MakeRandTextLines(uint row, uint column);
void MakeRandTextFileRange(char *file, uint64 rowCounter, uint lenmin, uint lenmax);
void MakeRandTextFile(char *file, uint64 rowCounter, uint column);

autoBlock_t *MakeRandBinaryBlock(uint size);
void MakeRandBinaryFile(char *file, uint64 fileSize);

void RandXorOneBit(autoBlock_t *block);
void RandXorOneBitFile_RW(char *rFile, char *wFile);
void RandXorOneBitFile(char *file);
