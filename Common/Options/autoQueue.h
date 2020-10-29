#pragma once

#include "C:\Factory\Common\all.h"

typedef struct autoQueue_st // static member
{
	autoBlock_t *Buffer;
	uint NextPos;
}
autoQueue_t;

autoQueue_t *newQueue(void);
void releaseAutoQueue(autoQueue_t *i);

void *directGetQueBuffer(autoQueue_t *i);
uint getQueSize(autoQueue_t *i);

void enqByte(autoQueue_t *i, uint chr);
uint deqByte(autoQueue_t *i);

void enqValue(autoQueue_t *i, uint value);
uint deqValue(autoQueue_t *i);

void enqBytes(autoQueue_t *i, autoBlock_t *bytes);
autoBlock_t *deqBytes(autoQueue_t *i, uint size);

void enqBlock(autoQueue_t *i, autoBlock_t *block);
autoBlock_t *deqBlock(autoQueue_t *i);

void enqLine(autoQueue_t *i, char *line);
char *deqLine(autoQueue_t *i);

// _x
void enqBytes_x(autoQueue_t *i, autoBlock_t *bytes);
void enqBlock_x(autoQueue_t *i, autoBlock_t *block);
void enqLine_x(autoQueue_t *i, char *line);
