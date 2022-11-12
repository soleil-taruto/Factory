#include "C:\Factory\Common\all.h"
#include "C:\Factory\OpenSource\camellia.h"
#include "C:\Factory\OpenSource\sha512.h"

static void XorBlock(void *block, void *subBlock)
{
	((uint *)block)[0] ^= ((uint *)subBlock)[0];
	((uint *)block)[1] ^= ((uint *)subBlock)[1];
	((uint *)block)[2] ^= ((uint *)subBlock)[2];
	((uint *)block)[3] ^= ((uint *)subBlock)[3];
}

static autoList_t *DataFileList;

static void ReadFileList(char *fileListFile, char *rootDir)
{
	autoList_t *files = readLines(fileListFile);
	char *file;
	uint index;

	DataFileList = newList();

	addCwd(rootDir);

	foreach (files, file, index)
	{
		file = makeFullPath(file);
		cout("file: %s\n", file);
		errorCase(!existFile(file));

		addElement(DataFileList, (uint)file);
	}
	unaddCwd();

	releaseDim(files, 1);
}
static void MEDF_MakeEncDataFile(autoBlock_t *fileData, autoBlock_t *rawKey, uint resId, uint tweek)
{
	ab_addValue(fileData, tweek); // addTweek

	// padding
	{
		uint padSize = 16 - getSize(fileData) % 16;
		uint count;

		for (count = padSize; count; count--)
		{
			addByte(fileData, padSize);
		}
	}
	// addHash
	{
		autoBlock_t gab;
		sha512_makeHashBlock(fileData);
		addBytes(fileData, gndBlockVar(sha512_hash, 16, gab));
	}
	// encrypt-rcbc
	{
		camellia_keyTable_t *key = camellia_createKeyTable(rawKey);
		uchar *dataBuff = directGetBuffer(fileData);
		uint dataSize = getSize(fileData);
		uint index;

		errorCase(dataSize < 32); // 2bs
		errorCase(dataSize % 16); // 2bs

		XorBlock(dataBuff, dataBuff + dataSize - 16);
		camellia_encrypt(key, dataBuff, dataBuff, 1);

		for (index = 16; index < dataSize; index += 16)
		{
			XorBlock(dataBuff + index, dataBuff + index - 16);
			camellia_encrypt(key, dataBuff + index, dataBuff + index, 1);
		}
		camellia_releaseKeyTable(key);
	}
}

static autoList_t *EncDataFileList;

static void MakeEncDataFiles(char *strRawKey, uint firstTweek)
{
	autoBlock_t *rawKey = makeBlockHexLine(strRawKey);
	char *file;
	uint index;

	EncDataFileList = newList();

	foreach (DataFileList, file, index)
	{
		char *encFile = makeTempFile(NULL);
		autoBlock_t *fileData;

		cout("< %s\n", file);
		cout("> %s\n", encFile);

		fileData = readBinary(file);

		MEDF_MakeEncDataFile(fileData, rawKey, index, firstTweek + index);

		writeBinary_cx(encFile, fileData);
		addElement(EncDataFileList, (uint)encFile);
	}
	releaseAutoBlock(rawKey);
}
static void MakeClusterFile(char *clusterFile)
{
	FILE *fp = fileOpen(clusterFile, "wb");
	char *file;
	uint index;

	writeValue(fp, getCount(EncDataFileList));

	foreach (EncDataFileList, file, index)
	{
		cout("file/I: %s\n", file);
		writeValue(fp, (uint)getFileSize(file));
	}
	foreach (EncDataFileList, file, index)
	{
		cout("file/W: %s\n", file);
		writeBinaryBlock_x(fp, readBinary(file));
	}
	fileClose(fp);
}
static void ClearEncDataFiles(void)
{
	char *file;
	uint index;

	foreach (EncDataFileList, file, index)
	{
		cout("remove: %s\n", file);
		removeFile(file);
	}
}
int main(int argc, char **argv)
{
	char *fileListFile;
	char *clusterFile;
	char *rawKey;
	uint firstTweek;
	char *rootDir;

	fileListFile = nextArg();
	clusterFile = nextArg();
	rawKey = nextArg();
	firstTweek = toValue(nextArg());

	fileListFile = makeFullPath(fileListFile);
	clusterFile = makeFullPath(clusterFile);
	rootDir = getParent(fileListFile);

	ReadFileList(fileListFile, rootDir);
	MakeEncDataFiles(rawKey, firstTweek);
	MakeClusterFile(clusterFile);
	ClearEncDataFiles();

	memFree(fileListFile);
	memFree(clusterFile);
	memFree(rootDir);
}
