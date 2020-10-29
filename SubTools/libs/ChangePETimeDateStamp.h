#pragma once

#include "C:\Factory\Common\all.h"

void ChangePETimeDateStamp(char *file, uint t);
void ChangeAllPETimeDateStamp(char *dir, uint t);
uint GetPETimeDateStamp(char *file);
