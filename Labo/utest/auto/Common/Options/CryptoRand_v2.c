#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand_v2.h"
#include "C:\Factory\OpenSource\camellia.h"
#include "C:\Factory\OpenSource\sha512.h"

#define CRAND_B_EXE "C:\\Factory\\Labo\\Tools\\CryptoRand_B_v2.exe"

static void IncrementSeed(autoBlock_t *seed)
{
	uint val = 1;
	uint index;

	for (index = 0; index < getSize(seed); index++)
	{
		val += getByte(seed, index);
		setByte(seed, index, val & 0xff);
		val >>= 8;
	}
}
static void DoTest_01_2(void)
{
	autoBlock_t *seed;
	autoBlock_t *seedCa;
	autoBlock_t *seedCa2;
	autoBlock_t *seedCa3;
	autoBlock_t *rSeed;
	autoBlock_t *rSeedCa;
	autoBlock_t *rSeedCa2;
	autoBlock_t *rSeedCa3;

	seed    = readBinary("C:\\Factory\\tmp\\CSeed.dat");
	seedCa  = readBinary("C:\\Factory\\tmp\\CSeedCa.dat");
	seedCa2 = readBinary("C:\\Factory\\tmp\\CSeedCa2.dat");
	seedCa3 = readBinary("C:\\Factory\\tmp\\CSeedCa3.dat");

	errorCase(getSize(seed) != 4096);
	errorCase(getSize(seedCa) != 4096);
	errorCase(getSize(seedCa2) != 4096);
	errorCase(getSize(seedCa3) != 4096);

	coExecute(CRAND_B_EXE " 1");

	IncrementSeed(seed);
	IncrementSeed(seedCa);
	IncrementSeed(seedCa2);
	IncrementSeed(seedCa3);

	rSeed    = readBinary("C:\\Factory\\tmp\\CSeed.dat");
	rSeedCa  = readBinary("C:\\Factory\\tmp\\CSeedCa.dat");
	rSeedCa2 = readBinary("C:\\Factory\\tmp\\CSeedCa2.dat");
	rSeedCa3 = readBinary("C:\\Factory\\tmp\\CSeedCa3.dat");

	errorCase(!isSameBlock(seed,    rSeed));
	errorCase(!isSameBlock(seedCa,  rSeedCa));
	errorCase(!isSameBlock(seedCa2, rSeedCa2));
	errorCase(!isSameBlock(seedCa3, rSeedCa3));

	releaseAutoBlock(seed);
	releaseAutoBlock(seedCa);
	releaseAutoBlock(seedCa2);
	releaseAutoBlock(seedCa3);
	releaseAutoBlock(rSeed);
	releaseAutoBlock(rSeedCa);
	releaseAutoBlock(rSeedCa2);
	releaseAutoBlock(rSeedCa3);
}
static void DoTest_01(void)
{
	uint c;

	LOGPOS();

	for (c = 0; c < 550; c++)
	{
		cout("%u\n", c);

		DoTest_01_2();
	}
	LOGPOS();
}
static void AddToCr2_Ca(autoBlock_t *cr2_ca, uint val, uint hiVal, char *seedFile)
{
	autoBlock_t *seed = readBinary(seedFile);
	autoBlock_t *rawKey;
	camellia_keyTable_t *kt;
	uchar buff[16];
	uint index;

	sha512_makeHashBlock(seed);
	rawKey = recreateBlock(sha512_hash, 32);
	kt = camellia_createKeyTable(rawKey);

	for (index = 0; index < 15; index++)
	{
		buff[index] = val & 0xff;
		val >>= 8;
	}
	buff[15] = hiVal;
	camellia_encrypt(kt, buff, buff, 1);
	camellia_releaseKeyTable(kt);
	releaseAutoBlock(rawKey);
	releaseAutoBlock(seed);

	ab_addBlock(cr2_ca, buff, 16);
}
static autoBlock_t *GetCr2_Ca(uint hiVal, char *seedFile)
{
	autoBlock_t *cr2_ca = newBlock();
	uint val;

	for (val = 0; getSize(cr2_ca) < 16640; val++)
	{
		AddToCr2_Ca(cr2_ca, val, hiVal, seedFile);
	}
	return cr2_ca;
}
static void MaskCr2(autoBlock_t *cr2, autoBlock_t *cr2_ca)
{
	uint index;

	errorCase(getSize(cr2) != getSize(cr2_ca));

	for (index = 0; index < getSize(cr2); index++)
	{
		b_(cr2)[index] ^= b_(cr2_ca)[index];
	}
}
static void DoTest_02_2(void)
{
	autoBlock_t *seed;
	autoBlock_t *cr1;
	autoBlock_t *cr2;
	autoBlock_t *cr2_c1;
	autoBlock_t *cr2_c2;
	autoBlock_t *cr2_c3;
	autoBlock_t *cr2_c4;
	uint val;

	LOGPOS();

#define CR_FILE "C:\\Factory\\tmp\\cr.tmp"

	removeFileIfExist(CR_FILE);
	coExecute(CRAND_B_EXE " 16640 " CR_FILE);
	cr1 = readBinary(CR_FILE);
	removeFile(CR_FILE);

#undef CR_FILE

	// seed の読み込みは、読み込み -> increment -> 書き出し -> 使う
	// なので後から読み込まないとダメ

	cr2_c1 = GetCr2_Ca(0x00, "C:\\Factory\\tmp\\CSeed.dat");
	cr2_c2 = GetCr2_Ca(0x40, "C:\\Factory\\tmp\\CSeedCa.dat");
	cr2_c3 = GetCr2_Ca(0x80, "C:\\Factory\\tmp\\CSeedCa2.dat");
	cr2_c4 = GetCr2_Ca(0xc0, "C:\\Factory\\tmp\\CSeedCa3.dat");

	cr2 = createBlock(16640);
	setSize(cr2, 16640);

	MaskCr2(cr2, cr2_c1);
	MaskCr2(cr2, cr2_c2);
	MaskCr2(cr2, cr2_c3);
	MaskCr2(cr2, cr2_c4);

//writeBinary("1.bin", cr1); // test
//writeBinary("2.bin", cr2); // test

	errorCase(!isSameBlock(cr1, cr2));

	releaseAutoBlock(cr1);
	releaseAutoBlock(cr2);
	releaseAutoBlock(cr2_c1);
	releaseAutoBlock(cr2_c2);
	releaseAutoBlock(cr2_c3);
	releaseAutoBlock(cr2_c4);

	LOGPOS();
}
static void DoTest_02(void)
{
	uint c;

	LOGPOS();

	for (c = 0; c < 10; c++)
	{
		cout("%u\n", c);

		DoTest_02_2();
	}
	LOGPOS();
}
static void DoTest(void)
{
	coExecute(CRAND_B_EXE " 1");

	DoTest_01();
	DoTest_02();
}
int main(int argc, char **argv)
{
	DoTest();
}
