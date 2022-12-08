#include "Aes.h"

// ---- Counter ----

autoBlock_t *MakeEncCounterSeed(void)
{
	return makeCryptoRandBlock(16);
}
autoBlock_t *MakeEncCounter(autoBlock_t *clSeed, autoBlock_t *svSeed)
{
	AES128_KeyTable_t *keyTable = AES128_CreateKeyTable(GetConcreteRawKey(), 1);
	autoBlock_t *encCounter = nobCreateBlock(16);
	uint index;
	char *tmp;

	// clSeed
	errorCase(getSize(svSeed) != 16);

	cout("clSeed: %s\n", tmp = makeHexLine(clSeed)); memFree(tmp);
	cout("svSeed: %s\n", tmp = makeHexLine(svSeed)); memFree(tmp);
	cout("rawKey: %s\n", tmp = makeHexLine(GetConcreteRawKey())); memFree(tmp);

	AES128_EncryptBlock(keyTable, clSeed, encCounter);

	for (index = 0; index < 16; index++)
		b_(encCounter)[index] ^= b_(svSeed)[index];

	AES128_EncryptBlock(keyTable, encCounter, encCounter);
	AES128_ReleaseKeyTable(keyTable);

	if (encCounter)
	{
		cout("encCounter: %s\n", tmp = makeHexLine(encCounter));
		memFree(tmp);
	}
	else
	{
		cout("encCounter: <NULL>\n");
	}
	return encCounter;
}

// ---- Crypt ----

static void AddCounter(autoBlock_t *counter)
{
	uint index;

	for (index = 0; index < 16; index++)
	{
		if (b_(counter)[index] < 0xff)
		{
			b_(counter)[index]++;
			break;
		}
		b_(counter)[index] = 0x00;
	}
}
void Crypt(autoBlock_t *data, autoBlock_t *rawKey, autoBlock_t *encCounter)
{
	AES128_KeyTable_t *keyTable = AES128_CreateKeyTable(rawKey, 1);
	autoBlock_t *mask = nobCreateBlock(16);
	uint index;

	for (index = 0; index < getSize(data); index++)
	{
		uint subPos = index % 16;

		if (!subPos)
		{
			AES128_EncryptBlock(keyTable, encCounter, mask);
			AddCounter(encCounter);
		}
		b_(data)[index] ^= b_(mask)[subPos];
	}
	AES128_ReleaseKeyTable(keyTable);
	releaseAutoBlock(mask);
}

// ---- Hash ----

static autoBlock_t *MakeHash(autoBlock_t *data)
{
	sha512_makeHashBlock(data);
	return recreateBlock(sha512_hash, 16);
}
void PutHash(autoBlock_t *data)
{
	autoBlock_t *hash = MakeHash(data);

	addBytes(data, hash);
	releaseAutoBlock(hash);
}
int UnputHash(autoBlock_t *data) // ret: ? 正常なデータ
{
	autoBlock_t *hash1;
	autoBlock_t *hash2;
	int retval;

	if (getSize(data) < 16)
		return 0;

	hash1 = getSubBytes(data, getSize(data) - 16, 16);
	setSize(data, getSize(data) - 16);
	hash2 = MakeHash(data);

	retval = isSameBlock(hash1, hash2);

	releaseAutoBlock(hash1);
	releaseAutoBlock(hash2);

	return retval;
}

// ---- RandPart_Padding ---

void Add_RandPart_Padding(autoBlock_t *block)
{
	uint size = ~getSize(block) & 0x0f | 0x10 | getCryptoByte() & 0xe0;
	uint index;

	for (index = 0; index < size; index++)
		addByte(block, getCryptoByte());

	addByte(block, size);
}
int Unadd_RandPart_Padding(autoBlock_t *block) // ret: ? 正常なデータ
{
	uint size;

	if (getSize(block) < 1)
		return 0;

	size = unaddByte(block);

	if (size < 0x10)
		return 0;

	if (getSize(block) < size)
		return 0;

	setSize(block, getSize(block) - size);
	return 1;
}

// ---- Ring_CBC ----

static void XorBlock(void *block, void *mask)
{
	((uint *)block)[0] ^= ((uint *)mask)[0];
	((uint *)block)[1] ^= ((uint *)mask)[1];
	((uint *)block)[2] ^= ((uint *)mask)[2];
	((uint *)block)[3] ^= ((uint *)mask)[3];
}
void RCBCEncrypt(autoBlock_t *block, autoBlock_t *rawKey)
{
	AES128_KeyTable_t *keyTable;
	uint prevPos;
	uint currPos;

	errorCase(!block);
	errorCase(getSize(block) < 32);
	errorCase(getSize(block) % 16 != 0);
	errorCase(!rawKey);
	errorCase(getSize(rawKey) != 16);

	keyTable = AES128_CreateKeyTable(rawKey, 1);
	prevPos = getSize(block) - 16;

	for (currPos = 0; currPos < getSize(block); currPos += 16)
	{
		XorBlock(b_(block) + currPos, b_(block) + prevPos);
		AES128_Encrypt(keyTable, b_(block) + currPos, b_(block) + currPos);
		prevPos = currPos;
	}
	AES128_ReleaseKeyTable(keyTable);
}
int RCBCDecrypt(autoBlock_t *block, autoBlock_t *rawKey) // ret: ? 成功
{
	AES128_KeyTable_t *keyTable;
	uint prevPos;
	uint currPos;

	errorCase(!block);
	errorCase(!rawKey);
	errorCase(getSize(rawKey) != 16);

	if (getSize(block) < 32)
		return 0;

	if (getSize(block) % 16 != 0)
		return 0;

	keyTable = AES128_CreateKeyTable(rawKey, 0);

	for (currPos = getSize(block) - 16; currPos; currPos -= 16)
	{
		prevPos = currPos - 16;
		AES128_Encrypt(keyTable, b_(block) + currPos, b_(block) + currPos);
		XorBlock(b_(block) + currPos, b_(block) + prevPos);
	}
	prevPos = getSize(block) - 16;
	AES128_Encrypt(keyTable, b_(block), b_(block));
	XorBlock(b_(block), b_(block) + prevPos);

	AES128_ReleaseKeyTable(keyTable);
	return 1;
}

// ----
