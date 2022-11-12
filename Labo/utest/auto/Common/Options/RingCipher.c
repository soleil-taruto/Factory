#include "C:\Factory\Common\all.h"
#include "C:\Factory\DevTools\libs\RandData.h"
#include "C:\Factory\Common\Options\RingCipher.h"

static void Test_rcphrEncryptor(void)
{
	uint testcnt;

	for (testcnt = 0; testcnt < 1000; testcnt++)
	{
		uint kw;
		uint ks;
		autoBlock_t *kb;
		autoList_t *ktList;
		autoBlock_t *block;
		autoBlock_t *encBlock;
		autoBlock_t *decBlock;
		int doEncrypt;

		kw = mt19937_range(2, 4) * 8;
		ks = mt19937_range(1, 10) * kw;
		kb = MakeRandBinaryBlock(ks);
		block = MakeRandBinaryBlock((mt19937_range(32, 1000) / 16) * 16);
		doEncrypt = mt19937_rnd(2);

		cout("rcphr: %u\n", testcnt);
		cout("kw: %u\n", kw);
		cout("ks: %u\n", ks);
		cout("block: %u bytes\n", getSize(block));
		cout("doEncrypt: %u\n", doEncrypt);

		ktList = cphrCreateKeyTableList(kb, kw);

		encBlock = copyAutoBlock(block);
		rcphrEncryptor(encBlock, ktList, doEncrypt);

		decBlock = copyAutoBlock(encBlock);
		rcphrEncryptor(decBlock, ktList, !doEncrypt);

		errorCase( isSameBlock(block, encBlock)); // ˆê’v‚·‚éŠm—¦‚Í–³Ž‹‚·‚éB
		errorCase(!isSameBlock(block, decBlock));

		cout("OK\n");

		releaseAutoBlock(kb);
		cphrReleaseKeyTableList(ktList);
		releaseAutoBlock(block);
		releaseAutoBlock(encBlock);
		releaseAutoBlock(decBlock);
	}
}
static void Test_rcphrEncryptorBlock_rcphrDecryptorBlock(void)
{
	uint testcnt;

	for (testcnt = 0; testcnt < 1000; testcnt++)
	{
		uint kw;
		uint ks;
		autoBlock_t *kb;
		autoList_t *ktList;
		autoBlock_t *block;
		autoBlock_t *encBlock;
		autoBlock_t *decBlock;
		uint64 counter2[2];
		uint64 decCounter2[2];
		int retval;

		kw = mt19937_range(2, 4) * 8;
		ks = mt19937_range(1, 10) * kw;
		kb = MakeRandBinaryBlock(ks);
		block = MakeRandBinaryBlock(mt19937_rnd(1000));
		counter2[0] = (uint64)mt19937_rnd32() << 32 | mt19937_rnd32();
		counter2[1] = (uint64)mt19937_rnd32() << 32 | mt19937_rnd32();

		cout("rcphrBlock: %u\n", testcnt);
		cout("kw: %u\n", kw);
		cout("ks: %u\n", ks);
		cout("block: %u bytes\n", getSize(block));
		cout("counter2[0]: %I64u\n", counter2[0]);
		cout("counter2[1]: %I64u\n", counter2[1]);

		ktList = cphrCreateKeyTableList(kb, kw);

		encBlock = copyAutoBlock(block);
		rcphrEncryptorBlock(encBlock, ktList, counter2);

		decBlock = copyAutoBlock(encBlock);
		retval = rcphrDecryptorBlock(decBlock, ktList, decCounter2);

		errorCase(!retval);
		errorCase(!isSameBlock(block, decBlock));
		errorCase(counter2[0] != decCounter2[0]);
		errorCase(counter2[1] != decCounter2[1]);

		cout("OK\n");

		releaseAutoBlock(kb);
		cphrReleaseKeyTableList(ktList);
		releaseAutoBlock(block);
		releaseAutoBlock(encBlock);
		releaseAutoBlock(decBlock);
	}
}
int main(int argc, char **argv)
{
	mt19937_init();

	Test_rcphrEncryptor();
	Test_rcphrEncryptorBlock_rcphrDecryptorBlock();
}
