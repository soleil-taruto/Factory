#include "all.h"

void ab_addBlock(autoBlock_t *i, void *block, uint size)
{
	autoBlock_t gab;
	addBytes(i, gndBlockVar(block, size, gab));
}
void ab_addBlock_x(autoBlock_t *i, void *block, uint size)
{
	ab_addBlock(i, block, size);
	memFree(block);
}
void ab_addBytes(autoBlock_t *i, autoBlock_t *block)
{
	addBytes(i, block);
}
void ab_addBytes_x(autoBlock_t *i, autoBlock_t *block)
{
	addBytes(i, block);
	releaseAutoBlock(block);
}
void ab_addSubBytes(autoBlock_t *i, autoBlock_t *block, uint start, uint size)
{
	autoBlock_t gab;
	addBytes(i, gndSubBytesVar(block, start, size, gab));
}
void ab_addSubBytes_x(autoBlock_t *i, autoBlock_t *block, uint start, uint size)
{
	ab_addSubBytes(i, block, start, size);
	releaseAutoBlock(block);
}
void ab_addFollowBytes(autoBlock_t *i, autoBlock_t *block, uint start)
{
	autoBlock_t gab;
	addBytes(i, gndFollowBytesVar(block, start, gab));
}
void ab_addFollowBytes_x(autoBlock_t *i, autoBlock_t *block, uint start)
{
	ab_addFollowBytes(i, block, start);
	releaseAutoBlock(block);
}
void ab_addLine(autoBlock_t *i, char *line)
{
	autoBlock_t gab;
	addBytes(i, gndBlockLineVar(line, gab));
}
void ab_addLine_x(autoBlock_t *i, char *line)
{
	ab_addLine(i, line);
	memFree(line);
}

/*
	Little Endian
*/
void ab_addValue(autoBlock_t *i, uint value)
{
	addByte(i, value >>  0 & 0xff);
	addByte(i, value >>  8 & 0xff);
	addByte(i, value >> 16 & 0xff);
	addByte(i, value >> 24 & 0xff);
}
uint ab_unaddValue(autoBlock_t *i)
{
	uint value = unaddByte(i) << 24;

	value |= unaddByte(i) << 16;
	value |= unaddByte(i) <<  8;
	value |= unaddByte(i) <<  0;

	return value;
}
void ab_insertValue(autoBlock_t *i, uint index, uint value)
{
	insertByte(i, index, value >> 24 & 0xff);
	insertByte(i, index, value >> 16 & 0xff);
	insertByte(i, index, value >>  8 & 0xff);
	insertByte(i, index, value >>  0 & 0xff);
}
uint ab_desertValue(autoBlock_t *i, uint index)
{
	uint value = desertByte(i, index);

	value |= desertByte(i, index) <<  8;
	value |= desertByte(i, index) << 16;
	value |= desertByte(i, index) << 24;

	return value;
}
void ab_setValue(autoBlock_t *i, uint index, uint value)
{
	setByte(i, index + 0, value >>  0 & 0xff);
	setByte(i, index + 1, value >>  8 & 0xff);
	setByte(i, index + 2, value >> 16 & 0xff);
	setByte(i, index + 3, value >> 24 & 0xff);
}
uint ab_getValue(autoBlock_t *i, uint index)
{
	uint value = getByte(i, index);

	value |= getByte(i, index + 1) <<  8;
	value |= getByte(i, index + 2) << 16;
	value |= getByte(i, index + 3) << 24;

	return value;
}
void ab_putValue(autoBlock_t *i, uint index, uint value)
{
	putByte(i, index + 0, value >>  0 & 0xff);
	putByte(i, index + 1, value >>  8 & 0xff);
	putByte(i, index + 2, value >> 16 & 0xff);
	putByte(i, index + 3, value >> 24 & 0xff);
}
uint ab_refValue(autoBlock_t *i, uint index)
{
	uint value = refByte(i, index);

	value |= refByte(i, index + 1) <<  8;
	value |= refByte(i, index + 2) << 16;
	value |= refByte(i, index + 3) << 24;

	return value;
}

/*
	Big Endian
*/
uint ab_getValueBE(autoBlock_t *i, uint index)
{
	return revEndian(ab_getValue(i, index));
}

void *ab_makeBlock(autoBlock_t *i)
{
	return unbindBlock(copyAutoBlock(i));
}
void *ab_makeBlock_x(autoBlock_t *i)
{
	return unbindBlock(i);
}
autoBlock_t *ab_makeSubBytes(autoBlock_t *i, uint start, uint size)
{
	autoBlock_t gab;
	return copyAutoBlock(gndSubBytesVar(i, start, size, gab));
}
autoBlock_t *ab_makeSubBytes_x(autoBlock_t *i, uint start, uint size)
{
	autoBlock_t *newi = ab_makeSubBytes(i, start, size);
	releaseAutoBlock(i);
	return newi;
}
autoBlock_t *ab_makeFollowBytes(autoBlock_t *i, uint start)
{
	autoBlock_t gab;
	return copyAutoBlock(gndFollowBytesVar(i, start, gab));
}
autoBlock_t *ab_makeFollowBytes_x(autoBlock_t *i, uint start)
{
	autoBlock_t *newi = ab_makeFollowBytes(i, start);
	releaseAutoBlock(i);
	return newi;
}
char *ab_makeLine(autoBlock_t *i)
{
	return unbindBlock2Line(copyAutoBlock(i));
}
char *ab_makeLine_x(autoBlock_t *i)
{
	return unbindBlock2Line(i);
}
autoBlock_t *ab_makeBlockLine(char *line)
{
	return recreateBlock(line, strlen(line));
}
autoBlock_t *ab_makeBlockLine_x(char *line)
{
	return bindBlock(line, strlen(line));
}

void ab_addTrailZero(autoBlock_t *i, uint count)
{
	while (count)
	{
		addByte(i, 0x00);
		count--;
	}
}
uint ab_unaddTrailZero(autoBlock_t *i)
{
	uint count = 0;

	while (getSize(i) && !getByte(i, getSize(i) - 1))
	{
		unaddByte(i);
		count++;
	}
	return count;
}

void ab_swap(autoBlock_t *i, autoBlock_t *j)
{
	autoBlock_t gab;

	gab = *i;
	*i = *j;
	*j = gab;
}
autoBlock_t *ab_eject(autoBlock_t *i)
{
	autoBlock_t *j = createBlock(0);
	ab_swap(i, j);
	return j;
}
void ab_uneject(autoBlock_t *i, autoBlock_t *j)
{
	ab_swap(i, j);
	releaseAutoBlock(j);
}
