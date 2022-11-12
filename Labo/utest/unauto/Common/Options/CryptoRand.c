#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

static void Test_01(void)
{
	uint c;

	for (c = 0; c < 1000; c++)
	{
		cout("%08x %08x %08x %016I64x\n"
			,getCryptoRand16()
			,getCryptoRand24()
			,getCryptoRand()
			,getCryptoRand64()
			);
	}
}
static void Test_02(void)
{
	uint c;

	for (c = 0; c < 1000; c++)
	{
		cout("%I64u %I64u %I64u %I64u\n"
			,getCryptoRand64Mod(1)
			,getCryptoRand64Mod(2)
			,getCryptoRand64Mod(3)
			,getCryptoRand64Mod(4)
			);
	}
}
static void Test_03(void)
{
	uint c;

	for (c = 0; c < 1000; c++)
	{
		cout("%I64u %I64u %I64u %I64u\n"
			,getCryptoRand64Range(10, 10)
			,getCryptoRand64Range(10, 11)
			,getCryptoRand64Range(10, 12)
			,getCryptoRand64Range(10, 13)
			);
	}
}
int main(int argc, char **argv)
{
//	Test_01();
//	Test_02();
	Test_03();
}
