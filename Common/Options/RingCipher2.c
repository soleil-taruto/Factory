#include "RingCipher2.h"

/*
	開放 -> cphrReleaseKeyTableList(ret);
*/
autoList_t *rngcphrCreateKeyTableList(autoBlock_t *rawKey)
{
	uint size = getSize(rawKey);
	uint ks;
	uint kn;

	errorCase_m(size < 16, "短すぎる鍵 @ RingCipher2.c");

	if (size == 32)
		return cphrCreateKeyTableList(rawKey, 16);

	if (size % 32 == 0)
		return cphrCreateKeyTableList(rawKey, 32);

	if (size == 24)
	{
		cout("Warning: 鍵が1つしかない。24 bit @ RingCipher2.c\n");
		return cphrCreateKeyTableList(rawKey, 24);
	}
	if (size % 24 == 0)
		return cphrCreateKeyTableList(rawKey, 24);

	if (size == 16)
	{
		cout("Warning: 鍵が1つしかない。16 bit @ RingCipher2.c\n");
		return cphrCreateKeyTableList(rawKey, 16);
	}
	errorCase_m(size % 16 != 0, "鍵の長さに問題があります。@ RingCipher2.c");
	return cphrCreateKeyTableList(rawKey, 16);
}

// ---- Padding ----

static void AddPadding(autoBlock_t *block)
{
	uint size = getSize(block);
	uint padSzLow;
	uint padSize;
	uint count;

	padSzLow = ~size & 0x0f;

	do
	{
		padSize = padSzLow | getCryptoByte() & 0xf0;
	}
	while (size + padSize < 0xff);

	for (count = padSize; count; count--)
	{
		addByte(block, getCryptoByte());
	}
	addByte(block, padSize);
}
static int UnaddPadding(autoBlock_t *block)
{
	if (getSize(block))
	{
		uint padSize = unaddByte(block);

		if (padSize <= getSize(block))
		{
			setSize(block, getSize(block) - padSize);
			return 1;
		}
	}
	return 0;
}

// ---- RandPart ----

#define RANDPARTSIZE 64

static void AddRandPart(autoBlock_t *block)
{
	uint count;

	for (count = RANDPARTSIZE; count; count--)
	{
		addByte(block, getCryptoRand());
	}
}
static int UnaddRandPart(autoBlock_t *block)
{
	if (RANDPARTSIZE <= getSize(block))
	{
		setSize(block, getSize(block) - RANDPARTSIZE);
		return 1;
	}
	return 0;
}

// ---- Hash ----

static void AddHash(autoBlock_t *block)
{
	sha512_makeHashBlock(block);
	ab_addBlock(block, sha512_hash, 64);
}
static int UnaddHash(autoBlock_t *block)
{
	if (64 <= getSize(block))
	{
		autoBlock_t *hash = unaddBytes(block, 64);

		sha512_makeHashBlock(block);

		if (!memcmp(directGetBuffer(hash), sha512_hash, 64))
		{
			releaseAutoBlock(hash);
			return 1;
		}
		releaseAutoBlock(hash);
	}
	return 0;
}

// ---- Encrypt ----

static void CipherMain(autoBlock_t *block, autoList_t *keyTableList, int doEncrypt)
{
	uint index;

	errorCase(!block);
	errorCase(getSize(block) < 32);
	errorCase(getSize(block) % 16);
	errorCase(!keyTableList);
	errorCase(!getCount(keyTableList));

	if (getCount(keyTableList) < 2)
		cout("Warning: 鍵が2つ未満 @ RingCipher2.c\n");

	for (index = 0; index < getCount(keyTableList); index++)
	{
		camellia_cbcRing(
			(camellia_keyTable_t *)getElement(keyTableList, doEncrypt ? index : getCount(keyTableList) - 1 - index),
			block,
			block,
			doEncrypt
			);
	}
}
void rngcphrEncryptBlock(autoBlock_t *block, autoList_t *keyTableList)
{
	CipherMain(block, keyTableList, 1);
}
void rngcphrDecryptBlock(autoBlock_t *block, autoList_t *keyTableList)
{
	CipherMain(block, keyTableList, 0);
}
void rngcphrEncrypt(autoBlock_t *block, autoList_t *keyTableList)
{
	errorCase(!block);
	errorCase(!keyTableList);
	errorCase(!getCount(keyTableList));

	AddPadding(block);
	AddRandPart(block);
	AddHash(block);
	AddRandPart(block);

	rngcphrEncryptBlock(block, keyTableList);
}
int rngcphrDecrypt(autoBlock_t *block, autoList_t *keyTableList)
{
	int retval = 0;

	errorCase(!block);
	errorCase(!keyTableList);
	errorCase(!getCount(keyTableList));

	if (
		getSize(block) < 16 + 64 + 64 + 64 ||
//		getSize(block) < 256 + 64 + 64 + 64 || // while (size + padSize < 0xff); してなかった時の暗号文はこれより短い (最小padding 旧->新: 16->256)
		getSize(block) % 16 != 0
		)
		goto fault;

	rngcphrDecryptBlock(block, keyTableList);

	if (
		!UnaddRandPart(block) ||
		!UnaddHash(block) ||
		!UnaddRandPart(block) ||
		!UnaddPadding(block)
		)
		goto fault;

	retval = 1;

fault:
	return retval;
}

