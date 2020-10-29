#pragma once

#include "C:\Factory\Common\all.h"

extern uint DOSTimeoutSec;
extern uint CC_NoDataTimeoutSec;

#define GetTimeoutTime(tmoutSec) \
	((tmoutSec) ? now() + (tmoutSec) : UINTMAX)
