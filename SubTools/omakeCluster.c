#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"
#include "C:\Factory\Common\Options\Progress.h"
#include "C:\Factory\OpenSource\aes128.h"
#include "C:\Factory\OpenSource\sha512.h"

#define EXTLENMAX 10

static char *ExtFltr(char *ext)
{
	static char *fExt;

	memFree(fExt);
	fExt = lineToPrintLine(ext, 0);

	if (EXTLENMAX < strlen(fExt))
		fExt[EXTLENMAX] = '\0';

	return fExt;
}
static void MakeCluster(char *dir, char *outFile)
{
	FILE *fp = fileOpen(outFile, "wb");
	autoList_t *files = lsFiles(dir);
	char *file;
	uint index;
	uint pos;

	sortJLinesICase(files);

	writeValue(fp, getCount(files));
	pos = 1 + getCount(files) * 2;
	pos *= sizeof(uint);

	foreach (files, file, index)
	{
		uint size = getFileSize(file);

		writeValue(fp, pos);
		writeValue(fp, size);
		pos += size;
		pos += strlen(ExtFltr(getExt(file))) + 1;
	}
	foreach (files, file, index)
	{
		writeBinaryBlock_x(fp, readBinary(file));
		writeLine(fp, ExtFltr(getExt(file)));
	}
	fileClose(fp);
	releaseDim(files, 1);
}
static void Pad128(char *file)
{
	uint64 size = getFileSize(file);
	FILE *fp;

	fp = fileOpen(file, "r+b");
	fileSeek(fp, SEEK_END, 0);

	while (size & 15)
	{
		writeChar(fp, getCryptoByte());
		size++;
	}
	fileClose(fp);
}
static void AddVerify(char *file)
{
	autoBlock_t *text = makeCryptoRandBlock(64);
	autoBlock_t gab;
	FILE *fp;

	sha512_makeHashBlock(text);

	fp = fileOpen(file, "r+b");
	fileSeek(fp, SEEK_END, 0);

	writeBinaryBlock(fp, text);
	writeBinaryBlock(fp, gndBlockVar(sha512_hash, 64, gab));

	fileClose(fp);
	releaseAutoBlock(text);
}
static void AddSHA512(char *file)
{
	FILE *fp;
	uint index;

	sha512_makeHashFile(file);

	fp = fileOpen(file, "r+b");
	fileSeek(fp, SEEK_END, 0);

	for (index = 0; index < 64; index++)
		writeChar(fp, sha512_hash[index]);

	fileClose(fp);
}
static void DoBlockMask(autoBlock_t *b1, autoBlock_t *b2, autoBlock_t *out)
{
	uint index;

	for (index = 0; index < getSize(out); index++)
	{
		setByte(out, index, getByte(b1, index) ^ getByte(b2, index));
	}
}
static void CounterIncrement(autoBlock_t *counter)
{
	uint index;

	for (index = 0; index < getSize(counter); index++)
	{
		uchar *p = (uchar *)directGetBuffer(counter) + index;

		if (*p < 0xff)
		{
			(*p)++;
			break;
		}
		*p = 0x00;
	}
}
static void EncryptCluster(char *file, AES128_KeyTable_t *keys[4], autoBlock_t *counter, int hashOnlyMode)
{
	char *midFile = makeTempPath(NULL);
	FILE *rfp;
	FILE *wfp;
	uint64 size = getFileSize(file);
	uint64 count;
	uint64 startCount = 0;

	cout("EncryptCluster hom=%d\n", hashOnlyMode);

	if (hashOnlyMode)
	{
		counter = copyAutoBlock(counter);

		errorCase(size < 192);
		startCount = size - 192;
	}
	rfp = fileOpen(file, "rb");
	wfp = fileOpen(midFile, "wb");

	ProgressBegin();

	for (count = 0; count < size; count += 16)
	{
		autoBlock_t *block = readBinaryBlock(rfp, 16);

		errorCase(getSize(block) != 16);

		if (startCount <= count)
		{
			autoBlock_t *maskBlock = nobCreateBlock(16);

			AES128_EncryptBlock(keys[0], counter, maskBlock);
			AES128_EncryptBlock(keys[1], maskBlock, maskBlock);
			AES128_EncryptBlock(keys[2], maskBlock, maskBlock);
			AES128_EncryptBlock(keys[3], maskBlock, maskBlock);

			DoBlockMask(block, maskBlock, block);
			releaseAutoBlock(maskBlock);
		}
		writeBinaryBlock(wfp, block);
		releaseAutoBlock(block);

		CounterIncrement(counter);

		if (eqIntPulseSec(1, NULL))
			Progress();
	}
	endLoop:
	ProgressEnd(0);

	fileClose(rfp);
	fileClose(wfp);

	removeFile(file);
	moveFile(midFile, file);
	memFree(midFile);

	if (hashOnlyMode)
		releaseAutoBlock(counter);
}
static void CheckSHA512(char *file)
{
	autoBlock_t *rdHash = nobCreateBlock(64);
	uint64 size = getFileSize(file);
	FILE *fp;

	errorCase(size < 64);

	fp = fileOpen(file, "rb");
	fileSeek(fp, SEEK_END, -64);
	fileRead(fp, rdHash);
	fileClose(fp);

	setFileSize(file, size - 64);
	sha512_makeHashFile(file);

	errorCase(memcmp(sha512_hash, directGetBuffer(rdHash), 64));

	releaseAutoBlock(rdHash);
}
static int CheckVerify(char *file) // ret: OK
{
	autoBlock_t *text = nobCreateBlock(64);
	autoBlock_t *rdHash = nobCreateBlock(64);
	FILE *fp;
	int retval;

	errorCase(getFileSize(file) < 192);

	fp = fileOpen(file, "rb");
	fileSeek(fp, SEEK_END, -192);
	fileRead(fp, text);
	fileRead(fp, rdHash);
	fileClose(fp);

	sha512_makeHashBlock(text);

	retval = !memcmp(sha512_hash, directGetBuffer(rdHash), 64);

	releaseAutoBlock(text);
	releaseAutoBlock(rdHash);

	return retval;
}
static void UnmakeCluster(char *file, char *outDir)
{
	FILE *fp = fileOpen(file, "rb");
	uint filenum;
	uint index;
	autoList_t *startPosList = newList();
	autoList_t *sizeList = newList();

	filenum = readValue(fp);

	for (index = 0; index < filenum; index++)
	{
		addElement(startPosList, readValue(fp));
		addElement(sizeList, readValue(fp));
	}
	for (index = 0; index < filenum; index++)
	{
		autoBlock_t *fileImage = nobCreateBlock(getElement(sizeList, index));
		char *ext;
		char *outFile;

		fileSeek(fp, SEEK_SET, getElement(startPosList, index));
		fileRead(fp, fileImage);

		ext = readLineLenMax(fp, EXTLENMAX);
		outFile = combine_cx(outDir, xcout("%010u.%s", index, ExtFltr(ext)));

		writeBinary(outFile, fileImage);

		releaseAutoBlock(fileImage);
		memFree(ext);
		memFree(outFile);
	}
	fileClose(fp);

	releaseAutoList(startPosList);
	releaseAutoList(sizeList);
}

