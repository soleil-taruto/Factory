#include "Prime2.h"

#if 1
#define P13_LEN 15015
#define P13_P_NUM (P13_LEN * 64)

//#define PBIT_LEN (P13_LEN * 16)
//#define PBIT_LEN (P13_LEN * 32)
//#define PBIT_LEN (P13_LEN * 64)
//#define PBIT_LEN (P13_LEN * 128)
//#define PBIT_LEN (P13_LEN * 256)
//#define PBIT_LEN (P13_LEN * 512)
//#define PBIT_LEN (P13_LEN * 1024)
//#define PBIT_LEN (P13_LEN * 2048)
//#define PBIT_LEN (P13_LEN * 3072)
#define PBIT_LEN (P13_LEN * 4096)

#define PBIT_P_NUM ((uint)PBIT_LEN * 64)
#else
#define P13_LEN 15015
#define P13_P_NUM (P13_LEN * 64) // == 960960
#define PBIT_LEN (P13_LEN * 4096) // == 61501440
#define PBIT_P_NUM ((uint)PBIT_LEN * 64) // == 3936092160
#endif

// ---- PBit ----

static uint *PBits; // bit_0 == prime, bit_1 == not prime
static uint64 BaseNumb;

static int GetPBit(uint prime)
{
	uint bit;
	uint index;

//	errorCase(prime % 2 == 0);

	bit = prime / 2;
	index = bit / 32;
	bit = bit % 32;

//	errorCase(PBIT_LEN <= index);

	return PBits[index] >> bit & 1;
}
static void SetPBit(uint prime, int flag)
{
	uint bit;
	uint index;
	uint c;

//	errorCase(prime % 2 == 0);

	bit = prime / 2;
	index = bit / 32;
	bit = bit % 32;

//	errorCase(PBIT_LEN <= index);

	c = PBits[index];

	if (flag)
		c |= 1u << bit;
	else
		c &= ~(1u << bit);

	PBits[index] = c;
}

// ----

static void PutP13(uint prime)
{
	uint64 count;

	for (count = prime; count < P13_P_NUM; count += prime * 2)
	{
		SetPBit(count, 1);
	}
}
static void PutPrimeTo13(void)
{
	uint primes[] = { 3, 5, 7, 11, 13 };
	uint index;

	for (index = 0; index < P13_LEN; index++)
		PBits[index] = 0;

	for (index = 0; index < lengthof(primes); index++)
		PutP13(primes[index]);

	for (index = P13_LEN; index < PBIT_LEN; index++)
		PBits[index] = PBits[index - P13_LEN];
}
static void PutPrime(uint prime)
{
	uint64 count = BaseNumb % prime;

	count = prime - count;
//	count %= prime; // count == prime はありえる気がするけど、+= prime; があるので意味ない。

	if (count % 2 == 0)
		count += prime;

	/*
		prime^2 が BaseNumb を超える範囲は sqrt(BaseNum + PBIT_P_NUM) - sqrt(BaseNumb)
		-> 狭いので prime^2 から始める必要無さそう？

		BaseNumb   sqrt(BaseNum + PBIT_P_NUM) - sqrt(BaseNumb)
		------------------------------------------------------
		42億       25392.882
		50億       23820.228
		100億      18051.227
		150億      15133.987
		200億      13291.578
	*/

	for (; count < PBIT_P_NUM; count += (uint64)prime * 2)
	{
		SetPBit(count, 1);
	}
}
static void PutPrimeFrom17(void)
{
	uint64 maxNumb;
	uint maxPrime;
	uint64 prime;

	cout("BaseNumb: %I64u\n", BaseNumb);

	if (BaseNumb == (UINT64MAX / PBIT_P_NUM) * PBIT_P_NUM) // ? 最後の範囲
		maxNumb = UINT64MAX;
	else
		maxNumb = BaseNumb + PBIT_P_NUM - 1;

	cout("maxNumb: %I64u\n", maxNumb);
	maxPrime = iSqrt64(maxNumb);

	if (maxPrime == UINTMAX) // UINTMAX is not prime
		maxPrime = UINTMAX - 1;

	cout("maxPrime: %u\n", maxPrime);

	for (prime = 17; prime <= maxPrime; prime += 2)
		if (IsPrime(prime))
			PutPrime(prime);
}
static void SetRange(uint64 value)
{
	uint index;

	BaseNumb = (value / PBIT_P_NUM) * PBIT_P_NUM;

	if (!PBits)
		PBits = (uint *)memAlloc(PBIT_LEN * sizeof(uint));

	LOGPOS();
	PutPrimeTo13();
	LOGPOS();
	PutPrimeFrom17();
	LOGPOS();
}
int IsPrime_R(uint64 value)
{
	uint index;
	uint bit;

	if (value <= UINTMAX)
		return IsPrime(value);

	if (value % 2 == 0)
		return 0;

	if (BaseNumb != (value / PBIT_P_NUM) * PBIT_P_NUM)
		SetRange(value);

	return !GetPBit(value - BaseNumb);
}
