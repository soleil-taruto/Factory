#include "autoQueue.h"

#define DEADSPACESIZEMAX (16 * 1024)

autoQueue_t *newQueue(void)
{
	autoQueue_t *i = (autoQueue_t *)memAlloc(sizeof(autoQueue_t));

	i->Buffer = newBlock();
	i->NextPos = 0;

	return i;
}
void releaseAutoQueue(autoQueue_t *i)
{
	releaseAutoBlock(i->Buffer);
	memFree(i);
}

static void ForceTrim(autoQueue_t *i)
{
	removeBytes(i->Buffer, 0, i->NextPos);
	i->NextPos = 0;
}
static void Trim(autoQueue_t *i)
{
	if (DEADSPACESIZEMAX < i->NextPos || i->NextPos == getSize(i->Buffer))
	{
		ForceTrim(i);
	}
}

void *directGetQueBuffer(autoQueue_t *i)
{
	return (uchar *)directGetBuffer(i->Buffer) + i->NextPos;
}
uint getQueSize(autoQueue_t *i)
{
	return getSize(i->Buffer) - i->NextPos;
}

void enqByte(autoQueue_t *i, uint chr)
{
	addByte(i->Buffer, chr);
}
uint deqByte(autoQueue_t *i)
{
	uint chr;

	errorCase(getSize(i->Buffer) <= i->NextPos);

	chr = getByte(i->Buffer, i->NextPos);
	i->NextPos++;
	Trim(i);
	return chr;
}

void enqValue(autoQueue_t *i, uint value)
{
	enqByte(i, value >>  0 & 0xff);
	enqByte(i, value >>  8 & 0xff);
	enqByte(i, value >> 16 & 0xff);
	enqByte(i, value >> 24 & 0xff);
}
uint deqValue(autoQueue_t *i)
{
	uint value = deqByte(i);

	value |= deqByte(i) <<  8;
	value |= deqByte(i) << 16;
	value |= deqByte(i) << 24;

	return value;
}

void enqBytes(autoQueue_t *i, autoBlock_t *bytes)
{
	addBytes(i->Buffer, bytes);
}
autoBlock_t *deqBytes(autoQueue_t *i, uint size)
{
	autoBlock_t *block;

	errorCase(getQueSize(i) < size);

	block = getSubBytes(i->Buffer, i->NextPos, size);
	i->NextPos += size;
	Trim(i);
	return block;
}

void enqBlock(autoQueue_t *i, autoBlock_t *block)
{
	enqValue(i, getSize(block));
	enqBytes(i, block);
}
autoBlock_t *deqBlock(autoQueue_t *i)
{
	return deqBytes(i, deqValue(i));
}

void enqLine(autoQueue_t *i, char *line)
{
	autoBlock_t gab;
	enqBlock(i, gndBlockLineVar(line, gab));
}
char *deqLine(autoQueue_t *i)
{
	return unbindBlock2Line(deqBlock(i));
}

// _x
void enqBytes_x(autoQueue_t *i, autoBlock_t *bytes)
{
	enqBytes(i, bytes);
	releaseAutoBlock(bytes);
}
void enqBlock_x(autoQueue_t *i, autoBlock_t *block)
{
	enqBlock(i, block);
	releaseAutoBlock(block);
}
void enqLine_x(autoQueue_t *i, char *line)
{
	enqLine(i, line);
	memFree(line);
}
