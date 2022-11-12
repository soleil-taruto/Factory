#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\camellia.h"

static void DispRawKey(uchar rawKey[32])
{
	uint index;

	for (index = 0; index < 32; index++)
	{
		cout("%02x", rawKey[index]);
	}
	cout("\n");
}
static void MKRAWKEY(
	uchar rawKey[32],
	uint x0,
	uint x5,
	uint x10,
	uint x15,
	uint x20,
	uint x25,
	uint x30
	)
{
	camellia_KeyTableType keyTable;

	cout("MKRAWKEY() ST\n");
	DispRawKey(rawKey);

	rawKey[0] ^= x0;
	rawKey[5] ^= x5;
	rawKey[10] ^= x10;
	rawKey[15] ^= x15;
	rawKey[20] ^= x20;
	rawKey[25] ^= x25;
	rawKey[30] ^= x30;

	DispRawKey(rawKey);

	camellia_Ekeygen(128, rawKey + 16, keyTable);
	camellia_EncryptBlock(128, rawKey, keyTable, rawKey);

	DispRawKey(rawKey);

	camellia_Ekeygen(128, rawKey, keyTable);
	camellia_EncryptBlock(128, rawKey + 16, keyTable, rawKey + 16);

	DispRawKey(rawKey);
	cout("MKRAWKEY() ED\n");
}
int main(int argc, char **argv)
{
	uchar rawKey[32];

	memset(rawKey, 0x00, 32);

	MKRAWKEY(rawKey, 0x34, 0x59, 0x65, 0x69, 0xa7, 0x12, 0x3c);
	MKRAWKEY(rawKey, 0x36, 0x00, 0x4e, 0x9f, 0xfe, 0x12, 0xc2);
	MKRAWKEY(rawKey, 0x6c, 0xb9, 0x99, 0x4f, 0x11, 0xbe, 0xce);
	MKRAWKEY(rawKey, 0xa8, 0x9e, 0x75, 0x6c, 0x17, 0xb3, 0x4c);
	MKRAWKEY(rawKey, 0x87, 0x98, 0xd7, 0x79, 0x65, 0xee, 0xda);
	MKRAWKEY(rawKey, 0x34, 0xed, 0xc1, 0x2c, 0xcc, 0x8a, 0xc7);
	MKRAWKEY(rawKey, 0xe0, 0xcf, 0x10, 0x45, 0x89, 0xfd, 0xef);

	DispRawKey(rawKey);
}
