#include "all.h"

#define BASE_SIZE_UNRESIZABLE (UINTMAX - 0)
#define BASE_SIZE_EXPAND_ONLY (UINTMAX - 1)
#define BLOCK_SIZE_MAX        (UINTMAX - 2) // autoBlock_t の Size, AllocSize, BaseSize の(実装上の)最大値とする。

autoBlock_t *createAutoBlock(void *block, uint size, uint allocSize)
{
	autoBlock_t *i = (autoBlock_t *)memAlloc(sizeof(autoBlock_t));

	errorCase(!block);
	errorCase(allocSize < size);

	i->Block = (uchar *)block;
	i->Size = size;
	i->AllocSize = allocSize;
	i->BaseSize = size / 2;

	return i;
}
autoBlock_t *copyAutoBlock(autoBlock_t *i)
{
	autoBlock_t *j = (autoBlock_t *)memAlloc(sizeof(autoBlock_t));

	errorCase(!i);

	j->Block = (uchar *)memClone(i->Block, i->Size);
	j->Size = i->Size;
	j->AllocSize = i->Size;
	j->BaseSize = i->Size / 2;

	return j;
}
void releaseAutoBlock(autoBlock_t *i)
{
	errorCase(!i);
	errorCase(i->BaseSize == BASE_SIZE_UNRESIZABLE);

	memFree(i->Block);
	memFree(i);
}
void releaseBlock(autoBlock_t *i)
{
	errorCase(!i);
	errorCase(i->BaseSize == BASE_SIZE_UNRESIZABLE);

	memFree(i);
}

autoBlock_t *bindBlock(void *block, uint size)
{
	return createAutoBlock(block, size, size);
}
autoBlock_t *recreateBlock(void *block, uint size)
{
	return createAutoBlock(memClone(block, size), size, size);
}
autoBlock_t *createBlock(uint allocSize)
{
	return createAutoBlock(memAlloc(allocSize), 0, allocSize);
}
autoBlock_t *newBlock(void)
{
	return createBlock(16);
}
void *unbindBlock(autoBlock_t *i)
{
	void *block = directGetBuffer(i);

	releaseBlock(i);
	return block;
}

void setByte(autoBlock_t *i, uint index, uint byte)
{
	errorCase(!i);
	errorCase(i->Size <= index);

	i->Block[index] = byte;
}
uint getByte(autoBlock_t *i, uint index)
{
	errorCase(!i);
	errorCase(i->Size <= index);

	return i->Block[index];
}
void putByte(autoBlock_t *i, uint index, uint byte)
{
	errorCase(!i);

	if (i->Size <= index)
	{
		errorCase(BLOCK_SIZE_MAX <= index); // ? ブロックサイズの上限を超える。
		setSize(i, index + 1);
	}
	i->Block[index] = byte;
}
uint refByte(autoBlock_t *i, uint index)
{
	errorCase(!i);

	if (i->Size <= index)
		return 0x00;

	return i->Block[index];
}
void swapByte(autoBlock_t *i, uint index1, uint index2)
{
	uint swap;

	errorCase(!i);
	errorCase(i->Size <= index1);
	errorCase(i->Size <= index2);

	swap = i->Block[index1];
	i->Block[index1] = i->Block[index2];
	i->Block[index2] = swap;
}

#define EXPAND_SPAN 16000000 // 16 MB

