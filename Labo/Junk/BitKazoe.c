#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CRRandom.h"

static uint BitKazoe_Test(uint64 value)
{
	uint64 bit;
	uint count = 0;

	for (bit = 1ui64 << 63; bit != 0; bit /= 2)
		if ((value & bit) != 0)
			count++;

	return count;
}
static uint BitKazoe(uint64 value)
{
	uint count = 0;

	while (value != 0)
	{
		value ^= value & (~value + 1);
		count++;
	}
	return count;
}
static uint BitKazoe_2(uint64 value)
{
#if 1
	value = ((value & 0xaaaaaaaaaaaaaaaaui64) >>  1) + (value & 0x5555555555555555ui64);
	value = ((value & 0xccccccccccccccccui64) >>  2) + (value & 0x3333333333333333ui64);
	value = ((value & 0xf0f0f0f0f0f0f0f0ui64) >>  4) + (value & 0x0f0f0f0f0f0f0f0fui64);
	value = ((value & 0xff00ff00ff00ff00ui64) >>  8) + value;
	value = ((value & 0xffff0000ffff0000ui64) >> 16) + value;
	value = ((value & 0xffffffff00000000ui64) >> 32) + value;

	return (uint)(value & 0x7f);
#else
	value = ((value & 0xaaaaaaaaaaaaaaaaui64) >>  1) + (value & 0x5555555555555555ui64);
	value = ((value & 0xccccccccccccccccui64) >>  2) + (value & 0x3333333333333333ui64);
	value = ((value & 0xf0f0f0f0f0f0f0f0ui64) >>  4) + (value & 0x0f0f0f0f0f0f0f0fui64);
	value = ((value & 0xff00ff00ff00ff00ui64) >>  8) + (value & 0x00ff00ff00ff00ffui64);
	value = ((value & 0xffff0000ffff0000ui64) >> 16) + (value & 0x0000ffff0000ffffui64);
	value = ((value & 0xffffffff00000000ui64) >> 32) + (value & 0x00000000ffffffffui64);

	return (uint)value;
#endif
}

// ---- map ----

static uchar *Map;

static void MakeMap(void)
{
	uint index;

	Map = memAlloc(256);

	for (index = 0; index < 256; index++)
	{
		uint count = 0;
		uint bit;

		for (bit = 1 << 7; bit; bit /= 2)
			if (index & bit)
				count++;

		Map[index] = count;
	}
}
static uint BitKazoe_Map(uint64 value)
{
	return
		Map[((uchar *)&value)[0]] +
		Map[((uchar *)&value)[1]] +
		Map[((uchar *)&value)[2]] +
		Map[((uchar *)&value)[3]] +
		Map[((uchar *)&value)[4]] +
		Map[((uchar *)&value)[5]] +
		Map[((uchar *)&value)[6]] +
		Map[((uchar *)&value)[7]];
}

// ---- map16 ----

static uchar *Map16;

static void MakeMap16(void)
{
	uint index;

	Map16 = memAlloc(65536);

	for (index = 0; index < 65536; index++)
	{
		uint count = 0;
		uint bit;

		for (bit = 1 << 15; bit; bit /= 2)
			if (index & bit)
				count++;

		Map16[index] = count;
	}
}
static uint BitKazoe_Map16(uint64 value)
{
	return
		Map16[((uint16 *)&value)[0]] +
		Map16[((uint16 *)&value)[1]] +
		Map16[((uint16 *)&value)[2]] +
		Map16[((uint16 *)&value)[3]];
}

// ----

static void TestValue(uint64 value)
{
	uint n1;
	uint n2;
	uint n3;
	uint n4;
	uint n5;

	n1 = BitKazoe_Test(value);
	n2 = BitKazoe(value);
	n3 = BitKazoe_2(value);
	n4 = BitKazoe_Map(value);
	n5 = BitKazoe_Map16(value);

	cout("%016I64x %2u %2u %2u %2u %2u\n", value, n1, n2, n3, n4, n5);

	errorCase(n1 != n2);
	errorCase(n1 != n3);
	errorCase(n1 != n4);
	errorCase(n1 != n5);
}
static uint64 NextValue(void)
{
#if 0
	return mt19937_rnd64();
#else
	return getCryptoRand64();
#endif
}
static void Test01(void)
{
	TestValue(0);
	TestValue(1);
	TestValue(0x8000000000000000ui64); // 1000...0000
	TestValue(0x5555555555555555ui64); // 0101...0101
	TestValue(0xaaaaaaaaaaaaaaaaui64); // 1010...1010
	TestValue(0x3333333333333333ui64); // 0011...0011
	TestValue(0xccccccccccccccccui64); // 1100...1100
	TestValue(0x0f0f0f0f0f0f0f0fui64); // 00001111...00001111
	TestValue(0xf0f0f0f0f0f0f0f0ui64); // 11110000...11110000
	TestValue(0xffffffffffffffffui64); // 1111...1111

	while (!waitKey(0))
	{
		TestValue(NextValue());
		TestValue(NextValue() & NextValue());
		TestValue(NextValue() | NextValue());
		TestValue(NextValue() & NextValue() & NextValue());
		TestValue(NextValue() | NextValue() | NextValue());
		TestValue(NextValue() & NextValue() & NextValue() & NextValue());
		TestValue(NextValue() | NextValue() | NextValue() | NextValue());
	}
}
static void Test02(void) // speed
{
	uint total = 0x7f;
	uint c;

	LOGPOS();

	for (c = 4100000000; c; c -= 41)
	{
		uint64 value = (uint64)c << 32 | c;

		/*
			@ 2016.3.6
			                sec
			----------------------
			BitKazoe_Test   13.931
			BitKazoe         5.538
			BitKazoe_2       0.718
			BitKazoe_Map     0.515
			BitKazoe_Map16   0.172
		*/

//		total |= BitKazoe_Test(value);
//		total |= BitKazoe(value);
		total |= BitKazoe_2(value);
//		total |= BitKazoe_Map(value);
//		total |= BitKazoe_Map16(value);
	}
	LOGPOS();
	cout("%u\n", total); // Å“K‰»‚ÅÁ‚³‚ê‚È‚¢‚æ‚¤‚ÉA•›ì—p
}
int main(int argc, char **argv)
{
	MakeMap();
	MakeMap16();

	mt19937_initCRnd();

	Test01();
//	Test02();
}
