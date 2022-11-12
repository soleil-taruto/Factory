#include "C:\Factory\Common\all.h"

typedef enum FType_et
{
	FT_UNKNOWN,
	FT_EXE,
	FT_ZIP,
	FT_GZ,
}
FType_t;

static char *FTypeNames[] =
{
	"unknown",
	"EXE",
	"ZIP",
	"GZ"
};

// ----

static FType_t GetFTypeByExt(char *file)
{
	char *ext = getExt(file);

	if (!_stricmp(ext, "EXE")) return FT_EXE;
	if (!_stricmp(ext, "ZIP")) return FT_ZIP;
	if (!_stricmp(ext, "GZ"))  return FT_GZ;

	return FT_UNKNOWN;
}
static FType_t GetFTypeBySignature(char *file)
{
	FType_t result = FT_UNKNOWN;
	FILE *fp;
	autoBlock_t *fDatTop;

	fp = fileOpen(file, "rb");
	fDatTop = readBinaryBlock(fp, 1024);
	fileClose(fp);

	if (
		refByte(fDatTop, 0) == 'M' &&
		refByte(fDatTop, 1) == 'Z'
		)
	{
		result = FT_EXE;
		goto solved;
	}

	if (
		refByte(fDatTop, 0) == 'P' &&
		refByte(fDatTop, 1) == 'K' &&
		refByte(fDatTop, 2) == '\x03' &&
		refByte(fDatTop, 3) == '\x04'
		)
	{
		result = FT_ZIP;
		goto solved;
	}

	if (
		refByte(fDatTop, 0) == 0x1f &&
		refByte(fDatTop, 1) == 0x8b
		)
	{
		result = FT_GZ;
		goto solved;
	}

solved:
	releaseAutoBlock(fDatTop);
	return result;
}

// ----

static void DispFType(FType_t ft)
{
	cout("%s\n", FTypeNames[ft]);
}
static int CheckFileFormat(char *file) // ret: ? マッチした。
{
	FType_t fte = GetFTypeByExt(file);
	FType_t fts = GetFTypeBySignature(file);

	DispFType(fte);
	DispFType(fts);

	if (fte != fts)
	{
		cout("############################################\n");
		cout("## 拡張子とシグネチャがマッチしていません ##\n");
		cout("############################################\n");
		return 0;
	}
	return 1;
}
static void CheckDirFormat(char *dir)
{
	autoList_t *files = lssFiles(dir);
	char *file;
	uint index;

	foreach (files, file, index)
	{
		if (index)
			cout("\n");

		cout("%s\n", file);

		if (!CheckFileFormat(file))
		{
			cout("stdn\n");
			clearGetKey();
		}
	}
	releaseDim(files, 1);
}
static void CheckFormat(char *path)
{
	if (existFile(path))
	{
		CheckFileFormat(path);
	}
	else
	{
		CheckDirFormat(path);
	}
}
int main(int argc, char **argv)
{
	if (hasArgs(1))
	{
		CheckFormat(nextArg());
	}
	else
	{
		for (; ; )
		{
			CheckFormat(c_dropDirFile());
			cout("\n");
		}
	}
}