static void Resize(autoBlock_t *i, uint newSize)
{
	if (i->BaseSize == BASE_SIZE_UNRESIZABLE)
	{
		errorCase(i->AllocSize < newSize);
	}
	else if (i->BaseSize == BASE_SIZE_EXPAND_ONLY)
	{
		if (i->AllocSize < newSize)
		{
			void *bkBlock = i->Block;

			errorCase(BLOCK_SIZE_MAX < newSize); // ? ブロックサイズの上限を超える。

			i->AllocSize = newSize;
			i->Block = memRealloc(i->Block, i->AllocSize);

			cout("[AB-Expand] %p -> %p [%c] (%u -> %u)\n", bkBlock, i->Block, bkBlock == i->Block ? ' ' : 'R', i->AllocSize, newSize);
		}
	}
	else
	{
		if (newSize < i->BaseSize || i->AllocSize < newSize)
		{
			if (newSize < 16)
			{
				i->BaseSize  = newSize;
				i->AllocSize = newSize;
			}
			else if (newSize < EXPAND_SPAN * 2)
			{
				i->BaseSize  = newSize / 2;
				i->AllocSize = newSize + newSize / 2; // newSize * 1.5
			}
			else
			{
				errorCase(BLOCK_SIZE_MAX - EXPAND_SPAN < newSize); // ? ブロックサイズの上限を超える。

				i->BaseSize  = newSize - EXPAND_SPAN;
				i->AllocSize = newSize + EXPAND_SPAN;
			}
			i->Block = memRealloc(i->Block, i->AllocSize);
		}
	}
	i->Size = newSize;
}

void addByte(autoBlock_t *i, uint byte)
{
	errorCase(!i);

	Resize(i, i->Size + 1);
	i->Block[i->Size - 1] = byte;
}
uint unaddByte(autoBlock_t *i)
{
	uint byte;

	errorCase(!i);
	errorCase(!i->Size);

	byte = i->Block[i->Size - 1];
	Resize(i, i->Size - 1);
	return byte;
}

void insertBytes(autoBlock_t *i, uint index, autoBlock_t *bytes)
{
	errorCase(!i);
	errorCase(i->Size < index);
	errorCase(BLOCK_SIZE_MAX - i->Size < bytes->Size); // ? ブロックサイズの上限を超える。

	if (bytes->Size)
	{
		uint n;

		Resize(i, i->Size + bytes->Size);

#if 1
		copyBlock(
			i->Block + index + bytes->Size,
			i->Block + index,
			i->Size - (index + bytes->Size)
			);
		memcpy(i->Block + index, bytes->Block, bytes->Size);
#else // old_same
		for (n = i->Size - 1; index + bytes->Size <= n; n--)
		{
			i->Block[n] = i->Block[n - bytes->Size];
		}
		while (1)
		{
			i->Block[n] = bytes->Block[n - index];
			if (n == index) break;
			n--;
		}
#endif
	}
}
void insertByteRepeat(autoBlock_t *i, uint index, uint byte, uint count)
{
	errorCase(!i);
	errorCase(i->Size < index);
	errorCase(BLOCK_SIZE_MAX - i->Size < count); // ? ブロックサイズの上限を超える。

	if (count)
	{
		uint n;

		Resize(i, i->Size + count);

#if 1
		copyBlock(
			i->Block + index + count,
			i->Block + index,
			i->Size - (index + count)
			);
		memset(i->Block + index, byte, count);
#else // old_same
		for (n = i->Size - 1; index + count <= n; n--)
		{
			i->Block[n] = i->Block[n - count];
		}
		while (1)
		{
			i->Block[n] = byte;
			if (n == index) break;
			n--;
		}
#endif
	}
}
void insertByte(autoBlock_t *i, uint index, uint byte)
{
	insertByteRepeat(i, index, byte, 1);
}
autoBlock_t *desertBytes(autoBlock_t *i, uint index, uint count)
{
	autoBlock_t *bytes;
	autoBlock_t gab;
	uint n;

	errorCase(!i);
	errorCase(i->Size < index);
	errorCase(i->Size - index < count);

	bytes = copyAutoBlock(gndSubBytesVar(i, index, count, gab));

#if 1
	copyBlock(
		i->Block + index,
		i->Block + index + count,
		i->Size - (index + count)
		);
#else // old_same
	for (n = index; n < i->Size - count; n++)
	{
		i->Block[n] = i->Block[n + count];
	}
#endif
	Resize(i, i->Size - count);
	return bytes;
}
uint desertByte(autoBlock_t *i, uint index)
{
	autoBlock_t *bytes = desertBytes(i, index, 1);
	uint byte;

	errorCase(getSize(bytes) != 1);

	byte = getByte(bytes, 0);
	releaseAutoBlock(bytes);
	return byte;
}
void removeBytes(autoBlock_t *i, uint index, uint count)
{
#if 1
	errorCase(!i);
	errorCase(i->Size < index);
	errorCase(i->Size - index < count);

	copyBlock(
		i->Block + index,
		i->Block + index + count,
		i->Size - index - count
		);
	Resize(i, i->Size - count);
#else // old_same
	releaseAutoBlock(desertBytes(i, index, count));
#endif
}

