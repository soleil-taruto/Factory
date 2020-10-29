#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRandom.h"
#include "C:\Factory\OpenSource\aes128.h"
#include "C:\Factory\OpenSource\sha512.h"

autoBlock_t *MakeEncCounterSeed(void);
autoBlock_t *MakeEncCounter(autoBlock_t *clSeed, autoBlock_t *svSeed);

void Crypt(autoBlock_t *data, autoBlock_t *rawKey, autoBlock_t *encCounter);

void PutHash(autoBlock_t *data);
int UnputHash(autoBlock_t *data);

void Add_RandPart_Padding(autoBlock_t *block);
int Unadd_RandPart_Padding(autoBlock_t *block);

void RCBCEncrypt(autoBlock_t *block, autoBlock_t *rawKey);
int RCBCDecrypt(autoBlock_t *block, autoBlock_t *rawKey);
