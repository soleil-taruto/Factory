#include "C:\Factory\Common\all.h"
#include "C:\Factory\Common\Options\CryptoRand.h"

#define CPART_NUM 1000
#define CPART_LEN 1000
#define CHK_SZ 5

static autoBlock_t *CBytes;

static void LoadCBytes(void)
{
	char *outFile = makeTempPath(NULL);
	uint index;

	CBytes = newBlock();

	for (index = 0; index < CPART_NUM; index++)
	{
		if (eqIntPulseSec(2, NULL))
			cout("LOAD %u\n", index);

		execute_x(xcout("start /b /wait \"\" \"%s\" /GET-CBYTES %u \"%s\"", getSelfFile(), CPART_LEN, outFile));

		ab_addBytes_x(CBytes, readBinary(outFile));
		removeFile(outFile);
	}
	errorCase(getSize(CBytes) != CPART_NUM * CPART_LEN); // 2bs

	cout("LOAD_OK\n");
	memFree(outFile);
}
static void CheckCBytes(void)
{
	uint lPos;
	uint rPos;

	for (lPos = 0;        lPos + CHK_SZ + 1 <= getSize(CBytes); lPos++)
	for (rPos = lPos + 1; rPos + CHK_SZ     <= getSize(CBytes); rPos++)
	{
		if (eqIntPulseSec(2, NULL))
			cout("CHECK %u %u\n", lPos, rPos);

		if (!memcmp(b_(CBytes) + lPos, b_(CBytes) + rPos, CHK_SZ))
		{
			cout("FOUND %u %u\n", lPos, rPos);
			error();
		}
	}
	cout("CHECK_OK\n");
}
int main(int argc, char **argv)
{
	if (argIs("/GET-CBYTES"))
	{
		uint size;
		char *outFile;
		FILE *outFp;
		uint index;

		size = toValue(nextArg());
		outFile = nextArg();
		outFp = fileOpen(outFile, "wb");

		for (index = 0; index < size; index++)
			writeChar(outFp, getCryptoByte());

		fileClose(outFp);
		return;
	}

	LoadCBytes();
	CheckCBytes();
}
