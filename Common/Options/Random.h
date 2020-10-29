#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"
#include "C:\Factory\OpenSource\sha512.h"

void sha512_expand(autoBlock_t *block, uint exnum);
void mt19937_initRnd(uint seed);
uint getRandElement(autoList_t *list);
