#include "aes128.h"

AES128_KeyTable_t *AES128_CreateKeyTable(autoBlock_t *rawKey, uint doEncrypt)
{
	AES128_KeyTable_t *i = (AES128_KeyTable_t *)memAlloc(sizeof(AES128_KeyTable_t));

	errorCase(!rawKey);
	errorCase(getSize(rawKey) != 16);

	(doEncrypt ? aes128_encrypt_init : aes128_decrypt_init)(i->Ctx, directGetBuffer(rawKey));
	i->DoEncrypt = doEncrypt;

	return i;
}
void AES128_ReleaseKeyTable(AES128_KeyTable_t *i)
{
	memFree(i);
}
void AES128_Encrypt(AES128_KeyTable_t *i, void *in, void *out) // in == out ‰Â
{
	(i->DoEncrypt ? aes128_encrypt : aes128_decrypt)(i->Ctx, (uchar *)in, (uchar *)out);
}
void AES128_EncryptBlock(AES128_KeyTable_t *i, autoBlock_t *in, autoBlock_t *out) // in == out ‰Â
{
	errorCase(getSize(in) != 16);
	errorCase(getSize(out) != 16);

	AES128_Encrypt(i, directGetBuffer(in), directGetBuffer(out));
}
