#pragma once

#include "C:\Factory\Common\all.h"

#define NECTAR2_E_MAX 10

typedef struct Nectar2_st // static member
{
	uint Handles[NECTAR2_E_MAX];
	autoBlock_t *RecvBuff;
}
Nectar2_t;

Nectar2_t *CreateNectar2(char *ident);
void ReleaseNectar2(Nectar2_t *i);
void Nectar2Send(Nectar2_t *i, autoBlock_t *message);
void Nectar2SendLine(Nectar2_t *i, char *line);
void Nectar2SendLine_x(Nectar2_t *i, char *line);
void Nectar2SendChar(Nectar2_t *i, int chr);
autoBlock_t *Nectar2Recv(Nectar2_t *i, int delimiter);
char *Nectar2RecvLine(Nectar2_t *i, int delimiter);
