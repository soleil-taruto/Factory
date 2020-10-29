#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

void mmpc_addPart_xxx(char *text, char *fileName, autoBlock_t *body);
void mmpc_addPart(char *text, char *fileName, autoBlock_t *body);
void mmpc_addPartFile(char *text, char *fileName, char *bodyFile);
autoBlock_t *makeMultiPartContent(void);
