#include "C:\Factory\Common\all.h"
#include "C:\Factory\Meteor\ZCluster.h"

#define SIGNATURE "DonutRes"
#define SIGNATURE_LEN (sizeof(SIGNATURE) - 1)
#define FIRST_RES_SIZE_POS (SIGNATURE_LEN + sizeof(uint))

static char *FileListFile;
static char *ClusterFile;
static autoList_t *FileList;

static void ReadFileList(void)
{
	FileList = readResourcePaths(FileListFile);
}

// ---- MakeClusterFile ----

static FILE *WFp;

static void WriteDummyResSizes(void)
{
	uint index;

	for (index = 0; index < getCount(FileList); index++)
	{
		writeValue(WFp, 0);
	}
}

static void WriteResFile(char *file, uint fileIndex)
{
	char *pkFile;
	autoBlock_t *pkFileData;

	errorCase(!existFile(file));

	pkFile = makeTempPath("gz");
	ZC_Pack(file, pkFile);
	pkFileData = readBinary(pkFile);
	writeBinaryBlock(WFp, pkFileData);

	fileSeek(WFp, SEEK_SET, (sint64)(FIRST_RES_SIZE_POS + fileIndex * sizeof(uint)));
	writeValue(WFp, getSize(pkFileData));
	fileSeek(WFp, SEEK_END, 0i64);

	removeFile(pkFile);

	memFree(pkFile);
	releaseAutoBlock(pkFileData);
}
static void WriteResFiles(void)
{
	char *file;
	uint index;

	foreach (FileList, file, index)
	{
		WriteResFile(file, index);
	}
}

static void WriteHash(void)
{
	autoBlock_t gab;

	sha512_makeHashFile(ClusterFile);

	writeJoinBinary(ClusterFile, gndBlockVar(sha512_hash, 64, gab));
}

static void MakeClusterFile(void)
{
	WFp = fileOpen(ClusterFile, "wb");

	LOGPOS();
	writeToken(WFp, SIGNATURE);
	LOGPOS();
	writeValue(WFp, getCount(FileList));
	LOGPOS();
	WriteDummyResSizes();
	LOGPOS();
	WriteResFiles();
	LOGPOS();

	fileClose(WFp);
	WFp = NULL;

	LOGPOS();
	WriteHash();
	LOGPOS();
}

// ----

int main(int argc, char **argv)
{
	FileListFile = makeFullPath(nextArg());
	ClusterFile  = makeFullPath(nextArg());

	LOGPOS();
	ReadFileList();
	LOGPOS();
	MakeClusterFile();
	LOGPOS();
}
