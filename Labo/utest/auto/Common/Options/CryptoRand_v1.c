#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"
#include "C:\Factory\OpenSource\camellia.h"
#include "C:\Factory\OpenSource\sha512.h"

#define CRAND_B_EXE "C:\\Factory\\Labo\\Tools\\CryptoRand_B_v1.exe"

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
	autoBlock_t *rSeed;
	autoBlock_t *rSeedCa;
	autoBlock_t *rSeedCa2;

	seed    = readBinary("C:\\Factory\\tmp\\CSeed_v1.dat");
	seedCa  = readBinary("C:\\Factory\\tmp\\CSeedCa_v1.dat");
	seedCa2 = readBinary("C:\\Factory\\tmp\\CSeedCa2_v1.dat");

	errorCase(getSize(seed) != 4096);
	errorCase(getSize(seedCa) != 4096);
	errorCase(getSize(seedCa2) != 4096);

	coExecute(CRAND_B_EXE " 1");

	IncrementSeed(seed);
	IncrementSeed(seedCa);
	IncrementSeed(seedCa2);

	rSeed    = readBinary("C:\\Factory\\tmp\\CSeed_v1.dat");
	rSeedCa  = readBinary("C:\\Factory\\tmp\\CSeedCa_v1.dat");
	rSeedCa2 = readBinary("C:\\Factory\\tmp\\CSeedCa2_v1.dat");

	errorCase(!isSameBlock(seed,    rSeed));
	errorCase(!isSameBlock(seedCa,  rSeedCa));
	errorCase(!isSameBlock(seedCa2, rSeedCa2));

	releaseAutoBlock(seed);
	releaseAutoBlock(seedCa);
	releaseAutoBlock(seedCa2);
	releaseAutoBlock(rSeed);
	releaseAutoBlock(rSeedCa);
	releaseAutoBlock(rSeedCa2);
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
static void AddToCr2(autoBlock_t *cr2, autoBlock_t *seed, uint v1, uint v2, uint v_num)
{
	autoBlock_t *text = newBlock();

	ab_addBytes(text, seed);

	if (1 <= v_num)
		addByte(text, v1);

	if (2 <= v_num)
		addByte(text, v2);

	sha512_makeHashBlock(text);
	releaseAutoBlock(text);

	ab_addBlock(cr2, sha512_hash, 64);
}
static void AddToCr2_ca(autoBlock_t *cr2_ca, uint val, uint hiVal, char *seedFile)
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
static void MaskCr2(autoBlock_t *cr2, autoBlock_t *cr2_ca)
{
	uint index;

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
	autoBlock_t *cr2_ca;
	uint val;

	LOGPOS();

#define CR_FILE "C:\\Factory\\tmp\\cr.tmp"

	removeFileIfExist(CR_FILE);
	coExecute(CRAND_B_EXE " 16640 " CR_FILE);
	cr1 = readBinary(CR_FILE);
	removeFile(CR_FILE);

#undef CR_FILE

	cr2 = newBlock();

	// seed の読み込みは、読み込み -> increment -> 書き出し -> 使う
	// なので後から読み込まないとダメ
	seed = readBinary("C:\\Factory\\tmp\\CSeed_v1.dat");

	AddToCr2(cr2, seed, 0x00, 0x00, 0);

	for (val = 0x00; val <= 0xff; val++)
	{
		AddToCr2(cr2, seed, val, 0x00, 1);
	}
	AddToCr2(cr2, seed, 0x00, 0x00, 2);
	AddToCr2(cr2, seed, 0x01, 0x00, 2);
	AddToCr2(cr2, seed, 0x02, 0x00, 2);

	cr2_ca  = newBlock();

	for (val = 0x00; getSize(cr2_ca) < getSize(cr2); val++)
	{
		AddToCr2_ca(cr2_ca, val, 0x00, "C:\\Factory\\tmp\\CSeedCa_v1.dat");
	}
	MaskCr2(cr2, cr2_ca);

	// ca2 >

	setSize(cr2_ca, 0);

	for (val = 0x00; getSize(cr2_ca) < getSize(cr2); val++)
	{
		AddToCr2_ca(cr2_ca, val, 0x80, "C:\\Factory\\tmp\\CSeedCa2_v1.dat");
	}
	MaskCr2(cr2, cr2_ca);

	// < ca2

//writeBinary("1.bin", cr1); // test
//writeBinary("2.bin", cr2); // test

	errorCase(!isSameBlock(cr1, cr2));

	releaseAutoBlock(cr1);
	releaseAutoBlock(cr2);
	releaseAutoBlock(cr2_ca);

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
