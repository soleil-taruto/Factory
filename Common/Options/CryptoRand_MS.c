#include "CryptoRand_MS.h"

int createKeyContainer(void) // ret: ? 成功
{
	HCRYPTPROV hp;

	if (!CryptAcquireContext(&hp, 0, 0, PROV_RSA_FULL, CRYPT_NEWKEYSET)) // ? キーコンテナ作成失敗
	{
		cout("Failed to create key container. Last error: %08x\n", GetLastError());
		return 0;
	}
	CryptReleaseContext(hp, 0);
	return 1;
}
int deleteKeyContainer(void) // ret: ? 成功
{
	HCRYPTPROV hp;

	if (!CryptAcquireContext(&hp, 0, 0, PROV_RSA_FULL, CRYPT_DELETEKEYSET)) // ? キーコンテナ削除失敗
	{
		cout("Failed to delete key container. Last error: %08x\n", GetLastError());
		return 0;
	}
	// hp 閉じなくて良いらしい。まぁ削除だし..
	return 1;
}
void getCryptoBlock_MS(uchar *buffer, uint size)
{
	HCRYPTPROV hp;

//	cout("Read sequence of %u bytes from 'CryptGenRandom' function.\n", size);

#if 1
	if (!CryptAcquireContext(&hp, 0, 0, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT))
	{
		cout("Failed to open CSP. Last error: %08x\n", GetLastError());
		error();
	}
#else
	if (!CryptAcquireContext(&hp, 0, 0, PROV_RSA_FULL, 0) &&
		(GetLastError() != NTE_BAD_KEYSET ||
			(cout("Create key container.\n"),
			!CryptAcquireContext(&hp, 0, 0, PROV_RSA_FULL, CRYPT_NEWKEYSET))))
	{
		cout("Failed to create/open key container. Last error: %08x\n", GetLastError());
		error();
	}
#endif

	if (!CryptGenRandom(hp, size, buffer))
	{
		CryptReleaseContext(hp, 0);
		error();
	}
	CryptReleaseContext(hp, 0);
}
autoBlock_t *makeCryptoBlock_MS(uint count)
{
	autoBlock_t *block = nobCreateBlock(count);

	getCryptoBlock_MS(directGetBuffer(block), count);
	return block;
}
