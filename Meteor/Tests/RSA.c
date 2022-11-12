#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\Random.h"
#include "C:\Factory\DevTools\libs\RandData.h"
#include "..\RSA.h"

static void ShowBlock(autoBlock_t *block, char *title)
{
	cout("%s: %s\n", title, block ? c_makeHexLine(block) : "<NULL>");
}
static autoBlock_t *DestroyBit(autoBlock_t *block)
{
	uint index;
	uint bit;

	errorCase(!getSize(block));

	block = copyAutoBlock(block);

	index = mt19937_rnd(getSize(block));
	bit   = mt19937_rnd(8);

	b_(block)[index] ^= 1 << bit;

	return block;
}
static void SetAllByte(autoBlock_t *block, int chr)
{
	memset(directGetBuffer(block), chr, getSize(block));
}

static void Test_01(void)
{
	autoBlock_t *plainData = MakeRandBinaryBlock(mt19937_rnd(RSA_PlainSizeMax) + 1);
	autoBlock_t *publicKey;
	autoBlock_t *encryptData;
	autoBlock_t *decryptData;
	uint64 t1;
	uint64 t2;
	uint64 t3;
	uint64 t4;

	ShowBlock(plainData, "PLAIN");

	t1 = nowTick();
	RSA_GenerateKey();
	t2 = nowTick();
	publicKey = RSA_GetPublicKey();
	encryptData = RSA_Encrypt(plainData, publicKey); // NULL ‚Í•Ô‚³‚È‚¢B
	t3 = nowTick();
	decryptData = RSA_Decrypt(encryptData);
	t4 = nowTick();

	cout("GEN_KEY_TIME: %I64u\n", t2 - t1);
	cout("ENCRYPT_TIME: %I64u\n", t3 - t2);
	cout("DECRYPT_TIME: %I64u\n", t4 - t3);

	ShowBlock(publicKey, "PUBLIC_KEY");
	ShowBlock(encryptData, "ENCRYPT_DATA");
	ShowBlock(decryptData, "DECRYPT_DATA");

	errorCase(!decryptData); // ? •œ†Ž¸”s
	errorCase(!isSameBlock(plainData, decryptData)); // ? •œŒ³‚µ‚½ƒf[ƒ^‚ªŒ³‚Ìƒf[ƒ^‚Æˆá‚¤B

	releaseAutoBlock(plainData);
	releaseAutoBlock(publicKey);
	releaseAutoBlock(encryptData);
	releaseAutoBlock(decryptData);
}
static void Test_02(void)
{
	autoBlock_t *plainData = MakeRandBinaryBlock(mt19937_rnd(RSA_PlainSizeMax) + 1);
	autoBlock_t *publicKey;
	autoBlock_t *encryptData;
	autoBlock_t *decryptData;
	autoBlock_t *badPublicKey;
	autoBlock_t *badEncryptData;
	autoBlock_t *badDecryptData;

	ShowBlock(plainData, "PLAIN");

	RSA_GenerateKey();
	publicKey = RSA_GetPublicKey();
	badPublicKey = DestroyBit(publicKey);

	ShowBlock(publicKey, "PUBLIC_KEY");
	ShowBlock(badPublicKey, "BAD_PUBLIC_KEY");

	// ---- ³íŒn ----

	encryptData = RSA_Encrypt(plainData, publicKey); // NULL ‚Í•Ô‚³‚È‚¢B
	decryptData = RSA_Decrypt(encryptData);

	errorCase(!decryptData); // ? •œ†Ž¸”s
	errorCase(!isSameBlock(plainData, decryptData)); // ? •œŒ³‚µ‚½ƒf[ƒ^‚ªŒ³‚Ìƒf[ƒ^‚Æˆá‚¤B

	// ---- ˆÙíŒnEŒöŠJŒ®”j‘¹ ----

	cout("¡1_ˆÙíŒn_ŒöŠJŒ®”j‘¹\n");

	badEncryptData = RSA_Encrypt(plainData, badPublicKey); // NULL ‚Í•Ô‚³‚È‚¢B
	badDecryptData = RSA_Decrypt(badEncryptData);

	ShowBlock(badEncryptData, "BAD_ENCRYPT_DATA");
	ShowBlock(badDecryptData, "BAD_DECRYPT_DATA");

	if (badDecryptData) // ? •œ†‚Å‚«‚½...
	{
		errorCase(isSameBlock(plainData, badDecryptData)); // ? •œ†¬Œ÷
		releaseAutoBlock(badDecryptData);
	}
	else // ? •œ†Ž¸”s
	{
		// noop
	}
	releaseAutoBlock(badEncryptData);

	// ---- ˆÙíŒnEŒöŠJŒ®”j‘¹ all 0x00 ----

	cout("¡2_ˆÙíŒn_ŒöŠJŒ®”j‘¹ all 0x00\n");

	SetAllByte(badPublicKey, 0x00);

	badEncryptData = RSA_Encrypt(plainData, badPublicKey); // NULL ‚Í•Ô‚³‚È‚¢B
	badDecryptData = RSA_Decrypt(badEncryptData);

	ShowBlock(badPublicKey, "BAD_PUBLIC_KEY");
	ShowBlock(badEncryptData, "BAD_ENCRYPT_DATA");
	ShowBlock(badDecryptData, "BAD_DECRYPT_DATA");

	if (badDecryptData) // ? •œ†‚Å‚«‚½...
	{
		errorCase(isSameBlock(plainData, badDecryptData)); // ? •œ†¬Œ÷
		releaseAutoBlock(badDecryptData);
	}
	else // ? •œ†Ž¸”s
	{
		// noop
	}
	releaseAutoBlock(badEncryptData);

	// ---- ˆÙíŒnEŒöŠJŒ®”j‘¹ all 0xff ----

	cout("¡3_ˆÙíŒn_ŒöŠJŒ®”j‘¹ all 0xff\n");

	SetAllByte(badPublicKey, 0xff);

	badEncryptData = RSA_Encrypt(plainData, badPublicKey); // NULL ‚Í•Ô‚³‚È‚¢B
	badDecryptData = RSA_Decrypt(badEncryptData);

	ShowBlock(badPublicKey, "BAD_PUBLIC_KEY");
	ShowBlock(badEncryptData, "BAD_ENCRYPT_DATA");
	ShowBlock(badDecryptData, "BAD_DECRYPT_DATA");

	if (badDecryptData) // ? •œ†‚Å‚«‚½...
	{
		errorCase(isSameBlock(plainData, badDecryptData)); // ? •œ†¬Œ÷
		releaseAutoBlock(badDecryptData);
	}
	else // ? •œ†Ž¸”s
	{
		// noop
	}
	releaseAutoBlock(badEncryptData);

	// ---- ˆÙíŒnEˆÃ†ƒf[ƒ^”j‘¹ ----

	cout("¡4_ˆÙíŒn_ˆÃ†ƒf[ƒ^”j‘¹\n");

	badEncryptData = DestroyBit(encryptData);
	badDecryptData = RSA_Decrypt(badEncryptData);

	ShowBlock(badEncryptData, "BAD_ENCRYPT_DATA");
	ShowBlock(badDecryptData, "BAD_DECRYPT_DATA");

	if (badDecryptData) // ? •œ†‚Å‚«‚½...
	{
		errorCase(isSameBlock(plainData, badDecryptData)); // ? •œ†¬Œ÷
		releaseAutoBlock(badDecryptData);
	}
	else // ? •œ†Ž¸”s
	{
		// noop
	}
	releaseAutoBlock(badEncryptData);

	// ----

	releaseAutoBlock(plainData);
	releaseAutoBlock(publicKey);
	releaseAutoBlock(encryptData);
	releaseAutoBlock(decryptData);
	releaseAutoBlock(badPublicKey);
}
static void Test_03(void)
{
	autoBlock_t *plainData = MakeRandBinaryBlock(mt19937_rnd(RSA_PlainSizeMax) + 1);
	autoBlock_t *publicKey;
	autoBlock_t *encryptData;
	autoBlock_t *decryptData;
	autoBlock_t *anotherPublicKey;
	autoBlock_t *anotherEncryptData;
	autoBlock_t *anotherDecryptData;

	ShowBlock(plainData, "PLAIN");

	RSA_GenerateKey();
	anotherPublicKey = RSA_GetPublicKey();

	RSA_GenerateKey();
	publicKey = RSA_GetPublicKey();

	ShowBlock(publicKey, "PUBLIC_KEY");
	ShowBlock(anotherPublicKey, "ANOTHER_PUBLIC_KEY");

	// ---- ³íŒn ----

	encryptData = RSA_Encrypt(plainData, publicKey); // NULL ‚Í•Ô‚³‚È‚¢B
	decryptData = RSA_Decrypt(encryptData);

	errorCase(!decryptData); // ? •œ†Ž¸”s
	errorCase(!isSameBlock(plainData, decryptData)); // ? •œŒ³‚µ‚½ƒf[ƒ^‚ªŒ³‚Ìƒf[ƒ^‚Æˆá‚¤B

	// ---- ˆÙíŒnE•Ê‚ÌŒöŠJŒ® ----

	cout("¡5_ˆÙíŒn_•Ê‚ÌŒöŠJŒ®\n");

	anotherEncryptData = RSA_Encrypt(plainData, anotherPublicKey); // NULL ‚Í•Ô‚³‚È‚¢B
	anotherDecryptData = RSA_Decrypt(anotherEncryptData);

	ShowBlock(anotherEncryptData, "ANOTHER_ENCRYPT_DATA");
	ShowBlock(anotherDecryptData, "ANOTHER_DECRYPT_DATA");

	if (anotherDecryptData) // ? •œ†‚Å‚«‚½...
	{
		errorCase(isSameBlock(plainData, anotherDecryptData)); // ? •œ†¬Œ÷
		releaseAutoBlock(anotherDecryptData);
	}
	else // ? •œ†Ž¸”s
	{
		// noop
	}
	releaseAutoBlock(anotherEncryptData);

	// ----

	releaseAutoBlock(plainData);
	releaseAutoBlock(publicKey);
	releaseAutoBlock(encryptData);
	releaseAutoBlock(decryptData);
	releaseAutoBlock(anotherPublicKey);
}

int main(int argc, char **argv)
{
	uint test_c = 0;

	hasArgs(0); // touch

	while (!hasKey() || getKey() != 0x1b)
	{
		cout("[%u]\n", test_c);

		Test_01();
		Test_02();
		Test_03();

		test_c++;
	}

	termination(0);
}
