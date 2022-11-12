#include "C:\Factory\Common\all.h"

// https://ja.wikipedia.org/wiki/Xorshift
// https://en.wikipedia.org/wiki/Xorshift

static uint Xorshift32(void)
{
	static uint x = 1;

	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;

	return x;
}
static uint Xorshift64(void)
{
	static uint64 x = 1;

	x ^= x << 13;
	x ^= x >> 7;
	x ^= x << 17;

	return x;
}
static uint Xorshift96(void)
{
	static uint x = 1;
	static uint y;
	static uint z;
	uint t;

	t = x;
	t ^= y;
	t ^= z;
	t ^= x << 3;
	t ^= y >> 19;
	t ^= z << 6;
	x = y;
	y = z;
	z = t;

	return t;
}
static uint Xorshift128(void)
{
	static uint x = 1;
	static uint y;
	static uint z;
	static uint a;
	uint t;

	t = x;
	t ^= x << 11;
	t ^= t >> 8;
	t ^= a;
	t ^= a >> 19;
	x = y;
	y = z;
	z = a;
	a = t;

	return t;
}
int main(int argc, char **argv)
{
	uint c;

	for (c = 1000; c; c--)
	{
		cout("%08x %08x %08x %08x\n", Xorshift32(), Xorshift64(), Xorshift96(), Xorshift128());
	}
}
