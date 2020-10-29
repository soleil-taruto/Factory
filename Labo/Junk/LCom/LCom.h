#pragma once

#include "C:\Factory\Common\all.h"

char *LComSendLine(char *line, uint id);
void LComRecvLoop(char *(*funcRecvEvent)(char *), uint id);
