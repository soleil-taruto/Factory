#pragma once

#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\sha512.h"

#define MUTECTOR_MTX_MAX 11

typedef struct Mutector_st // static member
{
	uint Mtxs[MUTECTOR_MTX_MAX];
	int Statuses[MUTECTOR_MTX_MAX];
}
Mutector_t;

Mutector_t *CreateMutector(char *name);
void ReleaseMutector(Mutector_t *i);

void MutectorSend(Mutector_t *i, autoBlock_t *message);
void MutectorSendLine(Mutector_t *i, char *line);

void MutectorRecv(Mutector_t *i, int (*interlude)(void), void (*recved)(autoBlock_t *));
