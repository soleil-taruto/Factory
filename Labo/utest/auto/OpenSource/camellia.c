#include "C:\Factory\Common\all.h"
#include "C:\Factory\DevTools\libs\RandData.h"
#include "C:\Factory\OpenSource\camellia.h"

static void Test_camellia_cbc(void)
{
	uint testcnt;

	for (testcnt = 0; testcnt < 1000; testcnt++)
	{
		uint size = 16 + 16 * mt19937_rnd(100);
		uint keySize = 8 * mt19937_range(2, 4);
		autoBlock_t *iv;
		autoBlock_t *in;
		autoBlock_t *out;
		autoBlock_t *rout;
		autoBlock_t *rawKey;
		camellia_keyTable_t *key;
		int doEncrypt = mt19937_rnd(2);

		cout("cbc: %u\n", testcnt);
		cout("size: %u\n", size);
		cout("keySize: %u\n", keySize);
		cout("doEncrypt: %u\n", doEncrypt);

		iv = MakeRandBinaryBlock(16);
		in = MakeRandBinaryBlock(size);
		out = nobCreateBlock(size);
		rout = nobCreateBlock(size);
		rawKey = MakeRandBinaryBlock(keySize);
		key = camellia_createKeyTable(rawKey);

		camellia_cbc(key, iv, in, out, doEncrypt);
		camellia_cbc(key, iv, out, rout, !doEncrypt);

		errorCase(isSameBlock(in, out)); // 1 / (2 ^ (8 * size)) ‚ÌŠm—¦‚Åˆê’v
		errorCase(!isSameBlock(in, rout));

		cout("OK\n");

		releaseAutoBlock(iv);
		releaseAutoBlock(in);
		releaseAutoBlock(out);
		releaseAutoBlock(rout);
		releaseAutoBlock(rawKey);
		camellia_releaseKeyTable(key);
	}
}
static void Test_camellia_cbcRing(void)
{
	uint testcnt;

	for (testcnt = 0; testcnt < 1000; testcnt++)
	{
		uint size = 32 + 16 * mt19937_rnd(100);
		uint keySize = 8 * mt19937_range(2, 4);
		autoBlock_t *in;
		autoBlock_t *out;
		autoBlock_t *rout;
		autoBlock_t *rawKey;
		camellia_keyTable_t *key;
		int doEncrypt = mt19937_rnd(2);

		cout("cbcRing: %u\n", testcnt);
		cout("size: %u\n", size);
		cout("keySize: %u\n", keySize);
		cout("doEncrypt: %u\n", doEncrypt);

		in = MakeRandBinaryBlock(size);
		out = nobCreateBlock(size);
		rout = nobCreateBlock(size);
		rawKey = MakeRandBinaryBlock(keySize);
		key = camellia_createKeyTable(rawKey);

		camellia_cbcRing(key, in, out, doEncrypt);
		camellia_cbcRing(key, out, rout, !doEncrypt);

		errorCase(isSameBlock(in, out)); // 1 / (2 p (8 * size)) ‚ÌŠm—¦‚Åˆê’vH
		errorCase(!isSameBlock(in, rout));

		cout("OK\n");

		releaseAutoBlock(in);
		releaseAutoBlock(out);
		releaseAutoBlock(rout);
		releaseAutoBlock(rawKey);
		camellia_releaseKeyTable(key);
	}
}
int main(int argc, char **argv)
{
	mt19937_init();

	Test_camellia_cbc();
	Test_camellia_cbcRing();
}
