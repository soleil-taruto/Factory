#pragma once

#include "C:\Factory\Common\all.h"

autoList_t *addBigInt(autoList_t *i, autoList_t *j);
autoList_t *subBigInt(autoList_t *i, autoList_t *j);
int compBigInt(autoList_t *i, autoList_t *j);
autoList_t *mulBigInt(autoList_t *i, autoList_t *j);
autoList_t *idivBigInt(autoList_t *i, uint j, uint *remain);
autoList_t *divBigInt(autoList_t *i, autoList_t *j, autoList_t **remain);
autoList_t *modBigInt(autoList_t *i, autoList_t *j);
extern autoList_t *modPowerBigInt(autoList_t *i, autoList_t *j, autoList_t *mod);
autoList_t *powerBigInt(autoList_t *i, autoList_t *j);
