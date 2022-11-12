#include "Prime.h"

// ---- PBit ----

#define PBIT_LEN 0x04000000

static uint *PBits; // bit_0 == prime, bit_1 == not prime

static int GetPBit(uint prime)
{
	uint bit;
	uint index;

	if (prime == 2)
		return 0;

	if (prime % 2 == 0)
		return 1;

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

// ---- Save Load ----

static char *GetDatFile(void)
{
	static char *file;

	if (!file)
	{
		if (isFactoryDirEnabled())
			file = "C:\\Factory\\tmp_Prime\\Prime.dat";
		else
			file = combine(getSelfDir(), "Prime.dat");
	}
	return file;
}
static void SavePBits(void)
{
	autoBlock_t gab;

	writeBinary(GetDatFile(), gndBlockVar(PBits, PBIT_LEN * sizeof(uint), gab));

//	coExecute_x(xcout("ATTRIB.EXE +S +H \"%s\"", GetDatFile()));
}
static int LoadPBits(void)
{
	FILE *fp;
	autoBlock_t gab;

	if (!existFile(GetDatFile()))
		return 0;

	errorCase(getFileSize(GetDatFile()) != PBIT_LEN * sizeof(uint));

	fp = fileOpen(GetDatFile(), "rb");
	fileRead(fp, gndBlockVar(PBits, PBIT_LEN * sizeof(uint), gab));
	fileClose(fp);

	return 1;
}

// ---- INIT ----

#define P13_LEN 15015

static void PutPrime(uint prime, uint maxNumb)
{
	uint64 count;

	for (count = prime * prime; count <= maxNumb; count += prime * 2)
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
	{
		PutPrime(primes[index], P13_LEN * 64 - 1);
		SetPBit(primes[index], 1);
	}
	for (index = P13_LEN; index < PBIT_LEN; index++)
		PBits[index] = PBits[index % P13_LEN];

	for (index = 0; index < lengthof(primes); index++)
		SetPBit(primes[index], 0);

	SetPBit(1, 1);
}
static void PutPrimeFrom17(void)
{
	uint prime;

	for (prime = 17; prime <= 0xffff; prime += 2)
		if (!GetPBit(prime))
			PutPrime(prime, UINTMAX);
}
static void DoINIT(void)
{
	PBits = (uint *)memAlloc(PBIT_LEN * sizeof(uint));

//	LOGPOS();

	mutex();
	{
		if (!LoadPBits())
		{
			LOGPOS();
			PutPrimeTo13();
			LOGPOS();
			PutPrimeFrom17();
			LOGPOS();
			SavePBits();
		}
	}
	unmutex();

//	LOGPOS();
}
static void INIT(void)
{
	static int inited;

	if (!inited)
	{
		DoINIT();
		inited = 1;
	}
}

// ----

static int IsPrime_32(uint value)
{
	INIT();
	return !GetPBit(value);
}
int IsPrime(uint64 value)
{
	uint64 denom; // maxDenom is max UINTMAX
	uint maxDenom;

	if (value <= UINTMAX)
		return IsPrime_32((uint)value);

	if (value % 2 == 0)
		return 0;

	maxDenom = iSqrt64(value);

	for (denom = 3; denom <= maxDenom; denom += 2)
		if (IsPrime_32(denom) && value % denom == 0) // IsPrime_32 <- value % denom == 0 より速いぽい。32bitだから???
			return 0;

	return 1;
}
void Factorization(uint64 value, uint64 dest[64]) // dest: 最大 63 個, 最後の要素は 0 で閉じる。1 のときは { 1 }, 0 のときは { } を返す。
{
	uint wPos = 0;

	if (value == 0)
	{
		// noop
	}
	else if (value == 1)
	{
		dest[wPos++] = 1;
	}
	else if (value <= UINTMAX && IsPrime_32((uint)value))
	{
		dest[wPos++] = value;
	}
	else
	{
		uint64 denom; // maxDenom is max UINTMAX -> 18446744073709551557 とか uint だと無限ループになる気がする。
		uint maxDenom;

		while (value % 2 == 0)
		{
			dest[wPos++] = 2;
			value /= 2;

			if (value < 2)
				goto value_one;
		}
		maxDenom = iSqrt64(value);

		for (denom = 3; denom <= maxDenom; denom += 2)
		{
			if (IsPrime_32(denom)) // value % denom == 0 より速いぽい。32bitだから???
			{
				while (value % denom == 0)
				{
					dest[wPos++] = denom;
					value /= denom;

					if (value < 2)
						goto value_one;

					maxDenom = iSqrt64(value);
				}
			}
		}
		dest[wPos++] = value;

	value_one:
		errorCase(value < 1); // 2bs
	}
	dest[wPos] = 0;
}