/*
	バッファサイズを実際のデータサイズに合わせる。
	以後、拡張のみ行うようにする。Resize()での挙動に注意すること。
*/
void fixBytes(autoBlock_t *i)
{
	errorCase(!i);
	setAllocSize(i, 0);
}
/*
	バッファサイズを少なくとも size バイトにする。
	以後、拡張のみ行うようにする。Resize()での挙動に注意すること。
*/
void setAllocSize(autoBlock_t *i, uint size)
{
	errorCase(!i);

	size = m_max(size, i->Size);

	i->Block = memRealloc(i->Block, size);
	i->AllocSize = size;
	i->BaseSize = BASE_SIZE_EXPAND_ONLY;
}
void setSize(autoBlock_t *i, uint size)
{
	uint oldSize;

	errorCase(!i);

	oldSize = i->Size;
	Resize(i, size);

	if (oldSize < i->Size)
	{
		memset(i->Block + oldSize, 0x00, i->Size - oldSize); // 未定義部をゼロで整地する。
	}
}
uint getSize(autoBlock_t *i)
{
	errorCase(!i);
	return i->Size;
}

// NO-Bulldoze
autoBlock_t *nobCreateBlock(uint size)
{
	return createAutoBlock(memAlloc(size), size, size);
}
void nobSetSize(autoBlock_t *i, uint size)
{
	errorCase(!i);
	Resize(i, size);
}

// Bulldoze-NEW
autoBlock_t *bCreateBlock(uint size)
{
	autoBlock_t *i = newBlock();

	setSize(i, size);
	return i;
}
void clearBlock(autoBlock_t *i)
{
	memset(i->Block, 0x00, i->Size);
}
void resetSize(autoBlock_t *i, uint size)
{
	setSize(i, 0);
	setSize(i, size);
}

/*
	使い方
		autoBlock_t gab = gndBlock(statically_declared_block, size);
		reverseBytes(&gab);
		開放しなくて良い。
*/
autoBlock_t gndBlock(void *block, uint size)
{
	autoBlock_t i;

	i.Block = (uchar *)block;
	i.Size = size;
	i.AllocSize = size;
	i.BaseSize = BASE_SIZE_UNRESIZABLE;

	return i;
}
autoBlock_t gndSubBytes(autoBlock_t *i, uint start, uint size)
{
	errorCase(!i);
	errorCase(i->Size < start);
	errorCase(i->Size - start < size);

	return gndBlock(i->Block + start, size);
}
autoBlock_t gndFollowBytes(autoBlock_t *i, uint start)
{
	return gndSubBytes(i, start, i->Size - start);
}
autoBlock_t *gndBlockVarPtr(void *block, uint size, autoBlock_t *varPtr)
{
	*varPtr = gndBlock(block, size);
	return varPtr;
}
autoBlock_t *gndSubBytesVarPtr(autoBlock_t *i, uint start, uint size, autoBlock_t *varPtr)
{
	*varPtr = gndSubBytes(i, start, size);
	return varPtr;
}
autoBlock_t *gndFollowBytesVarPtr(autoBlock_t *i, uint start, autoBlock_t *varPtr)
{
	*varPtr = gndFollowBytes(i, start);
	return varPtr;
}

