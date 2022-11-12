#include "mt19937.h"

static void PostInit(void)
{
#if 1 // zantei -- ‰Šú‰»‚É•sˆÀ @ 2020.8.20
	uint count;

//	                                          ‘ª’è @ 2020.8.20
//	for (count = 1000000; count; count--)   // ‚¾‚¢‚½‚¢    3 millis
	for (count = 3000000; count; count--)   // ‚¾‚¢‚½‚¢   10 millis
//	for (count = 10000000; count; count--)  // ‚¾‚¢‚½‚¢   30 millis
//	for (count = 30000000; count; count--)  // ‚¾‚¢‚½‚¢  100 millis
//	for (count = 100000000; count; count--) // ‚¾‚¢‚½‚¢  300 millis
//	for (count = 300000000; count; count--) // ‚¾‚¢‚½‚¢ 1000 millis
	{
		mt19937_genrand_int32();
	}
#endif
}
void mt19937_initByArray(autoBlock_t *initKey)
{
	errorCase(getSize(initKey) < sizeof(uint));

	mt19937_init_by_array((uint *)directGetBuffer(initKey), getSize(initKey) / sizeof(uint));
	PostInit();
}
void mt19937_init32(uint seed)
{
	mt19937_init_genrand(seed);
	PostInit();
}
void mt19937_init(void)
{
	mt19937_init32((uint)time(NULL));
}

uint mt19937_rnd32(void) // ret: 0 ` UINTMAX
{
	return mt19937_genrand_int32();
}
uint mt19937_rnd(uint modulo) // ret: 0 ` (modulo - 1)
{
	uint r_mod;
	uint r;

	errorCase(!modulo);

#if 1
	r_mod = 0x100000000ui64 % modulo;
#else
	r_mod = (0xffffffffui % modulo + 1) % modulo;
#endif

	do
	{
		r = mt19937_rnd32();
	}
	while (r < r_mod);

#if 0
	r -= r_mod;
	r /= 0x100000000ui64 / modulo;
#else
	r %= modulo;
#endif

	return r;
}
uint mt19937_range(uint minval, uint maxval) // ret: minval ` maxval
{
	errorCase(maxval < minval);

	if (minval == 0 && maxval == UINTMAX)
	{
		return mt19937_rnd32();
	}
	return minval + mt19937_rnd(maxval - minval + 1);
}

uint64 mt19937_rnd64(void) // ret: 0 ` UINT64MAX
{
	return (uint64)mt19937_rnd32() << 32 | (uint64)mt19937_rnd32();
}
uint64 mt19937_rnd64Mod(uint64 modulo) // ret: 0 ` (modulo - 1)
{
	uint64 r_mod;
	uint64 r;

	errorCase(modulo == 0ui64);

	r_mod = (UINT64MAX % modulo + 1ui64) % modulo;

	do
	{
		r = mt19937_rnd64();
	}
	while (r < r_mod);

	r %= modulo;

	return r;
}
uint64 mt19937_range64(uint64 minval, uint64 maxval) // ret: minval ` maxval
{
	errorCase(maxval < minval);

	if (minval == 0 && maxval == UINT64MAX)
	{
		return mt19937_rnd64();
	}
	return minval + mt19937_rnd64Mod(maxval - minval + 1);
}
void mt19937_rndBlock(void *block, uint size)
{
	uint index;

	for (index = 0; index < size / sizeof(uint); index++)
	{
		((uint *)block)[index] = mt19937_rnd32();
	}
	if (size & 3)
	{
		uint value = mt19937_rnd32();

		memcpy((uint *)block + index, &value, size & 3);
	}
}
