#include "resCluster.h"

#define FIRST_PADDING 0xff
#define SECOND_PADDING 0x00

static camellia_keyTable_t *KeyTable;

static void InitEncryptor(autoBlock_t *rawKey)
{
	KeyTable = camellia_createKeyTable(rawKey);
}
static void FnlzEncryptor(void)
{
	camellia_releaseKeyTable(KeyTable);
}
static camellia_keyTable_t *MakeTweekKeyTable(uint tweek)
{
	autoBlock_t *rawKey = createBlock(16);
	camellia_keyTable_t *keyTable;

	ab_addValue(rawKey, tweek);
	setSize(rawKey, 16);

	keyTable = camellia_createKeyTable(rawKey);

	releaseAutoBlock(rawKey);
	return keyTable;
}
static void AddPadding(autoBlock_t *image)
{
	addByte(image, FIRST_PADDING);

	while (getSize(image) & 0x0f)
	{
		addByte(image, SECOND_PADDING);
	}
}
static void UnaddPadding(autoBlock_t *image)
{
	while (unaddByte(image) != FIRST_PADDING)
	{
		noop();
	}
}
static void EncryptResImage(autoBlock_t *image, uint tweek)
{
	camellia_keyTable_t *tweekKeyTable = MakeTweekKeyTable(tweek);
	autoBlock_t *hash = md5_makeHashBlock(image);

	AddPadding(image);
	ab_addBytes(image, hash);

	camellia_cbcRing(KeyTable, image, image, 1);
	camellia_cbcRing(tweekKeyTable, image, image, 0);
	camellia_cbcRing(tweekKeyTable, image, image, 0);
	camellia_ecb(tweekKeyTable, image, image, 0);
	camellia_cbcRing(KeyTable, image, image, 1);

	camellia_releaseKeyTable(tweekKeyTable);
	releaseAutoBlock(hash);
}
static void DecryptResImage(autoBlock_t *image, uint tweek)
{
	camellia_keyTable_t *tweekKeyTable = MakeTweekKeyTable(tweek);
	autoBlock_t *hash;
	autoBlock_t *imageHash;

	camellia_cbcRing(KeyTable, image, image, 0);
	camellia_ecb(tweekKeyTable, image, image, 1);
	camellia_cbcRing(tweekKeyTable, image, image, 1);
	camellia_cbcRing(tweekKeyTable, image, image, 1);
	camellia_cbcRing(KeyTable, image, image, 0);

	hash = unaddBytes(image, 16);
	UnaddPadding(image);
	imageHash = md5_makeHashBlock(image);

	errorCase(!isSameBlock(hash, imageHash));

	camellia_releaseKeyTable(tweekKeyTable);
	releaseAutoBlock(hash);
	releaseAutoBlock(imageHash);
}

/*
	files -- リソースファイルのリスト
	rootDir -- リソースファイルが相対であったときのルートディレクトリ
	rawKey -- 暗号鍵、16, 24, 32 バイトであること。
	clusterFile -- クラスタファイル (出力先)

	リソースの個数は 2^32 未満であること。
	リソースの合計サイズは 2^32 バイトより十分に小さいこと。(管理領域とリソース合わせて 2^32 バイト未満であること)
*/
void createResourceCluster(autoList_t *files, char *rootDir, autoBlock_t *rawKey, char *clusterFile)
{
	autoList_t *entFiles = newList();
	autoList_t *entIndexes = newList();
	autoList_t *entAddresses = newList();
	FILE *fp = fileOpen(clusterFile, "wb");
	char *file;
	uint index;
	char *eFile;
	uint eIndex;
	uint64 sumSize;
	uint address;

	files = copyAutoList(files);

	addCwd(rootDir);
	foreach (files, file, index)
	{
		setElement(files, index, (uint)makeFullPath(file));
	}
	unaddCwd();

	foreach (files, file, index)
	{
		foreach (entFiles, eFile, eIndex)
		{
			if (isSameFile(eFile, file))
			{
				break;
			}
		}
		if (!eFile)
		{
			addElement(entFiles, (uint)file);
		}
		addElement(entIndexes, eIndex);
	}
	writeValue(fp, getCount(entIndexes));

	foreach (entIndexes, eIndex, index)
	{
		writeValue(fp, eIndex);
	}
	foreach (entFiles, file, index)
	{
		writeValue(fp, 0); // dummy
	}
	writeValue(fp, 0); // dummy

	InitEncryptor(rawKey);
	sumSize = (1 + getCount(entIndexes) + getCount(entFiles) + 1) * sizeof(uint);

	foreach (entFiles, file, index)
	{
		autoBlock_t *resImage = readBinary(file);

		EncryptResImage(resImage, sumSize);

		addElement(entAddresses, sumSize);
		sumSize += getSize(resImage);
		writeBinaryBlock(fp, resImage);

		releaseAutoBlock(resImage);
	}
	addElement(entAddresses, sumSize);
	FnlzEncryptor();

	fileSeek(fp, SEEK_SET, (1 + getCount(entIndexes)) * sizeof(uint));

	foreach (entAddresses, address, index)
	{
		writeValue(fp, address);
	}
	fileClose(fp);

	releaseDim(files, 1);
	releaseAutoList(entFiles);
	releaseAutoList(entIndexes);
	releaseAutoList(entAddresses);
}

static char *GetExt(autoBlock_t *image) // 超テキトー
{
	uchar sptn[4];
	uchar mptn[4];

	sptn[0] = refByte(image, 0);
	sptn[1] = refByte(image, 1);
	sptn[2] = refByte(image, 2);
	sptn[3] = refByte(image, 3);

	mptn[0] = refByte(image, 8);
	mptn[1] = refByte(image, 9);
	mptn[2] = refByte(image, 10);
	mptn[3] = refByte(image, 11);

	if (!memcmp(sptn, "RIFF", 4) && !memcmp(mptn, "WAVE", 4))
		return "wav";

	if (!memcmp(sptn, "ID3", 3))
		return "mp3";

	if (!memcmp(sptn + 1, "PNG", 3))
		return "png";

	if (!memcmp(sptn, "BM", 2))
		return "bmp";

	return "bin";
}
void restoreResourceCluster(char *clusterFile, autoBlock_t *rawKey, char *destDir) // destDir: 存在すること。
{
	FILE *fp = fileOpen(clusterFile, "rb");
	uint resCount;
	uint resIndex;
	uint entIndex;
	uint entAddress;
	uint entEndAddress;
	autoBlock_t *image;
	char *file;

	InitEncryptor(rawKey);
	resCount = readValue(fp);

	for (resIndex = 0; resIndex < resCount; resIndex++)
	{
		fileSeek(fp, SEEK_SET, (1 + resIndex) * sizeof(uint));
		entIndex = readValue(fp);

		fileSeek(fp, SEEK_SET, (1 + resCount + entIndex) * sizeof(uint));
		entAddress = readValue(fp);
		entEndAddress = readValue(fp);

		fileSeek(fp, SEEK_SET, entAddress);
		image = readBinaryBlock(fp, entEndAddress - entAddress);

		DecryptResImage(image, entAddress);

		file = combine_cx(destDir, xcout("%05u_%05u_%08x_%08x.%s", resIndex, entIndex, entAddress, entEndAddress, GetExt(image)));
		writeBinary(file, image);

		releaseAutoBlock(image);
		memFree(file);
	}
	FnlzEncryptor();
	fileClose(fp);
}
