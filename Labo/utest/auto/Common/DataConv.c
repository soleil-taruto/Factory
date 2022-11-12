#include "C:\Factory\Common\all.h"
#include "C:\Factory\DevTools\libs\RandData.h"

#define IsBase64Char(chr) \
	( \
		'A' <= (chr) && (chr) <= 'Z' || \
		'a' <= (chr) && (chr) <= 'z' || \
		'0' <= (chr) && (chr) <= '9' || \
		(chr) == '+' || \
		(chr) == '/' || \
		(chr) == '=' \
	)

static void Test_encodeBase64_decodeBase64(void)
{
	uint testcnt;

	for (testcnt = 0; testcnt < 1000; testcnt++)
	{
		autoBlock_t *src = MakeRandBinaryBlock(mt19937_rnd(100000));
		autoBlock_t *enc;
		autoBlock_t *dec;
		uint index;

		cout("src: %u\n", getSize(src));
		enc = encodeBase64(src);
		cout("enc: %u\n", getSize(enc));

		for (index = 0; index < getSize(enc); index++)
			errorCase(!IsBase64Char(getByte(enc, index)));

		dec = decodeBase64(enc);
		cout("dec: %u\n", getSize(dec));

		errorCase(!isSameBlock(src, dec));

		releaseAutoBlock(src);
		releaseAutoBlock(enc);
		releaseAutoBlock(dec);
	}
	cout("OK\n");
}

/*
	https://tools.ietf.org/html/rfc4648
*/
static char *TEST_VECTOR[] =
{
	"", "",
	"f", "Zg==",
	"fo", "Zm8=",
	"foo", "Zm9v",
	"foob", "Zm9vYg==",
	"fooba", "Zm9vYmE=",
	"foobar", "Zm9vYmFy",
};

static void Test_encodeBase64_decodeBase64_02(void)
{
	uint index;

	LOGPOS();

	for (index = 0; index < lengthof(TEST_VECTOR); index += 2)
	{
		char *plain   = TEST_VECTOR[index + 0];
		char *encoded = TEST_VECTOR[index + 1];
		autoBlock_t *enc;
		autoBlock_t *dec;
		autoBlock_t gab;

		enc = encodeBase64(gndBlockLineVar(plain, gab));
		dec = decodeBase64(gndBlockLineVar(encoded, gab));

		errorCase(!isSameBlock(enc, gndBlockLineVar(encoded, gab)));
		errorCase(!isSameBlock(dec, gndBlockLineVar(plain, gab)));
	}
	LOGPOS();
	cout("OK\n");
}

int main(int argc, char **argv)
{
	Test_encodeBase64_decodeBase64();
	Test_encodeBase64_decodeBase64_02();
}