// ---- F_Padding ----

static void F_AddPadding(char *file)
{
	uint64 fileSize = getFileSize(file);
	uint padSzLow;
	uint padSize;
	FILE *fp;
	uint count;

	padSzLow = ~(uint)fileSize & 0x0f;

	do
	{
		padSize = padSzLow | getCryptoByte() & 0xf0;
	}
	while (fileSize + padSize < 0xff);

	fp = fileOpen(file, "ab");

	for (count = padSize; count; count--)
	{
		writeChar(fp, getCryptoByte());
	}
	writeChar(fp, padSize);
	fileClose(fp);
}
static int F_UnaddPadding(char *file)
{
	uint64 fileSize = getFileSize(file);
	FILE *fp;
	uint padSize;

	if (fileSize < 1)
		return 0;

	fp = fileOpen(file, "rb");
	fileSeek(fp, SEEK_END, -1);
	padSize = readChar(fp);
	fileClose(fp);
	padSize++;

	if (fileSize < padSize)
		return 0;

	fileSize -= padSize;
	setFileSize(file, fileSize);
	return 1;
}

// ---- F_RandPart ----

static void F_AddRandPart(char *file)
{
	FILE *fp = fileOpen(file, "ab");
	uint index;

	for (index = 0; index < RANDPARTSIZE; index++)
		writeChar(fp, getCryptoByte());

	fileClose(fp);
}
static int F_UnaddRandPart(char *file)
{
	uint64 fileSize = getFileSize(file);

	if (fileSize < RANDPARTSIZE)
		return 0;

	setFileSize(file, fileSize - RANDPARTSIZE);
	return 1;
}

// ---- F_Hash ----

static void F_AddHash(char *file)
{
	FILE *fp;
	autoBlock_t gab;

	sha512_makeHashFile(file);
	fp = fileOpen(file, "ab");
	fileWrite(fp, gndBlockVar(sha512_hash, 64, gab));
	fileClose(fp);
}
static int F_UnaddHash(char *file)
{
	uint64 fileSize = getFileSize(file);
	FILE *fp;
	uchar r_hash[64];
	autoBlock_t gab;

	if (fileSize < 64)
		return 0;

	fp = fileOpen(file, "rb");
	fileSeek(fp, SEEK_END, -64);
	fileRead(fp, gndBlockVar(r_hash, 64, gab));
	fileClose(fp);
	setFileSize(file, fileSize - 64);
	sha512_makeHashFile(file);

LOGPOS();
	{
	char *tmp;
	cout("fileのハッシュ：%s\n", tmp = makeHexLine(gndBlockVar(r_hash,      64, gab))); memFree(tmp);
	cout("再計算ハッシュ：%s\n", tmp = makeHexLine(gndBlockVar(sha512_hash, 64, gab))); memFree(tmp);
	}

	if (memcmp(sha512_hash, r_hash, 64)) // ? 生成したハッシュと読み込んだハッシュが違う。
	{
LOGPOS();
		return 0;
	}
LOGPOS();
	return 1;
}

// ---- F_Encrypt ----

