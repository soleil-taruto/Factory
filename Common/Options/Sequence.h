#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\mt19937.h"

void makeSq(autoList_t *list, uint start, sint step);
autoList_t *createSq(uint count, uint start, sint step);
void shuffle(autoList_t *list);
void sortSq(autoList_t *list);
uint chooseOneSq(autoList_t *list, int (*isTarget)(uint));