void *directGetBuffer(autoBlock_t *i)
{
	errorCase(!i);
	return i->Block;
}
uchar *directGetBlock(autoBlock_t *i, uint index, uint size)
{
	errorCase(!i);
	errorCase(i->Size < index);
	errorCase(i->Size - index < size);
	return (uchar *)i->Block + index;
}
void reverseBytes(autoBlock_t *i)
{
	uint n, f;
	uint swap;

	errorCase(!i);

	if (i->Size)
	{
		for (n = 0, f = i->Size - 1; n < f; n++, f--)
		{
			swap = i->Block[n];
			i->Block[n] = i->Block[f];
			i->Block[f] = swap;
		}
	}
}
void addBytes(autoBlock_t *i, autoBlock_t *bytes)
{
#if 1
	errorCase(!i);
	errorCase(!bytes);
	errorCase(BLOCK_SIZE_MAX - i->Size < bytes->Size); // ブロックサイズの上限を超える。

	Resize(i, i->Size + bytes->Size);
	memcpy(i->Block + i->Size - bytes->Size, bytes->Block, bytes->Size);
#else // old_same @ 2019.3.24
	uint index;

	errorCase(!i);
	errorCase(!bytes);

	for (index = 0; index < getSize(bytes); index++)
	{
		addByte(i, getByte(bytes, index));
	}
#endif
}
autoBlock_t *unaddBytesRev(autoBlock_t *i, uint count) // [... a b c] -> [c b a]
{
#if 1
	autoBlock_t *block = unaddBytes(i, count);

	reverseBytes(block);
	return block;
#else // old_same @ 2019.3.24
	autoBlock_t *block = createBlock(count);

	while (count)
	{
		addByte(block, unaddByte(i));
		count--;
	}
	return block;
#endif
}
autoBlock_t *unaddBytes(autoBlock_t *i, uint count) // [... a b c] -> [a b c]
{
#if 1
	autoBlock_t *block;

	errorCase(!i);
	errorCase(i->Size < count);

	block = nobCreateBlock(count);
	memcpy(block->Block, i->Block + (i->Size - count), count);
	Resize(i, i->Size - count);
	return block;
#else // old_same @ 2019.3.24
	autoBlock_t *block = unaddBytesRev(i, count);

	reverseBytes(block);
	return block;
#endif
}
void unaddBytesRevToBlock(autoBlock_t *i, void *block, uint size)
{
	autoBlock_t *subBlock = unaddBytesRev(i, size);

	memcpy(block, directGetBuffer(subBlock), size);
	releaseAutoBlock(subBlock);
}
void unaddBytesToBlock(autoBlock_t *i, void *block, uint size)
{
	autoBlock_t *subBlock = unaddBytes(i, size);

	memcpy(block, directGetBuffer(subBlock), size);
	releaseAutoBlock(subBlock);
}
autoBlock_t *getSubBytes(autoBlock_t *i, uint index, uint count)
{
	autoBlock_t gab;
	return copyAutoBlock(gndSubBytesVar(i, index, count, gab));
}
autoBlock_t *getFollowBytes(autoBlock_t *i, uint index)
{
	autoBlock_t gab;
	return copyAutoBlock(gndFollowBytesVar(i, index, gab));
}
char *unbindBlock2Line(autoBlock_t *i)
{
	addByte(i, '\0');
	return unbindBlock(i);
}

uint findByteMatch(autoBlock_t *i, int (*match)(uint))
{
	uint index;

	for (index = 0; index < i->Size; index++)
		if (match(i->Block[index]))
			break;

	return index;
}

static uint FB_Byte;

static int FB_Match(uint byte)
{
	return FB_Byte == byte;
}
uint findByte(autoBlock_t *i, uint byte)
{
	FB_Byte = byte;
	return findByteMatch(i, FB_Match);
}
