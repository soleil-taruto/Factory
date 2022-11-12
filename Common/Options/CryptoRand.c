#include "CryptoRand.h"

#define POOLSIZE_BGN 8
#define POOLSIZE_MAX (16 * 1024 * 1024) // == POOLSIZE_BGN * (2^n)

uint getCryptoByte(void)
{
	static uchar *pool;
	static uint poolSize = POOLSIZE_BGN;
	static uint index    = POOLSIZE_BGN;

	if (index == poolSize)
	{
		if (poolSize < POOLSIZE_MAX)
		{
			poolSize *= 2;
			pool = memRealloc(pool, poolSize);
		}
		getCryptoBlock_MS(pool, poolSize);
		index = 0;
	}
	return pool[index++];
}
void getCryptoBytes(uchar *buff, uint size)
{
	uint index;

	for (index = 0; index < size; index++)
	{
		buff[index] = getCryptoByte();
	}
}
uint getCryptoRand16(void)
{
	uchar r[2];

	getCryptoBytes(r, 2);

	return
		((uint)r[0] << 0) |
		((uint)r[1] << 8);
}
uint getCryptoRand24(void)
{
	uchar r[3];

	getCryptoBytes(r, 3);

	return
		((uint)r[0] << 0) |
		((uint)r[1] << 8) |
		((uint)r[2] << 16);
}
uint getCryptoRand(void)
{
	uchar r[4];

	getCryptoBytes(r, 4);

	return
		((uint)r[0] << 0) |
		((uint)r[1] << 8) |
		((uint)r[2] << 16) |
		((uint)r[3] << 24);
}
uint64 getCryptoRand64(void)
{
	uchar r[8];

	getCryptoBytes(r, 8);

	return
		((uint64)r[0] << 0) |
		((uint64)r[1] << 8) |
		((uint64)r[2] << 16) |
		((uint64)r[3] << 24) |
		((uint64)r[4] << 32) |
		((uint64)r[5] << 40) |
		((uint64)r[6] << 48) |
		((uint64)r[7] << 56);
}
uint64 getCryptoRand64Mod(uint64 modulo) // ret: 0 ` (modulo - 1)
{
	uint64 r_mod;
	uint64 r;

	errorCase(modulo == 0ui64);

	r_mod = (UINT64MAX % modulo + 1ui64) % modulo;

	do
	{
		r = getCryptoRand64();
	}
	while (r < r_mod);

	return r % modulo;
}
uint64 getCryptoRand64Range(uint64 minval, uint64 maxval) // ret: minval ` maxval
{
	errorCase(maxval < minval);

	if (minval == 0 && maxval == UINT64MAX)
	{
		return getCryptoRand64();
	}
	return minval + getCryptoRand64Mod(maxval - minval + 1);
}
autoBlock_t *makeCryptoRandBlock(uint count)
{
	autoBlock_t *block = createBlock(count);

	while (count)
	{
		addByte(block, getCryptoByte());
		count--;
	}
	return block;
}