static void XorBlock(uchar dest[16], uchar mask[16])
{
	((uint *)dest)[0] ^= ((uint *)mask)[0];
	((uint *)dest)[1] ^= ((uint *)mask)[1];
	((uint *)dest)[2] ^= ((uint *)mask)[2];
	((uint *)dest)[3] ^= ((uint *)mask)[3];
}
static void F_EncryptMain(char *file, camellia_keyTable_t *keyTable)
{
	FILE *rfp = fileOpen(file, "rb");
	FILE *wfp = fileOpen(file, "r+b");
	uchar buff[32];
	uchar *prevBlock;
	uchar *currBlock;
	uint64 size;
	uint64 index;
	autoBlock_t gab;

	prevBlock = buff;
	currBlock = buff + 16;

	size = getFileSizeFP(rfp);

	errorCase(size < 32); // 2bs
	errorCase(size % 16 != 0); // 2bs

	fileSeek(rfp, SEEK_END, -16);
	fileRead(rfp, gndBlockVar(prevBlock, 16, gab));
	fileSeek(rfp, SEEK_SET, 0);

	for (index = 0; index < size; index += 16)
	{
		fileRead(rfp, gndBlockVar(currBlock, 16, gab));
		XorBlock(currBlock, prevBlock);
		camellia_encrypt(keyTable, currBlock, currBlock, 1);
		fileWrite(wfp, gndBlockVar(currBlock, 16, gab));

		m_swap(prevBlock, currBlock, uchar *);
	}
	fileClose(rfp);
	fileClose(wfp);
}
static void F_Encrypt(char *file, autoList_t *keyTableList)
{
	camellia_keyTable_t *keyTable;
	uint index;

	foreach (keyTableList, keyTable, index)
	{
		F_EncryptMain(file, keyTable);
	}
}
static void F_DecryptPart(FILE *fp, uint64 fpPos, uint size, camellia_keyTable_t *keyTable)
{
	uchar *buff = memAlloc(size);
	autoBlock_t gab;
	uint index;

	fileSeek(fp, SEEK_SET, fpPos - size);
	fileRead(fp, gndBlockVar(buff, size, gab));

	for (index = size; 32 <= index; index -= 16)
	{
		uchar *prevBlock = buff + index - 32;
		uchar *currBlock = buff + index - 16;

		camellia_encrypt(keyTable, currBlock, currBlock, 0);
		XorBlock(currBlock, prevBlock);
	}
	fileSeek(fp, SEEK_SET, fpPos - size);
	fileWrite(fp, gndBlockVar(buff, size, gab));

	memFree(buff);
}
static void F_DecryptMain(char *file, camellia_keyTable_t *keyTable)
{
	FILE *fp = fileOpen(file, "r+b");
	uchar buff[32];
	uchar *prevBlock;
	uchar *currBlock;
	uint64 size;
	uint64 index;
	autoBlock_t gab;
	int retval = 0;

	prevBlock = buff;
	currBlock = buff + 16;

	size = getFileSizeFP(fp);

	errorCase(size < 32); // 2bs
	errorCase(size % 16 != 0); // 2bs

	for (index = size; 16 <= index; )
	{
		const uint H_PART = 32000000;
		const uint M_PART = 320000;
		const uint S_PART = 3200;

		if (H_PART <= index)
		{
			F_DecryptPart(fp, index, H_PART, keyTable);
			index -= H_PART - 16;
		}
		else if (M_PART <= index)
		{
			F_DecryptPart(fp, index, M_PART, keyTable);
			index -= M_PART - 16;
		}
		else if (S_PART <= index)
		{
			F_DecryptPart(fp, index, S_PART, keyTable);
			index -= S_PART - 16;
		}
		else
		{
			if (32 <= index)
			{
				fileSeek(fp, SEEK_SET, index - 32);
				fileRead(fp, gndBlockVar(buff, 32, gab));
			}
			else
			{
				fileSeek(fp, SEEK_END, -16);
				fileRead(fp, gndBlockVar(prevBlock, 16, gab));
				fileSeek(fp, SEEK_SET, 0);
				fileRead(fp, gndBlockVar(currBlock, 16, gab));
			}
			camellia_encrypt(keyTable, currBlock, currBlock, 0);
			XorBlock(currBlock, prevBlock);
			fileSeek(fp, SEEK_SET, index - 16);
			fileWrite(fp, gndBlockVar(currBlock, 16, gab));

			index -= 16;
		}
	}
	fileClose(fp);
}
static int F_Decrypt(char *file, autoList_t *keyTableList)
{
	uint64 fileSize = getFileSize(file);
	camellia_keyTable_t *keyTable;
	uint index;

	if (
		fileSize < 16 + 64 + 64 + 64 ||
//		fileSize < 256 + 64 + 64 + 64 || // while (fileSize + padSize < 0xff); してなかった時の暗号文はこれより短い (最小padding 旧->新: 16->256)
		fileSize % 16 != 0
		)
		return 0;

	for (index = getCount(keyTableList); index; )
	{
		index--;
		keyTable = (camellia_keyTable_t *)getElement(keyTableList, index);

		F_DecryptMain(file, keyTable);
	}
	return 1;
}

// ---- EncryptFile ----

void rngcphrEncryptFile(char *file, autoList_t *keyTableList)
{
	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));
	errorCase(!keyTableList);
	errorCase(!getCount(keyTableList));

	F_AddPadding(file);
	F_AddRandPart(file);
	F_AddHash(file);
	F_AddRandPart(file);
	F_Encrypt(file, keyTableList);
}
int rngcphrDecryptFile(char *file, autoList_t *keyTableList)
{
	int retval = 0;

	errorCase(m_isEmpty(file));
	errorCase(!existFile(file));
	errorCase(!keyTableList);
	errorCase(!getCount(keyTableList));

	if (
		F_Decrypt(file, keyTableList) &&
		F_UnaddRandPart(file) &&
		F_UnaddHash(file) &&
		F_UnaddRandPart(file) &&
		F_UnaddPadding(file)
		)
		retval = 1;

	return retval;
}

// ----
