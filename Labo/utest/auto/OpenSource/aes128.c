#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\aes128.h"

static void DoTest(char *key, char *plain, char *cipher)
{
	autoBlock_t *bKey = makeBlockHexLine(key);
	autoBlock_t *bPlain = makeBlockHexLine(plain);
	autoBlock_t *bCipher = makeBlockHexLine(cipher);
	autoBlock_t *ansPlain = nobCreateBlock(16);
	autoBlock_t *ansCipher = nobCreateBlock(16);

	cout("KEY %s\n", c_makeHexLine(bKey));
	cout("PLAIN %s\n", c_makeHexLine(bPlain));
	cout("CIPHER %s\n", c_makeHexLine(bCipher));

	{
		AES128_KeyTable_t *i = AES128_CreateKeyTable(bKey, 1);

		AES128_EncryptBlock(i, bPlain, ansCipher);
		AES128_ReleaseKeyTable(i);
	}
	cout("ENCRYPT %s\n", c_makeHexLine(ansCipher));
	errorCase(!isSameBlock(bCipher, ansCipher));

	{
		AES128_KeyTable_t *i = AES128_CreateKeyTable(bKey, 0);

		AES128_EncryptBlock(i, bCipher, ansPlain);
		AES128_ReleaseKeyTable(i);
	}
	cout("DECRYPT %s\n", c_makeHexLine(ansPlain));
	errorCase(!isSameBlock(bPlain, ansPlain));

	releaseAutoBlock(bKey);
	releaseAutoBlock(bPlain);
	releaseAutoBlock(bCipher);
	releaseAutoBlock(ansPlain);
	releaseAutoBlock(ansCipher);

	cout("OK\n");
}
int main(int argc, char **argv)
{
	// test-vector from: http://www.inconteam.com/software-development/41-encryption/55-aes-test-vectors#aes-ecb-128

	char *key = "2b7e151628aed2a6abf7158809cf4f3c";

	DoTest(key, "6bc1bee22e409f96e93d7e117393172a", "3ad77bb40d7a3660a89ecaf32466ef97");
	DoTest(key, "ae2d8a571e03ac9c9eb76fac45af8e51", "f5d3d58503b9699de785895a96fdbaaf");
	DoTest(key, "30c81c46a35ce411e5fbc1191a0a52ef", "43b1cd7f598ece23881b00e3ed030688");
	DoTest(key, "f69f2445df4f9b17ad2b417be66c3710", "7b0c785e27e8ad3f8223207104725dd4");
}