int main(int argc, char **argv)
{
	char *resourceDir;
	char *clusterFile;
	char *passphrase;
	uint initVector[4];
	int restoreMode;
	int batchMode;
	autoBlock_t *rawKey;
	AES128_KeyTable_t *keys[4];
	autoBlock_t *counter;

	resourceDir = nextArg(); // "/R" のとき無視する。
	clusterFile = nextArg();
	passphrase = nextArg();
	initVector[0] = toValue(nextArg());
	initVector[1] = toValue(nextArg());
	initVector[2] = toValue(nextArg());
	initVector[3] = toValue(nextArg());
	restoreMode = argIs("/R");
	batchMode = argIs("/B");

	errorCase(isEmptyJTkn(resourceDir));
	errorCase(isEmptyJTkn(clusterFile));
	errorCase(isEmptyJTkn(passphrase));

	cout("リソースフォルダ: %s\n", resourceDir);
	cout("クラスタファイル: %s\n", clusterFile);
	cout("パスフレーズ: %s\n", passphrase);
	cout("初期化ベクトル1: %u\n", initVector[0]);
	cout("初期化ベクトル2: %u\n", initVector[1]);
	cout("初期化ベクトル3: %u\n", initVector[2]);
	cout("初期化ベクトル4: %u\n", initVector[3]);
	cout("モード: %s%s\n", restoreMode ? "復元" : "作成", batchMode ? "(バッチモード)" : "");

	if (batchMode)
	{
		if (passphrase[0] == '*') // '*' 以降パスフレーズと見なす。
			passphrase++;
		else // ファイル名と見なす。
			passphrase = readFirstLine(passphrase);
	}

	sha512_makeHashLine(passphrase);
	rawKey = recreateBlock(sha512_hash, 64);

	cout("暗号鍵の素: %s\n", c_makeHexLine(rawKey));

	{
		autoBlock_t gab;

		keys[0] = AES128_CreateKeyTable(gndSubBytesVar(rawKey, 16 * 0, 16, gab), 1);
		keys[1] = AES128_CreateKeyTable(gndSubBytesVar(rawKey, 16 * 1, 16, gab), 1);
		keys[2] = AES128_CreateKeyTable(gndSubBytesVar(rawKey, 16 * 2, 16, gab), 1);
		keys[3] = AES128_CreateKeyTable(gndSubBytesVar(rawKey, 16 * 3, 16, gab), 1);
	}

	counter = newBlock();

	ab_addValue(counter, initVector[0]);
	ab_addValue(counter, initVector[1]);
	ab_addValue(counter, initVector[2]);
	ab_addValue(counter, initVector[3]);

	cout("カウンタ初期値: %s\n", c_makeHexLine(counter));

	if (!restoreMode)
	{
		MakeCluster(resourceDir, clusterFile);
		Pad128(clusterFile);
		AddVerify(clusterFile);
		AddSHA512(clusterFile);
		EncryptCluster(clusterFile, keys, counter, 0);
	}
	else if (!batchMode)
	{
		char *midFile = makeTempPath(NULL);
		char *destDir = makeFreeDir();

		copyFile(clusterFile, midFile);

		EncryptCluster(midFile, keys, counter, 0); // as Decrypt
		CheckSHA512(midFile);
		errorCase(!CheckVerify(midFile));
		UnmakeCluster(midFile, destDir);

		execute_x(xcout("START %s", destDir));

		removeFile(midFile);
		memFree(midFile);
		memFree(destDir);
	}
	else
	{
		char *midFile = nextArg(); // 削除しない。
		char *destDir = nextArg(); // 失敗時、何もしない。

		copyFile(clusterFile, midFile);

		EncryptCluster(midFile, keys, counter, 1); // as Decrypt

		if (CheckVerify(midFile))
		{
			EncryptCluster(midFile, keys, counter, 1); // as Encrypt
			EncryptCluster(midFile, keys, counter, 0); // as Decrypt

			if (CheckVerify(midFile))
			{
				UnmakeCluster(midFile, destDir);
			}
		}
	}

	releaseAutoBlock(rawKey);
	AES128_ReleaseKeyTable(keys[0]);
	AES128_ReleaseKeyTable(keys[1]);
	AES128_ReleaseKeyTable(keys[2]);
	AES128_ReleaseKeyTable(keys[3]);
	releaseAutoBlock(counter);
}
