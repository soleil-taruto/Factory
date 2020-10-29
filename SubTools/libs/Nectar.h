#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\sha512.h"

typedef struct Nectar_st // static member
{
	uint EvData;
	uint EvCtrl;
	uint EvSend;
	uint EvRecv;
}
Nectar_t;

Nectar_t *CreateNectar(char *name);
void ReleaseNectar(Nectar_t *i);

void NectarSend(Nectar_t *i, autoBlock_t *buff);
void NectarSendLine(Nectar_t *i, char *line);

autoBlock_t *NectarRecv(Nectar_t *i);
autoBlock_t *NectarReceipt(Nectar_t *i);
